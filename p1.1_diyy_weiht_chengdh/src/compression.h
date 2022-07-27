#ifndef COMPRESSION_H
#define COMPRESSION_H
/* Your code here... */

#include "utils.h"
/* */

/* */
CompressedInstruction Compress(InstructionBits inst, int index,InstructionType* itype,CompressedInstType* citype);

void parse2ndOffset(int index,int count,int* pre,InstructionBits* insts,CompressedInstruction* Cresult,int* isCompressed,InstructionType* itype,CompressedInstType* citype);/* */

#endif


