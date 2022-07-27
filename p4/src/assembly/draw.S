.section .text
.globl draw
.type draw, @function
draw:

	addi sp, sp, -24
	sw a0, 0(sp)
	sw a1, 4(sp)
	sw a2, 8(sp)
	sw a3, 12(sp)
	sw a4, 16(sp)
    sw ra, 20(sp)

	li a0, 50 # x
	li a1, 15 # y
	li a2, 70 # F
	li a3, 1 # mode
	li a4, 0x1111
    jal LCD_ShowChar

	li a0, 60 # x
	li a1, 15 # y
	li a2, 108 # l
    li a3, 1 # mode
    li a4, 0x2222
    jal LCD_ShowChar

    li a0, 70 # x
	li a1, 15 # y
	li a2, 121 # y
	li a3, 1 # mode
	li a4, 0x3333
    jal LCD_ShowChar

    li a0, 80 # x
	li a1, 15 # y
	li a2, 68 # D
	li a3, 1 # mode
	li a4, 0x4444
    jal LCD_ShowChar

    li a0, 90 # x
	li a1, 15 # y
	li a2, 105 # i
	li a3, 1 # mode
	li a4, 0x5555
    jal LCD_ShowChar

    li a0, 100 # x
	li a1, 15 # y
	li a2, 110 # n
	li a3, 1 # mode
	li a4, 0x6666
    jal LCD_ShowChar

    li a0, 110 # x
	li a1, 15 # y
	li a2, 111 # o
	li a3, 1 # mode
	li a4, 0x7777
    jal LCD_ShowChar

	li a0, 100
	jal delay_1ms


	li a0, 50 # x
	li a1, 15 # y
	li a2, 70 # F
	li a3, 1 # mode
	li a4, 0x3333
    jal LCD_ShowChar

	li a0, 60 # x
	li a1, 15 # y
	li a2, 108 # l
    li a3, 1 # mode
    li a4, 0x4444
    jal LCD_ShowChar

    li a0, 70 # x
	li a1, 15 # y
	li a2, 121 # y
	li a3, 1 # mode
	li a4, 0x5555
    jal LCD_ShowChar

    li a0, 80 # x
	li a1, 15 # y
	li a2, 68 # D
	li a3, 1 # mode
	li a4, 0x6666
    jal LCD_ShowChar

    li a0, 90 # x
	li a1, 15 # y
	li a2, 105 # i
	li a3, 1 # mode
	li a4, 0x7777
    jal LCD_ShowChar

    li a0, 100 # x
	li a1, 15 # y
	li a2, 110 # n
	li a3, 1 # mode
	li a4, 0x8888
    jal LCD_ShowChar

    li a0, 110 # x
	li a1, 15 # y
	li a2, 111 # o
	li a3, 1 # mode
	li a4, 0x9999
    jal LCD_ShowChar

	li a0, 100
	jal delay_1ms


	li a0, 50 # x
	li a1, 15 # y
	li a2, 70 # F
	li a3, 1 # mode
	li a4, 0x5555
    jal LCD_ShowChar

	li a0, 60 # x
	li a1, 15 # y
	li a2, 108 # l
    li a3, 1 # mode
    li a4, 0x6666
    jal LCD_ShowChar

    li a0, 70 # x
	li a1, 15 # y
	li a2, 121 # y
	li a3, 1 # mode
	li a4, 0x7777
    jal LCD_ShowChar

    li a0, 80 # x
	li a1, 15 # y
	li a2, 68 # D
	li a3, 1 # mode
	li a4, 0x8888
    jal LCD_ShowChar

    li a0, 90 # x
	li a1, 15 # y
	li a2, 105 # i
	li a3, 1 # mode
	li a4, 0x9999
    jal LCD_ShowChar

    li a0, 100 # x
	li a1, 15 # y
	li a2, 110 # n
	li a3, 1 # mode
	li a4, 0xAAAA
    jal LCD_ShowChar

    li a0, 110 # x
	li a1, 15 # y
	li a2, 111 # o
	li a3, 1 # mode
	li a4, 0xBBBB
    jal LCD_ShowChar

	li a0, 100
	jal delay_1ms


	li a0, 50 # x
	li a1, 15 # y
	li a2, 70 # F
	li a3, 1 # mode
	li a4, 0x7777
    jal LCD_ShowChar

	li a0, 60 # x
	li a1, 15 # y
	li a2, 108 # l
    li a3, 1 # mode
    li a4, 0x8888
    jal LCD_ShowChar

    li a0, 70 # x
	li a1, 15 # y
	li a2, 121 # y
	li a3, 1 # mode
	li a4, 0x9999
    jal LCD_ShowChar

    li a0, 80 # x
	li a1, 15 # y
	li a2, 68 # D
	li a3, 1 # mode
	li a4, 0xAAAA
    jal LCD_ShowChar

    li a0, 90 # x
	li a1, 15 # y
	li a2, 105 # i
	li a3, 1 # mode
	li a4, 0xBBBB
    jal LCD_ShowChar

    li a0, 100 # x
	li a1, 15 # y
	li a2, 110 # n
	li a3, 1 # mode
	li a4, 0xCCCC
    jal LCD_ShowChar

    li a0, 110 # x
	li a1, 15 # y
	li a2, 111 # o
	li a3, 1 # mode
	li a4, 0xDDDD
    jal LCD_ShowChar

	li a0, 100
	jal delay_1ms


	li a0, 50 # x
	li a1, 15 # y
	li a2, 70 # F
	li a3, 1 # mode
	li a4, 0x7777
    jal LCD_ShowChar

	li a0, 60 # x
	li a1, 15 # y
	li a2, 108 # l
    li a3, 1 # mode
    li a4, 0x8888
    jal LCD_ShowChar

    li a0, 70 # x
	li a1, 15 # y
	li a2, 121 # y
	li a3, 1 # mode
	li a4, 0x9999
    jal LCD_ShowChar

    li a0, 80 # x
	li a1, 15 # y
	li a2, 68 # D
	li a3, 1 # mode
	li a4, 0xAAAA
    jal LCD_ShowChar

    li a0, 90 # x
	li a1, 15 # y
	li a2, 105 # i
	li a3, 1 # mode
	li a4, 0xBBBB
    jal LCD_ShowChar

    li a0, 100 # x
	li a1, 15 # y
	li a2, 110 # n
	li a3, 1 # mode
	li a4, 0xCCCC
    jal LCD_ShowChar

    li a0, 110 # x
	li a1, 15 # y
	li a2, 111 # o
	li a3, 1 # mode
	li a4, 0xDDDD
    jal LCD_ShowChar

	li a0, 100
	jal delay_1ms

	li a0, 50 # x
	li a1, 15 # y
	li a2, 70 # F
	li a3, 1 # mode
	li a4, 0x1111
    jal LCD_ShowChar

	li a0, 60 # x
	li a1, 15 # y
	li a2, 108 # l
    li a3, 1 # mode
    li a4, 0x2222
    jal LCD_ShowChar

    li a0, 70 # x
	li a1, 15 # y
	li a2, 121 # y
	li a3, 1 # mode
	li a4, 0x3333
    jal LCD_ShowChar

    li a0, 80 # x
	li a1, 15 # y
	li a2, 68 # D
	li a3, 1 # mode
	li a4, 0x4444
    jal LCD_ShowChar

    li a0, 90 # x
	li a1, 15 # y
	li a2, 105 # i
	li a3, 1 # mode
	li a4, 0x5555
    jal LCD_ShowChar

    li a0, 100 # x
	li a1, 15 # y
	li a2, 110 # n
	li a3, 1 # mode
	li a4, 0x6666
    jal LCD_ShowChar

    li a0, 110 # x
	li a1, 15 # y
	li a2, 111 # o
	li a3, 1 # mode
	li a4, 0x7777
    jal LCD_ShowChar


	lw a0, 0(sp)
	lw a1, 4(sp)
	lw a2, 8(sp)
	lw a3, 12(sp)
	lw a4, 16(sp)
    lw ra, 20(sp)
	addi sp, sp, 24
    jr ra
