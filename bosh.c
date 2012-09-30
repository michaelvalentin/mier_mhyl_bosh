/* 

   bosh.c : BOSC shell 

*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "parser.h"
#include "print.h"
#include <fcntl.h>

/* --- symbolic constants --- */
#define HOSTNAMEMAX 100

/* --- use the /proc filesystem to obtain the hostname --- */
char *gethostname(char *hostname)
{
  /*TODO: Should we clear the hostname first? */
  /*TODO: Can the hostname change? Should we read it every time? */
  FILE* file = fopen("/proc/sys/kernel/hostname","r");
  if(file == NULL){
    return NULL;
  }
  int ch = getc(file);
  int length = 0;
  while(ch != '\n' && ch != EOF) {
    hostname[length] = ch;
    ch = getc(file);
    length++;
  }
  fclose(file);
  return hostname;
}

/* --- execute a shell command --- */
int executeshellcmd (Shellcmd *shellcmd){
  printshellcmd(shellcmd);

  Cmd *cmdlist = shellcmd->the_cmds;
  char **cmd = cmdlist->cmd;
  //Need to find a dynamic way to detect array size
  char *args[10];
  int i = 0;
  while(*cmd != NULL){                            //Adds the command and all arguments tot the args array
    args[i]=*cmd++;
    i++;
  }
  args[i] = NULL;

  if(strcmp(args[0],"exit")==0){                   //If cmd is equal to "exit" -> terminate shell
    return 1;
  }

  pid_t pid = fork();                             //Make child process
  switch(pid){
    case -1: printf("Failed to fork\n");          //If fork failed
             return 0;
    case 0: if(shellcmd -> rd_stdin){             //If it is a child process
              int fd = open(shellcmd->rd_stdin, O_RDONLY);
              dup2(fd,0);
              close(fd);
            }

            if(shellcmd -> rd_stdout){
              int fd = open(shellcmd->rd_stdout, O_RDWR|O_CREAT,0666);
              dup2(fd,1);
              close(fd);
            }

            if(execvp(args[0],args) == -1){
              printf("Command not found\n");
            }
            return 0;
    default: if((shellcmd -> background) == 0){       
               waitpid(pid,NULL,0 );                //If not a background-process, wait for child process to finish
             }
             return 0;
  }
}

/* --- main loop of the simple shell --- */
int main(int argc, char* argv[]) {

  /* initialize the shell */
  char *cmdline;
  char hostname[HOSTNAMEMAX];
  int terminate = 0;
  Shellcmd shellcmd;

  if (gethostname(hostname)) {

    /* parse commands until exit or ctrl-c */
    while (!terminate) {
      printf("%s", hostname);
      if (cmdline = readline(":# ")) {
        if(*cmdline) {
          add_history(cmdline);
          if (parsecommand(cmdline, &shellcmd)) {
            terminate = executeshellcmd(&shellcmd);
          }
        }
        free(cmdline);
      } else terminate = 1;
    }
    printf("Exiting bosh.\n");
  }
  return EXIT_SUCCESS;
}

