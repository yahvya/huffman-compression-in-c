#include "../include/headers.h"
#include "../include/huffman.h"

int main(void)
{
    if(!compress_file_with_huffman("resources/test_file_to_compress.txt","resources/compressed_file",false) )
        printf("\nError : %s\n",get_huffman_last_error().text);
    else 
        printf("\nThe file is successfuly compressed\n");

    return 0;
}

   