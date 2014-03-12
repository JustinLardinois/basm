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
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define OUTPUT_NAME_MAX_LENGTH 1024

char* program_name;
char* input;
char output[OUTPUT_NAME_MAX_LENGTH];

void usage()
{
	fprintf(stderr,"Usage: %s SOURCE [-o OBJECT]\n",program_name);
	exit(EXIT_FAILURE);
}

bool streq(char* a , char* b)
{
	return strcmp(a,b) == 0;
}

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

int main(int argc , char** argv)
{
	program_name = argv[0];
	parse_args(argc,argv);
	printf("input file: %s\n",input);
	printf("output file: %s\n",output);
	char test[1024];
	sprintf(test,"           	dsafdwsfasd    	    ");
	printf("test string: \"%s\"\n",test);
	printf("trimmmed: \"%s\"\n",trim(test));
	exit(EXIT_SUCCESS);
}