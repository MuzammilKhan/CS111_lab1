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

bool isValidWordChar(char c);

struct command_stream
{
	char* a; //File. Make char* instead of char[] to allow assignment
        int total_cases;
        int cur_case;
	char** forest; // keep a command forest
};

void  split_forest(struct command_stream* resultStream, char* buffer, int buffer_length) {
  char** forest = resultStream->forest;
  int buffer_index = 0;
  int case_index = 0;
  int char_index = 0;
  int found_semicolon = false;
  int first_semicolon_index = -1;
  
  while (buffer_index < buffer_length) {
    if (buffer[buffer_index] == EOF)
      break;

    resultStream->forest[case_index][char_index] = buffer[buffer_index];
    if (buffer[buffer_index] != '~' && buffer[buffer_index] != ';')
      found_semicolon = false;
    if (!found_semicolon && (buffer[buffer_index] == '~' || buffer[buffer_index] == ';')) {
      found_semicolon = true;
      first_semicolon_index = char_index;
    }
    else if (found_semicolon  && (buffer[buffer_index] == '~' || buffer[buffer_index] == ';')) { //start next case                                                 
      resultStream->forest[case_index][first_semicolon_index] = '\0'; 
      case_index++;
      char_index = -1;
      found_semicolon = false;
    }
    buffer_index++;
    char_index++;
  }

  resultStream->forest[case_index][char_index] = '\0';
  resultStream->total_cases = case_index;

  return;

}


int charArrLen(char** str_array) {
	if (str_array == NULL)
		return 0;
	int i = 0;
	while (str_array[i] != NULL) {
		i++;
	}
	return i;
}
/*
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
	
	if (result_word_index == -1) // no '<'/'>' found, that means no input/output
		return NULL;
	
	//if symbol is last char in the word, look at next word
	if (result_char_index == (int)(strlen(str_array[result_word_index]))-1 ) {
		if (result_word_index != charArrLen(str_array)-2)	//no more words after the symbol or multiple word in input/output
			return NULL; // error
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
		return NULL; // error
		
	int index_after_symbol = result_char_index;
	int counter = 0;
	char* subword_after_symbol;
	subword_after_symbol = checked_malloc(strlen(str_array[result_word_index]));
	
	//copy in the output word, ready for returning
	for ( ; index_after_symbol < (int)strlen(str_array[result_word_index]); index_after_symbol++) {
		subword_after_symbol[counter] = str_array[result_word_index][index_after_symbol];
		counter++;
	}
	//manually terminate string
	subword_after_symbol[counter] = '\0';
	
	//remove the input/output from str_array (removing one string)
	str_array[charArrLen(str_array)-1] = NULL;
	
	return subword_after_symbol;
}
*/

char* returnInputOutput( char* input, char delimiter) {

  int char_index = 0;
  int total_length = strlen(input);

  while ( char_index < total_length) {
    if (input[char_index] == delimiter)
      break;
    char_index++;
  }

  bool no_input_output = false;
  if (char_index == total_length-1 || char_index == total_length)
    no_input_output = true;

  int new_index;
  int counter = 0;
  char* inputOutput = (char*) checked_malloc(total_length);
  bool keep_removing_spaces = true;
  for (new_index = char_index + 1; new_index < total_length; new_index++) {
    if (isValidWordChar(input[new_index]))
      keep_removing_spaces = false;
    if (!keep_removing_spaces && input[new_index] != '~') { //strip initial spaces;
      inputOutput[counter] = input[new_index];
      counter++;
    }
  }
  
  inputOutput[counter] = '\0';
  input[char_index] = '\0';

  //strip final spaces
  int index_from_end = counter - 1;
  while (inputOutput[index_from_end] == ' ' || inputOutput[index_from_end] == '\n' || inputOutput[index_from_end] == '\t') {
    inputOutput[index_from_end] = '\0';
    index_from_end--;
  }

  index_from_end = char_index - 1;
  while (input[index_from_end] == ' ' || input[index_from_end] == '\n' || input[index_from_end] == '\t') {
    input[index_from_end] = '\0';
    index_from_end--;
  }

  //remove tildas and initial spaces from index
  keep_removing_spaces = true; // reuse from above
  int temp_index = 0;
  int temp_counter = 0;
  while (input[temp_index] != '\0') {
    if (isValidWordChar(input[temp_index])) {
      keep_removing_spaces = false;
    }
    if (input[temp_index] != '~' && !keep_removing_spaces) {
      input[temp_counter] = input[temp_index];
      temp_counter++;
    }
    temp_index++;
  }
  input[temp_counter] = '\0';

  //update the input we want, and delete the old input

  if (no_input_output)
    return NULL;
  return inputOutput;
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

bool isValidWordChar (char c)
{
	if ( (c >= 65 && c <= 90)     //check for A-Z
       || (c >= 97 && c <= 122) //check for a-z
       || (c >= 48 && c < 57)   //check for 0-9
       || c == '!' || c == '%' || c == '+'
       || c == ',' || c == '-' || c == '.'
       || c == '/' || c == ':' || c == '@'
       || c == '^' || c == '_')
		{return true;}
	return false;
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

    	while(input[input_index] != ' ' && input[input_index] != '\t' && input[input_index] != '\0') //make word
    	{
    		if(letter_count == max_word_size)
    		{
    		    word = checked_grow_alloc(word, &max_word_size);
    		}
    		word[letter_count] = input[input_index];
    		letter_count++;
    		input_index++;
    	}
		if (input[input_index] == ' ' || input[input_index == '\t'])
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
	if (strlen(input) == 0)
		return false;
	
	int index = 0;
	bool open_bracket_found = false;
	bool char_after_closed_bracket = false;
	int open_bracket_count = 0;
	int closed_bracket_count = 0;
	char current;

	while(input[index] != '\0' && input[index] != EOF)
	{
		current = input[index];
		if(!open_bracket_found && current != ' ' && current != '\t' && current != '\n') 
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

	
	if(is_subshell(input)) //subshell case
	{ 
	  	//set input and output
	  	// MUST SET OUTPUT FIRST, OR ELSE WILL NOT WORK. OUTPUT IS ALWAYS WRITTEN
	  	// AFTER INPUT IN LAB SPECS, SO MUST BE CHECKED FIRST
	  	cmd->output =  returnInputOutput(input, '>');
	  	cmd->input = returnInputOutput(input, '<');
	  	
                char** str_array = make_word_stream(input);

		cmd->type = SUBSHELL_COMMAND;
		cmd->status = -1;
		strip_first_and_last(input); //removes brackets
		cmd->u.subshell_command = parse(input);
		return cmd;
	}
	else if(!contains_operator(input)) //simple command
	{
	    	//set input and output
	  	// MUST SET OUTPUT FIRST, OR ELSE WILL NOT WORK. OUTPUT IS ALWAYS WRITTEN
	  	// AFTER INPUT IN LAB SPECS, SO MUST BE CHECKED FIRST
	  	cmd->output =  returnInputOutput(input, '>');
	  	cmd->input = returnInputOutput(input, '<');

		char** str_array = make_word_stream(input);

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
					  if (1) {    //if operator is not ;, since left right associativity
					    operator = ';';
					    operator_index = index;
					  }
					  break;
					case '*': //&&
					  if (operator != ';') {    //if operator ';' hasn't been set yet
					    operator = '*';
					    operator_index = index;
					  } 
					  break;
					case '$': //||
					  if (operator != ';') {    //if operator ';' hasn't been set yet
					    operator = '$';
					    operator_index = index;
					  }
					  break;
			            	case '|':
				          if (operator != ';' && operator != '*' && operator != '$') {    //left-right associativity                           
				            operator = '|';
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
		int size = strlen(input);

		//TODO: check if this works
		char left_half[size]; //need enough space for null byte
		strncpy(left_half, input, size);
		left_half[index] = '\0';

		char right_half[size]; //need enough space for the null byte
		strncpy(right_half, input + index + 1, size);
		right_half[strlen(input) - index] = '\0';

		cmd->u.command[0] = parse(left_half);
		cmd->u.command[1] = parse(right_half);
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
	size_t case_count = 0;
	size_t buffer_size = 1024;
	char* buffer = checked_malloc(buffer_size);
	char next;
	char prev;
	char prevprev;
	size_t line_count = 0;

	//variables used for syntax checking
	int open_paren_count = 0;
	int closed_paren_count = 0;
	bool input_redirect_hit = false; //<
	bool output_redirect_hit = false; //>
	bool operator_hit = false;
	bool operator_finished = false;
	bool word_present = false; //signifies that a word was just made or is being made 
	bool simple_command_present = false;


	char* new_buffer = (char*) checked_malloc(1000000);

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
		

		//PREPROCESSING

		//check for comments and remove them
		if (next == '#')   //TODO: what about ordinary token right before # ???? //TODO: are we supposed to let "'" through otherwise stuff like echo '#lol' will fail what about """
		{
			do //TODO: are we supposed to let "'" through otherwise stuff like echo '#lol' will fail what about """
			{
					next = get_next_byte(get_next_byte_argument); 
			} while ((next > -1) && (next != EOF) && (next != '\n'));
			
		}

                if (next != EOF && isInvalidChar(next)) {               //check for bad characters: any other than a-zA-Z0-9 ! % + , - . / : @ ^ _  ; | && || ( ) < >                                   
		  error(1,0,"%zu: Invalid character\n", line_count);      //invalid character, return line number of error                                                                        

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

		
      		else if (count >= 1 && prev == ')' && next == '\n') { //newline evaluated as ";"
			next = '~';
		}
		
		if (count >= 1 && (next == '\n')) //case for else b == word: newline = ";" //NOTE:testing this case 
		{
		  
			int i = count;
			
			while(i > 0) {
			  i--;
			  
			  if (isValidWordChar(buffer[i]) || buffer[i] == ')') {
			    next = '~';
			    break;
			  }
			  if (is_operator(buffer[i]) || buffer[i] == '<' || buffer[i] == '>' || buffer[i] == '(') {
			    next = '\n';
			    break;
			  }
     			}

			if (i == 0) {
			  next = '\n';
			}
		}

		if (count >= 1 && (prev == '~' || prev == ';') && (next == '~' || next == ';')) {
		        case_count++;
		}
		
		//END PREPROCESSING


		//SYNTAX CHECKING   //TODO:COMPLETE THIS

		// < > syntax error checks

                //if notice a non-word, but input/output symbols have not had suffix words, then return error                                                                                         
                if( (input_redirect_hit || output_redirect_hit) && (next != ' ' && next != '\n' && next != '\t' && !isValidWordChar(next))) {
                  error(1,0, "%zu, Invalid syntax2 char %c", line_count,next);
                }


		if(next == '<')
			{input_redirect_hit = true;}
		else if(next == '>')
			{output_redirect_hit = true;}

                //if notice a break, reset word_present                                                                                                                                                
                if( next == ';' || next == '~' || next == '\n') {
                  word_present = false;
		  simple_command_present = false;
		}

		//if a wordchar, word_present is true
		if (isValidWordChar(next)) {
		  word_present = true;
		  simple_command_present = true;
		}

		//if input/output symbols are read, immediately return error if there is no prefix word
		if ( (input_redirect_hit || output_redirect_hit) && !word_present) {
		  error(1,0, "%zu, Invalid syntax1", line_count);
		}

		//if notice a wordchar, then set input/output hit to false
		if(isValidWordChar(next))
		{
		  input_redirect_hit = false;
		  output_redirect_hit = false;
		}

		//operator related checks

		//special case
		bool transformed_operator = (prev == '|' && next == '$') || (prev == '&' && next == '*');

                //if notice a non-word, but operator symbols have not had suffix words, then return error                                                                                              
                if( operator_hit && ( !transformed_operator  && next != ' ' && next != '\n' && next != '\t' && !isValidWordChar(next))) {
                  error(1,0, "%zu, Invalid syntax4 prev %c next %c\n", line_count, prev, next);
                }

		if(is_operator(next))
		  operator_hit = true;

		//if no prefix command to operator
		if(is_operator(next) && !simple_command_present)
		{
		  error(1,0,"%zu: Invalid syntax3 prev %c next %c\n", line_count, prev, next);
		}

		//if operator_hit, and we see the start of a simple_command, then set operator_hit to false
		if(operator_hit == true && isValidWordChar(next)) {
		  operator_hit = false;
		}

		//if too many operators in a row return error
		if(count >= 2 && is_operator(next) && is_operator(prev) && is_operator(prevprev))
		{
		  error(1,0,"%zu: Invalid syntax5 prevprev %c prev %c next %c\n", line_count, prevprev, prev, next);    
		}

		//counter increments for paren syntax check after loop
		if(next == '(')
			{open_paren_count++;}
		else if (next == ')')
			{closed_paren_count++;}

		//END SYNTAX CHECKING  //ONE MORE SYNTAX CHECK AFTER LOOP


		
		//buffer loading and resizing
		if(next > -1)
		{
			if ( next == '\n' || next == '~') 
				line_count++;	//keep track of line number to return proper errors
			
			//load buffer
			buffer[count] = next;
			new_buffer[count] = next;
			count++;

			//adjust buffer size
			if(count == buffer_size)
			{
				buffer = checked_grow_alloc(buffer, &buffer_size);
			}
		}
		
                if(count >= 2)
		  {prevprev = prev;}

		prev = next;
	}while (next > -1);

	//paren syntax check
	if(open_paren_count != closed_paren_count)
	{error(1,0,"%zu: Invalid syntax\n", line_count);}


	struct command_stream* resultStream = (struct command_stream*) checked_malloc(sizeof(struct command_stream));
	//resultStream->a = (char*) checked_malloc(1000000); //TODO: adjust size
	//resultStream->a = buffer;
	resultStream->total_cases = 0; //will be set in split_forest
	resultStream->cur_case = 0;

	resultStream->forest = (char**) checked_malloc(500*sizeof(char*));
	size_t loop_counter;
	for ( loop_counter = 0; loop_counter < 500; loop_counter++) {
	  resultStream->forest[loop_counter] = (char*) checked_malloc(1000);
	}

      	split_forest(resultStream, new_buffer, count);

	return resultStream;
}


command_t
read_command_stream(command_stream_t s)
{
  if (s->cur_case > s->total_cases)
    return 0;

  struct command* cmd = parse(s->forest[s->cur_case]);
  
  s->cur_case++;
  return cmd;
}



void free_word_stream(char ** stream) //TODO: check this
{
	int i = 0;
	while(stream[i] != NULL) //free words
	{
		free(stream[i]);
		i++;
	}

	//free array holding words
	free(stream);
	return;
}

void free_command(struct command c)
{
	if(c.input != NULL)
	{
		free_word_stream(&(c.input));
	}
	if(c.output != NULL)
	{
		free_word_stream(&(c.output));
	}

	switch(c.type)
	{
		case AND_COMMAND:         // A && B
    	case SEQUENCE_COMMAND:    // A ; B
     	case OR_COMMAND:         // A || B
    	case PIPE_COMMAND:        // A | B

    	if(c.command[0] != NULL)
    	{
    		free_command(*(command[0]);
    	}

    	if(c.command[1] != NULL)
    	{
    		free_command(*(command[1]);
    	}

    	break;
    	case SIMPLE_COMMAND:      // a simple command
    	if(c.word != NULL)
    	{
    		free_word_stream(c.word);
    	}

    	break;
    	case SUBSHELL_COMMAND:    // ( A )
    	if( != NULL)
    	{
    		free_command(*(c.subshell_command));
    	}
    	break;
    	default:
    	break;
	}
}


void free_command_stream(struct command_stream_t stream) //TODO: check this
{
	//free buffer
	free(stream->a);

	//free forest
	int i = 0;
	while(stream->forest[i] != NULL)
	{
		//free tree

		free_command(stream->forest[i]);

		i++;
	}

	//free command stream struct
	free(stream->a);
	return;
}