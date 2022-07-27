#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


InstructionBits getinst(FILE** input){/* function */
    InstructionBits res=0;
    int ct=0;/* */
    int ch=fgetc(*input);
    while(ct<=30 && ch!=EOF){/* check */
        if(ch=='0' || ch=='1'){
            res|=ch-'0';/* reset */
            res<<=1;
            ct++;/* add */
        }
        ch=fgetc(*input);/* value */
    }
    if(ct<31)return 0;/* check */
    if(ch=='0' || ch=='1')res|=ch-'0';
    return res;/* */
}
/* */

void compressedInstOutput(FILE** output, CompressedInstruction cinst) { /* check */
    CompressedInstruction mask = 0x8000;
    while (mask) {
        (cinst & mask) ? fprintf(*output, "1") : fprintf(*output, "0"); /* set */
        if(mask&1)break;
        mask >>= 1;
    } /* end */
    fprintf(*output, "\n");
} /* */

void InstOutput(FILE** output, InstructionBits cinst) { /* function */
    InstructionBits mask = 0x80000000;
    while (mask) { /* check */
        (cinst & mask) ? fprintf(*output, "1") : fprintf(*output, "0");
        if(mask&1)break;
        mask >>= 1; /* reset */
    }
    fprintf(*output, "\n");
}
