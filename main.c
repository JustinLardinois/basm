// main.c
// a component of basm
// latest version available at:
// https://github.com/JustinLardinois/ls-for-Windows
//
// Copyright 2014 Justin Lardinois
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//  http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
///////////////////////////////////////////////////////////////////////////

#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define EMPTY 0xFF
#define LINE_BUFFER_SIZE 1024
#define OUTPUT_NAME_MAX_LENGTH 1024

typedef unsigned char byte;

char* program_name;
char* input;
char output[OUTPUT_NAME_MAX_LENGTH];

// symbolic names of all BASIL opcodes, save for push which is a special case
char* ops[] = {
	NULL ,
	"and" ,
	"not" ,
	"or" ,
	"mul" ,
	"div" ,
	"add" ,
	"cmp" ,
	"pop" ,
	"swp" ,
	"dup" ,
	"ppc" ,
	"get" ,
	"put" ,
	"br" ,
	NULL
};

void usage()
{
	fprintf(stderr,"Usage: %s SOURCE [-o OBJECT]\n",program_name);
	exit(EXIT_FAILURE);
}

bool streq(char* a , char* b)
{
	return strcmp(a,b) == 0;
}

// figures out names of input and output files
void parse_args(int argc , char** argv)
{
	if(argc == 2)
	{
		input = argv[1];
		snprintf(output,OUTPUT_NAME_MAX_LENGTH-2,"%s",input);
		int length = strlen(output);
		if(output[length-4] == '.' && output[length-3] == 'a' // .asm extension
			&& output[length-2] == 's' && output[length-1] == 'm')
		{
			output[length-3] = 'o';
			output[length-2] = '\0';
		}
		else strcat(output,".o");
	}
	else if (argc == 4)
	{
		if(streq(argv[1],"-o"))
		{
			snprintf(output,OUTPUT_NAME_MAX_LENGTH,"%s",argv[2]);
			input = argv[3];
		}
		else if(streq(argv[2],"-o"))
		{
			input = argv[1];
			snprintf(output,OUTPUT_NAME_MAX_LENGTH,"%s",argv[3]);
		}
		else usage();
	}
	else usage();
}

// removes leading and trailing whitespace from a string
char* trim(char* s)
{
	if(s[0] == '\0') return s;
	else if(isspace(s[0])) return trim(s+1);
	else
	{
		int length = strlen(s);
		for(int i = length -1; i >= 0; i--)
		{
			if(isspace(s[i])) s[i] = '\0';
			else return s;
		}
	}
	return s;
}

// plugs comments (which start with a semicolon) with a null byte
char* trim_comment(char* s)
{
	char* semicolon = strchr(s,';');
	if(semicolon != NULL) *semicolon = '\0';
	return s;
}

// converts all characters in a string to lowercase
char* make_lower(char* s)
{
	for(int i = 0; s[i] != '\0'; i++)
	{
		s[i] = tolower(s[i]);
	}
	return s;
}

// checks if s begins with the contents of substr
bool starts_with(char* s , char* substr)
{
	for(int i = 0;;i++)
	{
		if(substr[i] == '\0') return true;
		else if(s[i] != substr[i]) return false;
	}
}

// reads an instruction line from the input file
// lines consisting solely of whitespace and/or comments are skipped
char* get_line(FILE* in , char* buffer)
{
	if(fgets(buffer,LINE_BUFFER_SIZE,in) == NULL) return NULL;
	char* line = make_lower(trim(trim_comment(buffer)));
	if(streq(line,"")) return get_line(in,buffer);
	else return line;
}

// returns the opcode for the symbolic instruction name passed in line
// if the instruction was push, immu is set to the operand
byte op_lookup(char* line , byte* immu)
{
	if(starts_with(line,"push"))
	{
		char* operand = strchr(line,'#');
		if(operand != NULL)
		{
			operand++;
			int value;
			if(sscanf(operand,"%d",&value) == 1)
			{
				if(value >= 0 && value <= 15)
				{

					*immu = value;
					return 0;
				}
			}
		}
	}
	else
	{
		for(int i = 1; ops[i] != NULL; i++)
		{
			if(streq(line,ops[i])) return i;
		}
	}
	fprintf(stderr,"%s: invalid expression \"%s\"\n",input,line);
	exit(EXIT_FAILURE);
}

// collects nibbles and outputs them to the output file
void assemble(FILE* in , FILE* out)
{
	char buffer[LINE_BUFFER_SIZE];
	char* line;
	byte upper_nibble = EMPTY;
	byte lower_nibble = EMPTY;
	while((line = get_line(in,buffer)) != NULL)
	{
		byte immu = 0;
		byte nibble = op_lookup(line,&immu);
		if(upper_nibble == EMPTY)
		{
			upper_nibble = nibble << 4;
			if(nibble == 0)
			{
				lower_nibble = immu;
				nibble = upper_nibble | lower_nibble;
				fwrite(&nibble,1,1,out);
				upper_nibble = EMPTY;
				lower_nibble = EMPTY;
			}
		}
		else
		{
			lower_nibble = nibble;
			nibble = upper_nibble | lower_nibble;
			fwrite(&nibble,1,1,out);
			if(lower_nibble == 0) upper_nibble = immu << 4;
			else upper_nibble = EMPTY;
			lower_nibble = EMPTY;
		}
	}
	if(upper_nibble != EMPTY)
	{
		fwrite(&upper_nibble,1,1,out);
	}
}

void die(char* file)
{
	fprintf(stderr,"%s: %s\n",file,strerror(errno));
	exit(EXIT_FAILURE);
}

int main(int argc , char** argv)
{
	setbuf(stdout,0);
	program_name = argv[0];
	parse_args(argc,argv);
	FILE* in = fopen(input,"r");
	if(in == NULL) die(input);
	
	
	FILE* out = fopen(output,"wb");
	// I had what was quite possibly the worst bug ever in this line.
	// Apparently, "w" is not sufficient on non-Unix C89 implementations
	// if you're writing binary data. You may wonder if people still use
	// such archaic systems. I do; it's called Windows 8.1.
	//
	// My problem was that if the hex byte 0A was fwritten (which is PUSH #10
	// in BASIL assembly) the byte 0D was written after it. After a lot of pain
	// and tears, I discovered that since 0A is a line feed (\n), a carriage
	// return (\r), which 0D represents, was being written after it. I solved
	// the bug by using "wb" instead of "w", which writes literal bytes.
	
	if(out == NULL) die(output);
	assemble(in,out);
	fclose(in);
	fclose(out);
	exit(EXIT_SUCCESS);
}