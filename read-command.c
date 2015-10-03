// UCLA CS 111 Lab 1 command reading

#include "alloc.h" //for checked_malloc
#include "command.h"
#include "command-internals.h"


#include <error.h>
#include <stdbool.h>
#include <stdio.h> //for EOF

int (*getbyte) (void*);
void* getbyte_arg;

struct command_stream
{
	//root of the tree
	command_t root;
};

enum command_type cmdtype(char* buffer)
{


}

command_stream_t
make_command_stream(int(*get_next_byte) (void *),
                    void *get_next_byte_argument)
{
	/* FIXME: Replace this with your implementation.  You may need to
	   add auxiliary functions and otherwise modify the source code.
	   You can also use external functions defined in the GNU C Library.  */
	//error (1, 0, "command reading not yet implemented");
	//return 0;

	size_t count = 0;
	size_t buffer_size = 1024;
	char* buffer = checked_malloc(buffer_size);
	char next;

	//create and load buffer
	do
	{
		next = get_next_byte(get_next_byte_argument); 

		/* //check for comments and remove them
		if ((next == '#')   //TODO: what about ordinary token right before # ????
		{
			do
			{
				next = get_next_byte(get_next_byte_argument); // should these func names be different
			} while ((next > -1) && (next != EOF) && (next != '\n');
		}*/


		//buffer loading and resizing
		if(next > -1)
		{
			//load buffer
			buffer[count] = next;
			count++;

			//adjust buffer size
			if(count == buffer_size)
			{
				buffer_size = buffer_size * 2;
				buffer = checked_grow_alloc(buffer, &buffer_size);
			}
		}
	} while (next > -1);


	//TODO: REST

	free(buffer);

	//TODO: return something
}

command_t
read_command_stream(command_stream_t s)
{
	/* FIXME: Replace this with your implementation too.  */
	error(1, 0, "command reading not yet implemented");
	return 0;


}
