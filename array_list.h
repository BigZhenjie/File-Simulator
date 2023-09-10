#include <stdio.h>
struct arraylist{
    char** arr;
    int size;
    int num_items;
};

struct arraylist* array_list_new();
void array_list_add_to_end(struct arraylist* list, char* str);