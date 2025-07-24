/*documentation
name:p praveen kumar
date:14-10-2024
project name:Lsb image steganography
description:In this project encoding part concealing information within another message or physical object to avoid detection
  */
#include <stdio.h>
#include "encode.h"
#include "types.h"
#include <string.h>
#include "decode.h"
int main(int argc,char *argv[])
{
        int op_type;
        if(argc<2)
        {
                printf("Please pass valid arguments\n");
                return 0;
        }
        else{
                //step1:check op_type equal to e_encode or not
                 op_type=check_operation_type(argv[1]);
        }
    if(op_type==e_encode)
    {
            EncodeInfo encInfo;
            if(read_and_validate_encode_args(argv,&encInfo)==e_success)
            {
                    printf("Info:Read and validation is Successfully executed\n");
                    do_encoding(&encInfo);

            }
            else
            {
                    printf("Info:Read and validations is failed\n");
                    return e_failure;
            }
    }
    else if(op_type==e_decode)
    {
        DecodeInfo decInfo;
        if(read_and_validate_decode_args(argv,&decInfo)==e_success)
        {
                printf("Info:Read and validations is successfully executed\n");
                do_decoding(&decInfo);
        }
        else{
                printf("Info:Read and validations is failed\n");
                return e_failure;
        }

    }
    else if(op_type==e_failure)
    {
            printf("Error:");

    }
    //step2:yes->start en
}
OperationType check_operation_type(char *arg)
{
         //step1:check arg is -e or not
        if(strcmp(arg,"-e")==0)
        {
                printf("Selected encoding,Encoding started\n");
                return e_encode;
        }
        //step4:yes->return e_Success,no->return e_failure
        else if(strcmp(arg,"-d")==0)
        {
                printf("Selected decoding,Decoding started\n");
                return e_decode;
                
                
        }
        else
        {
                return e_failure;
        }
}                              
