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

/* --- execute a shell command --- */
int executeshellcmd (Shellcmd *shellcmd){
  printshellcmd(shellcmd);
  
  Cmd *cmdlist = shellcmd->the_cmds;
  Cmd *cmdlist2 = shellcmd->the_cmds;
  int cmd_count;
  int i = 0;
  
  //Find out the number of commands
  while(cmdlist2 != NULL){
     i++;
     cmdlist2 = cmdlist2->next;
  }
  cmd_count = i;
  
  //Reset i
  i = 0;
  
  //Setup variables
  int pipe_fd[cmd_count][2];

  while(cmdlist != NULL){
     i++;
     char **cmd = cmdlist->cmd;
     cmdlist = cmdlist->next;
     
     //Is this the exit command?
     if(strcmp(cmd[0],"exit")==0){
       return 1;
     }

     if(i != cmd_count){
       if(pipe(pipe_fd[i]) < 0){ //Make a new pipe;
         printf("Pipe failed\n");
         return 0;
       }else{
         printf("PIPPING SUCCESS %d \n",i);
       }
     }

     child_pids[i] = fork();
  
     switch(child_pids[i]){
       case -1 : printf("ERROR: Failed to fork."); return 0;
       case 0 : //Child process
         printf("Reaching child process for #%s#\n",cmd[0]);
         //Check if this is the first command
         if(i != 1){
           //If not first, set StdInput to be the pipe of before
           dup2(pipe_fd[i-1][0],1); //Use the pipe from before. Set command input to pipe output.
           close(pipe_fd[i-1][0]); //Close the pipe output.
         }else{
           //If first (last command from left), set StdOutput to StdOutput file, if one is given
            if(shellcmd -> rd_stdout){
               int fd = open(shellcmd->rd_stdout, O_RDWR|O_CREAT,0666);
               dup2(fd,1);
               close(fd);
            }

         }
     
         printf("Reaching2 child process for #%s#\n",cmd[0]);
         //Check if this is the last command
         if(i != cmd_count){
            //If not last, set StdOut to the newly made pipe  
            dup2(pipe_fd[i][1],0); //Set pipe input to command output
            close(pipe_fd[i][1]); //Close pipe input
         }else{
           //If last (first command from left), set StdInput to StdInput file, if one is given
           if(shellcmd -> rd_stdin){
             int fd = open(shellcmd->rd_stdin, O_RDONLY);
             dup2(fd,0);
             close(fd);
           }
         }

         printf("RUNNING COMMAND: %s\n",cmd[0]);
         if(execvp(cmd[0],cmd) == -1){
            printf("Command not found\n");
         }
         return 0;
     } 
     if(child_pids[i]){
     if((shellcmd -> background) == 0){       
       waitpid(child_pids[i],NULL,0); 
     }
     return 0;
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
    signal(SIGINT, InteruptHandler);
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

