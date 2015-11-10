// UCLA CS 111 Lab 1 main program

#include <errno.h>
#include <error.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

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

int parse_ssize(const char *arg, int *result)
{
  char *end_arg;
  ssize_t val = strtol(arg, &end_arg, 0);
  if (*arg && !*end_arg) {
    *result = val;
    return 1;
  } else
    return 0;
}


int
main (int argc, char **argv)
{
  int opt;
  int command_number = 1;
  int print_tree = 0;
  int time_travel = 0;
  int limit_processes = false;
  int num_processes = -1;
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

  // There must be exactly one or two file argument.

  if (optind != argc - 1 && optind != argc - 2)
    usage ();

  script_name = argv[optind];

  //there are two arguments
  if (optind == argc - 2) {
    limit_processes = true;
    parse_ssize(argv[optind+1], &num_processes);
  }

  if (limit_processes && num_processes > 0) {
    printf("Running with %i processes\n", num_processes);
    update_subprocess_limit(num_processes);
  }
  else if (limit_processes && num_processes == 0) {
    error (1, errno, "Cannot run with 0 processes");
  }
  else {
    printf("Running with unlimited processes\n");
    update_subprocess_limit(-1);
  }

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
    execute_command_time_travel(command_stream);
    return 1;
  }
  else
  {
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
	    increment_subprocess_count(count_processes_needed(command));
	    execute_command (command, time_travel);
            decrement_subprocess_count(count_processes_needed(command));
	  }
      }
    
    return print_tree || !last_command ? 0 : command_status (last_command);
  }
}
