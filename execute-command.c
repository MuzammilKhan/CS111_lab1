// UCLA CS 111 Lab 1 command execution

#include "command.h"
#include "command-internals.h"

#include <error.h>
#include <stdlib.h> //for NULL

/* FIXME: You may need to add #include directives, macro definitions,
   static function definitions, etc.  */

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

     //random stuff so that make will work. otherwise gcc complains about unused parameters
     if(c != NULL && time_travel == time_travel)
     {
     	 error (1, 0, "command execution not yet implemented");
     }


  error (1, 0, "command execution not yet implemented");
}
