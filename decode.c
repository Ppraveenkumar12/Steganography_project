#include <stdio.h>
#include <string.h>
#include "decode.h"
#include "types.h"
#include "common.h"


Status open_source_files(DecodeInfo *decInfo)
{
    // Open Src Image file in write mode
    decInfo->fptr_src_image = fopen(decInfo->src_image_fname, "w");
    // Do Error handling
    if (decInfo->fptr_src_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", decInfo->src_image_fname);

        return e_failure;
    }

    // Open Stego Image file in read mode
    decInfo->fptr_stego_image = fopen(decInfo->stego_image_fname, "r");
    // Do Error handling
    if (decInfo->fptr_stego_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", decInfo->stego_image_fname);

        return e_failure;
    }

    // No failure return e_success
    return e_success;
}
Status read_and_validate_decode_args(char *argv[],DecodeInfo *decInfo)
{
    if(argv[2]!=NULL)//check argv[2] is present or not
    {
	    printf("INFO: Source File Saved\n\n");
	    decInfo->stego_image_fname = argv[2];//store argv[2] into structer
    }
    else
    {
	    return e_failure;
    }
    if(argv[3]!=NULL)//check argv[3] is present or not
    {
	    printf("INFO: Decode file saved\n\n");
	    strcpy(decInfo->string, argv[3]);//store argv[3] into structer
    }
    else
    {
	    printf("INFO: Decode file saved\n\n");
	    strcpy(decInfo->string, "decode.txt");//if argv[3] is not present , manualy store decode file
    }
    return e_success;
}
Status decode_byte_to_lsb1(char *arr)
{       
	int res1=0;
	for(int i=8;i>=0;i--)
	{
		res1= res1 * 10 + (arr[i] & (1<<0));//get a bit from the data at lsb end and store into variable,it is in binary form
	}
	int res=res1;
	int res2=0,num=0,r=1;
	//convert binary data into decimal
          while(res)
          {
                res2 = res % 10;
                num = num + (res2*r);
                r = r*2;
                res = res/10;
          }
	  return num;//return decimal value to function
}
Status decode_byte_to_lsb2(char *arr)
{
        int res1=0;
        for(int i=32;i>=0;i--)//run a loop 32 times
        {
                res1= res1 * 10 + (arr[i] & (1<<0));///get a bit from the data at lsb end and store into variable,it is in binary form
        }
        int res=res1;
        int res2=0,num=0,r=1;
	//convert binary data into decimal
          while(res)
          {
                res2 = res % 10;
                num = num + (res2*r);
                r = r*2;
                res = res/10;
          }
          return num;//return decimal value to function
}
Status decode_magic_string(char *magic_string,FILE *fptr_stego_image)
{
	int size = strlen(magic_string);//find size of the magic string
	char arr[8];//declare one character array
	int count=0;
	fseek(fptr_stego_image,54,SEEK_CUR);//set fseek at 54 position
	for(int i=0;i<size;i++)//run loop size time
	{
	  fread(arr,8,1,fptr_stego_image);//read 8 bytes of data into fptr_stego_image file
	  int res = decode_byte_to_lsb1(arr);//call decode_byte_to_lsb1 function for decode the data
          char str = (char)res;//type cast decimal into character
	  if(str == magic_string[i])
	  {
		  count++;
	  }
	  else
	  {
		  return e_failure;
	  }
	}
	if(count==size)//compare magic string size to decoded magic string size
	{
		return e_success;
	}
}
Status  decode_extn_size(DecodeInfo *decInfo)
{
	char arr[32];//declare one character array
        fread(arr,32,1,decInfo->fptr_stego_image);//read 32 bytes of data from the fptr_stego_image file
	int res = decode_byte_to_lsb2(arr);//call decode_byte_to_lsb2 function for decode the data
	printf("INFO: Decoding Output File Extenstion Size\n\n");
	char str[res/8];
	int i;
	for(i=0;i<(res/8);i++)
	{
		char arr2[8];
		fread(arr2,8,1,decInfo->fptr_stego_image);//read 8 bytes of data form the fptr_stego_image file
		int res2 = decode_byte_to_lsb1(arr2);//call decode_byte_to_lsb1 function for decode data
		str[i]=(char)res2;//type cast decimal data into character
	}
	str[i]='\0';
	strcat(decInfo->string,str);//concatenate secret file name and extension
	decInfo->fptr_src_image = fopen(decInfo->string,"w");//open string file and store concatenated file name into fptr_src_image file
	if (decInfo->fptr_src_image == NULL)//check fptr_src_image file open or not
        {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", decInfo->src_image_fname);
        return e_failure;
        }
	return e_success;
}
Status decode_secret_file_size(DecodeInfo *decInfo)
{
	char arr[32];//declare one character array
        fread(arr,32,1,decInfo->fptr_stego_image);//read 32 bytes of data from the fptr_stego_image file
        int res = decode_byte_to_lsb2(arr);//call decode_byte_to_lsb2 function for decode the data
	printf("INFO: Decoding  Secret File Size\n\n");
        char str[res];
	int i;
        for(i=0;i<res;i++)
        {
                char arr2[8];
                fread(arr2,8,1,decInfo->fptr_stego_image);//read 8 bytes of data form the fptr_stego_image file
                int res2 = decode_byte_to_lsb1(arr2);//call decode_byte_to_lsb1 function for decode data
                str[i]=(char)res2;;//type cast decimal data into character
	}
	str[i]='\0';
	fwrite(str,1,res,decInfo->fptr_src_image);//write size time,decoded data into fptr_src_image file 
	return e_success;
}
Status do_decoding(DecodeInfo *decInfo)
{
	if(open_source_files(decInfo) == e_success)//call open_source_files function for open files
	{
	  printf("INFO: Source File is Open\n\n");
          if(decode_magic_string(MAGIC_STRING,decInfo->fptr_stego_image) == e_success)//call decode_magic_string function for decode the magic string form the fptr_stego_image file
	  {
		  printf("INFO: Decoding Magic String Signature\n\n");
                  if(decode_extn_size(decInfo) == e_success)//call decode_extn_size function for decode extension size from the fptr_stego_image file
		  {
		       printf("INFO: Decoding Output File Extenstion\n\n");
		       if(decode_secret_file_size(decInfo) == e_success)//call decode_secret_file_size function for decode secret file size
		       {
			  printf("INFO: Decoding Secret File Data\n\n");
			  printf("INFO: ## Decoding Done Successfully\n");
                       }
		       else
		       {
			   printf("Error: secret data not copied\n");
		       }
	          }
		  else
		  {
			  printf("Error: Not decoded extention file\n");
		  }
	  }
	  else
	  {
		  printf("Error: magic string not decoded\n");
	  }

	}
	else
	{
		printf("Error: source files not open\n");
	}
}
