/* 
ini.c
Rudimentary file I/O functions

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Biome-BGCMuSo v5.0.
Original code: Copyright 2000, Peter E. Thornton
Numerical Terradynamic Simulation Group, The University of Montana, USA
Modified code: Copyright 2018, D. Hidy [dori.hidy@gmail.com]
Hungarian Academy of Sciences, Hungary
See the website of Biome-BGCMuSo at http://nimbus.elte.hu/bbgc/ for documentation, model executable and example input files.
*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
*/

#include <stdlib.h>
#include <stdio.h>

# include "ini.h"

/* file_open() is the generic file opening routine using the file structure
defined above */
int file_open (file *target, char mode)
/*	Possible values for mode
	'r' for read binary
    'i' for read ascii with error message
	'j' for read ascii without error message
    'w' for write binary
    'o' for write ascii
*/
{
	int ok=1;
	switch (mode)
	{
        case 'r':
            if ((target->ptr = fopen(target->name,"rb")) == NULL)
            {
                printf("Can't open %s for binary read\n",target->name);
                ok=0;
            }
            break;

        case 'i':
            if ((target->ptr = fopen(target->name,"r")) == NULL)
            {
                printf("Can't open %s for ascii read\n",target->name);
                ok=0;
            }
            break;
		case 'j':
            if ((target->ptr = fopen(target->name,"r")) == NULL)
            {
                ok=0;
            }
            break;

        case 'w':
            if ((target->ptr = fopen(target->name,"wb")) == NULL)
            {
                printf("Can't open %s for binary write\n",target->name);
                ok=0;
            }
            break;

        case 'o':
            if ((target->ptr = fopen(target->name,"w")) == NULL)
            {
                printf("Can't open %s for ascii write\n",target->name);
                ok=0;
            }
            break;

        default:
            printf("Invalid mode specification for file_open ... Exiting\n");
            ok=0;
    }
    return(!ok);
}

/* scan_value is the generic ascii input function for use with text
initialization files. Reads the first whitespace delimited word on a line,
and discards the remainder of the line. Returns a pointer to value depending
on the specified scan type */
int scan_array (file ini, void *var, char type, int nl)
/* Possible values for type
    'i' for integer
    'd' for double
    's' for string
*/

{
    int ok_scan;
    int ok=1;

    switch (type)
    {
        case 'i':
			if (nl) ok_scan = fscanf(ini.ptr, "%d%*[^\n]",(int*)var);
			else ok_scan = fscanf(ini.ptr, "%d",(int*)var);
            if (ok_scan == 0 || ok_scan == EOF) 
			{
				printf("Error reading int value from %s \n",ini.name);
				ok=0;
			}
            break;

        case 'd':
			if (nl) ok_scan = fscanf(ini.ptr, "%lf%*[^\n]",(double*)var);
			else ok_scan = fscanf(ini.ptr, "%lf",(double*)var);
            if (ok_scan == 0 || ok_scan == EOF)
			{
				printf("Error reading double value from %s\n",ini.name);
				ok=0;
			}
            break;

        case 's':
			if (nl) ok_scan = fscanf(ini.ptr, "%s%*[^\n]",(char*)var);
			else ok_scan = fscanf(ini.ptr, "%s",(char*)var);
            if (ok_scan == 0 || ok_scan == EOF) 
			{
				printf("Error reading string value from %s\n",ini.name);
				ok=0;
			}
            break;

        default:
            printf("Invalid type specifier for scan_value ... Exiting\n");
            ok=0;
    }
    return(!ok);
}


int scan_value (file ini, void *var, char type)
{
	return scan_array (ini, var, type, 1);
}

/* combines scan_value with file_open for reading a filename from an
initialization file and then opening it with a specified access mode */
int scan_open (file ini,file *target,char mode)
/* Possible values for mode
	'r' for read binary
	'i' for read ascii
	'w' for write binary
	'o' for write ascii
*/
{	
	int ok=1;

	if (scan_value(ini,target->name,'s'))
	{
		printf("Error reading filename from %s... Exiting\n",ini.name);
		ok=0;
	}
	if (ok)
	{
		if (file_open(target,mode))
		{
			ok=0;
		}
	}
	return(!ok);
}

