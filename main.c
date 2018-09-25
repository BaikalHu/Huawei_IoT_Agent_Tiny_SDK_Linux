#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

int main()
{
    pthread_t tidp;
    extern void agent_tiny_entry(void);

    if ((pthread_create(&tidp, NULL, agent_tiny_entry, NULL)) == -1)
    {
        printf("create error!\n");
        return 1;
    }
    while(1);
    printf("end\n");

    
    return 0;
}
