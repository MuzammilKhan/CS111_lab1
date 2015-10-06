// UCLA CS 111 Lab 1 command reading

#include "alloc.h" //for checked_malloc
#include "command.h"
#include "command-internals.h"


#include <error.h>
#include <stdbool.h>
#include <stdio.h> //for EOF
#include <stdlib.h> //for free
#include <string.h>

int (*getbyte) (void*);
void* getbyte_arg;

struct command_stream
{
	char* a; //File. Make char* instead of char[] to allow assignment
	int index; 
	command** forest; // keep a command forest
};


int charArrLen(char** str_array) {
	if (str_array == NULL)
		return 0;
	int i = 0;
	while (str_array[i] != NULL) {
		i++;
	}
	return i;
}

char* returnInputOutput (char** str_array, char delimiter) {
// when delimiter == '<', this function checks for input
// when delimiter == '>', this function checks for output

	size_t line_offset = 0;
	size_t word_index = 0;
	size_t array_length = charArrLen(str_array);
	int result_word_index = -1;
	int result_char_index = -1;
	
	for( ; word_index < array_length; word_index++) {
		size_t char_index = 0;
		size_t word_length = strlen(str_array[word_index]);
		for ( ; char_index < word_length; char_index++) {
			if (str_array[word_index][char_index] == delimiter) {
				result_word_index = (int)word_index;
				result_char_index = (int)char_index;
			}
		}
	}
	
	if (result_index == -1) // no '<'/'>' found, that means no input/output
		return NULL;
	
	//if symbol is last char in the word, look at next word
	if (result_char_index == strlen(str_array[result_word_index])-1 ) {
		if (result_word_index != charArrLen(str_array)-2)	//no more words after the symbol or multiple word in input/output
			return -1; // error
		//remove the output from str_array (removing two strings)
		if (result_char_index == 0)	{//if '>' had no prefix
			str_array[charArrLen(str_array)-2] = NULL;
		}
		else  {	//be careful not to remove the prefix of '>'
			str_array[charArrLen(str_array)-1] = NULL;
			str_array[charArrLen(str_array)-2][result_char_index] = '\0';
		}
		return str_array[result_word_index + 1];
	}
	
	//if symbol is not last char in the word, return the chars after the symbol
	if (result_word_index != charArrLen(str_array)-1)	//no more words after the symbol or multiple word in input/output
		return -1; // error
		
	int index_after_symbol = result_char_index;
	int counter = 0;
	char* subword_after_symbol;
	subword_after_symbol = checked_malloc(strlen(str_array[result_word_index]));
	
	//copy in the output word, ready for returning
	for ( ; index_after_symbol < strlen(str_array[result_word_index]); index_after_symbol++) {
		subword_after_symbol[counter] = str_array[result_word_index][index_after_symbol];
		counter++;
	}
	//manually terminate string
	subword_after_symbol[counter] = '\0';
	
	//remove the input/output from str_array (removing one string)
	str_array[charArrLen(str_array)-1] = NULL;
	
	return subword_after_symbol;
}

bool isInvalidChar (char c) {
  if ( (c >= 65 && c <= 90)     //check for A-Z
       || (c >= 97 && c <= 122) //check for a-z
       || (c >= 48 && c < 57)   //check for 0-9
       || c == '!' || c == '%' || c == '+'
       || c == ',' || c == '-' || c == '.'
       || c == '/' || c == ':' || c == '@'
       || c == '^' || c == '_' || c == ';'
       || c == '|' || c == '&' || c == '('
       || c == ')' || c == '<' || c == '>' 
       || c == ' ' || c == '\t' || c == '\n'){
    return false;
  }
  return true;
}

void strip_first_and_last(char* input) //strips the first and last char from string
{
	int limit = strlen(input);
	if(limit <= 2)
	  {input = ""; return;}

	int i = 0;
	for( ; i < limit-1; i++) //remove first char	
	{
      	  input[i] = input[i+1];  //copies up to and including ')'
	}

	input[limit-2] = '\0'; //replaces ')' with null byte

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

bool contains_operator(char* input) //check if input contains an operator
{
    int limit = strlen(input);
    int i = 0;
    for( ; i < limit; i++) //remove first char	
    {      
      if(is_operator(input[i]))
	return true;
    }
    return false; //no operator found
}

char**  make_word_stream(char* input) //make array of words  //TODO: check and test this
{
    size_t stream_size = 100 * sizeof(char*);
    size_t word_count = 0;
    char** word_stream = checked_malloc(stream_size);

    size_t input_length = strlen(input);
    size_t input_index = 0;

    while(input_index < input_length)
    {
    	size_t letter_count = 0;
    	size_t max_word_size = 16;
    	char* word = (char *)checked_malloc(max_word_size);

    	while(input[input_index] != ' ' && input[input_index] != '\0') //make word
    	{
    		if(letter_count == max_word_size)
    		{
    		    word = checked_grow_alloc(word, &max_word_size);
    		}
    		word[letter_count] = input[input_index];
    		letter_count++;
    		input_index++;
    	}
		if (input[input_index] == ' ')
	 	 input_index++;  //increase the index to skip the space   //TODO: adjust for '\t' and need to consider multiple whitespaces

    		word[letter_count] = '\0'; //insert zero byte at end of word

    	if(word_count == stream_size) 
    	{
    	word_stream = checked_grow_alloc(word_stream, &stream_size);
    	}
    	
    	word_stream[word_count] = word;
    	word_count++;
    }

    word_stream[word_count] = NULL; //indicate end of array, for read_stream purposes

    return word_stream;
}



bool is_subshell(char* input) //checks if the input string is bounded by brackets
{
	int index = 0;
	bool open_bracket_found = false;
	bool char_after_closed_bracket = false;
	int open_bracket_count = 0;
	int closed_bracket_count = 0;
	char current;

	while(input[index] != '\0')
	{
		current = input[index];
		if(!open_bracketfound && current != ' ' && current != '\t' && current != '\n') //Question: should i take EOF into account?
		{
			return false;
		}

		if(current == '(')
			{open_bracket_count++;}
		else if (current == ')')
			{closed_bracket_count++;
				char_after_closed_bracket = false;}

		if(open_bracket_count == closed_bracket_count)
		{
			if(current != ')' && current != '(' && current != ' ' && current != '\t' && current != '\n')
			{
				char_after_closed_bracket = true;
			}
		}

		index++;
	}

	return !char_after_closed_bracket;
}


command_t
parse(char* input)
{
	struct command* cmd = checked_malloc(sizeof(struct command));

	
	if(issubshell(input)) //subshell case
	{ 
	  	char** str_array = make_word_stream(input);
	  
	  	//set input and output
	  	// MUST SET OUTPUT FIRST, OR ELSE WILL NOT WORK. OUTPUT IS ALWAYS WRITTEN
	  	// AFTER INPUT IN LAB SPECS, SO MUST BE CHECKED FIRST
	  	cmd->output =  returnInputOutput(temp, '>');
	  	cmd->input = returnInputOutput(temp, '<');
	  	
		cmd->type = SUBSHELL_COMMAND;
		cmd->status = -1;
		strip_first_and_last(input); //removes brackets
		cmd->u.subshell_command = parse(input);
		return cmd;
	}
	else if(!contains_operator(input)) //simple command
	{
	  	char** str_array = make_word_stream(input);
	  
	  	//set input and output
	  	// MUST SET OUTPUT FIRST, OR ELSE WILL NOT WORK. OUTPUT IS ALWAYS WRITTEN
	  	// AFTER INPUT IN LAB SPECS, SO MUST BE CHECKED FIRST
	  	cmd->output =  returnInputOutput(temp, '>');
	  	cmd->input = returnInputOutput(temp, '<');

 		cmd->type = SIMPLE_COMMAND;
		cmd->status = -1;
		cmd->u.word = str_array;
		return cmd;
	}
	else
	{
	  char operator = '\0';
	  int operator_index = 0;
	  char current_char;
	  int index = 0;
	  int open_bracket_count = 0;
		
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
				switch(current_char)
				{
					case ';':
					  if (operator != ';') {    //if operator is not ;, since left right associativity
					    operator = ';';
					    operator_index = index;
					  }
					  break;
					case '|':
					  if (operator != ';' && operator != '|') {    //left-right associativity
					    operator = '|';
					    operator_index = index;
					  }
					  break;
					case '*': //&&
					  if (operator == '\0') {    //if operator hasn't been set yet
					    operator = '*';
					    operator_index = index;
					  } 
					  break;
					case '$': //||
					  if (operator == '\0') {    //if operator hasn't been set yet
					    operator = '$';
					    operator_index = index;
					  }
					  break;
					default:
					  break;
				}
			}

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

		index = operator_index; //operator_index is the index of the lowest precedence operator

		//TODO: check if this works
		char left_half[index+1]; //need enough space for null byte
		strncpy(left_half, input, index - 1);
		left_half[index] = '\0';

		char* left;
		left = (char*) checked_malloc(index);
		memcpy(left, left_half, index);
		

		char right_half[strlen(input) - index + 1]; //need enough space for the null byte
		strncpy(right_half, input + index + 1, strlen(input) - index);
		right_half[strlen(input) - index] = '\0';


		char* right;
		right = (char*) checked_malloc(index);
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
		
		//check if newlines should be ; or spaces
		// PSEUDOCODE
		// char* prev
		// char* cur
		//
		// check if first and second word have semicolons
		//
		// prev = first word in buffer
		// cur = second word in buffer
		//
		// while cur does not have a semicolon
		// 	prev = cur
		// 	cur = next word
		//
		// if cur[0] is a semicolon
		// 	return prev
		// else
		// 	read cur up till the semicolon
		// 	return cur[0] up to the char before the semicolon
		
		// b ; a
		// use b to determine if \n is ; or space
		// if \n is ;, convert the \n to - (pseudo-semicolon)
		// if \n is space, convert the \n to =	(psuedo-space)
		

		//check for comments and remove them
		if (next == '#')   //TODO: what about ordinary token right before # ????
		{
			do
			{
				next = get_next_byte(get_next_byte_argument); 
			} while ((next > -1) && (next != EOF) && (next != '\n'));
		}

		//convert && to * and || to $
		if (count >= 1 && prev == '&' && next == '&') {
			count--;	//decrease count since converting from two-char to one char
			next = '*';
		} 
		else if (count >= 1 && prev == '|' && next == '|') {
			count--;
			next = '$';
		}
		else if (count >= 1 && prev == ';' && next == ';') { //TODO: look at this case again. 
			count--;
			next = '~';
		}
		else if (next == EOF)
		{
			next = '\0';
		}



		//buffer loading and resizing
		if(next > -1)
		{
			if ( isInvalidChar(next)) {		//check for bad characters: any other than a-zA-Z0-9 ! % + , - . / : @ ^ _  ; | && || ( ) < >
				fprintf(stderr, "%zu: Invalid character\n", line_count);	//invalid character, return line number of error
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

	struct command_stream* resultStream = (struct command_stream*) checked_malloc(sizeof(struct command_stream));
	resultStream->a = (char*) checked_malloc(1000000); //TODO: adjust size
	resultStream->a = buffer;
	resultStream->index = count;

	//TODO: REST

	free(buffer);
	
	return resultStream;

	//TODO: return something
}

void recursive_print(command_t cmd) {
  if (cmd->type == SIMPLE_COMMAND) {
    char** c_ptr = cmd->u.word;
    size_t word_index = 0;
    bool first_word = true;
    while (c_ptr[word_index] != NULL) {
      if (!first_word)  //print a before every word, except for the first word
	printf(" ");
      first_word = false;
      printf("%s", c_ptr[word_index]);
      word_index++;
    }

    if (cmd->input != NULL) {
      printf(" < %s", cmd->input);
    }
    if (cmd->output != NULL) {
      printf(" > %s", cmd->output);
    }

  } 
  else if (cmd->type == SUBSHELL_COMMAND) {
    printf("(");
    recursive_print(cmd->u.subshell_command);
    printf(")");
  }
  else {
    recursive_print(cmd->u.command[0]);

    switch(cmd->type) {
    case AND_COMMAND:
      printf("&&");
      break;
    case SEQUENCE_COMMAND:
      printf(";");
      break;
    case OR_COMMAND:
      printf("||");
      break;
    case PIPE_COMMAND:
      printf("|");
      break;
    default:
      break;
    }
    recursive_print(cmd->u.command[1]);
  }

}

command_t
read_command_stream(command_stream_t s)
{
  struct command* cmd= parse(s->a);
  recursive_print(cmd);
	/* FIXME: Replace this with your implementation too.  */
	
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
