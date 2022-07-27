#   Project 1.2 RVC instructions to RISC-V instructions in RISC-V

#   You can suppose you will get the number of machine codes you should translate from 
#   "lines_of_rvc_codes" and get the 16 or 32 bits-machine codes from "rvc_codes". 
#   Both of them are stored in file "input.S"

#   When we test your project, we will change the input files content.

# DO NOT REMOVE THIS LINE WHEN SUBMITTING
.import input.S

# Your code to convert the RVC codes to RVI codes.
.text

la     a0 lines_of_rvc_codes # address for # of lines
lw     t6 0(a0)              # # of lines
la     a2 rvc_codes          # address for codes to translate
mv     fp sp                 # save start of sp to fp (x8/s0 frame pointer)

# All the 32-bit instructions in the base ISA(Instruction Set Architecture) have their lowest two bits set to 11. 
# The optional compressed 16-bit instruction-set extensions have their lowest two bits equal to 00, 01, or 10

main_loop: # check each line of code

    li     t3 3             # set t3 to 0b11

    beqz   t6 finish  # no more lines, finish
    addi   t6 t6 -1        # t6--

    lw     a3 0(a2)        # read line into a3
    and    t1 a3 t3        # t1 = a3 & 0b11, check last two digits

    beq    t1 t3 save_32   # if t1 ends with 11, t0 is a 32-bit instruction, output

    # !!! BELOW ARE CASES FOR 16-BIT               !!! 
    # !!! T1 IS USED TO CHECK BIT, INIT BEFORE USE !!!
    # !!! T6 IS USED TO COUNT LINES, DO NOT USE    !!!
    # !!! A3 IS ORIGINAL 16-BIT STRING             !!!
    # !!! A4 IS USED TO SAVE 32-BIT STRING         !!!

 # RVC instructions can be distinguished 
 # when funct3/4/6 and opcode work together   
 # below we switch the MSBs of instructions, which is funct
 # cases denoted as AAA_BB where A is funct B is opcode

    li     a4 0  # set a4 to 0
    slli   a3 a3 16   # remove first 16 bits
    srli   a3 a3 16   # reset
    srli   t1 a3 15   # get bit[15] of the 16-bit instruction
    beqz   t1 0_
    
1_: # funct starts with 1
    srli   t1 a3 14 # bit[14]
    andi   t1 t1 1  # remove other bits
    beqz   t1 10_

    11_: # funct starts with 11
        addi   t1 a3 0 # copy to t1
        andi   t1 t1 1 # bit[0]
        beqz   t1 C_SW 
        # since 11 is reserved for 32-bit instructions, only need to judge bit[0]

        11_01: # opcode 01
            # same operations for c.beqz and c.bnez
            li     a4 99 # opcode 0b1100011

            andi   t2 a3 896   #0b1110000000(7-9)
            addi   t2 t2 1024  #x8-x15
            slli   t2 t2 8     #19:15
            add    a4 a4 t2    #[rd]
            slli   t2 t2 15 # bit[19:15]
            or     a4 a4 t2
            slli   t2 t2 5  # bit[24:20]
            or     a4 a4 t2
            # imm format: 12|11|10   6| 5| 4| 3| 2
            #              8| 4| 3   7| 6| 2| 1| 5
            srli   t2 a3 12 # bit[12], imm[8]
            andi   t2 t2 1  # remove other bits
            slli   t2 t2 2  # make space for 76
            srli   t3 a3 5  # bit[6:5], imm[7:6]
            andi   t3 t3 3  # & 0b11
            or     t2 t2 t3 # concat
            slli   t2 t2 1  # make space for 5
            srli   t3 a3 2  # bit[2], imm[5]
            andi   t3 t3 1  # remove other bits
            or     t2 t2 t3 # concat
            slli   t2 t2 2  # make space for 43
            srli   t3 a3 10 # bit[11:10], imm[4:3]
            andi   t3 t3 3  # & 0b11
            or     t2 t2 t3 # concat
            slli   t2 t2 2  # make space for 21
            srli   t3 a3 3  # bit[4:3], imm[2:1]
            andi   t3 t3 3  # & 0b11
            or     t2 t2 t3 # concat
            # imm does not follow convention, corrected later
            # bit[31:24] - imm[8:1] - bit[7:0]
            # borrows 2 bits from rs2, but rs2 is x0 = 0b00000
            slli   t2 t2 24 # first bit in t2 is now at the first bit in the register
            or     a4 a4 t2
            # finished

            srli   t1 a3 13 # bit[13]
            andi   t1 t1 1  # remove other bits
            beqz   t1 C_BEQZ

            C_BNEZ: # 111_01
                # f3 = 001
                li     t2 1  # set t2 to 1
                slli   t2 t2 12   # f3 001
                or     a4 a4 t2
                j      save_16

            C_BEQZ: # 110_01
                # f3 = 000, do nothing
                j      save_16

        C_SW: # 11_00 opcode 00
            li   a4 35    #[opcode] 0b0100011
            andi t2 a3 28    #0b11100(2-4)
            addi t2 t2 32    #x8-x15
            slli t2 t2 18    #move to 24:20
            add  a4 a4 t2    #[rs2]
            li   t3 2        #0b010
            slli t3 t3 12    #move to 14:12
            add  a4 a4 t3    #[func3]
            andi t2 a3 896   #0b1110000000(7-9)
            addi t2 t2 1024  #x8-x15
            slli t2 t2 8     #move to 19:15
            add  a4 a4 t2    #[rs1]
            li   t4 3
            slli t4 t4 10
            and  t3 a3 t4    #0b110000000000(10-11)
            add  a4 a4 t3    #imm[4:3]
            li   t4 1
            slli t4 t4 12
            and  t3 a3 t4    #0b1000000000000(12)
            slli t3 t3 13    #move to 25
            add  a4 a4 t3    #imm[5]
            andi t3 a3 32    #0b100000(5)
            slli t3 t3 21    #move to 26(no sign-extended)
            add  a4 a4 t3    #imm[6]
            andi t3 a3 64    #0b1000000(6)
            slli t3 t3 3     #move to 9
            add  a4 a4 t3    #imm[2]
            j      save_16

    10_: # funct starts with 10
        addi   t1 a3 0 # copy to t1
        andi   t1 t1 1 # bit[0]
        beqz   t1 10_10

        10_01:
            srli   t1 a3 13 # bit[13]
            andi   t1 t1 1  # remove other bits
            beqz   t1 100_01

            C_J: # 101_01
                # opcode
                li     a4 111 # opcode 0b1101111
                # rd is x0 = 00000, do nothing
                # imm format: 12|11|10| 9| 8| 7| 6| 5: 3| 2| 1| 0
                #             11| 4| 9| 8|10| 6| 7| 3: 1| 5|  |  
                srli   t2 a3 12 # bit[12], imm[11]
                andi   t2 t2 1  # remove other bits
                slli   t2 t2 1  # make space for 10
                srli   t3 a3 8  # bit[8], imm[10]
                andi   t3 t3 1  # remove other bits
                or     t2 t2 t3 # concat
                slli   t2 t2 2  # make space for 98
                srli   t3 a3 9  # bit[10:9], imm[9:8]
                andi   t3 t3 3  # & 0b11
                or     t2 t2 t3 # concat
                slli   t2 t2 1  # make space for 7
                srli   t3 a3 6  # bit[6], imm[7]
                andi   t3 t3 1  # remove other bits
                or     t2 t2 t3 # concat
                slli   t2 t2 1  # make space for 6
                srli   t3 a3 7  # bit[7], imm[6]
                andi   t3 t3 1  # remove other bits
                or     t2 t2 t3 # concat
                slli   t2 t2 1  # make space for 5
                srli   t3 a3 2  # bit[2], imm[5]
                andi   t3 t3 1  # remove other bits
                or     t2 t2 t3 # concat
                slli   t2 t2 1  # make space for 4
                srli   t3 a3 11 # bit[11], imm[4]
                andi   t3 t3 1  # remove other bits
                or     t2 t2 t3 # concat
                slli   t2 t2 3  # make space for 321
                srli   t3 a3 3  # bit[5:3], imm[3:1]
                andi   t3 t3 7  # & 0b111
                or     t2 t2 t3 # concat
                # imm does not follow convention, corrected later
                # bit[31:21] - imm[11:1] - bit[10:0]
                slli   t2 t2 21 # first bit in t2 is now at the first bit in the register
                or     a4 a4 t2
                # finished
                j      save_16

            100_01:
                srli   t1 a3 11 # bit[11]
                andi   t1 t1 1  # remove other bits
                beqz   t1 10000_01

                10001_01:
                    srli   t1 a3 10 # bit[10]
                    andi   t1 t1 1  # remove other bits
                    beqz   t1 C_ANDI

                    100011_01:
                        srli   t1 a3 6 # bit[6]
                        andi   t1 t1 1 # remove other bits
                        beqz   t1 100011_0_01

                        100011_1_01:
                            srli   t1 a3 5 # bit[5]
                            andi   t1 t1 1 # remove other bits
                            beqz   t1 C_OR

                            C_AND: #100011_11_01
                                li   a4 51    #[opcode] 0b0110011
                                andi t2 a3 896   #0b1110000000(7-9)
                                addi t2 t2 1024  #x8-x15
                                add  a4 a4 t2    #[rd]
                                li   t4 7        #0b111
                                slli t4 t4 12    #0b111000000000000(12-14)
                                add  a4 a4 t4    #[func3]
                                slli t2 t2 8     #rs1=rd
                                add  a4 a4 t2    #[rs1]
                                andi t2 a3 28    #0b11100(2-4)
                                addi t2 t2 32    #x8-x15
                                slli t2 t2 18    #move to 24:20
                                add  a4 a4 t2    #[rs2]
                                nop              #[func7]
                                j      save_16

                            C_OR: #100011_10_01
                                li   a4 51    #[opcode] 0b0110011
                                andi t2 a3 896   #0b1110000000(7-9)
                                addi t2 t2 1024  #x8-x15
                                add  a4 a4 t2    #[rd]
                                li   t4 3
                                slli t4 t4 13    #0b110000000000000(13-14)
                                add  a4 a4 t4    #[func3] 
                                slli t2 t2 8     #rs1=rd
                                add  a4 a4 t2    #[rs1]
                                andi t3 a3 28    #0b11100(2-4)
                                addi t3 t3 32    #x8-x15
                                slli t3 t3 18    #move to 24:20
                                add  a4 a4 t3    #[rs2]
                                nop              #[func7]
                                j      save_16
                        
                        100011_0_01:
                            srli   t1 a3 5 # bit[5]
                            andi   t1 t1 1 # remove other bits
                            beqz   t1 C_SUB

                            C_XOR: #100011_01_01
                                li   a4 51    #[opcode] 0b0110011
                                andi t2 a3 896   #0b1110000000(7-9)
                                addi t2 t2 1024  #x8-x15
                                add  a4 a4 t2    #[rd]
                                li   t4 1
                                slli t4 t4 14    #0b100000000000000(14)
                                add  a4 a4 t4    #[func3]
                                slli t2 t2 8     #rs1=rd
                                add  a4 a4 t2    #[rs1]
                                andi t3 a3 28    #0b11100(2-4)
                                addi t3 t3 32    #x8-x15
                                slli t3 t3 18    #move to 24:20
                                add  a4 a4 t3    #[rs2]
                                nop              #[func7]
                                j      save_16

                            C_SUB: #100011_00_01
                                li   a4 51    #[opcode] 0b0110011
                                andi t2 a3 896   #0b1110000000(7-9)
                                addi t2 t2 1024  #x8-x15
                                add  a4 a4 t2    #[rd]
                                nop              #[func3] 0b000000000000000(12-14)
                                slli t2 t2 8     #rs1=rd
                                add  a4 a4 t2    #[rs1]
                                andi t3 a3 28    #0b11100(2-4)
                                addi t3 t3 32    #x8-x15
                                slli t3 t3 18    #move to 24:20
                                add  a4 a4 t3    #[rs2]
                                addi t3 t3 32    #0b0100000
                                slli t3 t3 25    #move to 31:25
                                add a4 a4 t3    #[func7]
                                j      save_16

                    C_ANDI: # 100010_01
                        li   a4 19  #[opcode] 0b0010011
                        andi t2 a3 896   #0b1110000000(7-9)
                        addi t2 t2 1024  #x8-x15
                        add  a4 a4 t2    #[rd]
                        li   t4 7
                        slli t4 t4 12    #0b111000000000000(12-14)
                        add  a4 a4 t4    #[func3]
                        slli t2 t2 8     #rs1=rd
                        add  a4 a4 t2    #[rs1]
                        andi t3 a3 124   #0b1111100(2-6)
                        slli t3 t3 18    #move to 24:20
                        add  a4 a4 t3    #imm[4:0]
                        li   t4 1
                        slli t4 t4 12    #0b1000000000000(12)
                        and  t3 a3 t4    
                        slli t3 t3 19    #move to 31
                        srai t3 t3 6     #move to 25
                        add  a4 a4 t3    #imm[5]
                        j    save_16

                10000_01:
                    srli   t1 a3 10 # bit[10]
                    andi   t1 t1 1  # remove other bits
                    beqz   t1 C_SRLI

                    C_SRAI: #100001_01
                        li   a4 19    #[opcode] 0b0010011
                        andi t2 a3 896   #0b1110000000(7-9)
                        addi t2 t2 1024  #x8-x15
                        add  a4 a4 t2    #[rd]
                        li   t4 5
                        slli t4 t4 12    #0b101000000000000(12-14)
                        add  a4 a4 t4    #[func3] 
                        slli t2 t2 8     #rs1=rd
                        add  a4 a4 t2    #[rs1]
                        andi t3 a3 124   #0b1111100(2-6)
                        slli t3 t3 18    #move to 24:20
                        add  a4 a4 t3    #shamt[4:0]
                        li   t3 32    #0b0100000
                        slli t3 t3 25    #move to 31:25
                        add  a4 a4 t3    #[func7]
                        j      save_16

                    C_SRLI: #100000_00
                        li   a4 19  #[opcode] 0b0010011
                        andi t2 a3 896   #0b1110000000(7-9)
                        addi t2 t2 1024  #x8-x15
                        add  a4 a4 t2    #[rd]
                        li   t4 5
                        slli t4 t4 12    #0b101000000000000(12-14)
                        add  a4 a4 t4    #[func3] 
                        slli t2 t2 8     #rs1=rd
                        add  a4 a4 t2    #[rs1]
                        andi t3 a3 124   #0b1111100(2-6)
                        slli t3 t3 18    #move to 24:20
                        add  a4 a4 t3    #shamt[4:0]
                        nop              #[func7] 0b0000000
                        j      save_16

        10_10: 
            srli   t1 a3 12 # bit[12]
            andi   t1 t1 1  # remove other bits
            beqz   t1 1000_10

            1001_10:
                srli   t1 a3 2  # bit[2:6]
                andi   t1 t1 31 # 31 = 0b11111
                beqz   t1 C_JALR

                C_ADD:
                    li   a4 51    #[opcode] 0b0110011
                    li   t4 31
                    slli t4 t4 7     #0b111110000000(7-11)
                    and  t2 a3 t4    
                    add  a4 a4 t2    #[rd]
                    nop              #[func3]
                    slli t2 t2 8     #rs1=rd
                    add  a4 a4 t2    #[rs1]
                    andi t3 a3 124   #0b1111100(2-6)
                    slli t3 t3 18    #move to 24:20
                    add  a4 a4 t3    #[rs2]
                    nop              #[func7]
                    j      save_16

                C_JALR: #1001_0000010
                    li   a4 103   #[opcode] 0b1100111
                    addi a4 a4 128   #[rd] 0b01 0000000
                    nop              #[func3]
                    li   t4 31
                    slli t4 t4 7
                    and  t2 a3 t4    #0b111110000000(7-11)
                    slli t2 t2 8     #move to 19:15
                    add  a4 a4 t2    #[rs1]
                    nop              #[rs2]
                    nop              #[func7]
                    j      save_16

            1000_10:
                srli   t1 a3 2  # bit[2:6]
                andi   t1 t1 31 # 31 = 0b11111
                beqz   t1 C_JR

                C_MV:
                    li   a4 51    #[opcode] 0b0110011
                    li   t4 31
                    slli t4 t4 7
                    and  t2 a3 t4    #0b111110000000(7-11)
                    add  a4 a4 t2    #[rd]
                    nop              #[func3]
                    nop              #[rs1]
                    andi t3 a3 124   #0b1111100(2-6)
                    slli t3 t3 18    #move to 24:20
                    add  a4 a4 t3    #[rs2]
                    nop              #[func7]
                    j      save_16

                C_JR: # 1000_0000010
                    li   a4 103   #[opcode] 0b1100111
                    nop              #[rd]
                    nop              #[func3]
                    li   t4 31
                    slli t4 t4 7     #0b111110000000(7-11)
                    and  t2 a3 t4    
                    slli t2 t2 8     #move to 19:15
                    add  a4 a4 t2    #[rs1]
                    nop              #[rs2]
                    nop              #[func7]
                    j      save_16

0_:  # funct starts with 0
    srli   t1 a3 1 # bit[1]
    andi   t1 t1 1 # remove other bits
    beqz   t1 0_0_

    C_SLLI: # 0_1_
        li   a4 19    #[opcode] 0b0010011
        li   t4 31
        slli t4 t4 7
        and  t2 a3 t4    #0b111110000000(7-11)
        add  a4 a4 t2    #[rd]
        li   t4 1
        slli t4 t4 12
        add  a4 a4 t4    #[func3] 0b001000000000000(12-14)
        slli t2 t2 8     #rs1=rd
        add  a4 a4 t2    #[rs1]
        andi t3 a3 124   #0b1111100(2-6)
        slli t3 t3 18    #move to 24:20
        add  a4 a4 t3    #shamt[4:0]
        nop              #[func7] 0b0000000
        j      save_16

    0_0_:
        addi   t1 a3 0 # copy to t1
        andi   t1 t1 1 # bit[0]
        beqz   t1 C_LW

        0_01:
            srli   t1 a3 14 # bit[14]
            andi   t1 t1 1  # remove other bits
            beqz   t1 00_01

            01_01:
                srli   t1 a3 13 # bit[13]
                andi   t1 t1 1  # remove other bits
                beqz   t1 C_LI

                C_LUI: # 011_01
                    li   a4 55    #[opcode] 0b0110111
                    li   t4 31
                    slli t4 t4 7
                    and  t2 a3 t4    #0b111110000000(7-11)
                    add  a4 a4 t2    #[rd]
                    andi t3 a3 124   #0b1111100(2-6)
                    slli t3 t3 10    #move to 16:12
                    add  a4 a4 t3    #imm[4:0]
                    li t4 1
                    slli t4 t4 12
                    and  t3 a3 t4    #0b1000000000000(12)
                    slli t3 t3 19    #move to 31
                    srai t3 t3 14    #move to 17
                    add  a4 a4 t3    #imm[5]
                    j      save_16

                C_LI: # 010_01
                    li   a4 19    #[opcode] 0b0010011
                    li   t4 31
                    slli t4 t4 7
                    and  t2 a3 t4    #0b111110000000(7-11)
                    add  a4 a4 t2    #[rd]
                    nop              #[func3] 0b000
                    nop              #[rs1] x0
                    andi t3 a3 124   #0b1111100(2-6)
                    slli t3 t3 18    #move to 24:20
                    add  a4 a4 t3    #imm[4:0]
                    li   t4 1
                    slli t4 t4 12
                    and  t3 a3 t4    #0b1000000000000(12)
                    slli t3 t3 19    #move to 31
                    srai t3 t3 6     #move to 25
                    add  a4 a4 t3    #imm[5]
                    j      save_16


            00_01:
                srli   t1 a3 13 # bit[13]
                andi   t1 t1 1  # remove other bits
                beqz   t1 C_ADDI

                C_JAL: # 001_01
                    # opcode, also rd is x1 = 00001
                    li     a4 239 # 0b11101111
                    # imm format: 12|11|10| 9| 8| 7| 6| 5: 3| 2| 1| 0
                    #             11| 4| 9| 8|10| 6| 7| 3: 1| 5|  |  
                    srli   t2 a3 12 # bit[12], imm[11]
                    andi   t2 t2 1  # remove other bits
                    slli   t2 t2 1  # make space for 10
                    srli   t3 a3 8  # bit[8], imm[10]
                    andi   t3 t3 1  # remove other bits
                    or     t2 t2 t3 # concat
                    slli   t2 t2 2  # make space for 98
                    srli   t3 a3 9  # bit[10:9], imm[9:8]
                    andi   t3 t3 3  # & 0b11
                    or     t2 t2 t3 # concat
                    slli   t2 t2 1  # make space for 7
                    srli   t3 a3 6  # bit[6], imm[7]
                    andi   t3 t3 1  # remove other bits
                    or     t2 t2 t3 # concat
                    slli   t2 t2 1  # make space for 6
                    srli   t3 a3 7  # bit[7], imm[6]
                    andi   t3 t3 1  # remove other bits
                    or     t2 t2 t3 # concat
                    slli   t2 t2 1  # make space for 5
                    srli   t3 a3 2  # bit[2], imm[5]
                    andi   t3 t3 1  # remove other bits
                    or     t2 t2 t3 # concat
                    slli   t2 t2 1  # make space for 4
                    srli   t3 a3 11 # bit[11], imm[4]
                    andi   t3 t3 1  # remove other bits
                    or     t2 t2 t3 # concat
                    slli   t2 t2 3  # make space for 321
                    srli   t3 a3 3  # bit[5:3], imm[3:1]
                    andi   t3 t3 7  # & 0b111
                    or     t2 t2 t3 # concat
                    # imm does not follow convention, corrected later
                    # bit[31:21] - imm[11:1] - bit[10:0]
                    slli   t2 t2 21 # first bit in t2 is now at the first bit in the register
                    or     a4 a4 t2
                    # finished
                    j      save_16

                C_ADDI: # 000_01
                    li   a4 19    #[opcode] 0b0010011
                    li   t4 31
                    slli t4 t4 7
                    and  t2 a3 t4    #0b111110000000(7-12)
                    add  a4 a4 t2    #[rd]
                    nop              #[func3] 0b000
                    slli t2 t2 8     #rs1=rd
                    add  a4 a4 t2    #[rs1]
                    andi t3 a3 124   #0b1111100(2-6)
                    slli t3 t3 18    #move to 24:20
                    add  a4 a4 t3    #imm[4:0]
                    li   t4 1
                    slli t4 t4 12
                    and  t3 a3 t4    #0b1000000000000(12)
                    slli t3 t3 19    #move to 31
                    srai t3 t3 6     #move to 25
                    add  a4 a4 t3    #imm[5]
                    j      save_16

        C_LW: # 0_00
            li   a4 3     #[opcode] 0b0000011
            andi t2 a3 28    #0b11100(2-4)
            addi t2 t2 32    #x8-x15
            slli t2 t2 5     #move to 11:7
            add  a4 a4 t2    #[rd]
            li   t3 2     #0b010
            slli t3 t3 12    #move to 14:12
            add  a4 a4 t3    #[func3]
            andi t2 a3 896   #0b1110000000(7-9)
            addi t2 t2 1024  #x8-x15
            slli t2 t2 8     #move to 19:15
            add  a4 a4 t2    #[rs1]
            li   t4 7
            slli t4 t4 10    #0b1110000000000(10-12)
            and  t3 a3 t4    
            slli t3 t3 13    #move to 25:23
            add  a4 a4 t3    #imm[5:3]
            andi t3 a3 32    #0b100000(5)
            slli t3 t3 21    #move to 26(no sign-extended)
            add  a4 a4 t3    #imm[6]
            andi t3 a3 64    #0b1000000(6)
            slli t3 t3 16    #move to 22
            add  a4 a4 t3    #imm[2]
            j      save_16


save_32: # is 32 bit instruction, write away
    addi   a2 a2 4  # add 4 to address, to next 32 bit
    addi   sp sp -8 # allocate 2 words on stack
    sw     a3 0(sp) # the 32-bit instruction
    li     a3 4
    sw     a3 4(sp) # size of the compressed instruction
    j      main_loop

save_16:
    addi   a2 a2 2  # add 2 to address, to next 16 bit
    addi   sp sp -8 # allocate 2 words on stack
    sw     a4 0(sp) # the 32-bit instruction
    li     a3 2
    sw     a3 4(sp) # size of the compressed instruction
    j      main_loop

finish:
    #        output with fp
    la     a0 lines_of_rvc_codes # address for # of lines
    lw     t6 0(a0)              # # of lines
    li     t0 4             # set t0 to 4, for checking line original size
    addi   s2 fp 0               # save fp to s2, for output_loop
    j      fix_loop

fix_end:
    # output every line
    lw     t6 0(a0)
    addi   fp s2 0 # copy s2 back to fp
    j      output_loop
exit:
    # Exit with error code 0
    li     a1 0
    li     a0 17
    ecall # call exit

output_loop:
    beqz   t6 exit    # no more lines, finish
    addi   t6 t6 -1        # t6--
    addi   fp fp -8        # next line
    lw     a3 0(fp)        # read line into a3

    li     t0 31      # set t0 to 31, counter for output
    j      output
  
output:
    srl    a1 a3 t0               # shift string right to current bit
    andi   a1 a1 1                # save the last bit to a1
    li     a0 1              # print int a1
    ecall # call print
    beqz   t0 output_done    # finished writing
    addi   t0 t0 -1               # t0--
    j      output                 # recurse

output_done:
    li     a0 11 # print char
    li     a1 10 # ASCII for \n
    ecall  # call print
    j      output_loop

fix_loop: 
    beqz   t6 fix_end      # no more lines, finish
    addi   t6 t6 -1        # t6--
    addi   fp fp -8        # next line
    addi   s1 fp 0         # copy curremt pos to s1
    lw     t1 4(fp)        # read line original size
    lw     a3 0(fp)        # read line into a3
    beq    t1 t0 fix_loop  # if original size is 4, aka 32-bit instruction

    srli   t2 a3 6 # bit[6]
    andi   t2 t2 1 # remove other bits
    beqz   t2 fix_loop

    srli   t2 a3 5 # bit[5]
    andi   t2 t2 1 # remove other bits
    beqz   t2 fix_loop
    
    srli   t2 a3 3 # bit[3]
    andi   t2 t2 1 # remove other bits
    bnez   t2 fix_jal

    srli   t2 a3 2 # bit[2]
    andi   t2 t2 1 # remove other bits
    bnez   t2 fix_loop

    srli   t2 a3 1 # bit[1]
    andi   t2 t2 1 # remove other bits
    bnez   t2 fix_b

    j      fix_loop

fix_jal: # opcode 1101111
         # bit[31:21] - imm[11:1]
    srai   t2 a3 20  # bit[31:20] which is imm
    li     t0 0 # set t0 to 0, used to store fixed imm
    blt    t2 x0 fix_jal_neg_loop_pre

fix_jal_pos_loop:
    lw     t1 4(s1) # get current line size
    sub    t2 t2 t1 # t2(num) -= t1(i)
    addi   t0 t0 4  # t0 += 4(process)
    beqz   t2 fix_jal_finish
    addi   s1 s1 -8  # next line
    j      fix_jal_pos_loop

fix_jal_neg_loop_pre:
    addi   s1 s1 8 # last line

fix_jal_neg_loop:
    lw     t1 4(s1) # get last line size
    add    t2 t2 t1 # t2 += t1
    addi   t0 t0 -4 # t0 -= 4
    beqz   t2 fix_jal_finish
    addi   s1 s1 8 # last line
    j      fix_jal_neg_loop

fix_jal_finish:
    li     a4 0 # set a4 to 0, used to store formatted imm, t0 is imm now
    # imm format: 31|30:21|20|19:12
    #             20|10: 1|11|19:12
    srli   t1 t0 20   # imm[20]
    andi   t1 t1 1    # remove other bits
    or     a4 a4 t1   # concat
    slli   a4 a4 10   # make space for imm[10:1]
    srli   t1 t0 1    # imm[10:1]
    andi   t1 t1 1023 # & 0b1111111111
    or     a4 a4 t1   # concat
    slli   a4 a4 1    # make space for imm[11]
    srli   t1 t0 11   # imm[11]
    andi   t1 t1 1    # remove other bits
    or     a4 a4 t1   # concat
    slli   a4 a4 8    # make space for imm[19:12]
    srli   t1 t0 12   # imm[19:12]
    andi   t1 t1 255  # & 0b11111111
    or     a4 a4 t1   # concat
    slli   a4 a4 12   # move to bit[12]

    li   t4 1
    slli t4 t4 12
    addi t4 t4 -1  # 0b111111111111(12)
    and  a3 a3 t4  # remove other bits
    or   a3 a3 a4  # combine the two
    sw   a3 0(fp)  # save fixed instruction
    j    fix_loop  # finished

fix_b: # opcode 1100011
       # bit[31:23] - imm[8:0]
    srai   t2 a3 23  # bit[31:23] which is imm
    li     t0 0 # set t0 to 0, used to store fixed imm
    blt    t2 x0 fix_b_neg_loop_pre

fix_b_pos_loop:
    lw     t1 4(s1) # get current line size
    sub    t2 t2 t1 # t2 -= t1
    addi   t0 t0 4  # t0 += 4
    beqz   t2 fix_b_finish
    addi   s1 s1 -8 # next line
    j      fix_b_pos_loop

fix_b_neg_loop_pre:
    addi   s1 s1 8 # last line

fix_b_neg_loop:
    lw     t1 4(s1) # get last line size
    add    t2 t2 t1 # t2 += t1
    addi   t0 t0 -4 # t0 -= 4 
    beqz   t2 fix_b_finish
    addi   s1 s1 8 # last line
    j      fix_b_neg_loop

fix_b_finish:
    li     a4 0 # set a4 to 0, used to store formatted imm, t0 is imm now
    # imm format: 31|30-25   11: 8| 7
    #             12|10- 5    4: 1|11
    srli   t1 t0 12 # imm[12]
    andi   t1 t1 1  # remove other bits
    or     a4 a4 t1 # concat
    slli   a4 a4 6  # make space for imm[10:5]
    srli   t1 t0 5  # imm[10:5]
    andi   t1 t1 63 # & 0b111111
    or     a4 a4 t1 # concat
    slli   a4 a4 17 # make space for imm[4:1]
    srli   t1 t0 1  # imm[4:1]
    andi   t1 t1 15 # & 0b1111
    or     a4 a4 t1 # concat
    slli   a4 a4 1  # make space for imm[11]
    srli   t1 t0 11 # imm[11]
    andi   t1 t1 1  # remove other bits
    or     a4 a4 t1 # concat
    slli   a4 a4 7  # move to correct location

    slli   a3 a3 8  # clear first 8 bits in a3
    srli   a3 a3 8  # reset pos
    or     a3 a3 a4 # combine the two
    sw     a3 0(fp) # save fixed instruction
    j      fix_loop # finished
