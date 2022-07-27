#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* */
#include "compression.h"
#include "utils.h"

/* Your code here... */

/* return 0 means compression failure */
CompressedInstruction Compress(InstructionBits inst, int index,InstructionType* itype,CompressedInstType* citype) /* */
{
    unsigned int opcode, tmp=0;
    int i,ii, imm = 0; /* */
    CompressedInstruction res = 0;
    /* Initialization */
    RtypeInst rinst;    StypeInst sinst;    BtypeInst binst;    ItypeInst iinst;    UtypeInst uinst;    JtypeInst jinst;
    CRtypeInst crinst;    CItypeInst ciinst;    CLtypeInst clinst;    CB2typeInst cbinst;
    /* Initialization */
    binst.zz.f3=0;    binst.zz.imm1=0;    binst.zz.imm2=0;    binst.zz.opcode=0;    binst.zz.rs1=0;    binst.zz.rs2=0;
    jinst.zz.imm1=0;    jinst.zz.imm2=0;    jinst.zz.opcode=0;    jinst.zz.rd=0;
    rinst.iinst=0;    sinst.iinst=0;    binst.iinst=0;    iinst.iinst=0;    uinst.iinst=0;    jinst.iinst=0;    crinst.cinst=0;    ciinst.cinst=0;    clinst.cinst=0;    cbinst.cinst=0;
    opcode = inst & 0x7f; /* 7 bit */
    switch (opcode)
    {
    case 0x33: /* Wht */
               /* Rtype */
        rinst.iinst = inst;
        /* add */
        if (rinst.zz.f3 == 0x00 && rinst.zz.f7 == 0x00 && rinst.zz.rd != 0x00 && rinst.zz.rs2 != 0x00) {
            res |= 0x02; /* opcode 10 */
            res |= rinst.zz.rs2 << 2;
            res |= rinst.zz.rd << 7;
            /* 1 CR/C.ADD add rd rd rs2 */
            if (rinst.zz.rs1 == rinst.zz.rd) {
                res |= 0x09 << 12; /* f4 1001 */
                return res;
            }
            /* 2 CR/C.MV add rd x0 rs2 */
            if (rinst.zz.rs1 == 0x00) {
                res |= 0x08 << 12; /* f4 1000 */
                return res;
            }
            return 0; /* is add function, but cannot be compressed */
        }

        /* only registers from x8 to x15 are accepted for the other 4, rs1 also == rd */
        if (rinst.zz.rd == rinst.zz.rs1 && 
            rinst.zz.rd >= 0x08 && rinst.zz.rd <= 0x0f &&  /* rd:  x8~x15 */
            rinst.zz.rs2 >= 0x08 && rinst.zz.rs2 <= 0x0f) {/* rs2: x8~x15 */

            res |= 0x01; /* opcode 01 */
            res |= (rinst.zz.rs2 - 0x08) << 2;
            res |= (rinst.zz.rd  - 0x08) << 7;
            res |= 0x23 << 10; /* f6 100011 */

            /* 11 CS/C.AND and rd' rd' rs2' */
            if (rinst.zz.f3 == 0x07 && rinst.zz.f7 == 0x00) {
                res |= 0x03 << 5; /* f2 11 */
                return res;
            }

            /* 12 CS/C.OR  or  rd' rd' rs2' */
            if (rinst.zz.f3 == 0x06 && rinst.zz.f7 == 0x00) {
                res |= 0x02 << 5; /* f2 10 */
                return res;
            }

            /* 13 CS/C.XOR xor rd' rd' rs2' */
            if (rinst.zz.f3 == 0x04 && rinst.zz.f7 == 0x00) {
                res |= 0x01 << 5; /* f2 01 */
                return res;
            }

            /* 14 CS/C.SUB sub rd' rd' rs2' */
            if (rinst.zz.f3 == 0x00 && rinst.zz.f7 == 0x20) {
                /* f2 00, do nothing */
                return res;
            }
        }
        return 0;
        break; /* end of rtype */
    case 0x13: /* Dyy */
               /* Arithmetic Itype */
        iinst.iinst = inst;
        /* 5 CI/C.LI addi rd x0 imm */
        imm=iinst.zz.imm;
        if (iinst.zz.imm & 0x800) { /* 11th sign bit is 1, negative */
                tmp = 0x800;
                for (ii = 11; ii <= 31; ii++)  { /* */
                    imm |= tmp;
                    if(ii<31)tmp <<= 1; /* */
                }
            }
        if (iinst.zz.f3 == 0x00 &&                                  /* addi == 0b000 */
            iinst.zz.rd != 0x00 &&                                  /* rd != x0 */
            iinst.zz.rs1 == 0x00 &&                                 /* rs1 == x0 */
            (imm >=-32 && imm <= 31)) /* 0~5 */
        {                                                           /* \ */
            ciinst.zz.opcode = 0x01;                                /* CI = 0b01 */
            ciinst.zz.imm1 = iinst.zz.imm & 0x1f;                   /* 4~0 */
            ciinst.zz.rd = iinst.zz.rd;                             /* rd */
            ciinst.zz.imm2 = (iinst.zz.imm & 0x20) >> 5;            /* 5 */
            ciinst.zz.funct3 = 0x2;                                /* C.LI = 0b010 */
            return ciinst.cinst;
        }

        /* 7 CI/C.ADDI addi rd rd nzimm */
        if (iinst.zz.f3 == 0x00 &&                                  /* addi == 0b000 */
            iinst.zz.rd != 0x00 &&                                  /* rd != x0 */
            iinst.zz.rs1 == iinst.zz.rd &&                          /* rs1 == rd */
            iinst.zz.imm != 0 &&                                    /* imm != 0 */
            (imm>=-32 && imm <= 31)) /* 0~5 */
        {                                                           /* \ */
            ciinst.zz.opcode = 0x01;                                /* CI */
            ciinst.zz.imm1 = iinst.zz.imm & 0x1f;                   /* 4~0 */
            ciinst.zz.rd = iinst.zz.rd;                             /* rd */
            ciinst.zz.imm2 = (iinst.zz.imm & 0x20) >> 5;            /* 5 */
            ciinst.zz.funct3 = 0x0;                                /* C.ADDI = 0b000 */
            return ciinst.cinst;
        }

        /* 8 CI/C.SLLI slli rd rd shamt */
        if (iinst.zz.f3 == 0x01 &&
            (iinst.zz.imm >> 5 & 0x7f) == 0x00 &&        /* slli == 001 0000000*/
            iinst.zz.rd != 0x00 &&                       /* rd != x0 */
            iinst.zz.rs1 == iinst.zz.rd)                 /* rs1 == rd */
        {                                                /* \ */
            ciinst.zz.opcode = 0x02;                     /* CI = 0b10 */
            ciinst.zz.imm1 = iinst.zz.imm & 0x1f;        /* 4~0 */
            ciinst.zz.rd = iinst.zz.rd;                  /* rd */
            ciinst.zz.imm2 = (iinst.zz.imm & 0x20) >> 5; /* 5 */
            ciinst.zz.funct3 = 0x00;                     /* C.SLLI = 0b000 */
            return ciinst.cinst;
        }

        /* 17 CB/C.SRLI srli rd' rd' shamt */
        if (iinst.zz.f3 == 0x05 &&
            (iinst.zz.imm >> 5 & 0x7f) == 0x00 &&         /* srli == 101 0000000*/
            iinst.zz.rd >= 0x08 && iinst.zz.rd <= 0x0f && /* rd: x8~x15 */
            iinst.zz.rs1 == iinst.zz.rd)                  /* rs1 == rd */
        {                                                 /* \ */
            cbinst.zz.opcode = 0x01;                      /* CB = 0b01 */
            cbinst.zz.imm1 = iinst.zz.imm & 0x1f;         /* 4~0 */
            cbinst.zz.rd = iinst.zz.rd - 0x08;            /* rd-8 */
            cbinst.zz.funct2 = 0x00;                      /* 0b00 */
            cbinst.zz.imm2 = 0x00;                        /* 5 */
            cbinst.zz.funct3 = 0x4;                      /* C.SRLI = 0b100 */
            return cbinst.cinst;
        }

        /* 18 CB/C.SRAI srai rd' rd' shamt */
        if (iinst.zz.f3 == 0x05 &&
            (iinst.zz.imm >> 5 & 0x7f) == 0x20 &&         /* srai == 101 0100000*/
            iinst.zz.rd >= 0x08 && iinst.zz.rd <= 0x0f && /* rd: x8~x15 */
            iinst.zz.rs1 == iinst.zz.rd)                  /* rs1 == rd */
        {                                                 /* \ */
            cbinst.zz.opcode = 0x01;                      /* CB = 0b01 */
            cbinst.zz.imm1 = iinst.zz.imm & 0x1f;         /* 4~0 */
            cbinst.zz.rd = iinst.zz.rd - 0x08;            /* rd-8 */
            cbinst.zz.funct2 = 0x01;                      /* 0b01 */
            cbinst.zz.imm2 = 0x00;                        /* 5 */
            cbinst.zz.funct3 = 0x04;                      /* C.SRAI = 0b100 */
            return cbinst.cinst;
        }
        /* 19 CB/C.ANDI andi rd' rd' imm */
        if (iinst.zz.f3 == 0x07 &&                                  /* andi == 111 */
            iinst.zz.rd >= 0x08 && iinst.zz.rd <= 0x0f &&           /* rd: x8~x15 */
            iinst.zz.rs1 == iinst.zz.rd &&                          /* rs1 == rd */
            (imm>=-32 && imm <= 31)) /* 0~5 */
        {                                                           /* \ */
            cbinst.zz.opcode = 0x01;                                /* CB = 0b01 */
            cbinst.zz.imm1 = iinst.zz.imm & 0x1f;                   /* 4~0 */
            cbinst.zz.rd = iinst.zz.rd - 0x08;                      /* rd-8 */
            cbinst.zz.funct2 = 0x02;                                /* 0b10 */
            cbinst.zz.imm2 = (iinst.zz.imm & 0x20) >> 5;            /* 5 */
            cbinst.zz.funct3 = 0x4;                                /* C.ANDI = 0b100 */
            return cbinst.cinst;
        }
        return 0; /* */
        break;
    case 0x03: /* Dyy */
               /* Memory Itype */
        iinst.iinst = inst;
        imm=iinst.zz.imm;
        if (iinst.zz.imm & (1<<11))return 0;
        /* 9 CL/C.LW lw rd' off(rs1') */
        if (iinst.zz.f3 == 0x02 &&                            /* lw == 010 */
            iinst.zz.rd >= 0x08 && iinst.zz.rd <= 0x0f &&     /* rd: x8~x15 */
            iinst.zz.rs1 >= 0x08 && iinst.zz.rs1 <= 0x0f &&   /* rs1: x8~x15 */
            (iinst.zz.imm & 0x03) == 0 &&                     /* 4|off */
            (iinst.zz.imm & 0xf80) == 0)                      /* 0~6 */
        {                                                     /* \ */
            clinst.zz.opcode = 0x00;                          /* CL = 0b00 */
            clinst.zz.rd = iinst.zz.rd - 0x08;                /* rd-8 */
            clinst.zz.imm1 = (iinst.zz.imm & 0x04) >> 1; /* 0b100 2 */
            clinst.zz.imm1 |= (iinst.zz.imm & 0x40) >> 6;     /* 0b1000000 6 */
            clinst.zz.rs1 = iinst.zz.rs1 - 0x08;              /* rs1-8 */
            clinst.zz.imm2 = (iinst.zz.imm & 0x38) >> 3;      /* 0b111000 5~3 */
            clinst.zz.funct3 = 0x2;                          /* C.LW = 0b10 */
            return clinst.cinst;
        }
        return 0;/* */
        break;
    case 0x37: /* Dyy */
               /* only lui */
        uinst.iinst = inst;
        imm=uinst.zz.imm;
        if (uinst.zz.imm & (1<<19)) { /* 31th sign bit is 1, negative */
                tmp = 1<<19;
                for (ii = 19; ii <= 31; ii++)  { /* */
                    imm |= tmp;
                    if(ii<31)tmp <<= 1; /* */
                }
            }
        /* 6 CI/C.LUI lui rd nzimm */
        if (uinst.zz.rd != 0x00 &&                                  /* rd != x0 */
            uinst.zz.rd != 0x02 &&                                  /* rd != x2 */
            uinst.zz.imm != 0 &&                                    /* imm != 0 */
            (imm>=-32 && imm<=31)) /* 0~5 */
        {                                                           /* \ */
            ciinst.zz.opcode = 0x01;                                /* CI =0b01 */
            ciinst.zz.imm1 = uinst.zz.imm & 0x1f;                   /* 16:12 */
            ciinst.zz.rd = uinst.zz.rd;                             /* rd */
            ciinst.zz.imm2 = (uinst.zz.imm & 0x20)>>5;                   /* 17 */
            ciinst.zz.funct3 = 0x3;                                /* C.LUI = 0b011 */
            return ciinst.cinst;                                    /* */
        }
        return 0;
        break;
    case 0x23: /* Wht */
               /* Stype */
        sinst.iinst = inst;
        /* 10 CS/C.SW sw rs2 off(rs1') */
        if (sinst.zz.f3 == 0x02)
        {
            /* check registers x8 to x15 */
            if (!(sinst.zz.rs1 >= 0x08 && sinst.zz.rs1 <= 0x0f && sinst.zz.rs2 >= 0x08 && sinst.zz.rs2 <= 0x0f))
                return 0;
            /* check offset, negative or last two bit exists */
            if ((sinst.zz.imm1 & 0x03) != 0x00 || (sinst.zz.imm2 & 0x7c) != 0x00)
                return 0;

            /* opcode 00, do nothing */
            res |= (sinst.zz.rs2  - 0x08) << 2;
            res |= (sinst.zz.imm1 & 0x04) << 4; /* 2 */
            res |= (sinst.zz.imm2 & 0x02) << 4; /* 6 */
            res |= (sinst.zz.rs1  - 0x08) << 7;
            res |= (sinst.zz.imm1 & 0x18) << 7; /* 4:3 */
            res |= (sinst.zz.imm2 & 0x01) << 12;
            res |= 0x6 << 13; /* f3 110 */
            return res;
        } 
        return 0; /* uncompressible */
        break;
    case 0x63: /* Cdh */
               /* Btype */
        binst.iinst = inst;
        /* concatenate the imm */
        imm |= binst.zz.imm1 & 0x1e;         /* 4:1 */
        imm |= (binst.zz.imm2 & 0x3f) << 5;  /* 5:10 */
        imm |= (binst.zz.imm1 & 0x01) << 11; /* 11 */
        if (binst.zz.imm2 & 0x40)
        { /* 12th sign bit is 1, negative */
            tmp = 0x1000;
            for (i = 12; i <= 31; i++)
            { /* */
                imm |= tmp;
                if(i<31)tmp <<= 1; /* set */
            }
        }
        /* 16 SB/C.BNEZ bne rs1' x0 off*/
        if (binst.zz.f3 == 0x01)
        {
            if (binst.zz.rs1 >= 0x08 && binst.zz.rs1 <= 0x0f && /* x8~x15 */
                binst.zz.rs2 == 0x00)
            {                                      /* rs2 == x0 */
                res = 0x01;                        /* opcode = 01 */
                res |= (0x07) << 13;               /* funct3 = 111 */
                res |= (binst.zz.rs1 - 0x08) << 7; /* src = rs1 (x8 will map to x0) */
                res |= ((imm & 0x06) << 2) ;                res |=  ((imm & 0x18) << 7) ;                res |=  ((imm & 0x20) >> 3) ;                res |=  ((imm & 0xc0) >> 1) ;                res |=  ((imm & 0x100) << 4);/* set */
                citype[index] = CB1type;
                return res; /* return */
            }
        }
        else if (binst.zz.f3 == 0x00)
        {                                                       /* beq */
            if (binst.zz.rs1 >= 0x08 && binst.zz.rs1 <= 0x0f && /* x8~x15 */
                binst.zz.rs2 == 0x00)
            {                                      /* rs2 == x0 */
                res = 0x01;                        /* opcode= 01 */
                res |= (0x06) << 13;               /* funct3= 110 */
                res |= (binst.zz.rs1 - 0x08) << 7; /* src = rs1 (x8 will map to x0) */
                res |= ((imm & 0x06) << 2) ;                res |=  ((imm & 0x18) << 7) ;                res |=  ((imm & 0x20) >> 3) ;                res |=  ((imm & 0xc0) >> 1) ;                res |=  ((imm & 0x100) << 4);
                citype[index] = CB1type; /* Set */
                return res;
            } /* */
        }
        itype[index] = Btype; /* */
        return 0;
        break;
    case 0x6f: /* Cdh */
               /* Jtype */
        /* 20 CJ/C.J jal x0 off */
        /* 21 CJ/C.JAL jal x1 off */
        jinst.iinst = inst;
        imm |= jinst.zz.imm2 & 0x7fe;     /* 10:1 0b011111111110*/
        imm |= (jinst.zz.imm2 & 1) << 11; /* 11     ?00000000000*/
        imm |= jinst.zz.imm1 << 12;       /* 19:12 */
        if (jinst.zz.imm2 & 0x800)        { /* 20th sign bit is 1, negative */
            tmp = 1<<20;
            for (i = 20; i <= 31; i++)            { /* */
                imm |= tmp;
                if(i<31)tmp <<= 1; /* */
            }
        } /* */
        if (jinst.zz.rd <= 1)        { /* */
            if (jinst.zz.rd == 0)            {
                citype[index] = CJtype;
                res = 0x01;       /* opcode */
                res |= 0x5 << 13; /* funct3 = 0b101 */
                res |= (imm & 0xe) << 2; /* 0b1110 */
                res |= (imm & 0x10) << 7;                 res |= (imm & 0x20) >> 3;                res |= (imm & 0x40) << 1;                 res |= (imm & 0x80) >> 1;                res |= (imm & 0x300) << 1;                res |= (imm & 0x400) >> 2;                res |= (imm & 0x800) << 1; /* set */
                return res;
            } /* */
            else if (jinst.zz.rd == 1)            {
                citype[index] = CJtype;
                res = 0x01;              /* opcode */
                res |= 0x1 << 13;        /* funct3 */
                res |= (imm & 0xe) << 2;                res |= (imm & 0x10) << 7;                res |= (imm & 0x20) >> 3;                res |= (imm & 0x40) << 1;                res |= (imm & 0x80) >> 1;                res |= (imm & 0x300) << 1;                res |= (imm & 0x400) >> 2;                res |= (imm & 0x800) << 1; /* set */
                return res;
            } /* */
        }
        itype[index] = Jtype;
        return 0; /* */
        break;  /* */
    case 0x67:
        /* 3 CR/C.JR jalr x0 0(rs1) */
        /* 4 CR/C.JALR jalr x1 0(rs1) */
        iinst.iinst = inst;
        crinst.cinst = 0x00;
        if (iinst.zz.rs1 && /* rs1 != 0 */
            iinst.zz.f3==0 &&
            iinst.zz.imm == 0) {
            crinst.zz.opcode = 0x2;
            if (iinst.zz.rd == 0)
            { /* jr */
                crinst.zz.funct4 = 0x8;
                crinst.zz.rs1 = iinst.zz.rs1; /* */
                citype[index] = CRJtype;
                return crinst.cinst; /* */
            }
            else if (iinst.zz.rd == 1)
            { /* jalr */
                crinst.zz.funct4 = 0x9;
                crinst.zz.rs1 = iinst.zz.rs1;
                citype[index] = CRJtype;
                return crinst.cinst;
            } /* */
        }
        itype[index] = JItype;
        return 0; /* */
        /* */
    default:
         /* other strange instructions like ecall */
        return 0;                  /* */
        break;
    }
} /* */

void parse2ndOffset(int i,int count,int* pre,InstructionBits* insts,CompressedInstruction* Cresult,int* isCompressed,InstructionType* itype,CompressedInstType* citype){
    CB1typeInst cb1inst, rcb1inst; /* */
    CJtypeInst cjinst, rcjinst;
    BtypeInst binst, rbinst; /* */
    JtypeInst jinst, rjinst;
    CompressedInstruction res = 0;
    int imm = 0; /* */
    int tmp = 0;
    int ii = 0; /* */
    cb1inst.zz.funct3=0;/* */
    cb1inst.zz.imm1=0;
    cb1inst.zz.imm2=0;
    cb1inst.zz.opcode=0;/* */
    cb1inst.zz.rd=0;

    rcb1inst.zz.funct3=0;/* */
    rcb1inst.zz.imm1=0;
    rcb1inst.zz.imm2=0;
    rcb1inst.zz.opcode=0;/* */
    rcb1inst.zz.rd=0;

    cjinst.zz.funct3=0;/* */
    cjinst.zz.o10=0;
    cjinst.zz.o11=0;
    cjinst.zz.o123=0;/* */
    cjinst.zz.o4=0;
    cjinst.zz.o5=0;
    cjinst.zz.o6=0;/* */
    cjinst.zz.o7=0;
    cjinst.zz.o89=0;
    cjinst.zz.opcode=0;/* */

    rcjinst.zz.funct3=0;
    rcjinst.zz.o10=0;/* */
    rcjinst.zz.o11=0;
    rcjinst.zz.o123=0;/* */
    rcjinst.zz.o4=0;
    rcjinst.zz.o5=0;/* */
    rcjinst.zz.o6=0;
    rcjinst.zz.o7=0;/* */
    rcjinst.zz.o89=0;
    rcjinst.zz.opcode=0;/* */

    binst.zz.f3=0;
    binst.zz.imm1=0;/* */
    binst.zz.imm2=0;
    binst.zz.opcode=0;
    binst.zz.rs1=0;
    binst.zz.rs2=0;/* */

    rbinst.zz.f3=0;
    rbinst.zz.imm1=0;/* */
    rbinst.zz.imm2=0;
    rbinst.zz.opcode=0;
    rbinst.zz.rs1=0;/* */
    rbinst.zz.rs2=0;

    jinst.zz.imm1=0;/* */
    jinst.zz.imm2=0;
    jinst.zz.opcode=0;
    jinst.zz.rd=0;/* */

    rjinst.zz.imm1=0;
    rjinst.zz.imm2=0;/* */
    rjinst.zz.opcode=0;
    rjinst.zz.rd=0;/* */

    if (isCompressed[i]) {
        if (citype[i] == CB1type) { /* C.BEQZ, C.BNEZ */
            cb1inst.cinst = Cresult[i];
            imm |= (cb1inst.zz.imm1 & 0x6);       /* 2:1 */
            imm |= (cb1inst.zz.imm2 & 0x3) << 3;  /* 4:3 */
            imm |= (cb1inst.zz.imm1 & 0x1) << 5;  /* 5 */
            imm |= (cb1inst.zz.imm1 & 0x18) << 3; /* 7:6 */
            if (cb1inst.zz.imm2 & 0x4) { /* 8th sign bit is 1, negative */
                tmp = 0x100;
                for (ii = 8; ii <= 31; ii++) {
                    imm |= tmp; /* */
                    if(ii<31)tmp <<= 1;
                }
            }        /* */
            if(!(imm%4)){
                if(i + (imm / 4)>count)imm -= pre[count] - pre[i];/* */
                else if(i + (imm / 4)<0)imm -= pre[0] - pre[i];
                else imm -= pre[i + (imm / 4)] - pre[i];/* */
            }
            res |= ((imm & 0x06) << 2) ;
            res |=  ((imm & 0x18) << 7) ;/* */
            res |=  ((imm & 0x20) >> 3) ;
            res |=  ((imm & 0xc0) >> 1) ;/* */
            res |=  ((imm & 0x100) << 4);
            rcb1inst.cinst = res; /* */
            rcb1inst.zz.opcode = cb1inst.zz.opcode;
            rcb1inst.zz.rd = cb1inst.zz.rd; /* */
            rcb1inst.zz.funct3 = cb1inst.zz.funct3;
            /*    compressedInstOutput(rcb1inst.cinst);*/
            Cresult[i] = rcb1inst.cinst;
        }
        else if (citype[i] == CJtype) {
            cjinst.cinst = Cresult[i];
            imm |= cjinst.zz.o123 << 1; /* 3:1 */
            imm |= cjinst.zz.o4 << 4;   /* 4 */
            imm |= cjinst.zz.o5 << 5;   /* */
            imm |= cjinst.zz.o6 << 6;   /* */
            imm |= cjinst.zz.o7 << 7;   /* */
            imm |= cjinst.zz.o89 << 8;  /* */
            imm |= cjinst.zz.o10 << 10; /* */
            if (cjinst.zz.o11) { /* 11th sign bit is 1, negative */
                tmp = 0x800;
                for (ii = 11; ii <= 31; ii++) { /* */
                    imm |= tmp;
                    if(ii<31)tmp <<= 1; /* */
                }
            }
            if(!(imm%4)){
                if(i + (imm / 4)>count)imm -= pre[count] - pre[i];/* */
                else if(i + (imm / 4)<0)imm -= pre[0] - pre[i];
                else imm -= pre[i + (imm / 4)] - pre[i];/* */
            }
            /*    compressedInstOutput(imm);*/
            rcjinst.cinst = 0; /* */
            rcjinst.zz.opcode = cjinst.zz.opcode;
            rcjinst.zz.funct3 = cjinst.zz.funct3;
            rcjinst.zz.o123 = (imm & 0xe) >> 1;
            rcjinst.zz.o4 = (imm & (1 << 4)) >> 4;
            rcjinst.zz.o5 = (imm & (1 << 5)) >> 5; /* */
            rcjinst.zz.o6 = (imm & (1 << 6)) >> 6;
            rcjinst.zz.o7 = (imm & (1 << 7)) >> 7;
            rcjinst.zz.o89 = (imm & 0x300) >> 8;
            rcjinst.zz.o10 = (imm & (1 << 10)) >> 10; /* */
            rcjinst.zz.o11 = (imm & (1 << 11)) >> 11;
            Cresult[i] = rcjinst.cinst;
        }
    } /* */
    else
    {
        if (itype[i] == Btype)
        { /* */
            binst.iinst = insts[i];
            imm |= binst.zz.imm1 & 0x1e; /* */
            imm |= (binst.zz.imm2 & 0x3f) << 5;
            imm |= (binst.zz.imm1 & 0x01) << 11;
            if (binst.zz.imm2 & 0x40) { /* 12th sign bit is 1, negative */
                tmp = 0x1000;
                for (ii = 12; ii <= 31; ii++)  { /* */
                    imm |= tmp;
                    if(ii<31)tmp <<= 1; /* */
                }
            }
            if(!(imm%4)){
                if(i + (imm / 4)>count)imm -= pre[count] - pre[i];/* */
                else if(i + (imm / 4)<0)imm -= pre[0] - pre[i];
                else imm -= pre[i + (imm / 4)] - pre[i];/* */
            }
            rbinst.zz.opcode = binst.zz.opcode; /* */
            rbinst.zz.f3 = binst.zz.f3;
            rbinst.zz.rs1 = binst.zz.rs1;
            rbinst.zz.rs2 = binst.zz.rs2; /* */
            rbinst.zz.imm1 = (imm & 0x1e) | ((imm & (1 << 11)) >> 11);
            rbinst.zz.imm2 = ((imm & 0x7e0) >> 5) | ((imm & (1 << 12)) >> 6);
            insts[i] = rbinst.iinst; /* */
        }
        else if (itype[i] == Jtype){
            jinst.iinst = insts[i];
            imm |= jinst.zz.imm2 & 0x7fe;     /* 10:1 */
            imm |= (jinst.zz.imm2 & 1) << 11; /* 11 */
            imm |= jinst.zz.imm1 << 12;       /* 19:12 */
            if (jinst.zz.imm2 & 0x800) { /* 20th sign bit is 1, negative */
                tmp = 0x100000;
                for (ii = 20; ii <= 31; ii++) { /* */
                    imm |= tmp;
                    if(ii<31)tmp <<= 1; /* */
                }
            } /* */
            if(!(imm%4)){
                if(i + (imm / 4)>count)imm -= pre[count] - pre[i];/* */
                else if(i + (imm / 4)<0)imm -= pre[0] - pre[i];
                else imm -= pre[i + (imm / 4)] - pre[i];/* */
            }
            rjinst.zz.opcode = jinst.zz.opcode; /* */
            rjinst.zz.rd = jinst.zz.rd;
            rjinst.zz.imm1 = (imm & 0xff000) >> 12;
            rjinst.zz.imm2 = (imm & 0x7fe) | ((imm & (1 << 11)) >> 11) | ((imm & (1 << 20)) >> 9); /* */
            insts[i] = rjinst.iinst;
        } /* */
    }
}
