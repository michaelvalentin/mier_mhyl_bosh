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
#include <signal.h>

/* --- symbolic constants --- */
#define HOSTNAMEMAX 100
#define MAX_CMDS 10

pid_t child_pids[MAX_CMDS];

/* --- use the /proc filesystem to obtain the hostname --- */
char *gethostname(char *hostname)
{
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

void InteruptHandler(int signal){
  
}

/* --- execute a command */
int executecommand(char **cmd, int fdin, int fdout){
  printf("Command run: %s with file descriptors in(%d) out(%d)\n",cmd[0],fdin,fdout);
  if(fdin!=0)  dup2(fdin,0);
  if(fdout!=0) dup2(fdout,1);
  close(fdin);
  close(fdout);
  execvp(cmd[0],cmd);
  printf("Command \"%s\" was not found!\n",cmd[0]);
  exit(0);
}

/* --- get std out file descriptor ---*/
int get stdoutfd(Shellcmd *shellcmd){
  if(shellcmd->rd_stdout){

  }else{
    return 0;
  }
}

/* --- get std out file descript ---*/
int getstdinfd(Shellcmd *shellcmd){
  if(shellcmd->rd_stdin){
    
  }else{
    return 0;
  }
}

/* --- execute a shell command --- */
int executeshellcmd (Shellcmd *shellcmd){
  printshellcmd(shellcmd);
  Cmd *cmdlist = shellcmd->the_cmds;
  int first = 1;
  int fdin = 0;
  int fdout = 0;

  while(cmdlist != NULL){
    char **cmd = cmdlist->cmd;
    printf("Command reached:%.s\n",cmd[0]);
    cmdlist = cmdlist->next;

    fdout = fdin; //We want to set the output to the input from the previous command.
    //WERE TO GO NOW : THE PIPE IS DESCRIBED BY TWO DIGITS!!! 

    if(first != 1){
      //Setup a pipe for output...
    }else{
      //setup a standard output...
      fdout = getstdinfd(shellcmd);
    }

    if(cmdlist != NULL){
      //Setup a pipe for input...
    }else{
      //Setup a standard input
      fd
    }

    first = 0;

    int pid = fork();
    switch(pid){
      case -1 : //Error!
        printf("Error forking!");
      case 0 : //Child
        executecommand(cmd, fdin, fdout); //Execute the command as the children process
      default : //Parent
        if(!shellcmd->background) waitpid(pid,NULL,0); //If not a bg process, wait for kid to finish
    }
  }
  return 0;
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
    //signal(SIGINT, InteruptHandler);
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

