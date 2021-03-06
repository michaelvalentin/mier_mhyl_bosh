/*   bosh.c : BOSC shell   */
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

pid_t parent_pgid;

/* --- use the /proc filesystem to obtain the hostname --- */
char *gethostname(char *hostname)
{
  FILE* file = fopen("/proc/sys/kernel/hostname","r");
  if(file == NULL) return NULL;
  int ch = getc(file);
  int length = 0;
  while(ch != '\n' && ch != EOF) { //Read until end of line or file
    hostname[length] = ch;
    ch = getc(file);
    length++;
  }
  fclose(file);
  return hostname;
}

/* --- InterruptHandler that does nothing --- */
void InterruptIgnore(int signal){
  /* Do NOTHING at all.. */
}

/* --- execute a command --- */
void executecommand(char **cmd, int fdin, int fdout){
  dup2(fdin,0); //Set the input of this command
  dup2(fdout,1); //Set the output of this command
  execvp(cmd[0],cmd);
  printf("Command \"%s\" was not found!\n",cmd[0]);
  exit(0); //Important - we must exit, to prevent the program from an unwanted, extra, parallel execution in child process!
}

/* --- get std in file descriptor ---*/
int getstdinfd(Shellcmd *shellcmd){
  if(shellcmd->rd_stdin){
    return open(shellcmd->rd_stdin,O_RDONLY);
  }else{
    return 0;
  }
}

/* --- get std out file descript ---*/
int getstdoutfd(Shellcmd *shellcmd){
  if(shellcmd->rd_stdout){
    return open(shellcmd->rd_stdout, O_RDWR|O_CREAT,0666);
  }else{
    return 1;
  }
}

/* --- count commands --- */
int countcmds(Cmd *cmdlist){
  int i = 1;
  while((cmdlist = cmdlist->next) != NULL) i++;
  return i;
}

/* --- execute a shell command --- */
int executeshellcmd (Shellcmd *shellcmd){
  //printshellcmd(shellcmd);
  Cmd *cmdlist = shellcmd->the_cmds;
  int cmd_count = countcmds(cmdlist);
  int i = 0;
  int p_fd[(cmd_count-1)][2];
  int pid[cmd_count];
  
  //Detect exit command (only if first and only) and exit
  if(strcmp(cmdlist->cmd[0],"exit")==0 && cmd_count == 1){
    killpg(parent_pgid, SIGINT); //We interrupt the child processes to stop them safely
    return 1; //Returning 1 will exit the bosh shell
  }

  //Iterate over the commands to run them
  while(cmdlist != NULL){
    //First setup all necessary variables
    char **cmd = cmdlist->cmd;
    cmdlist = cmdlist->next;
    int fdin; int fdout;    
    pid_t child_pid;    

    //If third iteration or more, close the pipe from two iterations back completely.
    //We definitely don't need it anymore.
    if(i>1){
      close(p_fd[i-2][0]); close(p_fd[i-2][1]);
    }

    if(cmdlist == NULL){ //Last command from right - setup the std input
      fdin = getstdinfd(shellcmd);
    }else{ //Else setup a pipe to read the input from
      pipe(p_fd[i]);
      fdin = p_fd[i][0];
    }

    if(i == 0){ //First command from right - setup the std output
      fdout = getstdoutfd(shellcmd);
    }else{ //Else write the output to the previous pipe
      fdout = p_fd[i-1][1];
    }

    pid[i] = fork(); // FORK
    switch(pid[i]){
      case -1 : //Error!
        printf("Error forking!\n");
      case 0 : //Child
        child_pid = getpid(); //Get the child pid
        setpgid(child_pid, parent_pgid); //Add child pid to process group of parent
        if(i>0) close(p_fd[i-1][0]); //If not first, close the 0 end of previous pipe
        if(cmdlist != NULL) close(p_fd[i][1]); //If not last, close the 1 end of pipe
        executecommand(cmd, fdin, fdout); //Execute the command
      default : //Parent
        break; //Does nothing.
    }
    i++;
  }
  
  //Make sure that all pipes are closed in parent process
  for(i=0; i<(cmd_count-1);i++){ 
    close(p_fd[i][0]); close(p_fd[i][1]);
  }

  //If not a background process, we must wait for all children
  for(i=0;i<cmd_count;i++) 
    if(!shellcmd->background) waitpid(pid[i],NULL,0);

  return 0; //Success!
}


/* --- main loop of the simple shell --- */
int main(int argc, char* argv[]) {

  /* initialize the shell */
  char *cmdline;
  char hostname[HOSTNAMEMAX];
  int terminate = 0;
  Shellcmd shellcmd;
  
  //Save the parent pid and start a new session (process group)
  parent_pgid = getpid();
  setsid();  

  if (gethostname(hostname)) {
    /* parse commands until exit or ctrl-c */
    signal(SIGINT, InterruptIgnore);
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

