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
  pid_t pid;
  int pipe_fd[cmd_count][2];

  while(cmdlist != NULL){
     i++;
     char **cmd = cmdlist->cmd;
     cmdlist = cmdlist->next;
     
     //Is this the exit command?
     if(strcmp(cmd[0],"exit")==0){
       return 1;
     }
     
     pid = fork();
  
     switch(pid){
       case -1 : printf("ERROR: Failed to fork."); return 0;
       case 0 : //Child process
         //Check if this is the first command
         if(i != 1){
           //If not first, set StdInput to be the pipe of before
           dup2(pipe_fd[i-1][1],0); //Use the pipe from before. Set command input to pipe output.
           close(pipe_fd[i-1][1]); //Close the pipe output.
         }else{
           //If first (last command from left), set StdOutput to StdOutput file, if one is given
            if(shellcmd -> rd_stdout){
               int fd = open(shellcmd->rd_stdout, O_RDWR|O_CREAT,0666);
               dup2(fd,1);
               close(fd);
            }

         }
     
         //Check if this is the last command
         if(i != cmd_count){
            //If not last, set StdOut to a new pipe  
            if(pipe(pipe_fd[i]) < 0){ //Make a new pipe
              printf("Pipe failed\n");
              return 0;
            }
            dup2(pipe_fd[i][0],1); //Set pipe input to command output
            close(pipe_fd[i][0]); //Close pipe input
         }else{
           //If last (first command from left), set StdInput to StdInput file, if one is given
           if(shellcmd -> rd_stdin){
             int fd = open(shellcmd->rd_stdin, O_RDONLY);
             dup2(fd,0);
             close(fd);
           }
         }
         
         int j = 0;
         for(j=0; j<cmd_count; j++){
           close(pipe_fd[j][0]);
           close(pipe_fd[j][1]);
         }

         if(execvp(cmd[0],cmd) == -1){
            printf("Command not found\n");
         }
     }  
  }
  
  if(pid){
     if((shellcmd -> background) == 0){       
       waitpid(pid,NULL,0); 
     }
     return 0;
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

