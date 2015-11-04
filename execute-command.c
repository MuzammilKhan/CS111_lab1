// UCLA CS 111 Lab 1 command execution

#include "alloc.h"
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
#include <string.h> //strcmp
#include <fcntl.h>

int
command_status (command_t c)
{
  return c->status;
}

void
parseReadWriteFiles (command_t c, char** readFiles, int* readIndex, char** writeFiles, int* writeIndex) {

  if (c->input != NULL) {
    readFiles[(*readIndex)++] = c->input;
  }
  if (c->output != NULL) {
    writeFiles[(*writeIndex)++] = c->output;
  }

  switch(c->type) 
  {
  case AND_COMMAND:
  case SEQUENCE_COMMAND:
  case OR_COMMAND:
  case PIPE_COMMAND:
    {
      parseReadWriteFiles(c->u.command[0], readFiles, readIndex, writeFiles, writeIndex);
      parseReadWriteFiles(c->u.command[1], readFiles, readIndex, writeFiles, writeIndex);
    }
  case SIMPLE_COMMAND:
    {
      if(c->u.word != NULL && c->u.word[1] != NULL)
      {
        char* beg_read_list = c->u.word[1];
        /*while(beg_read_list != ' ')
        {
          beg_read_list++;
        }
        beg_read_list++;
        readFiles[(*readIndex)++] = beg_read_list;
	*/
      }
    }
  case SUBSHELL_COMMAND: 
    {
      parseReadWriteFiles(c->u.subshell_command, readFiles, readIndex, writeFiles, writeIndex);
    }
  default:
    ; // do nothing
  }
}

void
execute_command_time_travel (command_stream_t command_stream) {

  //allocate graph[num_commands][num_commands]                                                                                                              
  int num_commands = command_stream->total_cases;
  fprintf(stderr, "%i", num_commands);
  int** graph = (int**) checked_malloc(num_commands * sizeof(int *));
  int i,j;
  for(i = 0; i < num_commands; i++)
    {
      graph[i] = checked_malloc(num_commands * sizeof(int));
    }

  //setup of 2-D array that keeps track of whether elements in the graph were visted
  int** visited = (int**) checked_malloc(num_commands * sizeof(int *)); 
  for(i = 0; i < num_commands; i++)
    {
      visited[i] = checked_malloc(num_commands * sizeof(int));
    }
  for(i = 0; i <num_commands; i++)
  {
    for(j = 0; j < num_commands; j++)
    {
      visited[i][j] = 0;
    }
  }

  //array of readFiles, each readFile holds an array of strings
  char ***readFilesArray = (char***) checked_malloc(num_commands * sizeof(char**) );
  char*** writeFilesArray = (char***) checked_malloc(num_commands * sizeof(char**) );
  
  int* readIndex = (int *) checked_malloc(num_commands * sizeof(int));
  int* writeIndex = (int *) checked_malloc(num_commands * sizeof(int));

  for (i = 0; i < num_commands; i++) {
    readFilesArray[i] = (char**) checked_malloc( 100 * sizeof(char*) ); // enough room for 100 read files per command
    writeFilesArray[i] = (char**) checked_malloc( 100 * sizeof(char*) ); // enough room for 100 write files per command
    readIndex[i] = 0; //start each array at 0
    writeIndex[i] = 0; //start each array at 0
  }

  int n,m;

  for (i = 0; i < num_commands; i++) 
  {
    struct command* cmd = parse(command_stream->forest[i]);
    parseReadWriteFiles(cmd, readFilesArray[i], &readIndex[i], writeFilesArray[i], &writeIndex[i]);  //form the read/write Files for each command tree

    // SHOULD USE HASH MAP FOR BETTER COMPLEXITY
    // RIGHT NOW IS O(N^2), N IS TOTAL NUMBER OF READ/WRITE FILES

    //check if there are any dependencies
    for (j = 0; j < i; j++)
     {  //only need to check the command trees before i
      for(n = 0; n < readIndex[i]; n++) 
      {
	     for (m = 0; m < writeIndex[j]; m++) //fill in dependencies
        {
	       if (!strcmp(readFilesArray[i][n] , writeFilesArray[j][m])
	        || !strcmp(writeFilesArray[i][n] , writeFilesArray[j][m])
		|| !strcmp(writeFilesArray[i][n] , readFilesArray[j][m]) ) // strcmp returns zero for match
	       {
	         graph[i][j] = 1;
	       }
	       else
	       {
	         graph[i][j] = 0;
	       }
	}
      }
    }
  }

  int a,b;
  //TEST DEPENDENCY GRAPH
  for (a = 0; a < num_commands; a++) {
    for (b = 0; b < a; b++) {
      fprintf(stderr,"%i " ,graph[i][j]);
    }
    fprintf(stderr,"\n");
  }
  fprintf(stderr, "REACHED END");
  return;

  //Topological Sort stuff
  int sorted_commands_index[num_commands]; //array containing index of command once they are sorted
  int sorted_commands_index_size = 0;
  int col_sum = 0;

  //Topological Sort Rough Idea
  while(sorted_commands_index_size != num_commands)
  {
  for(j = 0; j < num_commands; j++)
   {
    if(!visited[0][j]) //if this node was visited skip it
    {
      col_sum = 0;
      for(i = 0; i < num_commands; i++)
      {
        if(!visited[i][j]) //if this node was visited skip it
        {
         col_sum += graph[i][j];  //sum up values in graph for the column 
        } 
      }
      if(!col_sum) //if the sum of the column is 0, then the node is independent, add to sorted array
      {
        sorted_commands_index[sorted_commands_index_size++] = j;
        for(m = 0; m < num_commands; m++)
        {
         visited[m][j] = 1;
        }
      }
    }
   }
  }

  //execute commands
  /*
  command_t last_command = NULL;
  for(i = 0; i < num_commands; i++)
  {
    last_command = command_stream->forest[sorted_commands_index[i]]; //command to be executed
    execute_command(last_command,1); //set time_travel = 1 
  }
  return !last_command ? 0 : command_status (last_command);
  */
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
