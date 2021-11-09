#include <stdio.h>
#include <stdint.h>
#include <sys/time.h>
#include <unistd.h>
#include <NDL.h>

int NDL_Init(uint32_t flags);
int main(){
    printf("START TIMER-TEST: NDL\n");
    struct timeval tv;
    uint32_t old_value = 0;

    NDL_Init(0);
    int i = 0;
    do{
        uint32_t cur = NDL_GetTicks();
        if(cur - old_value >= 1000){
            printf("%ds\n", i++);
            old_value = cur;
        }
    }while(1);

    return 0;
}

