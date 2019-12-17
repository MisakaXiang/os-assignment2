#include <stdio.h>
#include <malloc.h>
#include <unistd.h>
#include <alloca.h>

int data_var1[3] = {1, 2, 3};  // 已赋值的全局变量在data段
int bss_var1[3];  // 未赋值的全局变量在bss段

void stack_fun(void)
{
    int stack_var2 = 2;// 函数中的变量在stack段
    printf("\taddress of stack_var2: %p\n", &stack_var2);
}

int main(int argc, char **argv) /* arguments aren't used */
{
    
    int stack_var1 = 1;  // 函数中的变量放在stack段
    static int bss_var2;  // 未赋值的静态变量放在bss段
    static int data_var2 = 1;  // 已赋值的静态变量放在data段
    char *stack_p;
    int *heap_b, *heap_nb;
    char *const_val = "hello world";

    printf("======Stack Locations======\n");         //stack grows downward
    printf("\tAddress of stack_var1: %p\n", &stack_var1);
    stack_fun();
    
    // alloca在stack中分配内存
    stack_p = (char *) alloca(32);
    if (stack_p != NULL)
    {
      printf("\tStart of alloca()'ed array: %p\n", stack_p);     //addresses are on the stack
      printf("\tEnd of alloca()'ed array: %p\n", stack_p + 31);
    }
    printf("========Heap Locations========\n");
    heap_b = sbrk((ptrdiff_t) 64);                   /* grow address space */
    heap_nb = sbrk((ptrdiff_t) 0);
    printf("\tInitial end of heap: %p\n", heap_b);
    printf("\tNew end of heap: %p\n", heap_nb);
    heap_b = sbrk((ptrdiff_t) - 32);                 /* shrink it */
    heap_nb = sbrk((ptrdiff_t) 0);
    printf("\tFinal end of heap: %p\n", heap_nb);          //heap is immediately above bss, and grows upward
    printf("========BSS Locations========\n");             //bss is above data variables
    printf("\tAddress of bss_var1: %p\n", &bss_var1);
    printf("\tAddress of bss_var2: %p\n", &bss_var2);
    printf("=======DATA Locations========\n");
    printf("\tAddress of data_var1: %p\n", &data_var1);   
    printf("\tAddress of data_var2: %p\n", &data_var2);
    printf("=======Text Locations========\n");
    printf("\tAddress of main: %p\n", main);
    printf("\tAddress of stack_func: %p\n", stack_fun);
    // 字符串常量地址在text段
    printf("\tAddress of const_val: %p\n", &("hello world"));
    sleep(99999999);
}

