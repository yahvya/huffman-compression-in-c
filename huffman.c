#include "../include/headers.h"
#include "../include/huffman.h"

// HUFFMAN PRIVATE TYPES

typedef enum
{
    BINARY,
    CHARACTER
}HFrom;

typedef struct
{
    char contained_char;
    char* binary;
    short count_of_unused_bit;
}HuffmanRealNode;

typedef struct Huffman
{
    HuffmanRealNode* content;
    struct Huffman* left_node;
    struct Huffman* right_node;
}Huffman;

typedef struct HuffmanList
{
    Huffman* content;
    int occur;
    struct HuffmanList* previous_item;
    struct HuffmanList* next_item;
}HuffmanList;

typedef struct
{
    char* binary;
    short count_of_unused_bit;
    bool success;
}HBinaryFuncReturn;

// HUFFMAN PRIVATE TYPES END

// GLOBAL VARIABLES

HError huffman_last_error;

// GLOBAL VARIABLES END

// ERRORS FUNCTIONS

HError get_huffman_last_error()
{
    return huffman_last_error;
}

// set the last error
void set_huffman_last_error(char* last_text_error,HErrorType last_error_type)
{
    strcpy(huffman_last_error.text,last_text_error);
    huffman_last_error.type = last_error_type;
}

// ERRORS FUNCTIONS END

// FREE MEMORY FUNCTIONS

// free an huffman tree items
void free_huffman_tree(Huffman* to_free,bool have_to_free_content,bool have_to_free_content_binary)
{
    if(to_free != NULL)
    {
        free_huffman_tree(to_free->left_node,have_to_free_content,have_to_free_content_binary);
        free_huffman_tree(to_free->right_node,have_to_free_content,have_to_free_content_binary);

        if(to_free->content != NULL && have_to_free_content)
        {
            if(have_to_free_content_binary && to_free->content->binary != NULL)
                free(to_free->content->binary);

            free(to_free->content);   
        }

        free(to_free);
    }
}

// free an huffman list items
void free_huffman_list(HuffmanList* to_free,bool have_to_free_content,bool have_to_free_content_value,bool have_to_free_content_binary)
{
    if(to_free != NULL)
    {
        free_huffman_list(to_free->next_item,have_to_free_content,have_to_free_content_value,have_to_free_content_binary);

        if(have_to_free_content && to_free->content != NULL)
            free_huffman_tree(to_free->content,have_to_free_content_value,have_to_free_content_binary);

        free(to_free);
    }
}

// FREE MEMORY FUNCTIONS END

// UTILS FUNCTIONS

// try to compress the given binary (from 00101101 to A) and return result
HBinaryFuncReturn compress_huffman_binary(char* binary)
{
    assert(binary != NULL && "The given binary is NULL");

    int char_as_number;
    int binary_length = (int) strlen(binary);
    int length = binary_length > 8 ? (binary_length / 8) + (binary_length % 8) : 1; 

    char* compressed_binary = malloc(sizeof(char) * (length + 1) );

    bool have_to_break_loop = false; 

    HBinaryFuncReturn return_value = {NULL,-1,false};

    if(compressed_binary != NULL)
    {
        *(compressed_binary + length) = '\0';

        // loop to compress
        for(int i = 0; !have_to_break_loop ; i++)
        {
            return_value.count_of_unused_bit = 8;
            char_as_number = 0;

            for(int bit_count = 0; bit_count < 8; bit_count++)
            {
                if(bit_count + (i * 8) >= binary_length)
                {
                    have_to_break_loop = true;

                    break;
                }

                if(*(binary + bit_count + (i * 8) ) == '1')
                    char_as_number += pow(2,7 - bit_count);

                return_value.count_of_unused_bit--;
            }

            *(compressed_binary + i) = (char) char_as_number;
        }

        return_value.binary = compressed_binary;
        return_value.success = true;

        return return_value;
    }

    set_huffman_last_error
    (
        "A binary allocation's failed during compression",
        ALLOCATION_FAILED
    );

    return return_value;
}

// try to discompress a compressed binary and return 
HBinaryFuncReturn discompress_huffman_binary(char* binary,short count_of_unused_bit)
{
    assert(binary != NULL && "The given binary is NULL");

    HBinaryFuncReturn return_value = {NULL,-1,false};

    int char_as_int;
    int reversed_binary;
    int binary_length = (int) strlen(binary);
    int length = ((8 * binary_length) - count_of_unused_bit);

    return_value.binary = malloc(sizeof(char) * (length + 1) );

    if(return_value.binary != NULL)
    {
        return_value.success = true;
        *(return_value.binary + length) = '\0';

        // write the merged binary in return_value
        for(int i = 0; i < binary_length; i++)
        {
            char_as_int = (int) *(binary + i); 
            reversed_binary = 0;

            // reverse the current compressed binary
            for(int bit_count = 0; bit_count < 8; bit_count++)
            {
                if(char_as_int & 1)
                    reversed_binary += pow(2,7 - bit_count);

                char_as_int >>= 1;
            }

            for(int bit_count = 0; bit_count < 8; bit_count++)
            {
                if(i + 1 == binary_length && (i * 8) + bit_count == length)
                    break;

                *(return_value.binary + (8 * i) + bit_count) = reversed_binary & 1 ? '1' : '0';
                
                reversed_binary >>= 1;
            }
        }
    }
    else
    {
        set_huffman_last_error
        (
            "Binary allocation fail while discompress",
            ALLOCATION_FAILED
        );
    }

    return return_value;
}

// try to merge two compressed binary and return result  
HBinaryFuncReturn merge_compressed_binaries(char* binary_one,short count_of_unused_bit_one,char* binary_two,short count_of_unused_bit_two)
{
    HBinaryFuncReturn discompressed_one = discompress_huffman_binary(binary_one,count_of_unused_bit_one);
    HBinaryFuncReturn discompressed_two = discompress_huffman_binary(binary_two,count_of_unused_bit_two);

    HBinaryFuncReturn return_value = {NULL,-1,false};

    if(discompressed_one.success && discompressed_two.success)
    {
        discompressed_one.binary = realloc(discompressed_one.binary,sizeof(char) * (((int) strlen(discompressed_one.binary) + (int) strlen(discompressed_two.binary) ) + 1) );

        if(discompressed_one.binary != NULL)
        {
            strcat(discompressed_one.binary,discompressed_two.binary);

            return_value = compress_huffman_binary(discompressed_one.binary);
        }
        else
        {
            set_huffman_last_error
            (
                "Binary allocation failed during merging",
                ALLOCATION_FAILED
            );
        }
    }

    if(discompressed_one.binary != NULL)
        free(discompressed_one.binary);

    if(discompressed_two.binary != NULL)
        free(discompressed_two.binary);

    return return_value;        
}

// UTILS FUNCTIONS END

// HUFFMAN LIST FUNCTIONS

// return a new list item or NULL if failed
HuffmanList* get_new_list_item(char contained_char,HuffmanList* previous_item,HuffmanList* next_item)
{
    HuffmanList* new_item = malloc(sizeof(HuffmanList) );

    bool have_to_free_new_item = false;

    if(new_item != NULL)
    {
        new_item->content = malloc(sizeof(Huffman) );

        if(new_item->content != NULL)
        {
            new_item->content->content = malloc(sizeof(HuffmanRealNode) );

            if(new_item->content->content != NULL)
            {
                new_item->content->content->contained_char = contained_char;
                new_item->content->content->binary = NULL;
                new_item->content->content->count_of_unused_bit = 0;

                new_item->content->left_node = NULL;
                new_item->content->right_node = NULL;

                new_item->occur = 1;
                new_item->previous_item = previous_item;
                new_item->next_item = next_item;
            }
            else
            {
                free(new_item->content);

                have_to_free_new_item = true;
            }
        }
        else have_to_free_new_item = true;

        if(have_to_free_new_item)
        {
            free(new_item);

            new_item = NULL;  

            set_huffman_last_error
            (
                "Item content allocation failed",
                ALLOCATION_FAILED
            );
        }
    }
    else
    {
        set_huffman_last_error
        (
            "Item content allocation failed",
            ALLOCATION_FAILED
        );
    }

    return new_item;
}

// HUFFMAN LIST FUNCTIONS END

// HUFFMAN TREE FUNCTIONS

// print an huffman tree items data
void print_huffman_tree(Huffman* to_print)
{
    if(to_print != NULL)
    {
        print_huffman_tree(to_print->left_node);
        print_huffman_tree(to_print->right_node);

        if(to_print->content != NULL)
        {
            if(to_print->content->contained_char == '\n' || to_print->content->contained_char == '\r')
                printf("\n<node> char(line break)");
            else
                printf("\n<node> char(%c)",to_print->content->contained_char);

            if(to_print->content->binary != NULL)
                printf(" binary(%s) count of unused bit(%d)\n",to_print->content->binary,to_print->content->count_of_unused_bit);
            else
                printf(" <node binary not set>\n");
        }
    }
}

// write a tree items data in file
void write_huffman_tree_items_in_file(Huffman* tree,FILE* dst_file)
{
    assert(dst_file != NULL && "The given file address is NULL");

    if(tree != NULL)
    {
        write_huffman_tree_items_in_file(tree->left_node,dst_file);
        write_huffman_tree_items_in_file(tree->right_node,dst_file);

        if(tree->content != NULL)
        {
            char line_break = '\n';

            fwrite(&tree->content->contained_char,sizeof(char),1,dst_file);
            fwrite(&line_break,sizeof(char),1,dst_file);
            fwrite(tree->content->binary,sizeof(char) * (int) strlen(tree->content->binary),1,dst_file);
            fwrite(&line_break,sizeof(char),1,dst_file);
            fwrite(&tree->content->count_of_unused_bit,sizeof(short),1,dst_file);
            fwrite(&line_break,sizeof(char),1,dst_file);
        }
    }
}

// return if success to set an huffman tree items binaries
bool set_huffman_tree_items_binary(Huffman* node,char* binary)
{
    assert(binary != NULL && "The given binary is NULL");

    if(node != NULL)
    {
        int binary_length = (int) strlen(binary);

        char* left_binary = malloc(sizeof(char) * (binary_length + 2) );
        char* right_binary = malloc(sizeof(char) * (binary_length + 2) );

        if(left_binary != NULL && right_binary != NULL)
        {
            strcpy(left_binary,binary);
            strcpy(right_binary,binary);

            strcat(left_binary,"0\0");
            strcat(right_binary,"1\0");

            if(node->content != NULL)
            {
                HBinaryFuncReturn compress_huffman_binary_return = compress_huffman_binary(binary);

                free(binary);

                if(compress_huffman_binary_return.success)
                {
                    node->content->binary = compress_huffman_binary_return.binary;
                    node->content->count_of_unused_bit = compress_huffman_binary_return.count_of_unused_bit;
                }
                else
                {
                    free(left_binary);
                    free(right_binary);

                    return false;
                }
            }
            else free(binary);

            return set_huffman_tree_items_binary(node->left_node,left_binary) && set_huffman_tree_items_binary(node->right_node,right_binary);
        }
        else
        {
            if(left_binary != NULL)
                free(left_binary);

            if(right_binary != NULL)
                free(right_binary);

            set_huffman_last_error
            (
                "Tree node binary allocation failed",
                ALLOCATION_FAILED
            );

            return false;
        }
    }
    else free(binary);

    return true;
}

// return an huffman tree item from the given data type or NULL not find
Huffman* get_tree_item_from(Huffman* tree,HFrom from,char* value)
{
    if(tree != NULL)
    {
        if
        (
            tree->content != NULL && 
            (
                (from == BINARY && strcmp(tree->content->binary,value) == 0) || 
                (from == CHARACTER && tree->content->contained_char == *(value) ) 
            )
        )
            return tree;

        Huffman* left_return_value = get_tree_item_from(tree->left_node,from,value);

        return left_return_value == NULL ? get_tree_item_from(tree->right_node,from,value) : left_return_value;
    }

    return NULL;
}

// return a new huffman node or NULL if failed
Huffman* create_huffman_node_from(Huffman* left_node,Huffman* right_node)
{
    Huffman* node = malloc(sizeof(Huffman) );

    if(node != NULL)
    {
        node->content = NULL;
        node->left_node = left_node;
        node->right_node = right_node;
    }
    else
    {
        set_huffman_last_error
        (
            "A node alloction failed",
            ALLOCATION_FAILED
        );
    }

    return node;
}

// parse a file to build an huffman tree and return the tree or NULL if failed
Huffman* get_huffman_tree_from_file(FILE* file_to_parse)
{
    assert(file_to_parse != NULL && "The given file address is NULL");

    HuffmanList* file_chars_list = NULL;
    HuffmanList* list_head = NULL;
    HuffmanList* list_tail = NULL;  

    Huffman* file_tree = NULL; 

    bool operation_successfuly_end = true;
    bool condition;

    char current_char;

    // get the occurence of each char in file - exclude EOF char
    while(!feof(file_to_parse) )
    {
        current_char = fgetc(file_to_parse);

        if(current_char == EOF)
            continue;

        list_head = file_chars_list;
        
        // represent here if need to create a new item 
        condition = true;

        // check if the char already exist in list
        while(file_chars_list != NULL)
        {
            if(file_chars_list->content->content->contained_char == current_char)
            {
                file_chars_list->occur++;

                condition = false;

                break;
            }

            file_chars_list = file_chars_list->next_item;
        }

        file_chars_list = list_head;

        // need to create a new item
        if(condition)
        {
            HuffmanList* new_item = get_new_list_item(current_char,list_tail,NULL);

            if(new_item != NULL)
            {
                if(list_tail != NULL)
                    list_tail->next_item = new_item;
                else 
                    file_chars_list = new_item;

                list_tail = new_item;
            }
            else
            {
                operation_successfuly_end = false;

                break;
            }
        }
    }

    if(operation_successfuly_end)
    {
        HuffmanList* smallest_one;
        HuffmanList* smallest_two;

        list_head = file_chars_list;

        if(file_chars_list != NULL)
        {
            // build the tree by linking the two smallest element
            while(file_chars_list != NULL)
            {
                list_head = file_chars_list;
                smallest_one = file_chars_list;
                smallest_two = file_chars_list;

                // get the smallest one
                while(file_chars_list != NULL)
                {
                    if(file_chars_list->occur < smallest_one->occur)
                        smallest_one = file_chars_list;

                    file_chars_list = file_chars_list->next_item;
                }

                file_chars_list = list_head;

                if(smallest_one == file_chars_list)
                    smallest_two = file_chars_list->next_item;

                // get the smallest two
                while(file_chars_list != NULL)
                {
                    if(file_chars_list != smallest_one && file_chars_list->occur <= smallest_two->occur)
                        smallest_two = file_chars_list;

                    file_chars_list = file_chars_list->next_item;
                }

                file_chars_list = list_head;

                // one element was in the list
                if(smallest_one == smallest_two)
                    smallest_two = NULL;

                file_tree = create_huffman_node_from(smallest_one->content,smallest_two != NULL ? smallest_two->content : NULL);

                if(file_tree != NULL)
                {   
                    // replace smallest one with the new node
                    if(smallest_two != NULL)
                        smallest_one->occur += smallest_two->occur;
                    
                    smallest_one->content = file_tree;

                    condition = true;

                    if(smallest_two != NULL)
                    {
                        // remove smallest two in the list
                        if(smallest_two->previous_item == NULL)
                            file_chars_list = smallest_two->next_item;
                        else
                            smallest_two->previous_item->next_item = smallest_two->next_item;
                        
                        if(smallest_two->next_item != NULL)
                            smallest_two->next_item->previous_item = smallest_two->previous_item;

                        free(smallest_two);

                        condition = false;
                    }
                    
                    if(condition || (smallest_one->previous_item == NULL && smallest_one->next_item == NULL) )
                    {
                        // it was the last element
                        file_chars_list = NULL;

                        list_head = NULL;

                        file_tree = smallest_one->content;
                        
                        free(smallest_one);
                    }
                }
                else
                {
                    operation_successfuly_end = false;

                    break;
                }
            }
        }
        else
        {
            set_huffman_last_error
            (
                "The given file was empty",
                EMPTY_FILE
            );

            operation_successfuly_end = false;
        }
    }

    char* binary = malloc(sizeof(char) );

    if(binary == NULL)
    {
        set_huffman_last_error
        (
            "A tree item binary allocation failed",
            ALLOCATION_FAILED
        );

        operation_successfuly_end = false;
    }
    else if(operation_successfuly_end)
    {
        *(binary) = '\0';

        if(!set_huffman_tree_items_binary(file_tree,binary) )
            operation_successfuly_end = false;
    }
    else free(binary);
    
    if(!operation_successfuly_end) 
    {
        file_chars_list = list_head;

        free_huffman_list(file_chars_list,true,true,true);

        if(file_chars_list == NULL)
            free_huffman_tree(file_tree,true,true);
        else
            free_huffman_tree(file_tree,false,false);

        file_tree = NULL;
    }

    fseek(file_to_parse,0,SEEK_SET);

    return file_tree;
}

// HUFFMAN TREE FUNCTIONS END

// COMPRESSION FUNCTIONS

bool compress_file_with_huffman(char* from_path,char* dst_path,bool have_to_print_tree)
{
    assert(from_path != NULL && dst_path != NULL && "The given path is/are NULL");

    printf("\ndiscompress -> %s\n",merge_compressed_binaries("",7,compress_huffman_binary("11101").binary,3).binary);

    printf("\nerror -> %s",get_huffman_last_error().text);

    exit(EXIT_SUCCESS);

    bool is_compressed = false;

    // try to allocate a string to add huff extension to dst_path
    char* dst_file_path = malloc(sizeof(char) * ((int) strlen(dst_path) + 6) );

    if(dst_file_path == NULL)
    {
        set_huffman_last_error
        (
            "Destination file path allocation's failed",
            ALLOCATION_FAILED
        );

        return false;
    }

    // add the extension
    strcpy(dst_file_path,dst_path);
    strcat(dst_file_path,".huff\0");

    // try to open files
    FILE* to_compress = fopen(from_path,"r");
    FILE* dst_file = fopen(dst_file_path,"wb");
    
    if(to_compress == NULL)
    {
        free(dst_file_path);

        set_huffman_last_error
        (
            "Failed to open the file to compress",
            FILE_OPEN_FAILED
        );

        if(dst_file != NULL)
            fclose(dst_file);

        return false;
    }

    if(dst_file == NULL)
    {
        fclose(to_compress);

        free(dst_file_path);

        set_huffman_last_error
        (
            "Failed to create the compressed file",
            FILE_CREATION_FAILED
        );

        return false;
    }

    // try to compress the file
    Huffman* file_tree = get_huffman_tree_from_file(to_compress);

    if(file_tree != NULL)
    {
        char *current_merged_binary = malloc(sizeof(char) );

        if(current_merged_binary != NULL)
        {
            short count_of_unused_bit;
        
            char current_char;

            Huffman* current_item;

            HBinaryFuncReturn binary_return_value = {"",8,true};

            write_huffman_tree_items_in_file(file_tree,dst_file);

            is_compressed = true;

            *(current_merged_binary) = '\0';

            // write the compressed version of each chars in dst_file
            while(!feof(to_compress) )
            {
                current_char = fgetc(to_compress);

                if(current_char == EOF)
                    continue;

                current_item = get_tree_item_from(file_tree,CHARACTER,&current_char);
            }

            free(current_merged_binary);

            if(have_to_print_tree && is_compressed)
                print_huffman_tree(file_tree);
        }
        else
        {
            set_huffman_last_error
            (
                "Binary allocation failed",
                ALLOCATION_FAILED
            );
        }

        free_huffman_tree(file_tree,true,true);
    }

    free(dst_file_path);

    fclose(to_compress);
    fclose(dst_file);

    return is_compressed;
}

// COMPRESSION FUNCTIONS END