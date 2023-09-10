#define  _GNU_SOURCE
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

void* checked_malloc(size_t size){/*caller has to free*/
    void *p;
    p = malloc(size);
    if(p == NULL){
        printf("malloc");
        exit(1);
    }
    return p;
}

char* add_null(char* og){ /*caller has to free*/
    char* copy = checked_malloc(sizeof(char) *33);
    int i = 0;
    while(og[i] != '\0' && i < 32){
        copy[i] = og[i];
        i++;
    }
    /*copy[i] = '\0';
    i++;*/
    for(i = i; i < 32; i++){
        copy[i] = '\0';
    }
    return copy;
}

char *uint32_to_str(uint32_t i)/*caller has to free*/
{
   int length = snprintf(NULL, 0, "%lu", (unsigned long)i);       // pretend to print to a string to get length
   char* str = checked_malloc(length + 1);                        // allocate space for the actual string
   snprintf(str, length + 1, "%lu", (unsigned long)i);            // print to string

   return str;
}

void validate_args(int argc, char **argv){
    int ch;
    if(argc == 1){
        printf("invalid argc\n");
        exit(1);
    }
    ch = chdir(argv[1]);
    if(ch < 0){
        printf("invalid argv\n");
        exit(1);
    }


}

FILE *try_read(char *input){ /*caller has to close*/
    FILE* fpointer = fopen(input, "rb");
    if(fpointer == NULL){
        printf("can't open file");
        exit(1);
    }
    return fpointer;
    
}

void print_directory_file(int inode_index){
    uint32_t inode;
    char *inode_index_str = uint32_to_str(inode_index);
    char *content = checked_malloc(sizeof(char) * 32);
    FILE *in = fopen(inode_index_str,"rb");
    
    while(fread(&inode, sizeof(uint32_t), 1, in)){
        printf("%d-", inode);
        fread(content, sizeof(char), 32, in);
        printf("%s\n", content);
    };
    fclose(in);
    free(content);
    free(inode_index_str);
    
    
}

void func_exit(){
    exit(0);
}

int func_cd(char *target_directory, int cur_dir, char* inode_list){
    uint32_t inode;
    int changed_dir = cur_dir;

    char* content = checked_malloc(sizeof(char) * 32);
    char* cur_dir_str = uint32_to_str(cur_dir);
    FILE *in = fopen(cur_dir_str,"rb");

    while(fread(&inode, sizeof(uint32_t), 1, in)){
        fread(content, sizeof(char), 32, in);
        if(!strcmp(target_directory, content)){
            if(inode_list[inode] == 'd'){
                changed_dir = inode;
                break;
            }

        }
        
    }
    free(content);
    fclose(in);
    free(cur_dir_str);
    if(changed_dir == cur_dir){
        printf("Directory does not exist.\n");
    }
    return changed_dir;
    
    
}

void func_ls(int cur_dir){
    print_directory_file(cur_dir);
}

void check_root_dir(FILE* in){
    uint32_t inode;
    char c;
    FILE* zero_inode = NULL;
    fread(&inode, sizeof(uint32_t), 1, in);
    fread(&c, sizeof(char), 1, in);
    if(c != 'd'){
        printf("root file cannot be found\n");
        exit(1);
    }
    rewind(in);
    /*also try to open file 0 to see if it actually exists*/
    zero_inode = fopen("0", "r");
    if(zero_inode == NULL){
        printf("0th inode file does not exist\n");
        exit(1);
    }
    fclose(zero_inode);

}

void initialize_inode_list(char* arr, FILE* in, int* inode_count){
    uint32_t inode;
    char c;
    while(fread(&inode, sizeof(uint32_t), 1, in)){
        fread(&c, sizeof(char), 1, in);
        arr[inode] = c;
        (*inode_count)++;
    }
}


void print_inode_list(FILE* in){
    uint32_t inode;
    char c;
     while(fread(&inode, sizeof(uint32_t), 1, in)){
        printf("%d", inode);
        fread(&c, sizeof(char), 1, in);
        printf("%c\n", c);
    }
}

void update_inodes_list_file(char* in, char* inode_list, int index, char type){
    FILE *file = fopen(in, "ab");
    fwrite(&index, sizeof(uint32_t), 1, file);
    fwrite(&type, sizeof(char), 1, file);
    fclose(file);
    inode_list[index] = type;
}

void func_mkdir(char* name, char* inode_list, int cur_dir, int* total_nodes){
    char* cur_dir_str = uint32_to_str(cur_dir);
    char* total_nodes_str = uint32_to_str(*total_nodes);
    char* p_null = add_null("."); /*period null*/
    char* pp_null = add_null(".."); /*period period null*/
    char* new_name = add_null(name);
    uint32_t inode;
    char* content = checked_malloc(sizeof(char)* 32);
    FILE* in = fopen(cur_dir_str, "rb");

    if ((*total_nodes) > 1024){
        printf("too much inodes\n");
        return;
    }

    
    while(fread(&inode, sizeof(uint32_t), 1, in)){
        fread(content, sizeof(char), 32, in);
        if(!strcmp(new_name, content)){
                printf("File/directory already exists in this directory.\n");
                free(content);
                free(cur_dir_str);
                free(total_nodes_str);
                free(p_null);
                free(pp_null);
                free(new_name);
                fclose(in);
                return;
            }
    }
    fclose(in);
    free(content);

    FILE *current_direct = fopen(cur_dir_str, "ab");

    /*start of writing to current directory file*/
    fwrite(total_nodes, sizeof(uint32_t), 1, current_direct);

    fwrite(new_name, sizeof(char), 32, current_direct);

    /*end of writing to current directory file*/
    /*close current directory file*/
    fclose(current_direct);


    FILE *new_file = fopen(total_nodes_str, "wb");

    /*start of writing to new file*/
    fwrite(total_nodes,  sizeof(uint32_t), 1, new_file);
    fwrite(p_null, sizeof(char), 32, new_file);
    fwrite(&cur_dir,  sizeof(uint32_t), 1, new_file);
    fwrite(pp_null, sizeof(char), 32, new_file);
    /*end of writing to new file*/

    fclose(new_file);

    update_inodes_list_file("inodes_list", inode_list, *total_nodes, 'd');
    (*total_nodes)++;/*increment total nodes*/

    free(cur_dir_str);
    free(total_nodes_str);
    free(p_null);
    free(pp_null);
    free(new_name);
}



void func_touch(char* name, char* inode_list, int cur_dir, int *total_nodes){
    char* cur_dir_str = uint32_to_str(cur_dir);
    char* total_nodes_str = uint32_to_str(*total_nodes);
    char* new_name = add_null(name);
    uint32_t inode;
    char* content = checked_malloc(sizeof(char)* 32);
    FILE* in = fopen(cur_dir_str, "rb");

    if ((*total_nodes) > 1024){
        printf("too much inodes\n");
        return;
    }

    
    while(fread(&inode, sizeof(uint32_t), 1, in)){
        fread(content, sizeof(char), 32, in);
        if(!strcmp(new_name, content)){
                printf("File/directory already exists in this directory.\n");
                free(content);
                free(cur_dir_str);
                free(total_nodes_str);
                free(new_name);
                fclose(in);
                return;
            }
    }
    fclose(in);
    
    FILE *current_direct = fopen(cur_dir_str, "ab");
    /*start of writing to current directory file*/
    fwrite(total_nodes, sizeof(uint32_t), 1, current_direct);
    fwrite(new_name , sizeof(char), 32, current_direct);
    /*end of writing to current directory file*/
    /*close current directory file*/
    fclose(current_direct);
    FILE *new_file = fopen(total_nodes_str, "w");

    /*start of writing to new file*/
    fwrite(name , strlen(name), 1, new_file);
    /*end of writing to new file*/
    fclose(new_file);
    free(new_name);
    update_inodes_list_file("inodes_list", inode_list, *total_nodes, 'f');
    (*total_nodes)++;/*increment total nodes*/
    free(content);
    free(cur_dir_str);
    free(total_nodes_str);
}


int main(int argc, char **argv){
    int cur_dir = 0;
    char inode_list[1024];
    int total_inodes = 0;
    char *token = NULL;
    char *line = NULL;
    size_t size;
    FILE* in = NULL;
    validate_args(argc, argv);
    in = try_read("inodes_list");
    check_root_dir(in);
    /*print_inode_list(in);*/
    /*initializing the array that holds the inode list and also the number of inodes*/
    initialize_inode_list(inode_list, in, &total_inodes); 
    fclose(in);

    while(getline(&line, &size, stdin)){
        token = strtok(line, " \n");
        while(token != NULL){
            if(!strcmp(token, "ls")){
                func_ls(cur_dir);
            }

            if(!strcmp(token, "exit") || !strcmp(token, "EOF") || !strcmp(token, "^D")){
                free(token);
                func_exit();
            }

            if(!strcmp(token, "mkdir")){
                token = (strtok(NULL, " \n"));
                func_mkdir(token, inode_list, cur_dir, &total_inodes);

            }

            if(!strcmp(token, "touch")){
                token = (strtok(NULL, " \n"));
                func_touch(token, inode_list, cur_dir, &total_inodes);

            }

            if(!strcmp(token, "cd")){
                token = (strtok(NULL, " \n"));
                cur_dir = func_cd(token, cur_dir, inode_list);

            }
            token = strtok(NULL, " \n");
            free(token);
        }
        free(token);
    }

    return 0;
}