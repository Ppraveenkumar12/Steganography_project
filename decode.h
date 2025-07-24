#ifndef DECODE_H
#define DECODE_H

#include "types.h" // Contains user defined types

/*
 * Structure to store information required for
 * encoding secret file to source Image
 * Info about output and intermediate data is
 * also stored
 */

#define MAX_SECRET_BUF_SIZE 1
#define MAX_IMAGE_BUF_SIZE (MAX_SECRET_BUF_SIZE * 8)
#define MAX_FILE_SUFFIX 4

typedef struct _DecodeInfo
{
    /* Source Image info */
    char *src_image_fname; //source image file name
    FILE *fptr_src_image; //file pointer for source image
    uint image_capacity;  //size of the source image file
    //uint bits_per_pixel;
    //char image_data[MAX_IMAGE_BUF_SIZE];

    /* Secret File Info */
    char string[20];
    char *secret_fname;  //secret file name(source)
    FILE *fptr_secret;   //file pointer for secret file
    char extn_secret_file[MAX_FILE_SUFFIX];  //secret file extension
    //char secret_data[MAX_SECRET_BUF_SIZE];
    long size_secret_file;  //size of the secret file

    /* Stego Image Info */
    char *stego_image_fname;  //destination file name
    FILE *fptr_stego_image;  //file pointer for destination image

} DecodeInfo;


/* Encoding function prototype */

/* Read and validate Encode args from argv */
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo);

/* Perform the encoding */
Status do_decoding(DecodeInfo *decInfo);

/* Get File pointers for i/p and o/p files */
Status open_source_files(DecodeInfo *decInfo);

/* Store Magic String */
Status decode_magic_string(char *magic_string, FILE *fptr_stego_image);

/* Encode secret file extenstion */
Status decode_secret_file_extn( char *file_extn, DecodeInfo *decInfo);

/* Encode secret file size */
Status decode_extn_size(DecodeInfo *decInfo);

/* Encode secret file data*/
Status decode_secret_file_data(DecodeInfo *decInfo);

/* Encode function, which does the real encoding */
Status decode_data_to_image(char *data, FILE *fptr_src_image, FILE *fptr_stego_image);

/* Encode a byte into LSB of image data array */
Status decode_byte_to_lsb1(char *image_buffer);

/* Encode a byte into LSB of image data array */
Status decode_byte_to_lsb2(char *image_buffer);

#endif