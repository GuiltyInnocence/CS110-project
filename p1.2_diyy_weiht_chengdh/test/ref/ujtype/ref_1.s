
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
    .half 0b0110001010100001
    .half 0b1100011011010000
    .half 0b0010000000011001
    .half 0b1000000100000101
    .half 0b1011111111100101
    .half 0b0000000111111001
