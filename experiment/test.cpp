#include <sched.h>
#include <stdio.h>

int main(void){
    printf("sizeof cpu_set_t is: %d\n",sizeof(cpu_set_t));
    return 0;
}
