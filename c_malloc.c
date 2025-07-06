#include <windows.h>
#include <stdio.h>

const int total_memory = 16384;
static int memory_used = 0;
static void *page_pointer = NULL;

void* allocate_phisical_memory(int s, void* p) {
    SIZE_T size = (SIZE_T)s;
    LPVOID reserve = VirtualAlloc(p, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    return reserve;
}

void* lazy_allocating_mmeory(int a) {
    SIZE_T size = (SIZE_T)a;
    return VirtualAlloc(NULL, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
}

typedef struct block {
    SIZE_T size;
    int free;
    struct block* next;
} block;

#define memory_block sizeof(block)
static block *free_list = NULL;

void *c_malloc(size_t size) {
    block *current, *prev = NULL;
    void *pointer_de_returnat = NULL;

    if(size <= 0) {
        printf("%s", "What are you doing ? >:( ");
        return NULL;
    }

    current = free_list;
    while(current != NULL) {
        if(current->free == 1 && current->size >= size) {
            current->free = 0;
            return (void*)(current + 1);
        }
        prev = current;
        current = current->next;
    }

    if(page_pointer == NULL) {
        page_pointer = lazy_allocating_mmeory(total_memory);
        memory_used = 0;
    }

    if(size > total_memory - memory_used) {
        void *temporary_pointer = lazy_allocating_mmeory(total_memory);
        pointer_de_returnat = allocate_phisical_memory(size + memory_block, lazy_allocating_mmeory(temporary_pointer));
        memory_used = 0;
        page_pointer = temporary_pointer;
    } else {
        pointer_de_returnat = (char*)page_pointer + memory_used;
        memory_used += size + memory_block;
    }

    current = (block *)pointer_de_returnat;
    current->size = size;
    current->free = 0;
    current->next = NULL;

    if(free_list == NULL) {
        free_list = current;
    } else {
        prev->next = current;
    }

    return (void*)(current + 1);
}

void c_free(void *detinutul) {
    block *eliberatorul = ((block*)detinutul) - 1;
    if(eliberatorul->free == 1) {
        printf("%s", "I'm already free");
    } else {
        eliberatorul->free = 1;
        detinutul = NULL;
    }
}

int main() {
    void *ptr1 = c_malloc(4000); 
    void *ptr2 = c_malloc(8000); 
    void *ptr3 = c_malloc(5000); 

    if (ptr1 && ptr2 && ptr3) {
        printf("Memory allocated successfully:\n");
        printf("ptr1: %p\n", ptr1);
        printf("ptr2: %p\n", ptr2);
        printf("ptr3: %p (new page expected)\n", ptr3);
    } else {
        printf("Memory allocation failed.\n");
    }

    c_free(ptr1);
    c_free(ptr2);
    c_free(ptr3);

    return 0;
}
