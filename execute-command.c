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


  switch(c->type) {
  case SIMPLE_COMMAND:
    pid_t pid;
    int status;
    if ( !(pid=fork()) ) {
      execvp(c->word[0], c->word);
    } 
    else {
      waitpid(pid, &status, 0);
      c->status = WEXITSTATUS(status);
    }
    break;

  case SUBSHELL_COMMAND:
    break;

  case SEQUENCE_COMMAND:
    break;

  case PIPE_COMMAND:
    break;

  case AND_COMMAND:
    break;
  
  case OR_COMMAND:
    break;
  default:
    break;
  }


}
