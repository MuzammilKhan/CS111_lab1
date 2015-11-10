// UCLA CS 111 Lab 1 command interface

struct command_stream
{
  int total_cases;
  int cur_case;
  char** forest; // keep a command forest                                                                                                                                                   
};

typedef struct command *command_t;
typedef struct command_stream *command_stream_t;

/* Create a command stream from LABEL, GETBYTE, and ARG.  A reader of
   the command stream will invoke GETBYTE (ARG) to get the next byte.
   GETBYTE will return the next input byte, or a negative number
   (setting errno) on failure.  */
command_stream_t make_command_stream (int (*getbyte) (void *), void *arg);

/* Read a command from STREAM; return it, or NULL on EOF.  If there is
   an error, report the error and exit instead of returning.  */
command_t read_command_stream (command_stream_t stream);

/* Makes a command tree out of a c-string */
command_t parse(char* input);

/* Print a command to stdout, for debugging.  */
void print_command (command_t);


void parseReadWriteFiles (command_t c, char** readFiles, int* readIndex, char** writeFiles, int* writeIndex);

/* Executes commands in time travel mode*/
void execute_command_time_travel(command_stream_t stream);

/* Execute a command.  Use "time travel" if the integer flag is
   nonzero.  */
void execute_command (command_t, int);

/* Return the exit status of a command, which must have previously been executed.
   Wait for the command, if it is not already finished.  */
int command_status (command_t);


/* design project - update max number of subprocesses */
void update_subprocess_limit(int limit);

void increment_subprocess_count(int num_processes_needed);
void decrement_subprocess_count(int num_processes);

int count_processes_needed(command_t c);
