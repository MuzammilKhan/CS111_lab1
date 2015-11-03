// UCLA CS 111 Lab 1 command execution

#include "command.h"
#include "command-internals.h"

#include <error.h>
#include <stdlib.h> //for NULL

/* FIXME: You may need to add #include directives, macro definitions,
   static function definitions, etc.  */
#include <unistd.h> //for pid_t fork()
#include <sys/wait.h> //waitpid and WEXITSTATUS
#include <sys/types.h>
#include <stdio.h> // debugging
#include <fcntl.h>

int
command_status (command_t c)
{
  return c->status;
}

void
execute_command_time_travel (command_t c) {

  //allocate graph[num_commands][num_commands]                                                                                                              
  int num_commands = command_stream->total_cases;
  graph = (int*) checked_malloc(num_commands * sizeof(int *));
  for(int i = 0; i < num_commands; i++)
    {
      graph[i] = checked_malloc(num_commands * sizeof(int));
    }



  //allocate array where each element is a pointer to the beginning of the command tree                                                                     
  char* command_ptrs = (char*) checked_malloc(num_commands * sizeof(char*)); //has ptrs to command tree in forest                                           
  char* command_list = (char*) checked_malloc(num_commands * sizeof(char*)); //used to keep index of command                                                
  //NOTE: will use command_list to keep track of what ptr is what and then will move around the ptrs in command_ptrs as we topological sort                 


  //set ptrs in command_ptrs and ptr in command_list to point to the location of the corresponding command in the command_ptrs array                        
  int temp_index = 0;
  while ((command = read_command_stream (command_stream)))
    {
      command_ptrs[temp_index] = command_stream->forest[command_stream->cur_case];  //Question: do we have permissions here to look this up?                  
      command_list[temp_index] = &(command_ptrs[temp_index]);
    }


  //fill in the dependencies in graph(0's and 1's)                                                                                                          

  //topological sort and execute                                                                                                                            

  //    last_command = command;                                                                                                                       
  // execute_command (command, time_travel);                                                                                                          

  // return !last_command ? 0 : command_status (last_command); //check if this is correct

}


void
execute_command (command_t c, int time_travel)
{
  /* FIXME: Replace this with your implementation.  You may need to
     add auxiliary functions and otherwise modify the source code.
     You can also use external functions defined in the GNU C Library.  */

     //TODO: if time_travel is nonzero have to do something

  switch(c->type) {
  case SIMPLE_COMMAND: {   //need the brackets because labels cannot be immediately before non-statements
    //    fprintf(stderr, "simple command\n");
    pid_t pid;
    int status;
    if ( !(pid=fork()) ) {
      //      fprintf(stderr, "executing %s\n", c->u.word[0]);
      int fd;
      if(c->input != NULL)  //TODO: Currently testing this
      {
	//        fprintf(stderr, "setting input to %s\n", c->input);
        fd = open(c->input, O_RDONLY);
        if(dup2(fd, 0) < 0)
        {
          fprintf(stderr, "error in dup2 - input\n");
        } 
      }
      if(c->output != NULL)
      {
        //fprintf(stderr, "setting output to %s\n", c->output);
        fd = open(c->output, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR| S_IROTH | S_IWOTH);
        if(dup2(fd, 1) < 0)
          {
            fprintf(stderr, "error in dup2 - output\n");
          }
      }
      execvp(c->u.word[0], c->u.word);
      fprintf(stderr, "execvp failure\n");
    } 
    else {
      waitpid(pid, &status, 0);
      //fprintf(stderr, "finished waiting\n");
      c->status = WEXITSTATUS(status);
    }
    break;
  }
  case SUBSHELL_COMMAND: {
    pid_t pid;
    int status;
    if ( !(pid=fork())) {
      execute_command(c->u.subshell_command, time_travel);
      exit(c->u.subshell_command->status);
    }
    else {
      waitpid(pid, &status, 0);
      c->status = WEXITSTATUS(status);
    }

    break;
  }
  case SEQUENCE_COMMAND: {
    int left = 0, right = 0, status;
    if (!(left = fork())) {
      execute_command(c->u.command[0], time_travel);
      exit(c->u.command[0]->status);
    }
    else {
      waitpid(left, &status, 0);
      if (!(right = fork())) {
	       execute_command(c->u.command[1], time_travel);
	       exit(c->u.command[1]->status);
      }
      else {
	       int retpid = waitpid(right, &status, 0);
	       c->status = WEXITSTATUS(status);
      }
    }
    break;
  }
  case PIPE_COMMAND: {
    //fprintf(stderr, "PIPE COMMAND\n");
    int pipefd[2], left = 0, right = 0;
    if (pipe(pipefd) == -1) {
      fprintf(stderr, "Pipe failed\n");
    }
    //fprintf(stderr, "finished pipe syscall\n");
    if(!(left = fork()))
    {
      close(pipefd[0]);
      if (dup2(pipefd[1],1) == -1)
	      {fprintf(stderr, "cannot dup2\n");}
      //fprintf(stderr, "entering left command\n");
      execute_command(c->u.command[0], time_travel);  //TODO: flag part?
      //fprintf(stderr, "executed left command\n");
      exit(c->u.command[0]->status);
    }
    else
    {
      int status = 0;
      if(!(right = fork()))
      {
        close(pipefd[1]);
        if (dup2(pipefd[0],0) == -1)
          {fprintf(stderr,"cannot dup2\n");}
        //fprintf(stderr, "entering right command\n");
        execute_command(c->u.command[1], time_travel);
        //fprintf(stderr, "executed right command\n");
	      exit(c->u.command[1]->status);
      }
      else
      {
	       close(pipefd[0]);
	       close(pipefd[1]);
	       int retpid = waitpid(-1, &status, 0); //wait for -1, meaning any child process
	       //fprintf(stderr, "waited for one\n");
	       if(retpid == right)
         {
	   //      fprintf(stderr,"waiting for left\n");
           c->status = WEXITSTATUS(status);
           waitpid(left, &status, 0);
	   //      fprintf(stderr, "waited for left\n");
          //exit(c->status);
        }
        else
        {
          //fprintf(stderr,"waiting for right\n");
          waitpid(right, &status, 0);
          c->status = WEXITSTATUS(status);
	  // fprintf(stderr,"waited for right\n");
    //exit(c->status);
        }
      }

    }
    break;
  }
  case AND_COMMAND: {
    //fprintf(stderr,"ANDCOMMAND\n");
    int left = 0, right = 0, status;
    if(!(left = fork()))
    {
      execute_command(c->u.command[0], time_travel);  //TODO: flag part?
      exit(c->u.command[0]->status);
    }
    else
    {
      waitpid(left, &status, 0);
      //fprintf(stderr,"status is: %i\n", status);
      if(!status) //if exit status is 0
      {
       if(!(right = fork()))
        {
          execute_command(c->u.command[1], time_travel);
          exit(c->u.command[1]->status);
        }
       else {
	 waitpid(right, &status, 0);
	 c->status = WEXITSTATUS(status);
       }
      }
    }
    break;
  }
  case OR_COMMAND: {
    //fprintf(stderr, "ORCOMMAND\n");
    int left = 0, right = 0, status;
    if(!(left = fork()))
    {
      execute_command(c->u.command[0], time_travel);  //TODO: flag part?
      exit(c->u.command[0]->status);
    }
    else
    {
      waitpid(left, &status, 0);
      //fprintf(stderr, "status is: %i\n", status);
      if(status) //if exit status is not 0
      {
       if(!(right = fork()))
        {
          execute_command(c->u.command[1], time_travel);
          exit(c->u.command[1]->status);
        }
       else {
	 waitpid(right, &status, 0);
	 c->status = WEXITSTATUS(status);
       }
      }
    }
    break;
  }
  default:
    break;
  }

  return;

}
