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

	for(int i = 0; i < limit-1; i++) //remove first char	
	{
      	  string[i] = string[i+1];  //copies up to and including ')'
	}

	string[limit-2] = '\0'; //replaces ')' with null byte

	return;
}

bool is_operator(char c) // check if the character is an operator
{
  switch (c) {
  case ';':
  case '|':
  case '$': // $ == ||
  case '*': // * == &&
    return true;
  default:
    return false;
  }
}

bool contains_operator(string input) //check if input contains an operator
{
    int limit = strlen(input);
    for(int i = 0; i < limit; i++) //remove first char	
    {      
      if(is_operator(input[i]))
	return true;
    }
    return false; //no operator found
}

char**  make_word_stream(string input) //make array of words  //TODO: check and test this
{
    int stream_size = 100 * sizeof(char*);
    int counter = 0;
    int word_count = 0;
    char** word_stream = checked_malloc(stream_size);

    char* ptr = &input;

    while(ptr != NULL)
    {
    	int letter_count = 0;
    	int max_word_size = 16;
    	char* word = (char *) checked_malloc(max_word_size);

    	while(ptr != ' ' && ptr != NULL) //make word
    	{
    		if(letter_count == max_word_size)
    		{
    			word = checked_grow_alloc(word, &max_word_size);
    		}
    		word[letter_count] = input[count];
    		letter_count++;
    		counter++;
    		ptr++;
    	}
    	word[letter_count] = '\0'; //insert zero byte at end of word

    	if(word_count == stream_size) 
    	{
    		word_stream = checked_grow_alloc(word_stream, &stream_size);
    	}
    	
    	word_stream[word_count] = word;
    	word_count++;
    	if(ptr != NULL)
    		{
    			ptr++;
    		}

    }

    return word_stream;
}

command_t
parse(string input)
{
	struct command_t cmd = checked_malloc(sizeof(struct command));

	
	if(input[strlen(input)-1] == ')') //subshell case
	{ 
		//TODO: set input and output
		cmd->type = SUBSHELL_COMMAND;
		cmd->status = -1;
		strip_first_and_last(input); //removes brackets
		cmd->u.subshell = parse(input);
		return cmd;
	}
	else if(!contains_operator(input))
	{
		//TODO: set input and output
		cmd->type = SIMPLE_COMMAND;
		cmd->status = -1;
		cmd->u.word = make_word_stream(input);
		return cmd;
	}
	else
	{
	  char operator;
	  int operator_found = 0;
	  char current_char;
	  int index = 0;
	  int open_bracket_count = 0;
	  int closed_bracket_count = 0;
		
		//find operator with greatest precedence in input
		while(input[index] != '\0')
		{
			current_char = input[index];
			if(current_char == '(')
			{
				open_bracket_count++;
			}
			else if(current_char == ')')
			{
				open_bracket_count--;
			}
			else if(is_operator(current_char) && open_bracket_count == 0)
			{
			  operator_found = 1;  // found an operator, so we must break the while loop
				
				switch(current_char)
				{
					case ';':
					  operator = ';';
					  break;
					case '|':
					  operator = '|';
					  break;
					case '*': //&&
					  operator = '*';
					  break;
					case '$': //||
					  operator = '$';
					  break;
					default:
					  operator_found = 0; // if not any of the above four operators...then not found
					  break;
				}
			}
			if (operator_found)
			  break;    //break before index increases again. Front half is 0 to index-1, right half is index+1 to strlen(input)-1

			index++;
		}

		switch(operator)
		{
		        case ';':
			  cmd->type = SEQUENCE_COMMAND;
			  break;
			case '|':
			  cmd->type = PIPE_COMMAND;
			  break;
			case '*': //&&
			  cmd->type = AND_COMMAND;
			  break;
			case '$': //||
			  cmd->type = OR_COMMAND;
			  break;
			default:
			  break;
		}


		//TODO: check if this works
		char left_half[index+1]; //need enough space for null byte
		strncpy(left_half, input, index - 1);
		left_half[index] = '\0';

		string left;
		memcpy(left, left_half, index);
		

		char right_half[strlen(input) - index + 1]; //need enough space for the null byte
		strncpy(right_half, input + index + 1, strlen(input) - index);
		right_half[strlen(input) - index] = '\0';


		string right;
		memcpy(right, right_half, index);

		cmd->u.command[0] = parse(left);
		cmd->u.command[1] = parse(right);
		cmd->status = -1;
		return cmd;


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
	char prev;
	size_t line_count = 0;

	//create and load buffer
	do
	{
		next = get_next_byte(get_next_byte_argument);
		
		//convert && to * and || to $
		if (count >= 1 && prev == '&' && next == '&') {
			count--;	//decrease count since converting from two-char to one char
			next = '*';
		} 
		else if (count >= 1 && prev == '|' && next == '|') {
			count--;
			next = '$';
		}

		 //check for comments and remove them
		if ((next == '#')   //TODO: what about ordinary token right before # ????
		{
			do
			{
				next = get_next_byte(get_next_byte_argument); 
			} while ((next > -1) && (next != EOF) && (next != '\n');
		}

		//buffer loading and resizing
		if(next > -1)
		{
			if ( isInvalidChar(next)) {		//check for bad characters: any other than a-zA-Z0-9 ! % + , - . / : @ ^ _  ; | && || ( ) < >
				fprintf(stderr, "%i: Invalid character\n", line_count);	//invalid character, return line number of error
			}
			if ( next == '\n')
				line_count++;	//keep track of line number to return proper errors
			
			//load buffer
			buffer[count] = next;
			count++;

			//adjust buffer size
			if(count == buffer_size)
			{
				buffer = checked_grow_alloc(buffer, &buffer_size);
			}
		}
		
		prev = next;
	} while (next > -1);

	struct command_stream* resultStream;
	resultStream->a = buffer;
	resultStream->index = count;

	//TODO: REST

	free(buffer);
	
	return resultStream;

	//TODO: return something
}

command_t
read_command_stream(command_stream_t s)
{
	/* FIXME: Replace this with your implementation too.  */
	error(1, 0, "command reading not yet implemented");
	return 0;


}

void free_command_stream() //TODO
{
	return;
}

void free_word_stream() //TODO
{
	return;
}
