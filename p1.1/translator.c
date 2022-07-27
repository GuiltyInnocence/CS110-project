/*  Project 1.1: RISC-V instructions to RISC-V compressed instructions in C89.
    The following is the starter code provided for you. To finish the task, you 
    should define and implement your own functions in translator.c, compression.c, 
    utils.c and their header files.
    Please read the problem description before you start.
*/

#include <stdio.h>
#include <stdlib.h>/* */
#include <string.h>
#include <stdint.h>/* */

#include "src/compression.h"
#include "src/utils.h"

#include "translator.h" /* include header file for translator.c*/

/* */
/*check if file can be correctly opened */
static int open_files(FILE** input, FILE** output, const char* input_name, const char* output_name){ 
    *input = fopen(input_name, "r");
    if (!*input){ /* open input file failed */
        printf("Error: unable to open input file: %s\n", input_name);
        return -1; /* open file failed */
    }
    *output = fopen(output_name, "w");
    if (!*output){ /* open output file failed */
        printf("Error: unable to open output file: %s\n", output_name);
        fclose(*input);
        return -1; /* open file failed */
    }
    return 0; /* no problem opening files */
}

static int close_files(FILE** input, FILE** output){/* */
    fclose(*input);
    fclose(*output); /* close the files at the end */
    return 0;
}/* */

static void print_usage_and_exit() {
    printf("Usage:\n"); /* wrong usage! */
    printf("Run program with translator <input file> <output file>\n"); /* print the correct usage of the program */
    exit(0);
}/* */

/* Run the translator */
int translate(const char*in, const char*out){
    FILE *input, *output;/* */
    int err = 0, i=0;
    InstructionBits iin;
    int offsetBit=0; /* current offset bit being compressed since the first instruction */
    int count=0; /* # of total instructions */
    /*int pre[MAX_INST];*/ /* offset perfix */
    int* pre=(int*)malloc(MAX_INST*sizeof(int));
    /*InstructionBits insts[MAX_INST];*/ /* origin instructions are here */
    InstructionBits* insts=(InstructionBits*)malloc(MAX_INST*sizeof(InstructionBits));
    /*CompressedInstruction Cresult[MAX_INST]; */ /* compressed instructions are here */
    CompressedInstruction* Cresult=(CompressedInstruction*)malloc(MAX_INST*sizeof(CompressedInstruction));
    /*int isCompressed[MAX_INST];*/ /* whether the instruction is compressed or not */
    int* isCompressed=(int*)malloc(MAX_INST*sizeof(int));
    /*InstructionType itype[MAX_INST];*/ /* instruction type */
    InstructionType* itype=(InstructionType*)malloc(MAX_INST*sizeof(InstructionType));
    /*CompressedInstType citype[MAX_INST];*/ /* compressed instruction type */
    CompressedInstType* citype=(CompressedInstType*)malloc(MAX_INST*sizeof(CompressedInstType));
    memset(pre,0,sizeof(int)*MAX_INST);
    memset(insts,0,sizeof(InstructionBits)*MAX_INST);
    memset(Cresult,0,sizeof(CompressedInstruction)*MAX_INST);
    memset(isCompressed,0,sizeof(int)*MAX_INST); /* initialize isCompressed array */
    for(i=0;i<MAX_INST;i++){
        itype[i]=None;
        citype[i]=CNone;
    }
    if (in){    /* correct input file name */
        if(open_files(&input, &output, in, out) != 0){
            free(pre);/* */
            free(insts);
            free(Cresult);/* */
            free(isCompressed);
            free(itype);
            free(citype);/* */
            exit(1);
        }/* */
        iin=getinst(&input);
        while(iin){ /* parse the instruction as strings */
            insts[count++]=iin; /* to uint32 */
            iin=getinst(&input);
        }
        /* 1st parse, check if can be compressed */
        for(i=0; i<count; i++){
            int compressed=Compress(insts[i],i,itype,citype); /* call compress */
            pre[i]=offsetBit; /* save the prefix offset bit, before current */
            if(compressed){
                isCompressed[i]=1; /* set to compressed */
                Cresult[i]=compressed; /* store compressed instruction */
                offsetBit += 2;
                #ifdef DBG_FLAG /* debugging tools */
                    compressedInstOutput(Cresult[i]); /* debugging output */
                #endif
            }
            else{
                #ifdef DBG_FLAG /* debugging tools */
                    InstOutput(insts[i]); /* debugging output */
                #endif
                isCompressed[i]=0; /* not compressed */
            }
        }
        pre[count]=offsetBit;
        /* 2nd parse, fix offset problem */
        for(i=0; i<count; i++){
            parse2ndOffset(i,count,pre,insts,Cresult,isCompressed,itype,citype); 
            /* do 2nd parse to update imm for jump and branch */
        }
        /* write correct result to the output file */
        for(i=0; i<count; i++){
            if(isCompressed[i]){ /* if compressed */
            #ifdef DBG_FLAG
                compressedInstOutput(Cresult[i]); /* write compressed instruction */
            #else
                compressedInstOutput(&output,Cresult[i]); /* write compressed instruction */
            #endif
            }
            else{ /* if not compressed */
                #ifdef DBG_FLAG
                    InstOutput(insts[i]); /* write origin instruction */
                #else
                    InstOutput(&output,insts[i]); /* write origin instruction */
                #endif
            }
        }
        close_files(&input, &output); /* close files */
    }
    free(pre);/* */
            free(insts);
            free(Cresult);/* */
            free(isCompressed);
            free(itype);/* */
            free(citype);
    return err; /* return whether error */
}

/* main func */
int main(int argc, char **argv){ /* accept input from command line */
    char* input_fname, *output_fname;
    int err;
    if (argc != 3) /* need correct arguments */
        print_usage_and_exit();
    input_fname = argv[1];
    output_fname = argv[2];
    err = translate(input_fname, output_fname); /* main translation process */
    if (err)
        printf("One or more errors encountered during translation operation.\n"); /* something wrong */
    else
        printf("Translation process completed successfully.\n"); /* correctly output */

    return 0;
}/* */
