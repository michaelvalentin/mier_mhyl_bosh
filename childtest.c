#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
int main()
{
    int i;
    pid_t cpid;
    pid_t child_pid;
    cpid = fork();

    switch (cpid) {
        case -1: printf("Fork failed; cpid == -1\n");
                 break;

        case 0: child_pid = getpid();
                for (i = 0; i < 10; i++) {
                    printf("%d: this is the child, pid = %d\n", i, child_pid);
                    sleep(1);
                }
                exit(0);

        default: printf("This is the parent: waiting for %d to finish\n", cpid);
                 //waitpid(cpid, NULL, 0);
                 printf("Ttttthat's all, folks\n");
    }
    return 0;
}
