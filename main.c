// UCLA CS 111 Lab 1 main program

#include <errno.h>
#include <error.h>
#include <getopt.h>
#include <stdio.h>

#include "command.h"

static char const *program_name;
static char const *script_name;

static void
usage (void)
{
  error (1, 0, "usage: %s [-pt] SCRIPT-FILE", program_name);
}

static int
get_next_byte (void *stream)
{
  return getc (stream);
}

int
main (int argc, char **argv)
{
  int opt;
  int command_number = 1;
  int print_tree = 0;
  int time_travel = 0;
  program_name = argv[0];

  for (;;)
    switch (getopt (argc, argv, "pt"))
      {
      case 'p': print_tree = 1; break;
      case 't': time_travel = 1; break;
      default: usage (); break;
      case -1: goto options_exhausted;
      }
 options_exhausted:;

  // There must be exactly one file argument.
  if (optind != argc - 1)
    usage ();

  script_name = argv[optind];
  FILE *script_stream = fopen (script_name, "r");
  if (! script_stream)
    error (1, errno, "%s: cannot open", script_name);
  command_stream_t command_stream =
    make_command_stream (get_next_byte, script_stream);

  command_t last_command = NULL;
  command_t command;

  //If time travel option is set make dependency graph
  int** graph; 
  if(time_travel)
  {
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

  }
  //else run the following code


  while ((command = read_command_stream (command_stream)))
    {
      if (print_tree)
	{
	  printf ("# %d\n", command_number++);
	  print_command (command);
	}
      else
	{
	  last_command = command;
	  execute_command (command, time_travel);
	}
    }

  return print_tree || !last_command ? 0 : command_status (last_command);
}
