// gcc -c HSKA.c -o HSKA.o
// ar -q libHSKA.a HSKA.o
// CHAINE DE COMPIL   gcc simulateurOptiV0-3.c -Iinclude -Llib -lHSKA -o simOpti

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "include/HSKA.h"

#define TAILLE_LIGNE_MAX 	1000
#define STOP 				3
#define CONTRATS_PAR_VOL	100000
#define VOLUME 				0.3
#define MONTANT				8500
#define LEVIER				600
#define IDX_PIVOT_MAX 		300 		// ok version 0.3
#define IDX_TP_MAX 			300 		// ok version 0.3
#define ANNEE_DEB			6
#define ANNEE_FIN			15
#define DPIVOT 				601 		// ok version 0.3
#define TAKE_PROFIT			301			// ok version 0.3
#define STOP_LOSS			501
#define ABS_STOP_LOSS_MAX   10			// En realité le pas 

#define ANNEE				18		  	// ok version 0.3
#define MOIS 				12			// ok version 0.3
#define JOUR 				31			// ok version 0.3
#define MINUTE 				1500		// ok version 0.3
#define MINUTE_DEBUT		180
#define MINUTE_FIN			720
#define STRATEGIE_SL		1          // 1 = MAX ;   2 = MIN_ANNEE_NEG  ;  3 = MIN_SOMME_ANNEE_NEG ; 
/*
Lu ds fic de config
#define DP_MIN				-200		// ok version 0.3
#define DP_MAX				200
			// ok version 0.3
#define TPA_MIN				1			// ok version 0.3
#define TPA_MAX				200			// ok version 0.3
#define TPV_MIN				1			// ok version 0.3
#define TPV_MAX				200			// ok version 0.3
#define TP_MIN				1			// ok version 0.3
#define TP_MAX				200			// ok version 0.3
*/

#define MAX(A,B) (B > A ? B : A)
#define MIN(A,B) (B < A ? B : A)

typedef struct dataMinute dataMinute ;  // ok version 0.3
struct dataMinute
{
	double open    		;
	double high    		;
	double low     		;
	double close   		;
} ;


// detail du resultat de l'ordre de la journée 
typedef struct sGainJour sGainJour ;  // ok version 0.3
struct sGainJour
{
	char 	achatOuVente 					; // 0 : Vente 			-  1 : Achat
	double 	tGain[DPIVOT][TAKE_PROFIT]    	;
	char 	tpOuCloture     				; // 0 : take profit 	-  1 : ordre relaché à la cloture
};


// Structure qui recupere le gain à la journée en fonction du STOPLOSS
typedef struct sGainJourSL sGainJourSL ;  // ok version 0.3
struct sGainJourSL
{
	double 	tGain[STOP_LOSS] 				;
	int 	isSl[STOP_LOSS] 				; // 0 : false 			-  1 : true
	char 	isTp							; // 0 : false 			-  1 : true
	char 	achatOuVente					; // 0 : Vente 			-  1 : Achat
	
};


// detail du resultat sur 1 mois 
// ...MinJrNeg ne sont pas encore remplis
typedef struct sGainMois sGainMois ;  // ok version 0.3
struct sGainMois
{
	double  tGainA[DPIVOT][TAKE_PROFIT]		;
	double  tGainV[DPIVOT][TAKE_PROFIT]		;
	int		tpaMinJrNeg						;
	int		tpvMinJrNeg						;
	int		nbAchat							;
	int		nbVente							;
} ;

typedef struct sGainMoisSL sGainMoisSL;
struct sGainMoisSL 
{
	double 	tGain[STOP_LOSS] 				;
	int 	nbStopLoss[STOP_LOSS]			;
	int 	nbTakeProfit[STOP_LOSS]			;
};

// detail du resultat par année 
typedef struct sGainAnnee sGainAnnee ;  // ok version 0.3
struct sGainAnnee
{
	double  tGainA[DPIVOT][TAKE_PROFIT]		;
	double  tGainV[DPIVOT][TAKE_PROFIT]		;
	int		tpaMinJrNeg						;
	int		tpvMinJrNeg						;
	int		tpaMinMoisNeg					;
	int		tpvMinMoisNeg					;
	int		nbAchat							;
	int		nbVente							;
} ;

typedef struct sGainAnneeSL sGainAnneeSL;
struct sGainAnneeSL 
{
	double 	tGain[STOP_LOSS] 				;
	int 	nbStopLoss[STOP_LOSS]			;
	int 	nbTakeProfit[STOP_LOSS]			;
};

typedef struct sGainTotal sGainTotal ;  // ok version 0.3
struct sGainTotal
{
	double 	gainTotalMax					;
	double	gainTotalMinMoisNeg				;
	double	gainTotalMinAnneeNeg			;
	double	gainTotalSommeAnneeNeg			;
	double	sommeGainAnneeNeg				;
	double  tGainA[DPIVOT][TAKE_PROFIT]		;
	double  tGainV[DPIVOT][TAKE_PROFIT]		;
	int 	deltaPivotMax 					;
	int 	deltaPivotMinMoisNeg			;
	int 	deltaPivotMinAnneeNeg			;
	int 	deltaPivotSommeAnneeNeg			;
	int		tpaGainMax						;
	int		tpvGainMax						;
	int		tpaMinJrNeg						;
	int		tpvMinJrNeg						;
	int		tpaMinMoisNeg					;
	int		tpvMinMoisNeg					;
	int		tpaMinAnneeNeg					;
	int		tpvMinAnneeNeg					;
	int		tpaSommeAnneeNeg				;
	int		tpvSommeAnneeNeg				;
	int		nbAchat							;
	int		nbVente							;
	int 	nbMoisNeg						;
	int 	nbAnneeNeg						;
} ;

typedef struct sGainTotalSL sGainTotalSL ;  // ok version 0.3
struct sGainTotalSL
{
	char	strategie;
	double  tGain[STOP_LOSS];
	double  tGainAchat[STOP_LOSS];
	double  tGainVente[STOP_LOSS];
	double 	meilleurGainSL;
	int		meilleurSL;
	double 	meilleurGainSLAchat;
	int 	meilleurSLAchat;
	double 	meilleurGainSLVente;
	int 	meilleurSLVente;
	double 	perteMax;
	int 	dp;
	int 	tpa;
	int 	tpv;
};


typedef struct chgtHeure chgtHeure ;  // ok version 0.3
struct chgtHeure
{
	int moisParisHE, jourParisHE;
	int moisParisHH, jourParisHH;
	int moisUSHE, jourUSHE;
	int moisUSHH, jourUSHH;
} ;

typedef struct infoTraitement infoTraitement ;  // contient les traitements à réaliser pour chaque devise
struct infoTraitement
{
	char *devise;
	char *nomFichierDevise;
	int   hDepart, hFin;
	int   dPivotMin, dPivotMax;
	int   tpaMin, tpaMax;
	int   tpvMin, tpvMax;
	int   slMin, slMax;
	int   ecartSSMin, ecartSSMax;
	char *creationBD;
	char *optimiserGain;
	char *optimiserSL;
	char *optimiserSS;
	char *afficher;
};

typedef struct sparamAppli sparamAppli;  // ok version 0.3
struct sparamAppli
{
	char 	repertoireData[200]		;
	char 	repertoireRes[200]		;
	char 	fichierAlire[500]		;
	char 	fichierHE[200]			;
	char 	fichierBD[500]			;
	char 	fichierHeureBD[500]		;
	char 	nomFichierDevise[100]	;
	char 	nomFichierRes[100]		;
	infoTraitement listeDevise[50]	;
	double 	tempsLectureFichiers	;
	double 	tempsEcritureBD			;
	double 	tempsCalculPivot		;
	double 	tempsCalculGainJour		;
	double 	tempsOpti				;
} paramAppli;

typedef struct params params;
struct params 
{
	int dp, tpa, tpv;
	char strategie[50];
}; 


dataMinute 		tableauDonnees[ANNEE][MOIS][JOUR][MINUTE]  ;  // ok version 0.3
double			tPivot[ANNEE][MOIS][JOUR];
sGainJour 		tGainJour[ANNEE][MOIS][JOUR];
sGainJourSL 	tGainJourSL[ANNEE][MOIS][JOUR];
double	 		tGainJourPtf[ANNEE][MOIS][JOUR];
double	 		tGainJourPtfSL[ANNEE][MOIS][JOUR];
sGainMois		tGainMois[ANNEE][MOIS];
sGainMoisSL		tGainMoisSL[ANNEE][MOIS];
sGainAnnee		tGainAnnee[ANNEE];
sGainAnneeSL	tGainAnneeSL[ANNEE];
sGainTotal		gainTotal;
sGainTotalSL 	gainTotalSL;

chgtHeure heureEteHiver[20];		// ok version 0.3
double res, deltaPivot;
int minuteFin = MINUTE_FIN ;

int  lectureFichierParam(char *nomFichierParam);
void initialisation(infoTraitement *infT);
void lectureFichiersData(infoTraitement *infT);
void lectureFichierHeureBD(char* fichierAlire);
void lectureFichierBD(char* fichierAlire);
int  lectureFichierHeureHE(char* fichierAlire);
void lectureFichierData(char* fichierAlire);
void ecritureBD(infoTraitement *infT);
void calculGains(infoTraitement *infT);
void calculDuPivot(int minuteDebut, int minuteFin);
double calculGainJourRapide(int anneeDebut, int anneeFin, int dPivot, double gainAchat, double gainVente, infoTraitement * infT);
void calculGainMinuteStopLoss(int anneeDebut, int anneeFin, infoTraitement * infT);
void optimisation(int anneeDebut, int anneeFin,  infoTraitement * infT);
void affichageResultat(int anneeDebut, int anneeFin, infoTraitement *infT);
void affichageRisque(int anneeDebut, int anneeFin);
void affichageRisqueSL(int anneeDebut, int anneeFin);
void cumulGainPtf(int anneeDebut, int anneeFin, infoTraitement *infT);
void cumulGainPtfSL(int anneeDebut, int anneeFin, int loss, infoTraitement *infT);
void afficherResDebug();
void writeSLResultsToFile(params p, int slMin, int slMax, infoTraitement *infT);
void verifierSiMeilleurSL(int l, int max, int min);
params getStrategy();
void writeDayResultWithSL(int loss, infoTraitement *infT );
void viderFichierResultat(int mF);
void setPathsInputOutputData(void);


int main(int argc, char *argv[])
{
	for(int m = MINUTE_FIN; m >= MINUTE_FIN; m-= 60 )
	{
		
		minuteFin = m ;
		setPathsInputOutputData();
		viderFichierResultat(minuteFin);
		int nbDevises = 0;
		nbDevises = lectureFichierParam("SimulateurParams.csv");

		memset(tGainJourPtf,0,sizeof(tGainJourPtf));
		memset(tGainJourPtfSL,0,sizeof(tGainJourPtfSL));
		for (int i = 0; i < nbDevises; i++)
		{

			initialisation(&paramAppli.listeDevise[i]);
			
			//printf("avant lectureFichiersData\n");
			lectureFichiersData(&paramAppli.listeDevise[i]);
			//printf("apres lecture\n");
			ecritureBD(&paramAppli.listeDevise[i]);
			calculGains(&paramAppli.listeDevise[i]);
			optimisation(ANNEE_DEB, ANNEE_FIN, &paramAppli.listeDevise[i]);
			calculGainMinuteStopLoss(ANNEE_DEB, ANNEE_FIN, &paramAppli.listeDevise[i]);
			affichageResultat( ANNEE_DEB, ANNEE_FIN, &paramAppli.listeDevise[i]);
			cumulGainPtf(ANNEE_DEB, ANNEE_FIN, &paramAppli.listeDevise[i]);
			cumulGainPtfSL(ANNEE_DEB, ANNEE_FIN, gainTotalSL.meilleurSL/10, &paramAppli.listeDevise[i]);

		}
		affichageRisque(ANNEE_DEB, ANNEE_FIN);
		affichageRisqueSL(ANNEE_DEB, ANNEE_FIN);
		//afficherResDebug();
	}
    return 0;
}


void viderFichierResultat(int mF)
{
	char filename[100] ;
	strcpy(filename, paramAppli.nomFichierRes);
	FILE *fichierResOpti = fopen(filename,"w");
		if(fichierResOpti != NULL)
		{
			fclose(fichierResOpti);
		}
		else exit(1);
}

void afficherResDebug()
{
	printf("ga :%f     gv :%f\n",gainTotal.tGainA[98][57] ,gainTotal.tGainV[98][162] );
}

int lectureFichierParam(char *nomFichierParam)
{
	
	int nbDevises = 0;
	char **ligne, l[300];
	int size = 300;
	char titre[200] ;
	// *devise = NULL, *nomFichier = NULL, *creationBD = NULL, *lireBD =NULL, *opti= NULL;
	FILE *fParam = fopen(nomFichierParam, "r");


	if (fParam != NULL)
	{
		fgets(titre, size, fParam);
		printf("%s\n",titre );

    	while (! feof(fParam))
    	{
    		fscanf(fParam, "%s\n",l);
    		ligne = eclaterChaine(l,';');
    		paramAppli.listeDevise[nbDevises].devise 			= ligne[0];
    		paramAppli.listeDevise[nbDevises].nomFichierDevise 	= ligne[1];
    		paramAppli.listeDevise[nbDevises].dPivotMin 		= atoi(ligne[2]);
    		paramAppli.listeDevise[nbDevises].dPivotMax 		= atoi(ligne[3]);
    		paramAppli.listeDevise[nbDevises].tpaMin 			= atoi(ligne[4]);
    		paramAppli.listeDevise[nbDevises].tpaMax 			= atoi(ligne[5]);
    		paramAppli.listeDevise[nbDevises].tpvMin 			= atoi(ligne[6]);
    		paramAppli.listeDevise[nbDevises].tpvMax 			= atoi(ligne[7]);
    		paramAppli.listeDevise[nbDevises].slMin 			= atoi(ligne[8]);
    		paramAppli.listeDevise[nbDevises].slMax 			= atoi(ligne[9]);
    		paramAppli.listeDevise[nbDevises].ecartSSMin 		= atoi(ligne[10]);
    		paramAppli.listeDevise[nbDevises].ecartSSMax 		= atoi(ligne[11]);
    		paramAppli.listeDevise[nbDevises].creationBD 		= ligne[12];
    		paramAppli.listeDevise[nbDevises].optimiserGain		= ligne[13];
    		paramAppli.listeDevise[nbDevises].optimiserSL		= ligne[14];
    		paramAppli.listeDevise[nbDevises].optimiserSS 		= ligne[15];
    		paramAppli.listeDevise[nbDevises].afficher 	 		= ligne[16];

			//printf("%s %s %s %s %s\n", paramAppli.listeDevise[nbDevises].devise, paramAppli.listeDevise[nbDevises].nomFichierDevise, paramAppli.listeDevise[nbDevises].creationBD, paramAppli.listeDevise[nbDevises].lireBD, paramAppli.listeDevise[nbDevises].optimiser);
			nbDevises++;
		}
		fclose(fParam);
	}
	else  printf("Impossible d'ouvrir le fichier : %s\n",nomFichierParam);

	for (int i=0; i<nbDevises; i++)
	{
		printf("%s %s %d %d %d %d %s %s %s \n",paramAppli.listeDevise[i].devise, paramAppli.listeDevise[i].nomFichierDevise, paramAppli.listeDevise[i].dPivotMin, paramAppli.listeDevise[i].dPivotMax, paramAppli.listeDevise[i].ecartSSMin, paramAppli.listeDevise[i].ecartSSMax ,paramAppli.listeDevise[i].creationBD, paramAppli.listeDevise[i].optimiserGain, paramAppli.listeDevise[i].optimiserSS );
	}


	return nbDevises;
}

void setPathsInputOutputData(void)
{
	strcpy(paramAppli.repertoireData, "/Users/kant1_sahal/Desktop/QuentinCocoon/donnees/HistData/ptf/archives/");
	strcpy(paramAppli.repertoireRes, "resultats/");
	sprintf(paramAppli.nomFichierRes, "%sresultats_9h_%dh.csv",paramAppli.repertoireRes,(minuteFin+360)/60);
}

void initialisation(infoTraitement *infT)
{

	//strcpy(paramAppli.repertoireData, "/Users/kant1_sahal/Desktop/QuentinCocoon/donnees/HistData/ptf/archives/");
	//strcpy(paramAppli.repertoireRes, "resultats/");
	strcpy(paramAppli.fichierAlire, paramAppli.repertoireData);
	strcat(paramAppli.fichierAlire, infT->nomFichierDevise) ;
	strcpy(paramAppli.fichierHE, paramAppli.repertoireData);
	strcat(paramAppli.fichierHE, "HeureEteHiver.csv") ;
	strcpy(paramAppli.fichierBD, paramAppli.repertoireData);
	strcat(paramAppli.fichierBD, infT->nomFichierDevise) ;
	//sprintf(paramAppli.nomFichierRes, "%sresultats_9h_%dh.csv",paramAppli.repertoireRes,(minuteFin+360)/60);
	int l = strlen(paramAppli.fichierBD);
	paramAppli.fichierBD[l-3] = 'T';
	paramAppli.fichierBD[l-2] = 'H';
	paramAppli.fichierBD[l-1] = 'S';
	strcpy(paramAppli.fichierHeureBD, paramAppli.repertoireData);
	strcat(paramAppli.fichierHeureBD, "HeureEteHiver.THS");
	memset(tableauDonnees,0,sizeof(tableauDonnees));
	memset(gainTotal.tGainV,0,sizeof(gainTotal.tGainV));
	memset(gainTotal.tGainA,0,sizeof(gainTotal.tGainA));
	memset(tGainMois,0,sizeof(tGainMois));
	memset(tGainMoisSL,0,sizeof(tGainMoisSL));
	memset(tGainAnnee,0,sizeof(tGainAnnee));
	memset(tGainAnneeSL,0,sizeof(tGainAnneeSL));
	memset(tGainJour,0,sizeof(tGainJour));
	memset(tGainJourSL,0,sizeof(tGainJourSL));
	//memset(gainJourPtf,0,sizeof(gainJourPtf));
	memset(&gainTotal,0,sizeof(gainTotal));
	memset(&gainTotalSL,0,sizeof(gainTotalSL));
	memset(tPivot,0,sizeof(tPivot));
	gainTotal.gainTotalMax = -1000000;
}

void lectureFichiersData(infoTraitement *infT)
{
	clock_t tDeb, tFin;

	tDeb = clock();
	//lectureFichierHeureHE(paramAppli.fichierHE);
	lectureFichierHeureBD(paramAppli.fichierHeureBD);
	//printf("apres lecture data: %c\n", infT->creationBD[0] );
	if (infT->creationBD[0] == 'O') lectureFichierData(paramAppli.fichierAlire ) ;
	//printf("apres lecture data 1: \n");
	if (infT->creationBD[0] != 'O') lectureFichierBD(paramAppli.fichierBD);
	tFin = clock();
	paramAppli.tempsLectureFichiers = (double)(tFin - tDeb)/CLOCKS_PER_SEC;
	/*for (int j=0; j<20; j++)
	{
		printf("%i:\t\t %i-%i \t %i-%i \t %i-%i \t %i-%i \n",j+2000, heureEteHiver[j].moisParisHE, heureEteHiver[j].jourParisHE, heureEteHiver[j].moisParisHH, heureEteHiver[j].jourParisHH, heureEteHiver[j].moisUSHE, heureEteHiver[j].jourUSHE, heureEteHiver[j].moisUSHH, heureEteHiver[j].jourUSHH );
	}*/
}

void lectureFichierHeureBD(char* fichierAlire)
{
	FILE *fhBD = fopen(fichierAlire, "rb");
	if (fhBD != NULL)
    {
    	printf("le fichier %s a bien été ouvert\n", fichierAlire);
		fread(heureEteHiver, sizeof(heureEteHiver), 1, fhBD);
		fclose(fhBD);
	}
	else printf("Impossible d'ouvrir le fichier : %s\n",fichierAlire);
}

void lectureFichierBD(char* fichierAlire)
{
	FILE *fBD = fopen(fichierAlire, "rb");
	if (fBD != NULL)
    {
    	printf("le fichier %s a bien été ouvert\n", fichierAlire);
		fread(tableauDonnees, sizeof(tableauDonnees), 1, fBD);
		fclose(fBD);
    	//printf("le fichier %s a bien été fermé\n", fichierAlire);
	}
	else printf("Impossible d'ouvrir le fichier BD : %s\n",fichierAlire);
}


void chargerHeureEteHiver(int *moisH, int *jourH , char *h)
{
	char **ligne;
	int j;

	//printf("h: %s\n", h);
	ligne = eclaterChaine(h,'/');
	//printf("an: %s\n", ligne[0]);
	*moisH = atoi(ligne[1]);
    *jourH = atoi(ligne[2]);
    for (j = 0; j < 3; j++) free(ligne[j]);
	free(ligne);
}

int lectureFichierHeureHE(char* fichierAlire)
{
	char enTete[200];
	char l[200];
	char **ligne;
	char parisHE[10], parisHH[10];
	char usHE[10], usHH[10];
	int annee, j;

	FILE* fichier = fopen(fichierAlire, "r");

	if (fichier != NULL)
    {
    	fscanf(fichier,"%s\n", enTete);
    	while (! feof(fichier))
    	{
    		fscanf(fichier, "%s\n",l);
    		ligne = eclaterChaine(l,';');
    		//printf("%s\n", l);
    		annee = atoi(ligne[0]) - 2000;
    		chargerHeureEteHiver(&heureEteHiver[annee].moisUSHE, &heureEteHiver[annee].jourUSHE,ligne[1]);
    		chargerHeureEteHiver(&heureEteHiver[annee].moisUSHH, &heureEteHiver[annee].jourUSHH,ligne[2]);
    		chargerHeureEteHiver(&heureEteHiver[annee].moisParisHE, &heureEteHiver[annee].jourParisHE,ligne[3]);
    		chargerHeureEteHiver(&heureEteHiver[annee].moisParisHH, &heureEteHiver[annee].jourParisHH,ligne[4]);

    		for (j = 0; j < 5; j++) free(ligne[j]);
			free(ligne);
   		}
   		fclose(fichier);
	}
	else printf("Impossible d'ouvrir le fichier : %s\n",fichierAlire);

	/*for (j=0; j<20; j++)
	{
		printf("%i:\t\t %i-%i \t %i-%i \t %i-%i \t %i-%i \n",j+2000, heureEteHiver[j].moisParisHE, heureEteHiver[j].jourParisHE, heureEteHiver[j].moisParisHH, heureEteHiver[j].jourParisHH, heureEteHiver[j].moisUSHE, heureEteHiver[j].jourUSHE, heureEteHiver[j].moisUSHH, heureEteHiver[j].jourUSHH );
	}
	*/

	return 0;
}

void lectureFichierData(char* fichierAlire)
{
	FILE* fichier = NULL;

    char charac[TAILLE_LIGNE_MAX] = "";
    char delimLine[2] = "," ; char delimDate[2] = "." ; char delimTime[2] = ":";

    int year = 0 , month = 0, day = 0, minutes ;
    int volume ;

	fichier = fopen(fichierAlire, "r"); // file descriptor

    if (fichier != NULL)
    {
    	printf("le fichier %s a bien été ouvert\n", fichierAlire);
    	dataMinute ptrSurDonnee;
    	while(fgets(charac, TAILLE_LIGNE_MAX, fichier) != NULL)
    	{
    		char dataDate[20] = ""; char dataTime[10] = "" ;
	    	//dataMinute *ptrSurDonnee = malloc(sizeof(dataMinute)) ;

			strcpy( dataDate , strtok(charac, delimLine) ) ;
			strcpy( dataTime , strtok(  NULL, delimLine) ) ;
			ptrSurDonnee.open  = atof( strtok(NULL, delimLine) ) ;
			ptrSurDonnee.high  = atof( strtok(NULL, delimLine) ) ;
			ptrSurDonnee.low   = atof( strtok(NULL, delimLine) ) ;
			ptrSurDonnee.close = atof( strtok(NULL, delimLine) ) ;

			//printf("%s\n", dataDate);

			year  = atoi( strtok(dataDate, delimDate) )  - 2000;
			month = atoi( strtok(NULL, delimDate) );
			day   = atoi( strtok(NULL, delimDate) );

			//printf("year = %d, month = %d , day = %d \n", year , month , day);

			//printf("%s\n", dataTime);
			//int heureAconvertir =  atoi( strtok(dataTime, &delimTime));
			//int minuteAconvertir = atoi( strtok(NULL    , &delimTime));

			minutes = 60 * atoi( strtok(dataTime, delimTime)) + atoi( strtok(NULL, delimTime) );
			//printf("minutes = %d\n", minutes);

			tableauDonnees[year][month - 1 ][day - 1][minutes].open = ptrSurDonnee.open;
			tableauDonnees[year][month - 1 ][day - 1][minutes].high  = ptrSurDonnee.high;
			tableauDonnees[year][month - 1 ][day - 1][minutes].low   = ptrSurDonnee.low;
			tableauDonnees[year][month - 1 ][day - 1][minutes].close = ptrSurDonnee.close;			//printf("%p\n", ptrSurDonnee);
			//printf("%p\n", (*tableau)[year][month][day][minutes]);
			//printf("open = %.6f\n", (*(*tableau)[year][month][day][minutes]).open );
			//printf("------------------\n");
		}
		fclose(fichier); // On suppose que le fichier se ferme bien, pas de verif...
    }
    else printf("Impossible d'ouvrir le fichier %s\n", fichierAlire);
}

void lectureFichierDataLent(char* fichierAlire)
{
	FILE* fichier = NULL;

    char l[200];
	char **ligne, **dateCotation, **heureCotation;
    char delimChamps = ',' ; char delimDate = '.' ; char delimTime = ':';

    int year = 0 , month = 0, day = 0, minutes ;
    int j ;

	fichier = fopen(fichierAlire, "r"); // file descriptor

	if (fichier != NULL)
    {
     	printf("le fichier %s a bien été ouvert\n", fichierAlire);
     	while (! feof(fichier))
    	{
    		fscanf(fichier, "%s\n",l);
    		ligne 			= eclaterChaine(l,delimChamps);

    		dateCotation 	= eclaterChaine(ligne[0],delimDate);
			year  = atoi( dateCotation[0] )  - 2000;
			month = atoi( dateCotation[1] );
			day   = atoi( dateCotation[2] );
			for (j = 0; j < 3; j++) free(dateCotation[j]);
			free(dateCotation);

    		heureCotation 	= eclaterChaine(ligne[1],delimTime);
    		minutes = 60 * atoi( heureCotation[0] ) + atoi( heureCotation[1] );
    		for (j = 0; j < 2; j++) free(heureCotation[j]);
			free(heureCotation);

	    	/*dataMinute *ptrSurDonnee = malloc(sizeof(dataMinute)) ;

			ptrSurDonnee->open  = atof( ligne[2] ) ;
			ptrSurDonnee->high  = atof( ligne[3] ) ;
			ptrSurDonnee->low   = atof( ligne[4] ) ;
			ptrSurDonnee->close = atof( ligne[5] ) ;
			j = 0;
			while (ligne[j] != NULL) free(ligne[j++]);
			free(ligne);

			tableauDonnees[year][month - 1 ][day - 1][minutes] = ptrSurDonnee ;
			*/

			tableauDonnees[year][month - 1 ][day - 1][minutes].open = atof( ligne[2] ) ;
			tableauDonnees[year][month - 1 ][day - 1][minutes].high  = atof( ligne[3] ) ;
			tableauDonnees[year][month - 1 ][day - 1][minutes].low   = atof( ligne[4] ) ;
			tableauDonnees[year][month - 1 ][day - 1][minutes].close = atof( ligne[5] ) ;
			j = 0;
			while (ligne[j] != NULL) free(ligne[j++]);
			free(ligne);

		}
		fclose(fichier); // On suppose que le fichier se ferme bien, pas de verif...
    }
    else printf("Impossible d'ouvrir le fichier \n");
}

void ecritureBD(infoTraitement * infT)
{
	clock_t tDeb, tFin;

	tDeb = clock();
	//printf("apres lecture data: %c\n", infT->creationBD[0] );
	if (infT->creationBD[0] == 'O')
	{
		FILE *fBD = fopen(paramAppli.fichierBD,"wb");
		fwrite( tableauDonnees, sizeof(tableauDonnees), 1, fBD);
		fclose(fBD);
		//fBD = fopen(paramAppli.fichierHeureBD,"wb");
		//fwrite( heureEteHiver, sizeof(heureEteHiver), 1, fBD);
		//fclose(fBD);
	}

	tFin = clock();
	paramAppli.tempsEcritureBD = (double)(tFin - tDeb)/CLOCKS_PER_SEC;
}

// Plus utilisé directement, son contenu a été basculé  dans cumulGainPtfSL
void writeDayResultWithSL(int loss, infoTraitement* infT )
{
	char filename[100] = "resultats/jour/";
	strcat(filename, infT->devise);
	strcat(filename, "_jour.csv");
	FILE* fd = fopen(filename,"w");
	if(fd != NULL)
	{
		fprintf(fd, "date;gain sans SL;gain SL = %d;type;sl;tp\n", loss);
		for (int year = ANNEE_DEB; year <= ANNEE_FIN ; year++)
		{
			for (int month = 0; month < MOIS ; month++)
			{
				for (int day = 0; day < JOUR ; day++)
				{
					fprintf(fd, "%d/%d/%d;%.2f;%.2f;%c;%c;%c\n",day+1,month+1,year,tGainJourSL[year][month][day].tGain[80],tGainJourSL[year][month][day].tGain[loss], tGainJourSL[year][month][day].achatOuVente, tGainJourSL[year][month][day].isSl[loss], tGainJourSL[year][month][day].isTp);
				}
			}
		}
		fclose(fd);
	}
}

void calculGains(infoTraitement * infT)
{
	//printf("avant calcul gain.\n");
	//if (infT->optimiserGain[0] == 'O' )
	//{
		printf("dans calcul gain : %c\n",infT->optimiserGain[0] );
		clock_t tDeb, tFin;

		tDeb = clock();

		calculDuPivot(MINUTE_DEBUT, minuteFin);

		tFin = clock();
		paramAppli.tempsCalculPivot = (double)(tFin - tDeb)/CLOCKS_PER_SEC;

		tDeb = tFin;
		gainTotal.gainTotalMax = -1000000; // Pq l'initialiser 2 fois ?? voir initialisation()
		for (int dP = infT->dPivotMin ; dP <= infT->dPivotMax  ; dP++)
		{
			deltaPivot = dP ; // modif kh avant deltaPivot = dP/100.0 ;
			calculGainJourRapide(ANNEE_DEB, ANNEE_FIN, deltaPivot, 0.0, 0.0, infT);
		}
		tFin = clock();
		paramAppli.tempsCalculGainJour = (double)(tFin - tDeb)/CLOCKS_PER_SEC;
	//} // if (infT->optimiserGain[0] == 'O' )
}


int calculDeltaMin(int year, int month, int day)
{
	int dMin = 0;
	int test=-1, test2;

	test   = (heureEteHiver[year].moisUSHE == month ) && (heureEteHiver[year].jourUSHE <= day);
	test   = test && ((heureEteHiver[year].moisParisHE == month) && (heureEteHiver[year].jourParisHE >= day));
	test2   =  ((heureEteHiver[year].moisUSHH == month) && (heureEteHiver[year].jourUSHH >= day));
	test2   = test2 || ((heureEteHiver[year].moisParisHH == month) && (heureEteHiver[year].jourParisHH <= day));

	if ( (test) || (test2))
		{
			dMin = 60;
			//printf("********** test: %d dMin: %d \t année: %d   mois: %d   jour: %d\n", test || test2, dMin, year + 2000, month, day);
		}

	return dMin;
}

// Calcul du pivot et du high low close de la journée
void calculDuPivot(int minuteDebut, int minuteFin)
{
	double pivotJournee = 0   ;
	dataMinute *donneeMinute ;
	for (int year = 0; year < ANNEE; year++)
	{
		for (int month = 0; month < MOIS; month++)
		{
			for (int day = 0; day < JOUR; day++)
			{
				double open = 0, close = 0, high = 0, low = 900 ;
				int deltaMin = calculDeltaMin(year, month+1, day+1);
				for (int minute = minuteDebut + deltaMin; minute <= minuteFin + deltaMin; minute++)
				{

					donneeMinute = &tableauDonnees[year][month][day][minute];
					if(donneeMinute->high > 0 )
					{
						tPivot[year][month][day] = pivotJournee ;

						if(donneeMinute->high  != 0 ) high  = MAX(high, donneeMinute->high) ;
						if(donneeMinute->low   != 0 ) low   = MIN(low, donneeMinute->low) ;
						if(donneeMinute->close != 0 ) close = donneeMinute->close ;
					}
				}
				if(high != 0)
				{
					int m=minuteDebut + deltaMin;

					donneeMinute = &tableauDonnees[year][month][day][MINUTE - 1] ;
					//if (tableauDonnees[year][month][day][minuteDebut + deltaMin].high <= 0)
					//{
					//	m = minuteDebut + deltaMin + 1;
						while ((m <  minuteFin + deltaMin) && (tableauDonnees[year][month][day][m].high <= 0) ) { m++; }
					//}
					donneeMinute->open  = tableauDonnees[year][month][day][ m].open;
					donneeMinute->high  = high;
					donneeMinute->low   = low;
					donneeMinute->close = close;
					//donneeMinute->pivot = pivotJournee;
					pivotJournee = ( high + low + close ) / 3 ;
				}

			}
		}
	}
}


double calculGainJourRapide(int anneeDebut, int anneeFin, int deltaP, double gainAchat, double gainVente, infoTraitement * infT)
{
	dataMinute *donneeMinute ;
	double pivotAjuste, takeProfit;
	double resYear, gainMaxA, gainMaxV;
	char transaction;
/*	double dpiv = dPivot*1000;  // mis en comment par KH à cause de la mauvaise précision
	int dP = (int)(dpiv);
*/
	int nbAchat=0, nbVente=0, tpaOpti, tpvOpti;

	int dP = deltaP;
	//printf("dP: %d   %f\n", dP, dpiv);
	for (int year = anneeDebut; year <= anneeFin ; year++)
	{
		resYear = 0;
		for (int month = 0; month < MOIS ; month++)
		{
			for (int day = 0; day < JOUR ; day++)
			{
				//printf("gain jour rapide - aa/mm/jj: %.2d/%.2d/%.2d\n",year,month, day );
				double open=0;
				donneeMinute = &tableauDonnees[year][month][day][MINUTE -1]  ;
				if (donneeMinute->high > 0)
				{
					pivotAjuste = tPivot[year][month][day] * (1.0 + deltaP/10000.0) ;
					open = donneeMinute->open;
					//printf("dPivot: %f   gainTotV: %f   gainTotA: %f\n",dPivot, paramGainOpti.gainTotV, paramGainOpti.gainTotA );
					if(donneeMinute->open >= pivotAjuste)
					{
						nbVente++;
						tGainJour[year][month][day].achatOuVente = 0;
						for (int tpv= infT->tpvMin; tpv<=infT->tpvMax; tpv++)
						{
							gainVente = tpv/100.0;
							takeProfit = donneeMinute->open * (1.0 - gainVente / 100.0)  ;
							//donneeMinute->open = (takeProfit <= donneeMinute->high) ? (donneeMinute->open - takeProfit) * MONTANT / donneeMinute->open :  (donneeMinute->open - donneeMinute->close) * MONTANT / donneeMinute->open;
							if (takeProfit >= donneeMinute->low)
								 tGainJour[year][month][day].tGain[dP + IDX_PIVOT_MAX][tpv] = (donneeMinute->open - takeProfit) * MONTANT / donneeMinute->open ;
							else tGainJour[year][month][day].tGain[dP + IDX_PIVOT_MAX][tpv] = (donneeMinute->open - donneeMinute->close) * MONTANT / donneeMinute->open;
							//printf("avant - gainTpvDp[tpv][%d + 200]: %f\n",dP, gainTpvDp[tpv][dP + 200]);
							
							double gainJournee = tGainJour[year][month][day].tGain[dP + IDX_PIVOT_MAX][tpv];
							tGainMois[year][month].tGainV[dP + IDX_PIVOT_MAX][tpv] += gainJournee;
							tGainAnnee[year].tGainV[dP + IDX_PIVOT_MAX][tpv] += gainJournee;
							gainTotal.tGainV[dP + IDX_PIVOT_MAX][tpv] += gainJournee;
							//printf("après - gainTpvDp[tpv][%d + 200]: %f\n",dP, gainTpvDp[tpv][dP + 200]);
							//resYear += tGainJour[year][month][day].tGain[dP + 200][tpv];

						}

					}
					else
					{
						nbAchat++;
						tGainJour[year][month][day].achatOuVente = 1;
						for (int tpa= infT->tpaMin; tpa <= infT->tpaMax; tpa++)
						{
							gainAchat = tpa/100.0;
							takeProfit = donneeMinute->open * (1.0 + gainAchat / 100.0)  ;
							//donneeMinute->open = (takeProfit >= donneeMinute->low) ? (takeProfit - donneeMinute->open ) * MONTANT / donneeMinute->open :  (donneeMinute->close - donneeMinute->open ) * MONTANT / donneeMinute->open;
							if (takeProfit <= donneeMinute->high)
								 tGainJour[year][month][day].tGain[dP + IDX_PIVOT_MAX][tpa] = (takeProfit - donneeMinute->open ) * MONTANT / donneeMinute->open;
							else tGainJour[year][month][day].tGain[dP + IDX_PIVOT_MAX][tpa] = (donneeMinute->close - donneeMinute->open ) * MONTANT / donneeMinute->open;
							
							
							// On remplit les structures du gain mois, année et totale
							double gainJournee = tGainJour[year][month][day].tGain[dP + IDX_PIVOT_MAX][tpa] ;
							tGainMois[year][month].tGainA[dP + IDX_PIVOT_MAX][tpa] += gainJournee;
							tGainAnnee[year].tGainA[dP + IDX_PIVOT_MAX][tpa] += gainJournee;
							gainTotal.tGainA[dP + IDX_PIVOT_MAX][tpa] += gainJournee;
							//printf("gainTpaDp[%d][%d + 200]: %f  gainTpvDp[%d][%d + 200]: %f\n",tpa, dP, gainTpaDp[tpa][dP + 200],97, dP, gainTpvDp[97][dP + 200]);
							//resYear += tGainJour[year][month][day].tGain[dP + 200][tpa];

							

						}
					}
					
					//printf("%.2d/%.2d/%d  %c   gain: %f    tp: %f  open: %f  high: %f  low: %f  close: %f\n",day, month, year,transaction, donneeMinute->gain, takeProfit,donneeMinute->open, donneeMinute->high, donneeMinute->low, donneeMinute->close );
					//gainTotal += donneeMinute->gain;
				}
			}
		}
		 // if (dP >= -290)
		 //   printf("year 20%.2d :  Gain achat: %f gain vente: %f\n", year, tGainAnnee[year].tGainA[dP + IDX_PIVOT_MAX][57], tGainAnnee[year].tGainV[dP+ IDX_PIVOT_MAX][162]);
	}
	//paramGainOpti.gainTotV = paramGainOpti.gainTotA = -1000000;
	gainMaxV = gainMaxA = -1000000;
	for (int tp= infT->tpaMin; tp<= infT->tpaMax; tp++)
	{
		if ( gainMaxA < gainTotal.tGainA[dP + IDX_PIVOT_MAX][tp] )
		{
			tpaOpti = tp;
			gainMaxA = gainTotal.tGainA[dP + IDX_PIVOT_MAX][tp] ;
		}
	}
	for (int tp= infT->tpvMin; tp<=  infT->tpvMax; tp++)
	{
		if ( gainMaxV < gainTotal.tGainV[dP + IDX_PIVOT_MAX][tp] )
		{
			tpvOpti = tp;
			gainMaxV = gainTotal.tGainV[dP + IDX_PIVOT_MAX][tp];
		}
	}
	if (gainTotal.gainTotalMax < gainTotal.tGainA[dP + IDX_PIVOT_MAX][tpaOpti] + gainTotal.tGainV[dP + IDX_PIVOT_MAX][tpvOpti])
		{
			gainTotal.tpaGainMax 	= tpaOpti ;
			gainTotal.deltaPivotMax = dP;
			gainTotal.tpvGainMax 	= tpvOpti;
			gainTotal.gainTotalMax	= gainTotal.tGainA[dP + IDX_PIVOT_MAX][tpaOpti] + gainTotal.tGainV[dP + IDX_PIVOT_MAX][tpvOpti];
		}

	return gainTotal.gainTotalMax;
}


void calculGainMinuteStopLoss(int anneeDebut, int anneeFin, infoTraitement * infT)
{
	if(infT->optimiserSL[0] == 'N' || minuteFin != MINUTE_FIN ) return; 

	clock_t tDeb, tFin;
	tDeb = clock();

	gainTotalSL.meilleurGainSL = -10000000000;

	printf("********************************** CALCUL GAIN MINUTE STOP LOSS ******************************************\n\n");
	dataMinute* donneeMinute, minuteCourante ;
	int dP, tpa, tpv, tpAtteint, slAtteint ; 
	int stopMin = infT->slMin;
	int stopMax = infT->slMax;
	double pivotAjuste, takeProfit, takeLoss, gainVente, gainAchat;
	char transaction; 

	params param ; 
	param = getStrategy();

	dP 	= param.dp ;
	tpa = param.tpa;
	tpv = param.tpv;

	for (int loss = stopMin; loss <= stopMax; loss++)
	{
		for (int year = anneeDebut; year <= anneeFin ; year++)
		{
			for (int month = 0; month < MOIS ; month++)
			{
				for (int day = 0; day < JOUR ; day++)
				{
					tpAtteint = 0;
					slAtteint = 0;
					int deltaMin = calculDeltaMin(year, month+1, day+1);
					donneeMinute = &tableauDonnees[year][month][day][MINUTE-1]  ;
					double open = donneeMinute->open;
					pivotAjuste = tPivot[year][month][day] * (1.0 + dP/10000.0) ;

					//printf("%d/%d/%d : deltaMin = %d\n",day, month, year, deltaMin);
					for (int min = MINUTE_DEBUT + deltaMin; min <= minuteFin + deltaMin ; min++)
					{
						
						minuteCourante = tableauDonnees[year][month][day][min]  ;

						if(minuteCourante.high <= 0) continue;

						if(open >= pivotAjuste) // Cas d'une vente
						{
							tGainJourSL[year][month][day].achatOuVente = '0' ; 
							gainVente = tpv/100.0;
							takeProfit = open * (1.0 - gainVente / 100.0) ;
							takeLoss = open * (1.0 + ABS_STOP_LOSS_MAX*loss / (10000.0) )  ; // On ajuste le pas
							
							if (minuteCourante.low < takeProfit )
							{

								tGainJourSL[year][month][day].tGain[loss] = (open - takeProfit) * MONTANT / open ;
								tGainJourSL[year][month][day].isTp = '1';
								tpAtteint = 1;
								break;
							
							}
							else if(minuteCourante.high > takeLoss )
							{
								//printf("VENTE: on dépasse le stopLoss (%d), high= %f à la minute : %d, %d/%d/%d\n",loss,minuteCourante.low, min, day+1,month+1,year);
								// On gere le cas d'un stopLoss;
								
								tGainJourSL[year][month][day].tGain[loss] = (open - takeLoss) * MONTANT / open ;
								tGainJourSL[year][month][day].isSl[loss] = '1';
								tGainMoisSL[year][month].nbStopLoss[loss]++ ;
								tGainAnneeSL[year].nbStopLoss[loss]++;
								slAtteint = 1;
								break;
							}

						}
						// A gerer de la meme maniere que que le cas d'une vente 
						else // cas d'un achat
						{
							gainAchat = tpa/100.0;
							takeProfit = open * (1.0 + gainAchat / 100.0)  ;
							tGainJourSL[year][month][day].achatOuVente = '1' ; 
							
							takeLoss = open * (1.0 - ABS_STOP_LOSS_MAX*loss / (10000.0) )  ; // On ajuste le pas
						
							if (minuteCourante.high > takeProfit )
							{

								tGainJourSL[year][month][day].tGain[loss] = (takeProfit - open) * MONTANT / donneeMinute->open ;
								tGainJourSL[year][month][day].isTp = '1';
								tpAtteint = 1;
								
								break;
							}
							else if(minuteCourante.low < takeLoss )
							{
								// On gere le cas d'un stopLoss;
								
								tGainJourSL[year][month][day].tGain[loss] = (takeLoss - open) * MONTANT / donneeMinute->open ;
								tGainJourSL[year][month][day].isSl[loss] = '1';
								tGainMoisSL[year][month].nbStopLoss[loss]++ ;
								tGainAnneeSL[year].nbStopLoss[loss]++;
								slAtteint = 1;
								//printf("ACHAT: on dépasse le stoploss (%d), avec une perte de %.2f:, %d/%d/%d\n",loss, tGainJourSL[year][month][day].tGain[loss], day,month,year);
								break;
							}
						}
					} // for min

					// On arrive en fin de journée , pas de sl ni tp
					if(slAtteint == 0 && tpAtteint == 0)
					{
						tGainJourSL[year][month][day].isSl[loss] = '0';
						tGainJourSL[year][month][day].isTp = '0';
						double diffOpenClose = tGainJourSL[year][month][day].achatOuVente == '1' ? (donneeMinute->close - open) : (open - donneeMinute->close);

						tGainJourSL[year][month][day].tGain[loss] = (diffOpenClose != 0) ? diffOpenClose * MONTANT / open : 0 ;
					}
					
					// Si la transaction de la journée etait une vente
					if(tGainJourSL[year][month][day].achatOuVente == '0')
					{
						gainTotalSL.tGainVente[loss] += tGainJourSL[year][month][day].tGain[loss] ; 
					}
					// Si la transaction de la journée est un achat
					else
					{
						gainTotalSL.tGainAchat[loss] += tGainJourSL[year][month][day].tGain[loss] ; 
					}
					
					gainTotalSL.perteMax = MIN(gainTotalSL.perteMax, tGainJourSL[year][month][day].tGain[loss] );
					tGainMoisSL[year][month].tGain[loss] += tGainJourSL[year][month][day].tGain[loss] ;

				} // for day
				//printf("%d/20%d : %.2f\n",month+1,year,tGainMoisSL[year][month].tGain[loss]);
				tGainAnneeSL[year].tGain[loss] += tGainMoisSL[year][month].tGain[loss] ;
			} // for month
			//printf("20%d : %.2f\n",year,tGainAnneeSL[year].tGain[loss]);
			gainTotalSL.tGain[loss] += tGainAnneeSL[year].tGain[loss];
		} // for year
		verifierSiMeilleurSL(loss, stopMin, stopMax);
		
	} // for loss
	tFin = clock() ;
	printf("temps du sl = %f\n",(double)(tFin - tDeb)/CLOCKS_PER_SEC );
	printf("*********************************************************************\n\n");
	writeSLResultsToFile(param,stopMin,stopMax, infT);


	// Pour comparer les valeurs entre algo jour et algo min sur un mois
	// printf("*********************************************************************\n\n");
	// printf("On compare la journée\n");
	// double sum1 = 0 ;
	// double sum2 = 0 ; 
	// int annee = 10;
	// int mois = 12;
	// for(int i=0; i<JOUR; i++)
	// {
	// 	if(tGainJour[annee][mois-1][i].achatOuVente == '0'){
	// 		printf("vente: %d/%d/%d : %.2f || ",i+1, mois, annee, tGainJour[annee][mois-1][i].tGain[-28+IDX_PIVOT_MAX][60] );
	// 			sum1+= tGainJour[annee][mois-1][i].tGain[-28+IDX_PIVOT_MAX][60];
	// 	}
	// 	else
	// 	{
	// 		printf("achat: %d/%d/%d : %.2f || ",i+1,mois, annee, tGainJour[annee][mois-1][i].tGain[-28+IDX_PIVOT_MAX][91] );
	// 			sum1+= tGainJour[annee][mois-1][i].tGain[-28+IDX_PIVOT_MAX][91];
	// 	}
	// 	//printf("%d/12/2010 : tGainJour: %.2f\n",i+1, tGainJour[10][11][i].tGain[] );
	// 	printf("%d/%d/%d : %.2f\n",i+1,mois, annee, tGainJourSL[annee][mois-1][i].tGain[1] );
	// 	sum2+= tGainJour[annee][mois-1][i].tGain[-28+IDX_PIVOT_MAX][91];
	// }
	// printf("sum1= %.2f\n", sum1);
	// printf("sum2= %.2f\n", sum2);
	// printf("*********************************************************************\n\n");
} 

params getStrategy(void)
{
	params p;

	switch(STRATEGIE_SL) 
	{
		case 1 : 
			p.dp 		= gainTotal.deltaPivotMax;
			p.tpa 		= gainTotal.tpaGainMax;
			p.tpv 		= gainTotal.tpvGainMax;
			strcpy(p.strategie, "MAX");
			break;

		case 2 : 
			p.dp 		= gainTotal.deltaPivotMinAnneeNeg;
			p.tpa 		= gainTotal.tpaMinAnneeNeg;
			p.tpv 		= gainTotal.tpvMinAnneeNeg;
			strcpy(p.strategie, "MIN_ANNEE_NEG");
			break;

		case 3 : 
			p.dp 		= gainTotal.deltaPivotSommeAnneeNeg;
			p.tpa 		= gainTotal.tpaSommeAnneeNeg;
			p.tpv		= gainTotal.tpvSommeAnneeNeg;
			strcpy(p.strategie, "MIN_SOMME_ANNEE_NEG");
			break;
	}

	return p ; 
}

void verifierSiMeilleurSL(int l, int min, int max)
{
	if( gainTotalSL.meilleurGainSL < gainTotalSL.tGain[l]){
		gainTotalSL.meilleurGainSL = gainTotalSL.tGain[l];
		gainTotalSL.meilleurSL     = ABS_STOP_LOSS_MAX*l;
		printf("on trouve un meilleur sl : %d qui donne un gain: %.2f\n",gainTotalSL.meilleurSL, gainTotalSL.meilleurGainSL);
	}
	
	if( gainTotalSL.meilleurGainSLAchat < gainTotalSL.tGainAchat[l]){
		gainTotalSL.meilleurGainSLAchat = gainTotalSL.tGainAchat[l];
		gainTotalSL.meilleurSLAchat     = ABS_STOP_LOSS_MAX*l;
		//printf("on trouve un meilleur slAchat : %d qui donne un gain: %.2f\n",gainTotalSL.meilleurSLAchat, gainTotalSL.meilleurGainSLAchat);
	}
	if( gainTotalSL.meilleurGainSLVente < gainTotalSL.tGainVente[l]){
		gainTotalSL.meilleurGainSLVente = gainTotalSL.tGainVente[l];
		gainTotalSL.meilleurSLVente     = ABS_STOP_LOSS_MAX*l;
		//printf("on trouve un meilleur slVente : %d qui donne un gain: %.2f\n",gainTotalSL.meilleurSLVente, gainTotalSL.meilleurGainSLVente);
	}
}



void writeSLResultsToFile(params p, int slMin, int slMax, infoTraitement *infT)
{
	char filename[50] ;

	sprintf(filename, "resultats/%s_%s_SL.csv", infT->devise, p.strategie);
	FILE *fichierResSL = fopen(filename,"w");
	if(fichierResSL != NULL){
		fprintf(fichierResSL, "strategie: %s\n", p.strategie);
		fprintf(fichierResSL, "dP= %d, tpa= %d, tpv=%d, slMin=%d, slMax=%d\n\n",p.dp, p.tpa, p.tpv, slMin, slMax);
		fprintf(fichierResSL, "gain total sans stoploss; gain total avec stoploss; meilleur sl;gain achat;slAchat; gain vente;slVente; perte max absolue\n");
		fprintf(fichierResSL, "%.2f;%.2f;%d(%.2f€);%.2f;%d(%.2f€);%.2f;%d(%.2f€);%.2f\n\n",gainTotal.gainTotalMax, gainTotalSL.meilleurGainSL, gainTotalSL.meilleurSL,gainTotalSL.meilleurSL*MONTANT/10000.0,gainTotalSL.meilleurGainSLAchat, gainTotalSL.meilleurSLAchat, gainTotalSL.meilleurSLAchat*MONTANT/10000.0,gainTotalSL.meilleurGainSLVente, gainTotalSL.meilleurSLVente,gainTotalSL.meilleurSLVente*MONTANT/10000.0, gainTotalSL.perteMax);
		fprintf(fichierResSL, "stoploss;gain;gain Achat; gain Vente\n");
		for(int i=0;i<STOP_LOSS;i++)
		{
			fprintf(fichierResSL, "%d;%.2f;%.2f;%.2f\n",i*ABS_STOP_LOSS_MAX, gainTotalSL.tGain[i],gainTotalSL.tGainAchat[i], gainTotalSL.tGainVente[i]);
		}
		fclose(fichierResSL);
	}
	else {
		printf("pas possible douvrir le fichier\n");
	}
}

void optimisation(int anneeDebut, int anneeFin, infoTraitement * infT)
{
	int y, m, dp, tpa, tpv, moisNeg = 0, anneeNeg = 0;
	double gainMinMoisNeg, gainMinAnneeNeg, sommeGainAnneeNeg ;
	clock_t tDeb, tFin;

	tDeb = clock();

	if (infT->optimiserGain[0] == 'O')
	{
		gainTotal.nbMoisNeg = 120; gainTotal.nbAnneeNeg = 20;
		gainTotal.gainTotalMinMoisNeg = -1000000; gainTotal.gainTotalMinAnneeNeg = -1000000;
		gainTotal.gainTotalSommeAnneeNeg = -1000000; gainTotal.sommeGainAnneeNeg = -1000000;
		for ( dp = infT->dPivotMin; dp <= infT->dPivotMax; dp++)
		{
			for ( tpa= infT->tpaMin; tpa <= infT->tpaMax; tpa++)
			{
				for ( tpv = infT->tpvMin; tpv <= infT->tpvMax; tpv++)
				{
					moisNeg = 0; gainMinMoisNeg = 0;
					anneeNeg =0; gainMinAnneeNeg = 0;
					sommeGainAnneeNeg = 0;
					for ( y = anneeDebut; y <= anneeFin; y++)
					{
						for ( m = 0; m < MOIS; m++)
						{
							if (tGainMois[y][m].tGainA[dp + IDX_PIVOT_MAX][tpa] + tGainMois[y][m].tGainV[dp + IDX_PIVOT_MAX][tpv] < 0)
							{
								moisNeg++;
								//printf("mois neg: %d\n",moisNeg );
							}
							gainMinMoisNeg += tGainMois[y][m].tGainA[dp + IDX_PIVOT_MAX][tpa] + tGainMois[y][m].tGainV[dp + IDX_PIVOT_MAX][tpv] ;
						} // for mois
						if (tGainAnnee[y].tGainA[dp + IDX_PIVOT_MAX][tpa] + tGainAnnee[y].tGainV[dp + IDX_PIVOT_MAX][tpv] < 0)
							{
								anneeNeg ++;
								sommeGainAnneeNeg += tGainAnnee[y].tGainA[dp + IDX_PIVOT_MAX][tpa] + tGainAnnee[y].tGainV[dp + IDX_PIVOT_MAX][tpv];
								//printf("somme année neg= %f\n", sommeGainAnneeNeg);
							}
						gainMinAnneeNeg += tGainAnnee[y].tGainA[dp + IDX_PIVOT_MAX][tpa] + tGainAnnee[y].tGainV[dp + IDX_PIVOT_MAX][tpv] ;
					} // for annee
					if ( (( moisNeg < gainTotal.nbMoisNeg) && (gainMinMoisNeg > 0)) || ( (moisNeg == gainTotal.nbMoisNeg) && ( gainMinMoisNeg > gainTotal.gainTotalMinMoisNeg) ) )
					{
						gainTotal.nbMoisNeg = moisNeg;
						gainTotal.gainTotalMinMoisNeg = gainMinMoisNeg;
						gainTotal.tpvMinMoisNeg = tpv;
						gainTotal.tpaMinMoisNeg = tpa;
						gainTotal.deltaPivotMinMoisNeg = dp;
						//printf("total mois neg: %d\n", moisNeg);
					}
					if ( (( anneeNeg < gainTotal.nbAnneeNeg) && (gainMinAnneeNeg > 7000)) || ( (anneeNeg == gainTotal.nbAnneeNeg) && ( gainMinAnneeNeg > gainTotal.gainTotalMinAnneeNeg) ) )
					{
						gainTotal.nbAnneeNeg = anneeNeg;
						gainTotal.gainTotalMinAnneeNeg = gainMinAnneeNeg;
						gainTotal.tpvMinAnneeNeg = tpv;
						gainTotal.tpaMinAnneeNeg = tpa;
						gainTotal.deltaPivotMinAnneeNeg = dp;
						//printf("total mois neg: %d\n", moisNeg);
					}
					if ( (( sommeGainAnneeNeg > gainTotal.sommeGainAnneeNeg) && (gainMinAnneeNeg > 8500)) || ( (sommeGainAnneeNeg == gainTotal.sommeGainAnneeNeg) && ( gainMinAnneeNeg > gainTotal.gainTotalSommeAnneeNeg) ) )
					{
						gainTotal.sommeGainAnneeNeg 		= sommeGainAnneeNeg;
						gainTotal.gainTotalSommeAnneeNeg 	= gainMinAnneeNeg;
						gainTotal.tpvSommeAnneeNeg 			= tpv;
						gainTotal.tpaSommeAnneeNeg 			= tpa;
						gainTotal.deltaPivotSommeAnneeNeg 	= dp;
					}
				} // for tpv
			} // for tpa
		} // for dp
	} // if (paramAppli.optimiserGain == 'O')

	tFin = clock();
	paramAppli.tempsOpti = (double)(tFin - tDeb)/CLOCKS_PER_SEC;
}

void affichageResultat(int anneeDebut, int anneeFin, infoTraitement *infT)
{
	if (infT->afficher[0] == 'O')
	{
		printf("affichage résultat: %s\n",paramAppli.nomFichierRes);
		FILE *fichierResOpti = fopen(paramAppli.nomFichierRes,"a");
		if(fichierResOpti == NULL)
		{
			printf("Le fichier %s n'a pas pu etre ouvert \n", paramAppli.nomFichierRes);
			return; 
		} 

		printf("**********************   ESPACE MEMOIRE *********************************\n");
		printf("données minutes: %lu\npivot: %lu\ngain jour: %lu\n", sizeof(tableauDonnees) , sizeof(tPivot) , sizeof(tGainJour));
		printf("gain mois: %lu\ngain année: %lu\ngain total: %lu\n", sizeof(tGainMois) , sizeof(tGainAnnee) , sizeof(gainTotal));
		printf("Espace mémoire total alloué: %lu\n", sizeof(tableauDonnees) + sizeof(tPivot) + sizeof(tGainJour) + sizeof(tGainMois) + sizeof(tGainAnnee) + sizeof(gainTotal));
		printf("**************************************************************************\n\n");

		printf("**********************   TEMPS EXECUTION *********************************\n");
		printf("temps de lecture des fichiers: %2.3f s\n\n", paramAppli.tempsLectureFichiers);
		//printf("temps d'écriture du fichier BD: %2.3f s\n", paramAppli.tempsEcritureBD);
		printf("Temps calcul pivot: %f   temps calcul gain jour:%f\n", paramAppli.tempsCalculPivot, paramAppli.tempsCalculGainJour );
		printf("Temps calcul opti min mois neg: %f   \n", paramAppli.tempsOpti );
		printf("**************************************************************************\n\n");


		printf("**********************   RESULTATS OPTIMISATION *********************************\n");
		printf("gain opti max: %f \t gainOpti Achat: %f \t gainOpti Vente: %f\n",gainTotal.gainTotalMax, gainTotal.tGainA[ gainTotal.deltaPivotMax + IDX_PIVOT_MAX][ gainTotal.tpaGainMax], gainTotal.tGainV[ gainTotal.deltaPivotMax + IDX_PIVOT_MAX][gainTotal.tpvGainMax]);
		printf("gain opti max: %f \t deltaPivot: %d \t tpa: %d \t tpv: %d \n\n",gainTotal.gainTotalMax, gainTotal.deltaPivotMax, gainTotal.tpaGainMax, gainTotal.tpvGainMax);
		printf("%s\tJan.\tFev.\tMars\tAvr.\tMai\tJuin\tJuil.\tAout\tSept.\tOct.\tNov.\tDec.\t  Total\n", infT->devise);
		fprintf(fichierResOpti, "\n\nMaximisation du gain 2006-2015 pour %s\n", infT->devise);
		fprintf(fichierResOpti, "delta pivot;%d;tpa;%d;tpv;%d\n\n", gainTotal.deltaPivotMax, gainTotal.tpaGainMax, gainTotal.tpvGainMax);
		fprintf(fichierResOpti, "%s;Jan.;Fev.;Mars;Avr.;Mai;Juin;Juil.;Aout;Sept.;Oct.;Nov.;Dec.;Total\n", infT->devise);

		for (int i = anneeDebut; i <= anneeFin; i++)
		{
			fprintf(fichierResOpti, "%d;", i + 2000);
			printf("%d :\t", i + 2000);
			for (int m = 0; m < 12; m++)
			{
				fprintf(fichierResOpti, "%5.0f;", tGainMois[i][m].tGainA[gainTotal.deltaPivotMax + IDX_PIVOT_MAX][gainTotal.tpaGainMax] + tGainMois[i][m].tGainV[gainTotal.deltaPivotMax + IDX_PIVOT_MAX][gainTotal.tpvGainMax]);
				printf("%5.0f\t", tGainMois[i][m].tGainA[gainTotal.deltaPivotMax + IDX_PIVOT_MAX][gainTotal.tpaGainMax] + tGainMois[i][m].tGainV[gainTotal.deltaPivotMax + IDX_PIVOT_MAX][gainTotal.tpvGainMax]);
			}
			fprintf(fichierResOpti, "%5.0f\n", tGainAnnee[i].tGainA[gainTotal.deltaPivotMax + IDX_PIVOT_MAX][gainTotal.tpaGainMax] + tGainAnnee[i].tGainV[gainTotal.deltaPivotMax + IDX_PIVOT_MAX][gainTotal.tpvGainMax]);
			printf("= %5.0f\n", tGainAnnee[i].tGainA[gainTotal.deltaPivotMax + IDX_PIVOT_MAX][gainTotal.tpaGainMax] + tGainAnnee[i].tGainV[gainTotal.deltaPivotMax + IDX_PIVOT_MAX][gainTotal.tpvGainMax]);
		}
		printf("\n-----------------------\n");

		if (infT->optimiserGain[0] == 'O')
		{			printf("gain opti max mois  neg: %f nb mois   neg: %d \t deltaPivot: %d \t tpa: %d \t tpv: %d \n",gainTotal.gainTotalMinMoisNeg, gainTotal.nbMoisNeg, gainTotal.deltaPivotMinMoisNeg, gainTotal.tpaMinMoisNeg, gainTotal.tpvMinMoisNeg);
			printf("gain opti max annee neg: %f nb annees neg: %d \t deltaPivot: %d \t tpa: %d \t tpv: %d \n\n",gainTotal.gainTotalMinAnneeNeg, gainTotal.nbAnneeNeg, gainTotal.deltaPivotMinAnneeNeg, gainTotal.tpaMinAnneeNeg, gainTotal.tpvMinAnneeNeg);
			printf("%s\tJan.\tFev.\tMars\tAvr.\tMai\tJuin\tJuil.\tAout\tSept.\tOct.\tNov.\tDec.\t  Total\n", infT->devise);
			fprintf(fichierResOpti, "\n\nMinimisation du nombre d'annees negatives et maximisation du gain 2006-2015 pour %s\n", infT->devise);
			fprintf(fichierResOpti, "delta pivot;%d;tpa;%d;tpv;%d\n\n", gainTotal.deltaPivotMinAnneeNeg, gainTotal.tpaMinAnneeNeg, gainTotal.tpvMinAnneeNeg);
			fprintf(fichierResOpti, "%s;Jan.;Fev.;Mars;Avr.;Mai;Juin;Juil.;Aout;Sept.;Oct.;Nov.;Dec.;Total\n", infT->devise);
			for (int i = anneeDebut; i <= anneeFin; i++)
			{
				fprintf(fichierResOpti, "%d;", i + 2000);
				printf("%d :\t", i + 2000);
				for (int m = 0; m < 12; m++)
				{
					fprintf(fichierResOpti, "%5.0f;", tGainMois[i][m].tGainA[gainTotal.deltaPivotMinAnneeNeg  + IDX_PIVOT_MAX][gainTotal.tpaMinAnneeNeg] + tGainMois[i][m].tGainV[gainTotal.deltaPivotMinAnneeNeg + IDX_PIVOT_MAX][gainTotal.tpvMinAnneeNeg]);
					printf("%5.0f\t", tGainMois[i][m].tGainA[gainTotal.deltaPivotMinAnneeNeg  + IDX_PIVOT_MAX][gainTotal.tpaMinAnneeNeg] + tGainMois[i][m].tGainV[gainTotal.deltaPivotMinAnneeNeg + IDX_PIVOT_MAX][gainTotal.tpvMinAnneeNeg]);
				}
				fprintf(fichierResOpti, "%5.0f\n", tGainAnnee[i].tGainA[gainTotal.deltaPivotMinAnneeNeg + IDX_PIVOT_MAX][gainTotal.tpaMinAnneeNeg] + tGainAnnee[i].tGainV[gainTotal.deltaPivotMinAnneeNeg + IDX_PIVOT_MAX][gainTotal.tpvMinAnneeNeg]);
				printf("= %5.0f\n", tGainAnnee[i].tGainA[gainTotal.deltaPivotMinAnneeNeg + IDX_PIVOT_MAX][gainTotal.tpaMinAnneeNeg] + tGainAnnee[i].tGainV[gainTotal.deltaPivotMinAnneeNeg + IDX_PIVOT_MAX][gainTotal.tpvMinAnneeNeg]);
			}

			printf("\n-----------------------\n");
			printf("gain opti somme total annee neg: %f somme annees neg: %f \t deltaPivot: %d \t tpa: %d \t tpv: %d \n\n",gainTotal.gainTotalSommeAnneeNeg, gainTotal.sommeGainAnneeNeg, gainTotal.deltaPivotSommeAnneeNeg, gainTotal.tpaSommeAnneeNeg, gainTotal.tpvSommeAnneeNeg);
			printf("%s\tJan.\tFev.\tMars\tAvr.\tMai\tJuin\tJuil.\tAout\tSept.\tOct.\tNov.\tDec.\t  Total\n", infT->devise);
			fprintf(fichierResOpti, "\n\nMinimisation des pertes cumulees des annees negative et maximisation du gain 2006-2015 pour %s\n", infT->devise);
			fprintf(fichierResOpti, "delta pivot;%d;tpa;%d;tpv;%d\n\n", gainTotal.deltaPivotSommeAnneeNeg, gainTotal.tpaSommeAnneeNeg, gainTotal.tpvSommeAnneeNeg);
			fprintf(fichierResOpti, "%s;Jan.;Fev.;Mars;Avr.;Mai;Juin;Juil.;Aout;Sept.;Oct.;Nov.;Dec.;Total\n", infT->devise);
			for (int i = anneeDebut; i <= anneeFin; i++)
			{
				fprintf(fichierResOpti, "%d;", i + 2000);
				printf("%d :\t", i + 2000);
				for (int m = 0; m < 12; m++)
				{
					fprintf(fichierResOpti, "%5.0f;", tGainMois[i][m].tGainA[gainTotal.deltaPivotSommeAnneeNeg  + IDX_PIVOT_MAX][gainTotal.tpaSommeAnneeNeg] + tGainMois[i][m].tGainV[gainTotal.deltaPivotSommeAnneeNeg + IDX_PIVOT_MAX][gainTotal.tpvSommeAnneeNeg]);
					printf("%5.0f\t", tGainMois[i][m].tGainA[gainTotal.deltaPivotSommeAnneeNeg  + IDX_PIVOT_MAX][gainTotal.tpaSommeAnneeNeg] + tGainMois[i][m].tGainV[gainTotal.deltaPivotSommeAnneeNeg + IDX_PIVOT_MAX][gainTotal.tpvSommeAnneeNeg]);
				}
				fprintf(fichierResOpti, "%5.0f\n", tGainAnnee[i].tGainA[gainTotal.deltaPivotSommeAnneeNeg + IDX_PIVOT_MAX][gainTotal.tpaSommeAnneeNeg] + tGainAnnee[i].tGainV[gainTotal.deltaPivotSommeAnneeNeg + IDX_PIVOT_MAX][gainTotal.tpvSommeAnneeNeg]);
				printf("= %5.0f\n",  tGainAnnee[i].tGainA[gainTotal.deltaPivotSommeAnneeNeg + IDX_PIVOT_MAX][gainTotal.tpaSommeAnneeNeg] + tGainAnnee[i].tGainV[gainTotal.deltaPivotSommeAnneeNeg + IDX_PIVOT_MAX][gainTotal.tpvSommeAnneeNeg]);
			}
			printf("--------------------------\n");
			printf("meilleur gain avec le SL: %f\n",gainTotalSL.meilleurGainSL);
			printf("SL: %.2f€\n",gainTotalSL.meilleurSL*MONTANT/10000.0);
			printf("--------------------------\n");
			printf("**************************************************************************\n\n");
		} // if (paramAppli.optimiserGain == 'O')
		fclose(fichierResOpti);
	} // if (paramAppli.afficher == 'O')
}


void affichageRisque(int anneeDebut, int anneeFin)
{
	double cumulCourant=0, cumulMaxPerte=0;
	int tp;
	FILE *fichierResOpti = fopen(paramAppli.nomFichierRes,"a");
	for (int year = anneeDebut; year <= anneeFin ; year++)
	{
		for (int month = 0; month < MOIS ; month++)
		{
			for (int day = 0; day < JOUR ; day++)
			{
				//tp = tGainJour[year][month][day].achatOuVente == 0 ? gainTotal.tpvGainMax : gainTotal.tpaGainMax;
				cumulCourant += tGainJourPtf[year][month][day];
				if (cumulCourant > 0) cumulCourant = 0;
				cumulMaxPerte = MIN(cumulCourant, cumulMaxPerte);
			} // for day
		} // for month
	} // for year
	printf("Perte cumulée max du ptf: %f\n", cumulMaxPerte);
	if(fichierResOpti != NULL )
	{
		fprintf(fichierResOpti, "\n\nPerte cumulée max du ptf: %.2f\n", cumulMaxPerte);
		fclose(fichierResOpti);
	}
	else printf("impossible d'ouvrir %s\n",paramAppli.nomFichierRes );
}


/*
 * ajoute les gains jours de gainJourPtfSL et lorsque le resultat est > 0
 * se réinitialise à 0, permet de determiner la plus période la plus négative 
 * possible sur l'ensemble des données
 */
void affichageRisqueSL(int anneeDebut, int anneeFin)
{
	double cumulCourant=0, cumulMaxPerte=0;
	FILE *fichierResOpti = fopen(paramAppli.nomFichierRes,"a");
	for (int year = anneeDebut; year <= anneeFin ; year++)
	{
		for (int month = 0; month < MOIS ; month++)
		{
			for (int day = 0; day < JOUR ; day++)
			{
				//tp = tGainJour[year][month][day].achatOuVente == 0 ? gainTotal.tpvGainMax : gainTotal.tpaGainMax;
				cumulCourant += tGainJourPtfSL[year][month][day];
				if (cumulCourant > 0) cumulCourant = 0;
				cumulMaxPerte = MIN(cumulCourant, cumulMaxPerte);
			} // for day
		} // for month
	} // for year
	printf("Perte cumulée max du ptf (SL): %f\n", cumulMaxPerte);
	if(fichierResOpti != NULL )
	{
		fprintf(fichierResOpti, "\n\nPerte cumulée max du ptf: %.2f\n", cumulMaxPerte);
		fclose(fichierResOpti);
	}
	else printf("impossible d'ouvrir %s\n",paramAppli.nomFichierRes );
}

void cumulGainPtf(int anneeDebut, int anneeFin, infoTraitement *infT)
{
	double cumulCourant=0, cumulMaxPerte=0;
	FILE *fichierResOpti = fopen(paramAppli.nomFichierRes,"a");
	int tp;
	for (int year = anneeDebut; year <= anneeFin ; year++)
	{
		for (int month = 0; month < MOIS ; month++)
		{
			for (int day = 0; day < JOUR ; day++)
			{
				tp = tGainJour[year][month][day].achatOuVente == 0 ? gainTotal.tpvGainMax : gainTotal.tpaGainMax;
				cumulCourant += tGainJour[year][month][day].tGain[gainTotal.deltaPivotMax + IDX_PIVOT_MAX][tp];
				if (cumulCourant > 0) cumulCourant = 0;
				cumulMaxPerte = MIN(cumulCourant, cumulMaxPerte);
				tGainJourPtf[year][month][day] += tGainJour[year][month][day].tGain[gainTotal.deltaPivotMax + IDX_PIVOT_MAX][tp];
			} // for day
		} // for month
	} // for year
	printf("Perte cumulée max(%s): %f€\n", infT->devise, cumulMaxPerte);
	if(fichierResOpti != NULL )
	{
		fprintf(fichierResOpti, "\nPerte cumulée max(%s): %.2f€\n", infT->devise, cumulMaxPerte);
		fclose(fichierResOpti);
	}
	else printf("impossible d'ouvrir %s\n",paramAppli.nomFichierRes );
}


/* 
 * Ecrit les résultats des stop loss dans des fichiers et le cumule de la perte max
 * Remplit le tableau de structure tGainJourPtfSL
 * Ecrit dans un fichier gain pour un SL tres large 
 * (sufisamment large pour qu'on considere qu'il n'y ait pas de SL)
 * Affichera 0 si on selectionne un sl particulier dans les parametres
*/
void cumulGainPtfSL(int anneeDebut, int anneeFin, int loss, infoTraitement *infT)
{
	double cumulCourant=0, cumulMaxPerte=0;
	char filename[100];
	sprintf(filename,"resultats/jour/%s_jour.csv",infT->devise) ; 

	FILE* fd = fopen(filename,"w");
	if(fd != NULL) fprintf(fd, "date;gain sans SL;gain SL = %.2f€;type;sl;tp\n", loss*ABS_STOP_LOSS_MAX*MONTANT/10000.0);

	for (int year = ANNEE_DEB; year <= ANNEE_FIN ; year++)
	{
		for (int month = 0; month < MOIS ; month++)
		{
			for (int day = 0; day < JOUR ; day++)
			{
				if(fd != NULL) fprintf(fd, "%d/%d/%d;%.2f;%.2f;%c;%c;%c\n",day+1,month+1,year,tGainJourSL[year][month][day].tGain[80],tGainJourSL[year][month][day].tGain[loss], tGainJourSL[year][month][day].achatOuVente, tGainJourSL[year][month][day].isSl[loss], tGainJourSL[year][month][day].isTp);

				cumulCourant += tGainJourSL[year][month][day].tGain[loss];
				if (cumulCourant > 0) cumulCourant = 0;
				cumulMaxPerte = MIN(cumulCourant, cumulMaxPerte);
				tGainJourPtfSL[year][month][day] += tGainJourSL[year][month][day].tGain[loss];
			}
		}
	}
	fclose(fd);

	FILE *fichierResOpti = fopen(paramAppli.nomFichierRes,"a");

	printf("Perte cumulée max(%s) pour un SL = %.2f€ : %f€\n", infT->devise,loss*ABS_STOP_LOSS_MAX*MONTANT/10000.0, cumulMaxPerte);
	if(fichierResOpti != NULL )
	{
		fprintf(fichierResOpti, "\n\nPerte cumulée max (%s) (SL=%.2f€): %.2f€\n", infT->devise, loss*ABS_STOP_LOSS_MAX*MONTANT/10000.0, cumulMaxPerte);
		fclose(fichierResOpti);
	}
	else printf("impossible d'ouvrir %s\n",paramAppli.nomFichierRes );
}



