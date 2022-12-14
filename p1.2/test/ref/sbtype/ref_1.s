

.data

# Constant integer specifying the lines of RVC codes

# DO NOT MODIFY THIS VARIABLE
.globl lines_of_rvc_codes
lines_of_rvc_codes:
    .word 6


# RVC codes, 16-bits instructions mixed with 32-bits instructions
# A 16/32-bits binary number represents one line of rvc code.
# You can suppose all of the input codes are valid. 

# DO NOT MODIFY THIS VARIABLE
.globl rvc_codes
rvc_codes:
    .word 0b00000001010000110000001010010011
    .half 0b0110001010100001
    .half 0b0000001100101110
    .word 0b00000000101100110101001100010011
    .half 0b1001100001000101
    .half 0b1101100001101101
    0b110 1 10 000 11 01 1 01
    111110010
