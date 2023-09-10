#include <stdio.h>
#include <stdlib.h>

struct arraylist{
    char** arr;
    int size;
    int num_items;
};


struct arraylist* array_list_new(){
    struct arraylist* list = malloc(sizeof(struct arraylist));
    list -> size = 2;
    list -> num_items = 0;
    list -> arr = malloc(sizeof(char*) * list -> size);
    return list;
}

void array_list_add_to_end(struct arraylist* list, char* str){
    if(list -> num_items == list -> size){
        list -> size = (list -> size) * 2;
        list -> arr = realloc(list -> arr, sizeof(char*) *list -> size);
    }
    (list -> arr)[(list -> num_items)++] = str;

}
