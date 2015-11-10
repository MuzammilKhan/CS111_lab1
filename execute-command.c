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
#include <pthread.h> //for locking

#include <sys/mman.h>

//Lock function, creation of mutex 
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static int* subprocess_limit;
static int* subprocess_count;

//function for design lab
void update_subprocess_limit(int limit)
{
  subprocess_limit = mmap(NULL, sizeof *subprocess_limit, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  subprocess_count = mmap(NULL, sizeof *subprocess_count, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

  *subprocess_limit = limit;
  *subprocess_count = 0;
  return;
}

int count_processes_needed(command_t c) {
  switch(c->type) {
  case SIMPLE_COMMAND: {
    return 1;
  }
    break;
  case SUBSHELL_COMMAND: {
    return 1+count_processes_needed(c->u.subshell_command);
  }
    break;
  default: {
    return 1+count_processes_needed(c->u.command[0]) + count_processes_needed(c->u.command[1]);
  }
  }
}

void
increment_subprocess_count(int num_processes_needed)
{
  if (num_processes_needed > *subprocess_limit) {
    error(1, 0, "cannot execute command, need a subproccess limit of %i", num_processes_needed);
  }
  //  fprintf(stderr, "number of subprocesses before increment: %i\n", *subprocess_count);
  if(*subprocess_limit > 0)
    {
      while(*subprocess_count + num_processes_needed > *subprocess_limit) //busy loop till conditions are met
	{
    ;
  }
      pthread_mutex_lock(&mutex);
      *subprocess_count += num_processes_needed;
      fprintf(stderr, "number of subprocesses after increment: %i\n", *subprocess_count);
      pthread_mutex_unlock(&mutex);
    }
  return;
}

void
decrement_subprocess_count(int num_processes)
{
  //  fprintf(stderr, "number of subprocesses before decrement: %i\n", *subprocess_count);
  if(*subprocess_limit > 0)
    {
      pthread_mutex_lock(&mutex);
      *subprocess_count -= num_processes;
      fprintf(stderr, "number of subprocesses after decrement: %i\n", *subprocess_count);
      pthread_mutex_unlock(&mutex);
    }
  return;
}

//end design lab functions

int max(int a, int b) {
  if (a>b)
    return a;
  return b;
}

int
command_status (command_t c)
{
  return c->status;
}

void
parseReadWriteFiles (command_t c, char** readFiles, int* readIndex, char** writeFiles, int* writeIndex) {
  
  switch(c->type) 
  {
  case AND_COMMAND:
  case SEQUENCE_COMMAND:
  case OR_COMMAND:
  case PIPE_COMMAND:
    {
      parseReadWriteFiles(c->u.command[0], readFiles, readIndex, writeFiles, writeIndex);
      parseReadWriteFiles(c->u.command[1], readFiles, readIndex, writeFiles, writeIndex);
      break;
    }
  case SIMPLE_COMMAND:
    {
      if (c->input != NULL) {
	readFiles[(*readIndex)++] = c->input;
	//fprintf(stderr, "r+%s %i\n", c->input, *readIndex);
      }
      if (c->output != NULL) {
	writeFiles[(*writeIndex)++] = c->output;
	//fprintf(stderr, "w+%s %i\n", c->output, *writeIndex);
      }

      if(c->u.word != NULL)
      {
	int i = 1;
	while (c->u.word[i] != NULL) {
	  readFiles[(*readIndex)++] = c->u.word[i];
	  //fprintf(stderr, "r+%s %i\n", c->u.word[i], *readIndex);
	  i++;
        }
	/*while(beg_read_list != ' ')
        {
          beg_read_list++;
        }
        beg_read_list++;
        readFiles[(*readIndex)++] = beg_read_list;
	*/
      }
      break;
    }
  case SUBSHELL_COMMAND: 
    {
      if (c->input != NULL) {
	readFiles[(*readIndex)++] = c->input;
        //fprintf(stderr, "r+%s %i\n", c->input, *readIndex);
      }
      if (c->output != NULL) {
	writeFiles[(*writeIndex)++] = c->output;
        //fprintf(stderr, "w+%s %i\n", c->output, *writeIndex);
      }
      parseReadWriteFiles(c->u.subshell_command, readFiles, readIndex, writeFiles, writeIndex);
      break;
    }
  default:
    ; // do nothing
  }
}

void
execute_command_time_travel (command_stream_t command_stream) {

  //allocate graph[num_commands][num_commands]                                                                                                              
  int num_commands = command_stream->total_cases;
  //fprintf(stderr, "%i\n", num_commands);
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
    for (j = 0; j <= i; j++)
     {  //only need to check the command trees before i
       int nlimit = max(writeIndex[i], readIndex[i]);
      for(n = 0; n < nlimit; n++) 
      {
	int mlimit = max(writeIndex[j], readIndex[j]);
	for (m = 0; m < mlimit; m++) //fill in dependencies
        {
	  //fprintf(stderr, "n is %i m is %i i is %i j is %i\n", n, m, i, j);
	  if ( n < readIndex[i] && m < writeIndex[j]
	       && !strcmp(readFilesArray[i][n] , writeFilesArray[j][m]) ) {
	    graph[i][j] = 1;
	    //fprintf(stderr, "REACHED IF\n");
	  }

	  else if ( n < writeIndex[i] && m < writeIndex[j]
	       && !strcmp(writeFilesArray[i][n] , writeFilesArray[j][m]) ) {
	    graph[i][j] = 1;
	    //fprintf(stderr, "REACHED ELSEIF1\n");
	  }
	  
	  else if ( n < writeIndex[i] && m < readIndex[j]
		    && !strcmp(writeFilesArray[i][n] , readFilesArray[j][m]) ) {
	    graph[i][j] = 1;
	    //fprintf(stderr, "REACHED ELSEIF2\n");
	  }
	}
	//fprintf(stderr, "graph[%i][%i] is %i\n", i, j, graph[i][j]);
      }
    }
  }

  /*
  int a,b;
  //TEST DEPENDENCY GRAPH
  
  for (a = 0; a < num_commands; a++) {
    for (b = 0; b <= a; b++) {
      fprintf(stderr,"%i ", graph[a][b]);
    }
    fprintf(stderr,"\n");
  }
  */

  //TOPOLOGICAL SORT

  //will store the sorted order of the commands to be processed in order
  //first number n for each int array will indicate how many commands to run in parallel
  //next n numbers will be the commands that we run in parallel at this step
  //i.e. sortedOrder[1] = [3, 5, 6, 9]
  //this means 3 commands, (5,6,9), should be run in parallel at this time
  int** sortedOrder = (int**) checked_malloc (num_commands * sizeof(int*));
  int sortedStep = 0;
  for (i = 0; i < num_commands; i++) {
    sortedOrder[i] = (int*) checked_malloc ((num_commands+1) * sizeof(int*)); //each step can execute n commands, plus first index for command sum
  }


  int* dependentEdges = (int*) checked_malloc (num_commands * sizeof(int));

  //fill in dependent edges
  for (i = 0; i < num_commands; i++) {
    dependentEdges[i] = 0;
    for (j = 0; j < i; j++) { //must be strictly less than i, dont want to overcount by 1
      if (graph[i][j] == 1)
	dependentEdges[i]++;
    }
    //fprintf(stderr, "%i\n", dependentEdges[i]);
  }

  int processedCommands = 0;
  int* processedCommandsList = (int*) checked_malloc (num_commands * sizeof(int) * 2);
  //each turn we can execute commands with 0 dependencies
  while (processedCommands < num_commands)
  {
    int commandCounter = 0; 
    for (i = 0; i < num_commands; i++) {
      int alreadySeen = 0;
      int counter;
      //if the command already processed/seen, don't process it again
      for (counter = 0; counter < processedCommands; counter++) {
	if (processedCommandsList[counter] == i) {
	  alreadySeen = 1;
	}
      }
      
      //add counter to current step as well as overall counter
      if (alreadySeen == 0 && dependentEdges[i] == 0) {
	sortedOrder[sortedStep][1+commandCounter] = i;
	commandCounter++;
	processedCommandsList[processedCommands] = i;
	processedCommands++;
      }
    }
    sortedOrder[sortedStep][0] = commandCounter;

    //remove dependencies to processed commands                                                                                                                                         
    for (i = 1; i < sortedOrder[sortedStep][0]+1; i++) {
      int removedCommand = sortedOrder[sortedStep][i];
      for (j = removedCommand+1; j < num_commands; j++) {
	if (graph[j][removedCommand] == 1 && dependentEdges[j] > 0)
	  dependentEdges[j]--;
      }
    }
    sortedStep++;
  }

  //TEST
  /*
  for (i = 0; i < sortedStep; i++) {
    fprintf(stderr, "\nStep %i: ", i);
    for (j = 1; j < sortedOrder[i][0]+1; j++) {
      fprintf(stderr, "%i ", sortedOrder[i][j]);
    }
  }
  */

    //EXECUTE THE COMMANDS IN EACH STEP IN PARALLEL

  pid_t* processesToWait = (pid_t*) checked_malloc (num_commands * sizeof(pid_t));
  int processes_needed_count = 0;

  for (i = 0; i < sortedStep; i++) {
    for (j = 1; j < sortedOrder[i][0]+1; j++) {
      pid_t pid;
      command_t cmd = parse(command_stream->forest[sortedOrder[i][j]]);
      processes_needed_count = count_processes_needed(cmd);
      //fprintf(stderr,"num processes needed for command tree %i: %i\n", sortedOrder[i][j] , processes_needed_count+1);
      //increment_subprocess_count(processes_needed_count+1);
      fprintf(stderr, "command tree %i acquires %i process locks\n",sortedOrder[i][j] ,processes_needed_count);
      if (!(pid=fork())) {
          execute_command(cmd, 1);
        //  fprintf(stderr, "command tree %i releases %i process locks\n",sortedOrder[i][j] ,processes_needed_count+1);
        //  decrement_subprocess_count(processes_needed_count+1);
          exit(0);
      }
      else {
        processesToWait[j] = pid;
        //keep looping and forking children                                                                                                                                                 
      }
    }
    for (j = 1; j < sortedOrder[i][0]+1; j++) {
      int status;
      waitpid(processesToWait[j], &status, 0);
      //fprintf(stderr, "Waited for %i\n", j);
    }
  }

  return; //should return status of last command, something like what was done in the comment below

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
      increment_subprocess_count(1);
      execvp(c->u.word[0], c->u.word);
      fprintf(stderr, "execvp failure\n");
    } 
    else {
      waitpid(pid, &status, 0);
      decrement_subprocess_count(1);
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

