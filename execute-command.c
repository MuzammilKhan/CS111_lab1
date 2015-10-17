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

int
command_status (command_t c)
{
  return c->status;
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
    fprintf(stderr, "simple command\n");
    pid_t pid;
    int status;
    if ( !(pid=fork()) ) {
      fprintf(stderr, "executing %s\n", c->u.word[0]);
      execvp(c->u.word[0], c->u.word);
    } 
    else {
      waitpid(pid, &status, 0);
      fprintf(stderr, "finished waiting\n");
      c->status = WEXITSTATUS(status);
      //exit(c->status);
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
      if (!(right = fork())) {
	execute_command(c->u.command[1], time_travel);
	exit(c->u.command[1]->status);
      }
      else {
	int retpid = waitpid(-1, &status, 0);
	if (retpid == right) {
	  c->status = WEXITSTATUS(status);
	  waitpid(left, &status, 0);
	}
	else {
	  waitpid(right, &status, 0);
	  c->status = WEXITSTATUS(status);
	}

      }
    }
    break;
  }
  case PIPE_COMMAND: {
    fprintf(stderr, "PIPE COMMAND\n");
    int pipefd[2], left = 0, right = 0;
    pipe(pipefd);
    fprintf(stderr, "finished pipe syscall\n");
    if(!(left = fork()))
    {
      dup2(1,pipefd[1]);
      close(pipefd[0]);
      fprintf(stderr, "entering left command\n");
      execute_command(c->u.command[0], time_travel);  //TODO: flag part?
      fprintf(stderr, "executed left command\n");
      exit(c->u.command[0]->status);
    }
    else
    {
      if(!(right = fork()))
      {
        dup2(0,pipefd[0]);
        close(pipefd[1]);
	      fprintf(stderr, "entering right command\n");
        execute_command(c->u.command[1], time_travel);
	      fprintf(stderr, "executed right command\n");
        exit(c->u.command[1]->status);
      }
      else
      {
        int status = 0;
	      int retpid = waitpid(-1, &status, 0); //wait for -1, meaning any child process
        fprintf(stderr, "waited for one\n");
	      if(retpid == right)
        {
	        fprintf(stderr,"waiting for left\n");
          c->status = WEXITSTATUS(status);
          waitpid(left, &status, 0);
	        fprintf(stderr, "waited for left\n");
	     	  exit(c->u.command[0]->status);
        }
        else
        {
	        fprintf(stderr,"waiting for right\n");
          c->status = WEXITSTATUS(status); //changed order for testing
          waitpid(right, &status, 0);
	        fprintf(stderr,"waited for right\n");
	       exit(c->u.command[1]->status);
        }
      }

    }
    break;
  }
  case AND_COMMAND: {
    fprintf(stderr,"ANDCOMMAND\n");
    int left = 0, right = 0, status;
    if(!(left = fork()))
    {
      execute_command(c->u.command[0], time_travel);  //TODO: flag part?
      exit(c->u.command[0]->status);
    }
    else
    {
      waitpid(left, &status, 0);
      fprintf(stderr,"status is: %i\n", status);
      if(!status) //if exit status is 0
      {
       if(!(right = fork()))
        {
          execute_command(c->u.command[1], time_travel);
          exit(c->u.command[1]->status);
        }
      }
    }
    break;
  }
  case OR_COMMAND: {
    fprintf(stderr, "ORCOMMAND\n");
    int left = 0, right = 0, status;
    if(!(left = fork()))
    {
      execute_command(c->u.command[0], time_travel);  //TODO: flag part?
      exit(c->u.command[0]->status);
    }
    else
    {
      waitpid(left, &status, 0);
      fprintf(stderr, "status is: %i\n", status);
      if(status) //if exit status is not 0
      {
       if(!(right = fork()))
        {
          execute_command(c->u.command[1], time_travel);
          exit(c->u.command[1]->status);
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
