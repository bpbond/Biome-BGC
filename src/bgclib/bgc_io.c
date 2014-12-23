/*
 *
 *	bgc_io.c created 20041013 by Andrew Neuschwander
 *	Some basic I/O Functionality for the bgclib
 *
 *	Biome-BGC version 4.2 (final release)
 *	See copyright.txt for Copyright information
 *
 */

#include "bgc.h"

signed char bgc_verbosity = BV_DETAIL;
FILE *bgc_logfile = NULL;

int bgc_logfile_setup(char *logfile)
{
	extern FILE* bgc_logfile;
	
	bgc_logfile = fopen(logfile, "w");

	if (bgc_logfile == NULL)
	{
		bgc_printf(BV_ERROR, "Couldn't Open logfile for writing: '%s' (Error: %s)\n", logfile, strerror(errno));
		bgc_print_usage();
		exit(EXIT_FAILURE);
	}
	return 1;
}

int bgc_logfile_finish(void)
{
	extern FILE *bgc_logfile;
	if (bgc_logfile != NULL)
	{
		fflush(bgc_logfile);
		fclose(bgc_logfile);
	}
	return 1;
}

void bgc_print_usage(void)
{
	extern char *argv_zero;

	bgc_printf(BV_ERROR, "\nusage: %s {-l <logfile>} {-s | -v [0..4]} {-p} {-V} {-a} {-u | -g | -m} <ini file>\n\n", argv_zero);
	bgc_printf(BV_ERROR, "       -l <logfile> send output to logfile, overwrite old logfile\n"); 
	bgc_printf(BV_ERROR, "       -V print version number and build information and exit\n");
	bgc_printf(BV_ERROR, "       -p do alternate calculation for summary outputs (see USAGE.TXT)\n");
	bgc_printf(BV_ERROR, "       -a output ascii formated data\n");
	bgc_printf(BV_ERROR, "       -s run in silent mode, no standard out or error\n");
	bgc_printf(BV_ERROR, "       -v [0..4] set the verbosity level \n");
	bgc_printf(BV_ERROR, "           0 ERROR - only report errors \n");
	bgc_printf(BV_ERROR, "           1 WARN - also report warnings\n");
	bgc_printf(BV_ERROR, "           2 PROGRESS - also report basic progress information\n");
	bgc_printf(BV_ERROR, "           3 DETAIL - also report progress details (default level)\n");
	bgc_printf(BV_ERROR, "           4 DIAG - also print internal diagnostics\n");
	bgc_printf(BV_ERROR, "       -u Run in spin-up mode (over ride ini setting).\n");
	bgc_printf(BV_ERROR, "       -g Run in spin 'n go mode: do spinup and model in one run\n");
	bgc_printf(BV_ERROR, "       -m Run in model mode (over ride ini setting).\n");
	bgc_printf(BV_ERROR, "       -n <ndepfile> use an external nitrogen deposition file.\n");

}

signed char bgc_verbosity_decode(char *keyword)
{
	signed char verbosity = BV_ERROR;
	int i; 
	if (isdigit((char)(keyword[0])))
	{
		verbosity = (signed char)atoi(keyword);
		if (verbosity <= BV_SILENT)
		{
			return BV_SILENT;
		}
		else if (verbosity >= BV_DIAG)
		{
			return BV_DIAG;
		}
		else
		{
			return verbosity;
		}
	}
	else
	{
		/* 	Conversion to Upper case allows to use strncmp() instead of strncasecmp 
				strncasecmp() is in strings.h from BSD and isn't available on win32 */
		for(i=0; i<4 && i<(int)strlen(keyword); i++)
		{
			keyword[i] = toupper(keyword[i]);
		}
		
		if (strncmp("SILENT", keyword, 4) == 0)
		{
			return BV_SILENT;
		}
		else if (strncmp("ERROR", keyword, 4) == 0)
		{
			return BV_ERROR;
		}
		else if (strncmp("WARN", keyword, 4) == 0)
		{
			return BV_WARN;
		}
		else if (strncmp("PROGRESS", keyword, 4) == 0)
		{
			return BV_PROGRESS;
		}
		else if (strncmp("DETAIL", keyword, 4) == 0)
		{
			return BV_DETAIL;
		}
		else if (strncmp("DIAG", keyword, 4) == 0)
		{
			return BV_DIAG;
		}
		else
		{
			bgc_printf(BV_ERROR, "Unknown Verbosity Keyword: %s\n", keyword);
			bgc_print_usage();
			exit(EXIT_FAILURE);
		}
	}
}

#ifdef __USE_ISOC99
int _bgc_printf(const char *file, int line, signed char verbosity, const char *format, ...)
#else
int bgc_printf(signed char verbosity, const char *format, ...)
#endif
{
	extern FILE *bgc_logfile;
	extern signed char bgc_verbosity;
	va_list ap; /* For variadic argument processing see 'man 3 vfprintf' for details */
	int printed = 0;

#ifdef DEBUG
	/* needed to debug the verbosity settings */
	printf("Function Verbosity: %d\nRequested Verbosity: %d\n", verbosity, bgc_verbosity);
	fflush(stdout);
#endif

	if (bgc_verbosity <= BV_SILENT || bgc_verbosity < verbosity)
	{
		return 0;
	}

	va_start(ap, format);
	
	if (bgc_logfile != NULL)
	{
#ifdef __USE_ISOC99
		if (bgc_verbosity == BV_DIAG)
			fprintf(bgc_logfile, "In %s at line %i: ", file, line);
#endif
		printed = vfprintf(bgc_logfile, format, ap);
		fflush(bgc_logfile);

	}
	else if (verbosity <= BV_WARN)
	{
		printed = vfprintf(stderr, format, ap);
		fflush(stderr);
	}
	else
	{
#ifdef __USE_ISOC99
		if (bgc_verbosity == BV_DIAG)
			fprintf(stdout, "In %s at line %i: ", file, line);
#endif

		printed = vfprintf(stdout, format, ap);
		fflush(stdout);
	}

	va_end(ap);
	return printed;
}
