// UCLA CS 111 Lab 1 command reading

#include "alloc.h" //for checked_malloc
#include "command.h"
#include "command-internals.h"


#include <error.h>
#include <stdbool.h>
#include <stdio.h> //for EOF
#include <string.h>

int (*getbyte) (void*);
void* getbyte_arg;

struct command_stream_t
{
	char a[1000000]; //File.  TODO: figure out how big to make this
	int index; 
};


void strip_first_and_last(string &input) //strips the first and last char from string
{
	int limit = strlen(input);
	if(limit <= 2)
		{input = ""; return;}

	for(int i = 0; i < len; i++) //remove first char	
	{
		string[i] = string[i+1];
	}

	string[limit-2] = '\0'; //replaces last char

	return;
}

bool is_operator(char c) // check if the character is an operator
{
switch (c)
case ';':
case '|':
case '$': // $ == ||
case '*': // * == &&
return true;
default:
return false;
}

bool contains_operator(string input) //check if input contains an operator
{
	int limit = strlen(input);
	for(int i = 0; i <= len; i++) //remove first char	
	{
		if(is_operator)
		{return true;}
	}
	return false; //no operator found
}

command_t
parse(string input)
{
	command_t cmd = checked_malloc(sizeof(command));

	//Question: does the other pointers in the command struct have to be intilalized to NULL or are they already NULL?

	if(input[strleng(input)-1] == ')') //subshell case
	{ 
		cmd->type = SUBSHELL_COMMAND;
		strip_first_and_last(input); //removes brackets
		cmd->u.subshell = parse(input);
		return cmd;
	}
	else if(!containts_operator(input))
	{
		cmd->type = SIMPLE_COMMAND;
		//cmd->u.word = words? delimiter is space.
		return cmd;
	}
	else
	{

		/*
				find the least precende operator (not in a paired bracket) in string //TODO: possibly write a function to do this?
		cmd type = ";" sequence-command
					"&&" and-command
					"||" or-command
					"|" pipe command
		cmd.u.command[0] = parse(left half of the string)
		cmd.u.command[1] = parse(right half of the string)
		return command
	*/
	}

		//TODO:once we know if simple or subshell consider redirection -- also figure out where to do this
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

		 //check for comments and remove them
		if ((next == '#')   //TODO: what about ordinary token right before # ???? also check if this is correct
		{
			do
			{
				next = get_next_byte(get_next_byte_argument); 
			} while ((next > -1) && (next != EOF) && (next != '\n');
		}


		//buffer loading and resizing
		if(next > -1)
		{
			//load buffer
			buffer[count] = next;
			count++;

			//adjust buffer size
			if(count == buffer_size)
			{
				buffer = checked_grow_alloc(buffer, &buffer_size);
			}
		}
	} while (next > -1);


	//run parser on buffer and create command stream

	//TODO: REST

	free(buffer);

	//TODO: return command stream
}

command_t
read_command_stream(command_stream_t s)
{
	/* FIXME: Replace this with your implementation too.  */
	error(1, 0, "command reading not yet implemented");
	return 0;


}
