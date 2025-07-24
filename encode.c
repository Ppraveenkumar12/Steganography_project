#include <stdio.h>
#include "encode.h"
#include "types.h"
#include<string.h>
#include"common.h"

/* Function Definitions */

/* Get image size
 * Input: Image file ptr
 * Output: width * height * bytes per pixel (3 in our case)
 * Description: In BMP Image, width is stored in offset 18,
 * and height after that. size is 4 bytes
 */
uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;
    // Seek to 18th byte
    fseek(fptr_image, 18, SEEK_SET);

    // Read the width (an int)
    fread(&width, sizeof(int), 1, fptr_image);
    printf("width = %u\n", width);

    // Read the height (an int)
    fread(&height, sizeof(int), 1, fptr_image);
    printf("height = %u\n", height);

    // Return image capacity
    return width * height * 3;
}

/* 
 * Get File pointers for i/p and o/p files
 * Inputs: Src Image file, Secret file and
 * Stego Image file
 * Output: FILE pointer for above files
 * Return Value: e_success or e_failure, on file errors
 */
Status open_files(EncodeInfo *encInfo)
{
    // Src Image file
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "r");
    // Do Error handling
    if (encInfo->fptr_src_image == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->src_image_fname);
    	return e_failure;
    }
    // Secret file
    encInfo->fptr_secret = fopen(encInfo->secret_fname, "r");
    // Do Error handling
    if (encInfo->fptr_secret == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->secret_fname);
    	return e_failure;
    }
    // Stego Image file
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "w");
    // Do Error handling
    if (encInfo->fptr_stego_image == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->stego_image_fname);

    	return e_failure;
    }
    // No failure return e_success
    return e_success;
}
Status read_and_validate_encode_args(char *argv[],EncodeInfo*encInfo)
{
    // Step 1: Check if argv[2] is a .bmp file
    if (strstr(argv[2], ".bmp") != NULL)
    {
        // Step 2: If yes, store argv[2] in the structure member
        encInfo->src_image_fname = argv[2];
        printf("Source image stored successfully.\n");
    }
    else
    {
        printf("Error: It's not a .bmp file.\n");
        return e_failure;
    }

    // Step 3: Check if argv[3] is a .txt file
    if (strstr(argv[3], ".txt") != NULL)
    {
        // Step 4: If yes, store argv[3] in the structure member
        encInfo->secret_fname = argv[3];
        printf("Secret file stored successfully.\n");
    }
    else
    {
        printf("Error: It's not a .txt file.\n");
        return e_failure;
    }

    // Step 5: Check if argv[4] is passed
    if (argv[4] == NULL || strlen(argv[4]) == 0) // No argv[4] or empty string
    {
        // Step 6: If not passed, store the default name into the structure member
          encInfo->stego_image_fname = "stego.bmp";
        printf("Default stego image name used: %s\n", "stego.bmp");
    }
    else
    {
        // Step 7: Check if argv[4] is a .bmp file
        if (strstr(argv[4], ".bmp") != NULL)
        {
            // Step 8: If yes, store argv[4] in the structure member
            encInfo->stego_image_fname = argv[4];
            printf("Stego image file stored successfully.\n");
        }
        else
        {
            printf("Error: It's not a .bmp file.\n");
            return e_failure;
        }
    }
    return e_success;
}
Status check_capacity(EncodeInfo *encInfo)
{
    // Step 1: Find the source image size
    encInfo->image_capacity = get_image_size_for_bmp(encInfo->fptr_src_image);
    printf("Image capacity (in bytes): %u\n", encInfo->image_capacity);

    // Step 2: Find the secret file size
    fseek(encInfo->fptr_secret, 0L, SEEK_END);  // Seek to end of file
    encInfo->size_secret_file = ftell(encInfo->fptr_secret);  // Get file size
    fseek(encInfo->fptr_secret, 0L, SEEK_SET);  // Reset to the beginning of the file
    printf("Secret file size (in bytes): %ld\n", encInfo->size_secret_file);

    // Step 3: Find the extension size (strlen of the file extension)
    int extn_size = strlen(encInfo->extn_secret_file);
    printf("Secret file extension size (in bytes): %d\n", extn_size);

    // Step 4: Check if the image has enough capacity to hold the secret data
    // Capacity should be enough for magic string, extension size, secret file size, and data
    //(strlen(MAGIC_STRING) + extn_size + sizeof(encInfo->size_secret_file) + encInfo->size_secret_file) * 8
    //16 + 32 + (extn_size * 8) + 32 + (encInfo->size_secret_file * 8) + 54 + 1;
    //uint required_capacity = (strlen(MAGIC_STRING) + extn_size + sizeof(encInfo->size_secret_file) + encInfo->size_secret_file) * 8;
    
    uint required_capacity = (strlen(MAGIC_STRING) + extn_size + sizeof(encInfo->size_secret_file) + encInfo->size_secret_file) * 8;  // 1 byte = 8 bits

    if (encInfo->image_capacity >= required_capacity)
    {
        printf("Sufficient image capacity.\n");
        return e_success;
    }
    else
    {
        printf("Error: Insufficient image capacity.\n");
        return e_failure;
    }
}

Status encode_byte_to_lsb(char data, char *image_buffer) {
    char cleared_bit, bit_to_encode;

    for (int i = 0; i < 8; i++) { // changed to < 8 for clarity
        // Step 1: Clear the least significant bit of the current byte in the image buffer
        cleared_bit = image_buffer[i] & (~1);

        // Step 2: Get the bit from the data byte
        bit_to_encode = (data >> i) & 1;

        // Step 3: Combine the cleared bit with the bit to encode
        image_buffer[i] = cleared_bit | bit_to_encode;
    }
    return e_success;
}

Status encode_data_to_image(char *data, int size, FILE *fptr_src_image, FILE *fptr_stego_image) {
    char image_buffer[8];
    
    for (int i = 0; i < size; i++) {
        // Read 8 bytes from source image
        if (fread(image_buffer, sizeof(char), 8, fptr_src_image) != 8) {
            fprintf(stderr, "Error reading from source image file.\n");
            return e_failure;
        }

        // Encode the byte from data into the LSB of the image
        if (encode_byte_to_lsb(data[i], image_buffer) != e_success) {
            fprintf(stderr, "Error encoding byte to LSB.\n");
            return e_failure;
        }

        // Write the modified 8 bytes to the stego image
        if (fwrite(image_buffer, sizeof(char), 8, fptr_stego_image) != 8) {
            fprintf(stderr, "Error writing to stego image file.\n");
            return e_failure;
        }
    }
    return e_success;
}
    

Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo) {
    // Call encode_data_to_image to encode the magic string
    if (encode_data_to_image((char *)magic_string, strlen(magic_string), encInfo->fptr_src_image, encInfo->fptr_stego_image) != e_success) {
        return e_failure;
    }
    return e_success;
} 
// Status encode_magic_string(char *magic_string, FILE *file_src_image, FILE *fptr_stego_image){
    
//     //STEP 1: call encode_data_to_image(magic_string, file_src_image, fptr_stego_image);
//     encode_data_to_image(magic_string, file_src_image, fptr_stego_image);
//     printf("Magic string encoded successfully.\n");
//     return e_success;
// }extra

Status encode_size_to_lsb(char *arr,int size){
        char clear,get;
    for(int i = 0; i < 32; i++){
        //STEP 1: clear a bit in the arr[0]
        clear = arr[i]&(~1);
        //STEP 2: get a bit from ch
        get = (size>>i)&1;
        //STEP 3: Replace the got bit into arr[0] th lsb position
        arr[i] = clear|get;
    }//STEP 4: Repet the Operation for 8 times    
    return e_success;
}

// Status encode_secret_file_ext_size(EncodeInfo *encInfo){
//     char arr[32];
//     //STEP 1: Read 32 bytes from source file and store into an array
//     //STEP 2: call encode_size_to_lsb(arr,extn_size);   
// }

Status encode_secret_file_ext_size(long int size_secret_file, FILE *fptr_src_image, FILE *fptr_stego_image){
    char arr[32]; // Array to store 32 bytes from the source image
    
    // STEP 1: Move the file pointer to the desired position
    // Skip 54 bytes (header) + 16 bytes (magic string) = 70 bytes in total
    // if (fseek(fptr_src_image, 70, SEEK_SET) != 0) {
    //     printf("Error: Unable to seek to the correct position in the source image file.\n");
    //     return e_failure;
    // }

    // STEP 2: Read 32 bytes from the source image
    if (fread(arr, 1, 32, fptr_src_image) != 32) {
        printf("Error: Unable to read 32 bytes from source image.\n");
        return e_failure;
    }

    // STEP 3: Encode the secret file extension size into the LSBs of the 32 bytes
    if (encode_size_to_lsb(arr, size_secret_file) != e_success) {
        printf("Error: Unable to encode size into LSBs.\n");
        return e_failure;
    }

    // Write the modified 32 bytes to the stego image (if needed)
    if (fwrite(arr, 1, 32, fptr_stego_image) != 32) {
        printf("Error: Unable to write encoded bytes to the stego image.\n");
        return e_failure;
    }
    return e_success;
}


Status encode_secret_file_extn(const char *file_ext, EncodeInfo *encInfo) {
     //STEP 1: call encode_data_to_image(encInfo
    if (encode_data_to_image((char *)file_ext, strlen(file_ext), encInfo->fptr_src_image, encInfo->fptr_stego_image) != e_success) {
        return e_failure;
    }
    return e_success;
}
Status encode_secret_file_data(EncodeInfo *encInfo) {
    char ch;
    
    // Loop through each byte of the secret file
    while (fread(&ch, 1, 1, encInfo->fptr_secret)) {
        // Encode each byte of the secret file to the image
        if (encode_data_to_image(&ch, 1, encInfo->fptr_src_image, encInfo->fptr_stego_image) != e_success) {
            return e_failure;
        }
    }
    return e_success;
}

// Function to copy the remaining image data after encoding
Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)
{
    int ch;

    // Read and write until the end of the file
    while ((ch = getc(fptr_src)) != EOF)
    {
        putc(ch, fptr_dest);  // Write each character to destination
    }

    // Check if there was an error in reading the file
    if (ferror(fptr_src))
    {
        fprintf(stderr, "Error: unable to read from source image file.\n");
        // clearerr(fptr_src);  // Clear error flag
        return e_failure;
    }

    // Check if there was an error in writing to the file
    if (ferror(fptr_dest))
    {
        fprintf(stderr, "Error: unable to write to destination image file.\n");
        // clearerr(fptr_dest);  // Clear error flag
        return e_failure;
    }

    return e_success;
}
Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_stego_image)
{
    // BMP header is typically 54 bytes
    char header[54];

    // Read the header from the source image
    rewind(fptr_src_image);  // Make sure to start from the beginning
    if (fread(header, 1, 54, fptr_src_image) != 54)
    {
        fprintf(stderr, "Error: Unable to read BMP header from source image.\n");
        return e_failure;
    }

    // Write the header to the stego image
    rewind(fptr_stego_image);  // Make sure to start from the beginning
    if (fwrite(header, 1, 54, fptr_stego_image) != 54)
    {
        fprintf(stderr, "Error: Unable to write BMP header to stego image.\n");
        return e_failure;
    }
    return e_success;
}
//STEP 1: call open files function(&encInfo)
Status do_encoding(EncodeInfo *encInfo){
    
    //STEP 2: check return e_success or e_failure
    if(open_files(encInfo)==e_success)
    {
        printf("Files opened successfully.\n");
    }
    else
    {
     //STEP 3: e_success -> Goto STEP4, e_failure -> Print error msg,and return e_failure   
        printf("Error: Enable to open file.\n");
        return e_failure;
    }
    //Step 4: call check_capacity(structure address is encInfo) 
    //step 5: check return e_success or e_failure
    if(check_capacity(encInfo)==e_success)
    {
        
        printf("Image has sufficient capacity for encoding.\n");
    }
    else
    {
        //step 6: e_success -> print msg,goto step7,e_failure -> print error msg,and return e_failure
        printf("Image has insufficient capacity.\n");
        return e_failure;
    }
    
       // STEP 7: Copy BMP header from source image to stego image
    if (copy_bmp_header(encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_success)
    {
        printf("BMP header copied successfully.\n");
    }
    else
    {
        printf("Error: Unable to copy BMP header.\n");
        return e_failure;
    }
    //STEP 10: call encode_magic_string(MAGIC_STRING,enc_info->fptr_src_image,encinfo->fptr_stego_image)
    //STEP 11: check return e_success or e_failure)
    if(encode_magic_string(MAGIC_STRING, encInfo) == e_success)
    { 
            //STEP 12: e_success -> print msg, GOTO STEP 13,e_failure->Printf error msg,and return e_failure
            printf("Magic string encoded successfully.\n");
        }
        else
        {
            printf("Error: Magic string encoding failed.\n");
            return e_failure;
        }

    //STEP 13: call encode_secret_file_ext_size(extn_size,encInfo->fptr_src_image, encInfo -> fptr_stego_imag)
    //STEP 14: check return e_success or e_failure
        if(encode_secret_file_ext_size(encInfo->size_secret_file,encInfo->fptr_src_image,encInfo->fptr_stego_image)==e_success)
        {
            //STEP 15: e_success -> print msg, GOTO STEP16, e_failure -> print error msg,and return e_failure
            printf("Encode Secret file extenction size successfully.\n");
        }else{
            printf("Error: Encode Secret file extenction failed.\n");
            return e_failure;
        }

        //STEP 16: call encode_secret_file_extn(extn,/*File pointer/)
        //STEP 17: check return e_success or e_failure
        if(encode_secret_file_extn(encInfo->extn_secret_file, encInfo) == e_success)
        {
            //STEP 18: e_success -> print msg, GOTO STEP19, e_failure -> print error msg,and return e_failure
            printf("Secret file extension including dot encoded successfully.\n");

        }else{
            printf("Error: Encoding of secret file failed.\n");
            return e_failure;
        }

        // STEP 19: call encode_secret_file_data(/*File pointers*/)
        if (encode_secret_file_data(encInfo) == e_success) {
            // STEP 20: e_success -> print msg, GOTO STEP21
            printf("Secret file data encoded successfully.\n");
        } else {
            // e_failure -> print error msg, and return e_failure
            printf("Error: Encoding of secret file data failed.\n");
            return e_failure;
        }

            if (copy_remaining_img_data(encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_success) {
            printf("Remaining image data copied successfully\n");
        } else {
            printf("Error copying remaining image data\n");
            return e_failure;
        }

return e_success;
}

