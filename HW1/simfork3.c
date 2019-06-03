#include <stdio.h>
#include <unistd.h>
int K;

main()
{
    int i;
    int j;

    j = 200;
    K = 300;

    printf("Before forking: j = %d, K = %d  ", j, K);
    i = fork();

    if (i > 0) {  /* Delay the parent */
        sleep(1);
        printf("After forking, parent: j = %d, K = %d\n", j, K);

    } else {
        j++;
        K++;
        printf("After forking, child: j = %d, K = %d\n", j, K);
    }
} 
