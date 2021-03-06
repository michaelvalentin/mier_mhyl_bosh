#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <fcntl.h>

int main(void){
  pid_t pid;
  pid_t pid2;
  int fd[2];

  char *argv[] = {"ls",0};
  char *argv2[] = {"wc","-w",0};
 
  pipe(fd); 
  
  pid2 = fork();

  switch(pid2){
    case -1 : printf("Error in fork!\n"); return EXIT_FAILURE;
    case 0 :
      dup2(fd[0],0);
      close(fd[1]);
      execvp(argv2[0],argv2);
    default :
      break;
  }

  
  pid = fork();

  switch(pid){
    case -1 : printf("Error in fork!\n"); return EXIT_FAILURE;
    case 0 :
      dup2(fd[1],1);
      close(fd[0]);
      execvp(argv[0],argv);
    default :
      break;
  }


  close(fd[0]); close(fd[1]);

  waitpid(pid, NULL, 0);
  waitpid(pid2, NULL, 0);
  
  char *argv3[] = {"ps",0};
  execvp(argv3[0],argv3);

  exit(0);
}
