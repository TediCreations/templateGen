﻿/******************************************************************************
	Code
******************************************************************************/

/******************************************************************************
	Include files
******************************************************************************/

#include "file_io.h"

#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <sys/time.h>
#include <stdlib.h>
#include <ctype.h>

#include <time.h>

/******************************************************************************
	Local definitions
******************************************************************************/

/******************************************************************************
	Local variables
******************************************************************************/

/******************************************************************************
	Local function declarations
******************************************************************************/

/******************************************************************************
	Function definitions
******************************************************************************/

/**
 * \brief Reads a file and puts characters in a char array.
 * 
 * User must free the result if result is non-NULL.
 * 
 * \param[in]	filename	The filename and path
 * 
 * \return	The character array. NULL if failed
 */
char* read_file( const char* filename )
{
	/* Read source file template */
	FILE* source_fp = fopen( filename, "r" );
	if ( source_fp == NULL )
	{
		printf( "[ERROR] File missing: " );
		printf( "%s", filename );
		printf( "\n" );
		return NULL;
	}
	
	/* Get the amount of characters in the file */
	fseek( source_fp , 0L , SEEK_END );// Go to the end of the file
	long lSize = ftell( source_fp );// Since we are at the end. Return is amount of characters
	rewind( source_fp );
	
	/* Allocate memory */
	char* buffer = calloc( 1, lSize+1 );
	if( buffer == NULL )
	{
		fclose( source_fp );
		printf("[ERROR] Memory allocation failed.c\n");
		return NULL;
	}

	/* Copy the file into the buffer */
	if( fread( buffer , lSize+1, 1 , source_fp ) == 1 )
	{
		fclose( source_fp);
		free( buffer );
		printf("[ERROR] Entire read failed.\n");
		return NULL;
	}
	fclose( source_fp );
	
	/* Exit with success */
	return buffer;
}

/**
 * \brief Replaces a sub_string inside a char array.
 * 
 * User must free the result if result is non-NULL.
 * 
 * \param orig
 * \param rep
 * \param with
 * 
 * \return *orig
 */
char* str_replace( char** orig, char *rep, char *with )
{
	char *result; // the return string
	char *ins;    // the next insert point
	char *tmp;    // varies
	size_t len_rep;  // length of rep (the string to remove)
	size_t len_with; // length of with (the string to replace rep with)
	size_t count;    // number of replacements

	// sanity checks and initialization
	if (!(*orig) && !rep)
		return NULL;
	len_rep = strlen(rep);
	if (len_rep == 0)
		return NULL; // empty rep causes infinite loop during count
	if (!with)
		with = "";
	len_with = strlen(with);

	// count the number of replacements needed
	ins = *orig;
	for ( count = 0; tmp = strstr(ins, rep); ++count )
	{
		ins = tmp + len_rep;
	}
	
	tmp = result = malloc( strlen( *orig) + (len_with - len_rep) * count + 1 );
	
	if (!result)
		return NULL;
	
	//	first time through the loop, all the variable are set correctly
	//	from here on,
	//		tmp points to the end of the result string
	//		ins points to the next occurrence of rep in orig
	//		orig points to the remainder of orig after "end of rep"
	while (count--)
	{
		ins = strstr(*orig, rep);
		size_t len_front = ins - *orig;// distance between rep and end of last rep
		tmp = strncpy(tmp, *orig, len_front) + len_front;
		tmp = strcpy(tmp, with) + len_with;
		*orig += len_front + len_rep; // move to next "end of rep"
	}
	
	strcpy( tmp, *orig );
	
	*orig = result;
	return result;
}

char* get_line(void)
{
	char * line = malloc(100), * linep = line;
	size_t lenmax = 100, len = lenmax;
	
	if(line == NULL)
		return NULL;

	for(;;)
	{
		int c = fgetc(stdin);
		if(c == EOF)
			break;
		if(c == '\n' )
			break;
		if(--len == 0)
		{
			len = lenmax;
			char * linen = realloc(linep, lenmax *= 2);
			
			if(linen == NULL)
			{
				free(linep);
				return NULL;
			}
			line = linen + (line - linep);
			linep = linen;
		}
		
		if((*line++ = c) == '\n')
			break;
	}
	*line = '\0';
	return linep;
}

int addStrings( char** buffer, const char* path, const char* name, const char* filetype )
{
	/* Get size os string and make it */
	size_t len_name = strlen( name );
	size_t len_path = strlen( path );
	size_t len_filetype = strlen( filetype );
	//free( *buffer );
	*buffer = malloc( len_path + len_name + len_filetype + 1 );//TODO: Check for +1
	if ( *buffer == NULL )
	{
		return 1;
	}
	/* Make it */
	strcpy( *buffer, path );
	strcat( *buffer, name );
	strcat( *buffer, filetype );
	
	/* Exit with success */
	return 0;
}

int make_template( char* date, char* filename, char* filepath, const char* template_file, const char* filetype )
{
	/* Read files */
	char* source = NULL;
	{
		char* buffer = NULL;
		if( addStrings( &buffer, "", template_file, filetype ) )
		{
			printf("[ERROR] Memory allocation failed.c\n");
			return 1;
		}
		source = read_file( buffer );
		if ( source == NULL )
		{
			printf("[ERROR] Memory allocation failed.c\n");
			return 1;
		}
	}
	/*
	 * Replace stuff
	 */
	 
	 /* $itemname$ */
	str_replace( &source, "$itemname$", filename );
	if ( source == NULL )
	{
		printf("[ERROR] Memory allocation failed.c\n");
		return 1;
	}
	
	/* $ITEMNAME$ */
	/* Find lenght of filename */
	size_t name_len = strlen( filename );
	char* filename_UPPER = (char *) malloc( name_len + 1 );//TODO: Check if +1 ommitted
	if( filename_UPPER == NULL )
	{
		printf("[ERROR] Memory allocation failed.c\n");
		return 1;
	}
	strcpy( filename_UPPER, filename );
	
	/* Calculate uppercase from lower case */
	char* tmp = filename_UPPER;
	while( *tmp != '\0' )
	{
		if( islower( *tmp ) )
		{
			*tmp = toupper( *tmp );
		}
		tmp++;
	}
	/* Replace in text */
	str_replace( &source, "$ITEMNAME$", filename_UPPER );
	if ( source == NULL )
	{
		printf("[ERROR] Memory allocation failed.c\n");
		return 1;
	}
	
	/* $time$ */
	str_replace( &source, "$time$", date );
	if ( source == NULL )
	{
		printf("[ERROR] Memory allocation failed.c\n");
		return 1;
	}
	
	/*
	 * Make new files
	 */	
	 {
		char* buffer = NULL;
		if( addStrings( &buffer, filepath, filename, filetype ) )
		{
			printf("[ERROR] Memory allocation failed.c\n");
			return 1;
		}
		FILE* fp = fopen( buffer, "w+" );
		free( buffer );
		if ( fp == NULL )
		{
			printf( "[ERROR] Could not open or create file: " );
			printf( "%s", buffer );
			printf( "\n" );
			return 1;
		}
		
		fprintf( fp,"%s", source );
		fclose(fp);
	 }
	 
	/* Garbage collection */
	free( source );
	source = NULL;
	
	/* Return with success */
	return 0; 
}