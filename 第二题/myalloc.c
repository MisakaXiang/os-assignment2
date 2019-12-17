#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>

//内存控制块结构定义
typedef struct block_memory_s
{
    size_t size;
    bool free;  //是否可用
    struct block_memory_s* next;
}block_memory_t;

block_memory_t* head = 0;
void* myalloc (size_t size)
{
    block_memory_t* new_mem_block;
    
    // 对堆中的内存块遍历，找到合适的内存块
    for (new_mem_block = head; new_mem_block != NULL; new_mem_block = new_mem_block->next)
    {
        // 该块可以用而且大小满足要求
        if (new_mem_block->free && new_mem_block->size >= size)
            break;
    }
    
    // 没有找到合适的内存块，用sbrk向操作系统请求空间
    if (!new_mem_block)
    {
        // 内存块实际大小，内存控制块大小+参数大小
        new_mem_block = sbrk(sizeof (block_memory_t) + size);
        // 申请失败，没有可用内存
        if (new_mem_block == (void*)-1)
            return NULL;
        new_mem_block->size = size;
        new_mem_block->next = head;
        head = new_mem_block;
    }
    
    new_mem_block->free = false;
    return new_mem_block + 1;
}

void myfree(void* ptr)
{
    if (ptr)
    {
        block_memory_t* new_mem_block = (block_memory_t*)ptr - 1;
        new_mem_block->free = true;
        for (new_mem_block = head; new_mem_block->next; new_mem_block = new_mem_block->next)
        {
            if (! new_mem_block->free)
                break;
        }
        
        if (new_mem_block->free)
        {
            head = new_mem_block->next;
            brk(new_mem_block);
        }
        else if (new_mem_block != head)
        {
            head = new_mem_block;
            brk((void*)new_mem_block + sizeof (block_memory_t) + new_mem_block->size);
        }
    }
}


int main()
{
    // sbrk(0)返回当前堆顶部的指针
    printf("address: %p\n", sbrk(0));
    getchar();
    int *p = myalloc(sizeof(int));
    printf("address: %p\n", sbrk(0));
    getchar();
    *p = 30;
    printf("%d\n", *p);
    myfree(p);
    printf("address: %p\n", sbrk(0));
    getchar();
    return 0;
}


