#ifndef UTILS_H
#define UTILS_H

/* Your code here... */
#include <stdint.h>
#include <stdio.h>
#define MAX_INST 1000 /* no more than 50 lines each input file */

/* remove this line when submitting! *//*
#define DBG_FLAG*/


/* NOT in Soren C
typedef uint32_t InstructionBits;
typedef uint16_t CompressedInstruction;
*/

typedef unsigned int InstructionBits; /* uint32_t */
typedef unsigned int CompressedInstruction; /* uint32_t for Soren C */

/* enumerate type used in switch() for convenience */
typedef enum {
    None,    Rtype,    Itype,    JItype,    Utype,    Stype,    Btype,    Jtype
} InstructionType;

typedef enum {/* enumerate type used in switch() for convenience */
    CNone,    CRtype,    CRJtype,    CItype,    CUtype,    CS1type,    CS2type,   CB1type,    CB2type,    CJtype
} CompressedInstType;

/* Use bit-field to facilitate accessing             */
/* code order goes from back to front(right to left) */

typedef union {
    InstructionBits iinst;/* */
    struct {
        InstructionBits opcode : 7; /* opcode */
        InstructionBits rd : 5; /* rd */
        InstructionBits f3 : 3; /* f3 */ 
        InstructionBits rs1 : 5; /* rs1 */
        InstructionBits rs2 : 5; /* rs2 */
        InstructionBits f7 : 7; /* f7 */
    } zz;
} RtypeInst;
/* */
typedef union {/* */
    InstructionBits iinst;
    struct {
        InstructionBits opcode : 7;/* */
        InstructionBits rd : 5;
        InstructionBits f3 : 3;/* */
        InstructionBits rs1 : 5;
        InstructionBits imm : 12;
    } zz;
} ItypeInst;

typedef union {/* */
    InstructionBits iinst;/* */
    struct {/* */
        InstructionBits opcode : 7;
        InstructionBits imm1 : 5;
        InstructionBits f3 : 3;/* */
        InstructionBits rs1 : 5;
        InstructionBits rs2 : 5;
        InstructionBits imm2 : 7;
    } zz;/* */
} StypeInst;

typedef union {
    InstructionBits iinst;/* */
    struct {
        InstructionBits opcode : 7;
        InstructionBits imm1 : 5;/* */
        InstructionBits f3 : 3;/* */
        InstructionBits rs1 : 5;
        InstructionBits rs2 : 5;
        InstructionBits imm2 : 7;/* */
    } zz;
} BtypeInst;/* */

typedef union {
    InstructionBits iinst;/* */
    struct {
        InstructionBits opcode : 7;
        InstructionBits rd : 5;/* */
        InstructionBits imm : 20;
    } zz;
} UtypeInst;/* */

typedef union {
    InstructionBits iinst;/* */
    struct {
        InstructionBits opcode : 7;/* */
        InstructionBits rd : 5;/* */
        InstructionBits imm1 : 8;
        InstructionBits imm2 : 12;/* */
    } zz;
} JtypeInst;/* */

/* below are RSV insttruction bit-field utilities */

typedef union {
    CompressedInstruction cinst;/* */
    struct {
        CompressedInstruction opcode : 2;/* */
        CompressedInstruction rs2 : 5;
        CompressedInstruction rs1 : 5;/* */
        CompressedInstruction funct4 : 4;
      /*  CompressedInstruction :16;*/
    } zz;
} CRtypeInst;/* */

typedef union {/* */
    CompressedInstruction cinst;
    struct {/* */
        CompressedInstruction opcode : 2;
        CompressedInstruction imm1 : 5;
        CompressedInstruction rd : 5;/* */
        CompressedInstruction imm2 : 1;
        CompressedInstruction funct3 : 3;/* */
       /* CompressedInstruction :16;*/
    } zz;
} CItypeInst;/* */

typedef union {/* */
    CompressedInstruction cinst;
    struct {
        CompressedInstruction opcode : 2;/* */
        CompressedInstruction rd : 3;
        CompressedInstruction imm1 : 2;/* */
        CompressedInstruction rs1 : 3;
        CompressedInstruction imm2 : 3;/* */
        CompressedInstruction funct3 : 3;
    /*    CompressedInstruction :16; */
    } zz;
} CLtypeInst;/* */

typedef union {
    CompressedInstruction cinst;/* */
    struct {
        CompressedInstruction opcode : 2;
        CompressedInstruction rs2 : 3;/* */
        CompressedInstruction imm1 : 2;
        CompressedInstruction rs1 : 3;/* */
        CompressedInstruction imm2 : 3;
        CompressedInstruction funct3 : 3;
     /*   CompressedInstruction :16;*/
    } zz;/* */
} CS1typeInst;

typedef union {/* */
    CompressedInstruction cinst;
    struct {
        CompressedInstruction opcode : 2;/* */
        CompressedInstruction rs2 : 3;
        CompressedInstruction funct2 : 2;
        CompressedInstruction rs1 : 3;/* */
        CompressedInstruction funct6 : 6;
       /* CompressedInstruction :16; */
    } zz;
} CS2typeInst;
/* */
typedef union {
    CompressedInstruction cinst;/* */
    struct {
        CompressedInstruction opcode : 2;
        CompressedInstruction imm1 : 5;/* */
        CompressedInstruction rd : 3;
        CompressedInstruction imm2 : 3;/* */
        CompressedInstruction funct3 : 3;
     /*   CompressedInstruction :16; */
    } zz;
} CB1typeInst;
/* */
typedef union {
    CompressedInstruction cinst;
    struct {/* */
        CompressedInstruction opcode : 2;
        CompressedInstruction imm1 : 5;/* */
        CompressedInstruction rd : 3;
        CompressedInstruction funct2 : 2;
        CompressedInstruction imm2 : 1;/* */
        CompressedInstruction funct3 : 3;
      /*  CompressedInstruction :16;*/
    } zz;/* */
} CB2typeInst;
/* */
typedef union {
    CompressedInstruction cinst;/* */
    struct {
        CompressedInstruction opcode : 2;/* */
        CompressedInstruction o5 : 1;
        CompressedInstruction o123 : 3;
        CompressedInstruction o7 : 1;/* */
        CompressedInstruction o6 : 1;
        CompressedInstruction o10 : 1;/* */
        CompressedInstruction o89 : 2;
        CompressedInstruction o4 : 1;/* */
        CompressedInstruction o11 : 1;
        CompressedInstruction funct3 : 3;
      /*  CompressedInstruction :16; */
    } zz;
} CJtypeInst;/* */

/* accept input string char[34], return uint32_t containing inst bits*/
InstructionBits instParser(char* inst);
InstructionBits getinst(FILE** input);

/* */

void compressedInstOutput(FILE** output, CompressedInstruction cinst);/* */
void InstOutput(FILE** output, InstructionBits inst);

#endif
