#if !defined(HUFFMAN_H)
#define HUFFMAN_H

    // huffman utils types 
    typedef enum
    {
        false,
        true
    }bool;

    typedef enum
    {
        ALLOCATION_FAILED,
        EMPTY_FILE,
        CONVERSION_FAILED,
        MERGE_FAILED,
        FILE_OPEN_FAILED,
        FILE_CREATION_FAILED
    }HErrorType;

    typedef struct
    {
        char text[50];
        HErrorType type;    
    }HError;

    // ERRORS

    // return the last error datas 
    HError get_huffman_last_error();

    // COMPRESSION 

    // return if success to compress file - to_path need to come without an extension
    bool compress_file_with_huffman(char* from_path,char* dst_path,bool have_to_print_tree);

#endif