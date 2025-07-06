```#include <windows.h>
#include <stdio.h>

const int total_memory = 16384;
static int memory_used = 0;   //monitorizam memoria folosita in pagina daca depaseste trebuie sa mai facem o pagina
static void *page_pointer = NULL;      // cu asta legam paginile daca avem nevoie de mai multe pagini

void* allocate_phisical_memory(int s, void* p) {
    SIZE_T size = (SIZE_T)s;
    LPVOID reserve = VirtualAlloc(p, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    return reserve;
}

void* lazy_allocating_mmeory(int a)
{
    SIZE_T size = (SIZE_T)a;
    return VirtualAlloc(NULL, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
}

typedef struct block {
    SIZE_T size;
    int free;
    struct block* next;
} block;

#define memory_block sizeof(block) //ne ajuta sa adaugam memorie si pentru struct cand creeam memorie
static block *free_list = NULL; //bun atunci cand cand parucurgem lista inlantuita

void *c_malloc(size_t size)
{
    block *current, *prev = NULL;
    void *pointer_de_returnat = NULL; //il folosim daca nu avem memorie motiv pentru care mai creeam o bucata de memorie FIZICA NU VIRTUALA

    if(size <= 0)
    {
        printf("%s", "THE FUCK YOU DOING ? >:( ");
        return NULL;
    }

    current = free_list;       //aici e coada listei
    while(current != NULL)   //fun fact implementare de first fit algorithm folosit si de os
    {

        if(current->free==1 && current->size>=size)
        {
            current->free = 0;
            return (void*)(current + 1);
        }
        prev = current;
        current = current->next;
    }


    if(page_pointer == NULL) { // folosim doar cand suntem la prima pagina
        page_pointer = lazy_allocating_mmeory(total_memory);
        memory_used = 0;
    }

    if(size > total_memory - memory_used)    //daca nu avem suficienta memorie mai cersim la os inca o pagina
    {
        void *temporary_pointer=lazy_allocating_mmeory(total_memory);
        pointer_de_returnat = allocate_phisical_memory(size + memory_block, lazy_allocating_mmeory(temporary_pointer));
        memory_used=0;
        page_pointer=temporary_pointer;  //this shit will create a lot of fragmentation issues,MUST BE TREATED CARREFUCLLY WHEN ALLOCATE MEMORY
                                        //din cauza ca pointerul de page va pointa la pagina curenta poate lasa multe locuri libere in pagina anterioara 
    }
    else {
        pointer_de_returnat = (char*)page_pointer + memory_used;
        memory_used += size + memory_block;
    }
    current = (block *)pointer_de_returnat;
    current->size = size;
    current->free = 0;
    current->next = NULL;

    if(free_list == NULL) //aici practic initializam capul listei
    {
        free_list = current;
    }
    else
    {
        prev->next = current;  //si aici legam bucata noua de memoriue de ce celelalte
    }
    return (void*)(current + 1);  // avem current+1 ca sa sarim peste metadata de la struct,practic adunam marimea lui block cu adreesa lui curent sarind astfel peste struct si obtinem un void pointer
}
void c_free(void *detinutul)
{


    block *eliberatorul=((block*)detinutul)-1;  //inversam ce facem cand returnam adresa lui memoriei din malloc
    if(eliberatorul->free==1)
    {
        printf("%s","I'm already free");
    }
    else
    {
        eliberatorul->free=1;
        detinutul=NULL;
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
```