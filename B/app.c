#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

void heap_alloc() {
    int bytes_to_allocate;
    void* base = sbrk(0);
    printf("initial end of the heap: %llx\n", base);
    printf("enter amount of bytes to allocate\n");
    scanf("%d", &bytes_to_allocate);
    int i = 0;
    int* x;
    for(i = 0; i < bytes_to_allocate / 1000; i++)
         x = malloc(1000);
    //x[bytes_to_allocate / 2] = 1;
    printf("%llx\n", x);
    printf("end of the heap: %llx\n", base);
}

void stack_alloc(int count) {
    if(count == 1){
        printf("recursive function is called as requested and this is the last call, use the module to check stack information, press a key (then enter) to exit\n");
        char s;
        scanf("%c", &s);
        return;
    }
    stack_alloc(count - 1);
}

int main(){
    int pid = getpid();
    printf("process id: %d\n", pid);
    while(1){
        int choice;
        printf("press 1 to use heap (malloc)\npress 2 to use stack (recursive)\npress 0 to exit\n");
        scanf("%d", &choice);
        if(choice == 0) {
            break;
        } else if(choice == 1) {
            heap_alloc();
        } else if (choice == 2) {
            int count;
            printf("enter the number of times to call the recursive function\n");
            scanf("%d", &count);
            int i = 0;
        	stack_alloc(count);
        } else {
            printf("invalid choice\n");
        }
    }
}