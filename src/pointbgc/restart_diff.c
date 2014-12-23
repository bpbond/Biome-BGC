/*
 diff.c
 tool used to compare restart results from different versions of Biome-BGC

 Biome-BGC version 4.2 (final release)
 See copyright.txt for Copyright information

 restart_diff Version 1.0
 Last Modified: 11-16-2004
 By: Andrew
*/

#include "bgc.h"

int main(int argc, char *argv[])
{
	restart_data_struct restart0, restart1, restart2;
	int i = 0, nzcnt=0, run_mode = 0;
	double x = 0.0, rsum=0.0, sum=0.0;
	double *r0ptr=(double *)&restart0, *r1ptr=(double *)&restart1, *r2ptr=(double *)&restart2; /* nice hack */
	char *var[72];

	FILE *f1ptr, *f2ptr;

	if (argc < 2 || argc > 3)
	{
		printf("usage: %s  <restart file 1 name> [restart file 2 name]\n", argv[0]);
		exit(EXIT_FAILURE);
	}  

	run_mode=argc-1;

	/* initialize structures with a cheap point trick reused this entire program */
	for (i=0;i<71;i++)
		r0ptr[i] = r1ptr[i] = r2ptr[i] = 0.0;
	/* don't forget the final integer member of the struct */
	r0ptr[i] = r1ptr[i] = r2ptr[i] = 0;


	if ((f1ptr = fopen(argv[1],"rb")) == NULL)
	{
		printf("Can't open %s for binary read ... Exiting\n",argv[1]);
		exit(EXIT_FAILURE);
	} 
	else
	{
		if ((fread(&restart1, sizeof(restart_data_struct),1,f1ptr))==0)
		{
			printf("Didn't read binary data from %s ... Exiting\n",argv[1]);
			exit(EXIT_FAILURE);
		} 
		fclose(f1ptr); 
	}

	if (run_mode==2)
	{
		if ((f2ptr = fopen(argv[2],"rb")) == NULL)
		{
			fclose(f1ptr); /* quick clean up before exit */
			printf("Can't open %s for binary read ... Exiting\n",argv[2]);
			exit(EXIT_FAILURE);
		}
		else
		{
			if ((fread(&restart2, sizeof(restart_data_struct),1,f2ptr))==0)
			{
				printf("Didn't read binary data from %s ... Exiting\n",argv[2]);
				exit(EXIT_FAILURE);
			}
			fclose(f2ptr);
		}
	}

	/* initialize the variable names */
	i = 0;
	var[i++] = "soilw\t\t\t\t";
	var[i++] = "snoww\t\t\t\t";
	var[i++] = "canopyw\t\t\t\t";
	var[i++] = "lafc\t\t\t\t";
	var[i++] = "lafc_storag\t\t\t";
	var[i++] = "lafc_transfr\t\t\t";
	var[i++] = "frootc\t\t\t\t";
	var[i++] = "frootc_storag\t\t\t";
	var[i++] = "frootc_transfr\t\t\t";
	var[i++] = "livstmc\t\t\t\t";
	var[i++] = "livstmc_storag\t\t\t";
	var[i++] = "livstmc_transfr\t\t\t";
	var[i++] = "dadstmc\t\t\t\t";
	var[i++] = "dadstmc_storag\t\t\t";
	var[i++] = "dadstmc_transfr\t\t\t";
	var[i++] = "livcrootc\t\t\t";
	var[i++] = "livcrootc_storag\t\t";
	var[i++] = "livcrootc_transfr\t\t";
	var[i++] = "dadcrootc\t\t\t";
	var[i++] = "dadcrootc_storag\t\t";
	var[i++] = "dadcrootc_transfr\t\t";
	var[i++] = "grsp_storag\t\t\t";
	var[i++] = "grsp_transfr\t\t\t";
	var[i++] = "cwdc\t\t\t\t";
	var[i++] = "litr1c\t\t\t\t";
	var[i++] = "litr2c\t\t\t\t";
	var[i++] = "litr3c\t\t\t\t";
	var[i++] = "litr4c\t\t\t\t";
	var[i++] = "soil1c\t\t\t\t";
	var[i++] = "soil2c\t\t\t\t";
	var[i++] = "soil3c\t\t\t\t";
	var[i++] = "soil4c\t\t\t\t";
	var[i++] = "cpool\t\t\t\t";
	var[i++] = "lafn\t\t\t\t";
	var[i++] = "lafn_storag\t\t\t";
	var[i++] = "lafn_transfr\t\t\t";
	var[i++] = "frootn\t\t\t\t";
	var[i++] = "frootn_storag\t\t\t";
	var[i++] = "frootn_transfr\t\t\t";
	var[i++] = "livstmn\t\t\t\t";
	var[i++] = "livstmn_storag\t\t\t";
	var[i++] = "livstmn_transfr\t\t\t";
	var[i++] = "dadstmn\t\t\t\t";
	var[i++] = "dadstmn_storag\t\t\t";
	var[i++] = "dadstmn_transfr\t\t\t";
	var[i++] = "livcrootn\t\t\t";
	var[i++] = "livcrootn_storag\t\t";
	var[i++] = "livcrootn_transfr\t\t";
	var[i++] = "dadcrootn\t\t\t";
	var[i++] = "dadcrootn_storag\t\t";
	var[i++] = "dadcrootn_transfr\t\t";
	var[i++] = "litr1n\t\t\t\t";
	var[i++] = "litr2n\t\t\t\t";
	var[i++] = "litr3n\t\t\t\t";
	var[i++] = "litr4n\t\t\t\t";
	var[i++] = "soil1n\t\t\t\t";
	var[i++] = "soil2n\t\t\t\t";
	var[i++] = "soil3n\t\t\t\t";
	var[i++] = "soil4n\t\t\t\t";
	var[i++] = "sminn\t\t\t\t";
	var[i++] = "rtransn\t\t\t\t";
	var[i++] = "npool\t\t\t\t";
	var[i++] = "day_lafc_litfall_incrmnt\t";
	var[i++] = "day_frootc_litfall_incrmnt\t";
	var[i++] = "day_livstmc_turnovr_incrmnt\t";
	var[i++] = "day_livcrootc_turnovr_incrmnt\t";
	var[i++] = "annmax_lafc\t\t\t";
	var[i++] = "annmax_frootc\t\t\t";
	var[i++] = "annmax_livstmc\t\t\t";
	var[i++] = "annmax_livcrootc\t\t";
	var[i++] = "dsr\t\t\t\t";
	var[i++] = "metyr\t\t\t\t";


	if (run_mode==1)
	{
		printf("\nExecuting in Print Mode\n");
		printf("Variable\t\t\t\tData Value\n\n");

		for (i=0; i<71; i++)
			printf("%s%.8g\n",var[i], r1ptr[i]);   
			
		printf("%s%d\n", var[i], restart1.metyr);
	}


	if (run_mode==2) 
	{
		/* let's do some math */
		for (i=0; i<71;i++)
			sum += fabs(r0ptr[i] = r1ptr[i]-r2ptr[i]);
			
		r0ptr[i] = r1ptr[i]-r2ptr[i];

		printf("\nExecuting in Difference Mode\n");
		printf("Variable\t\t\tData 1 Value\tData 2 Value\tDifference\tdiff/data_ratio  ratio_sum\n\n");

		for (i=0; i<71; i++)
		{
			x = 0.0;

			if (r1ptr[i] != 0.0)
			{
				rsum += fabs(x = r0ptr[i] / r1ptr[i]);
				nzcnt++;
			}

			printf("%s%.8e\t%.8e\t%.8e\t%.8e\t%.8e\n",var[i], r1ptr[i], r2ptr[i], r0ptr[i], x, rsum);
		}
		
		printf("%s%d\n", var[i], restart1.metyr);
		printf("\nAbsolute Differences: \tSum: %.4g\t\tAverage: %.4g\n",sum, sum/nzcnt);
		printf("Absoulte Ratios: \tSum: %.4e\t\tAverage: %.4e\tNon-Zero-Count: %d\n",rsum, rsum/nzcnt, nzcnt);
	}

	return(EXIT_SUCCESS);
}


