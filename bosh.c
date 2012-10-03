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
  if(fdin!=-1)  dup2(fdin,0);
  if(fdout!=-1) dup2(fdout,1);
  execvp(cmd[0],cmd);
  printf("Command \"%s\" was not found!\n",cmd[0]);
  exit(0);
}

/* --- get std in file descriptor ---*/
int getstdinfd(Shellcmd *shellcmd){
  if(shellcmd->rd_stdin){
    return open(shellcmd->rd_stdin,O_RDONLY);
  }else{
    return -1;
  }
}

/* --- get std out file descript ---*/
int getstdoutfd(Shellcmd *shellcmd){
  if(shellcmd->rd_stdout){
    return open(shellcmd->rd_stdout, O_RDWR|O_CREAT,0666);
  }else{
    return -1;
  }
}

/* --- execute a shell command --- */
int executeshellcmd (Shellcmd *shellcmd){
  printshellcmd(shellcmd);
  Cmd *cmdlist = shellcmd->the_cmds;
  int i = 0;
  int p_fd[2] = {-1,-1};
  int last_p_fd[2] = {-1,-1};
  int first_pid = -1;

  while(cmdlist != NULL){
    char **cmd = cmdlist->cmd;
    printf("Command reached:%.s\n",cmd[0]);
    cmdlist = cmdlist->next;

    i++;

    int fdin = -1;
    int fdout = -1;

    last_p_fd[0] = p_fd[0];
    last_p_fd[1] = p_fd[1];
    pipe(p_fd);

    if(cmdlist != NULL){
      //Setup a pipe for input...
      fdin = p_fd[0];
    }else{
      //Setup a standard input
      fdin = getstdinfd(shellcmd);
    }

    if(i != 1){
      //Setup output to previous pipe...
      fdout = last_p_fd[1];
    }else{
      //setup a standard output...
      fdout = getstdoutfd(shellcmd);
    }

    int pid = fork();
    if(first_pid = -1 && pid!=0) first_pid = pid;
    switch(pid){
      case -1 : //Error!
        printf("Error forking!");
      case 0 : //Child
        executecommand(cmd, fdin, fdout); //Execute the command as the children process
      default : //Parent
        break;
    }
  }
  if(!shellcmd->background) waitpid(first_pid,NULL,0); //If not a bg process, wait for kid to finish
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

