// gcc -c HSKA.c -o HSKA.o
// ar -q libHSKA.a HSKA.o
// CHAINE DE COMPIL   gcc simulateurOptiV0-5.c -Iinclude -Llib -lHSKA -o simOpti

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "include/HSKA.h"



//#define DEBUG_MOY 			1
//#define DEBUG_SL				1
//#define DEBUG_MOY_SL			1
//#define DEBUG_VOL				1

// CONFIG
#define CONTRATS_PAR_VOL	100000
#define STOP 				3
#define VOLUME 				0.3
#define MONTANT				30000
#define SPREAD_PRICE		MONTANT/10000
#define ANNEE_DEB			6
#define ANNEE_FIN			15
#define ABS_STOP_LOSS_MAX   10			// En realité le pas 
#define MINUTE_DEBUT		180
#define MINUTE_FIN_MIN		660
#define MINUTE_FIN_MAX		780			// 720 -> 18h
#define STRATEGIE_SL		1          // 1 = MAX ;   2 = MIN_ANNEE_NEG  ;  3 = MIN_SOMME_ANNEE_NEG ;
#define NB_VOL_REF			5 
#define DCLGE_HOR				6

// TAILLES TABLEAUX
#define TAILLE_LIGNE_MAX 	1000
#define LEVIER					600
#define IDX_PIVOT_MAX 		300 		// ok version 0.3
#define IDX_TP_MAX 			300 		// ok version 0.3
#define DPIVOT 				601 		// ok version 0.3
#define TAKE_PROFIT			501			// ok version 0.3
#define STOP_LOSS			501
#define DM					1001
#define RM					101
#define ANNEE				18		  	// ok version 0.3
#define MOIS 				12			// ok version 0.3
#define JOUR 				31			// ok version 0.3
#define MINUTE 				1500		// ok version 0.3

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
	char 	achatOuVente[DPIVOT]			; // 0 : Vente 			-  1 : Achat
	char 	tpOuCloture[DPIVOT][TAKE_PROFIT]; // 1 : take profit 	-  0 : cloture
	double 	tGain[DPIVOT][TAKE_PROFIT];
	// double	gainHMax 				;
	// char 		isTpHmax;
};


// Structure qui recupere le gain à la journée en fonction du STOPLOSS
typedef struct sGainJourSL sGainJourSL ;  // ok version 0.3
struct sGainJourSL
{
	double 	tGain[STOP_LOSS];
	double	tGainA[STOP_LOSS];
	double	tGainV[STOP_LOSS];
	char	isSl[STOP_LOSS]; // 0 : false 			-  1 : true
	char 	isTp[STOP_LOSS]; // 0 : false 			-  1 : true
	char 	achatOuVente; // 0 : Vente 			-  1 : Achat

};

typedef struct sGainJourHeure sGainJourHeure;
struct sGainJourHeure
{
	double gain;
	char isTp;
};

typedef struct sGainJourHeureSL sGainJourHeureSL;
struct sGainJourHeureSL
{
	double gain;
	char isTp;
	char isSl;
};



typedef struct sGainJourMOY sGainJourMOY ;  // ok version 0.3
struct sGainJourMOY
{
	double tGain[DM][RM];
	int nbMoy;
};

typedef struct sGainJourVOL sGainJourVOL ;  // ok version 0.3
struct sGainJourVOL
{
	double tGain[TAKE_PROFIT];
	char isTp[TAKE_PROFIT];
	char achatOuVente; // 0 :achat - 1 : vente
};


// detail du resultat sur 1 mois 
// ...MinJrNeg ne sont pas encore remplis
typedef struct sGainMois sGainMois ;  // ok version 0.3
struct sGainMois
{
	double  tGainA[DPIVOT][TAKE_PROFIT];
	double  tGainV[DPIVOT][TAKE_PROFIT];
	int tpaMinJrNeg;
	int tpvMinJrNeg;
	int nbAchat;
	int nbVente;
};

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
	double 	gainTotalMax;
	//double 	gainTotalHeureMax;
	double	gainTotalMinMoisNeg;
	double	gainTotalMinAnneeNeg;
	double	gainTotalSommeAnneeNeg;
	double	sommeGainAnneeNeg;
	double  tGainA[DPIVOT][TAKE_PROFIT];
	double  tGainV[DPIVOT][TAKE_PROFIT];
	//int 	hFermetureMax;
	int 	deltaPivotMax;
	int 	deltaPivotMinMoisNeg;
	int 	deltaPivotMinAnneeNeg;
	int 	deltaPivotSommeAnneeNeg	;
	int	tpaGainMax;
	int	tpvGainMax;
	int	tpaMinJrNeg;
	int	tpvMinJrNeg;
	int	tpaMinMoisNeg;
	int	tpvMinMoisNeg;
	int	tpaMinAnneeNeg;
	int	tpvMinAnneeNeg;
	int	tpaSommeAnneeNeg;
	int	tpvSommeAnneeNeg;
	int	nbAchat;
	int	nbVente;
	int 	nbMoisNeg;
	int 	nbAnneeNeg;
} ;

typedef struct sGainTotalSL sGainTotalSL ;  // ok version 0.3
struct sGainTotalSL
{
	char	strategie;
	double  tGain[STOP_LOSS];
	double  tGainAchat[STOP_LOSS];
	double  tGainVente[STOP_LOSS];
	double 	meilleurGainSL;
	double 	meilleurGainSLAchat;
	double 	meilleurGainSLVente;
	double 	perteMax;
	int	maxSL;
	int 	maxSLAchat;
	int 	maxSLVente;
	int 	dp;
	int 	tpa;
	int 	tpv;
};

typedef struct sGainTotalHeure sGainTotalHeure ;  // ok version 0.3
struct sGainTotalHeure
{
	int heureF;
	int heureFavecSL;
	double gainHeureMax;
	double gainHeureMaxSL;
	int dp;
	int tpa;
	int tpv;
};

typedef struct sGainTotalMOY sGainTotalMOY ;  // ok version 0.3
struct sGainTotalMOY
{
	double meilleurGainMoy;
	double tGain[DM][RM];
	int meilleurDM; // departMoy
	int meilleurRM; // reclacherMoy
	int dp;
	int tpa;
	int tpv;
};

typedef struct sGainTotalMOYetSL sGainTotalMOYetSL ;  // ok version 0.3
struct sGainTotalMOYetSL
{
	double 	meilleurGainMOYetSL;
	double  tGain[DM][RM][STOP_LOSS];
	int   maxSL;
	int	meilleurDM; // departMoy
	int	meilleurRM; // reclacherMoy
	int 	dp;
	int 	tpa;
	int 	tpv;
};

typedef struct sGainTotalVOL sGainTotalVOL ;  // ok version 0.3
struct sGainTotalVOL
{
	double 	gainTotalMax;
	double  gainAchatMax;
	double  gainVenteMax;
	int 	tpaMax;
	int 	tpvMax;
	double  tGainA[TAKE_PROFIT];
	double  tGainV[TAKE_PROFIT];
	int  	nbTpa[TAKE_PROFIT];
	int  	nbTpv[TAKE_PROFIT];
	int 	dP;
};


typedef struct chgtHeure chgtHeure ;  // ok version 0.3
struct chgtHeure
{
	int moisParisHE, jourParisHE;
	int moisParisHH, jourParisHH;
	int moisUSHE, jourUSHE;
	int moisUSHH, jourUSHH;
} ;

typedef struct sParamTp sParamTp ;  // ok version 0.3
struct sParamTp
{
	double open;
	int tp;
	double moyAjusteVol;
	char achatOuVente;
} ;


typedef struct infoTraitement infoTraitement ;  // contient les traitements à réaliser pour chaque devise
struct infoTraitement
{
	char *devise;
	int  valeurPip;
	char *nomFichierDevise;
	int   hDepart, hFin;
	int   dPivotMin, dPivotMax;
	int   tpaMin, tpaMax;
	int   tpvMin, tpvMax;
	int   slMin, slMax;
	int   departMoyMin, departMoyMax;
	int   relacherMoyMin, relacherMoyMax;
	int   ecartSSMin, ecartSSMax;
	char *creationBD;
	char *optimiserGain;
	char *optimiserSL;
	char *optimiserMOY;
	char *optimiserSS;
	char *optimiserMOYavecSL;
	char *afficher;
	char *ecrireGainJ;
};

typedef struct sparamAppli sparamAppli;  // ok version 0.3
struct sparamAppli
{
	char 	repertoireData[200];
	char 	repertoireRes[200];
	char 	fichierAlire[500]	;
	char 	fichierHE[200]	;
	char 	fichierBD[500]	;
	char 	fichierHeureBD[500];
	char 	nomFichierDevise[100];
	char 	nomFichierRes[100];
	infoTraitement listeDevise[50];
	double 	tempsLectureFichiers;
	double 	tempsEcritureBD;
	double 	tempsCalculPivot;
	double 	tempsCalculGainJour;
	double   tempsCalculGainMinuteStopLoss;
	double	tempsOptimiserTpAvecVolatilite;
	double 	tempsOpti;
} paramAppli;

typedef struct params params;
struct params 
{
	int dp, tpa, tpv;
	char strategie[50];
}; 


dataMinute 				tableauDonnees[ANNEE][MOIS][JOUR][MINUTE]  ;  // ok version 0.3
double					tPivot[ANNEE][MOIS][JOUR];
double					tVol[ANNEE][MOIS][JOUR]; // volatilité
// Pour calcul en deux temps
double					tMoy[ANNEE][MOIS][JOUR]; 
double					tVolDeuxPasses[ANNEE][MOIS][JOUR]; // volatilité
//
sGainJour 				tGainJour[ANNEE][MOIS][JOUR];
sGainJourSL 			tGainJourSL[ANNEE][MOIS][JOUR];
sGainJourSL 			tGainJourSL_VOL[ANNEE][MOIS][JOUR];
sGainJourVOL			tGainJourVOL[ANNEE][MOIS][JOUR];

// stocke le res de 18h FIXE ET VOL
sGainJourHeure			tGainJourH18[ANNEE][MOIS][JOUR];
sGainJourHeureSL		tGainJourH18SL[ANNEE][MOIS][JOUR];
sGainJourHeure			tGainJourH18_VOL[ANNEE][MOIS][JOUR];
sGainJourHeureSL		tGainJourH18SL_VOL[ANNEE][MOIS][JOUR]; 

// stocke les res de la meilleure heure SL FIXE ET VOL
sGainJourHeure 		tGainJourHeure[ANNEE][MOIS][JOUR];
sGainJourHeureSL 		tGainJourHeureSL[ANNEE][MOIS][JOUR];
sGainJourHeure 		tGainJourHeure_VOL[ANNEE][MOIS][JOUR];
sGainJourHeureSL 		tGainJourHeureSL_VOL[ANNEE][MOIS][JOUR]; 

double	 				tGainJourPtf[ANNEE][MOIS][JOUR];
double	 				tGainJourPtfSL[ANNEE][MOIS][JOUR];

sGainMois				tGainMois[ANNEE][MOIS];
sGainMoisSL				tGainMoisSL[ANNEE][MOIS];

sGainAnnee				tGainAnnee[ANNEE];
sGainAnneeSL			tGainAnneeSL[ANNEE];

sGainTotal				gainTotal;
sGainTotalSL 			gainTotalSL;
sGainTotalSL 			gainTotalSL_VOL;
sGainTotalMOY 			gainTotalMOY;
sGainTotalMOYetSL 		gainTotalMOYetSL;
sGainTotalVOL			gainTotalVOL;
sGainTotalHeure			gainTotalHeure;
sGainTotalHeure			gainTotalHeureVOL;



chgtHeure heureEteHiver[20];		// ok version 0.3
double res, deltaPivot;
int minuteFin = MINUTE_FIN_MIN ;
char *tCor[20] = {"[pathToData]","[pathToResults]"}; // tableau de correspondance

int  lectureFichierParam(char *nomFichierParam);
void initialisation(infoTraitement *infT);
void lectureFichiersData(infoTraitement *infT);
void lectureFichierHeureBD(char* fichierAlire);
void lectureFichierBD(char* fichierAlire);
int  lectureFichierHeureHE(char* fichierAlire);
void lectureFichierData(char* fichierAlire);
void ecritureBD(infoTraitement *infT);
void calculGains(infoTraitement *infT);
int calculDeltaMin(int year, int month, int day);
void calculDuPivotEtVolatilite(int minuteDebut, int minuteFin);
double calculGainJourRapide(int anneeDebut, int anneeFin, int dPivot, double gainAchat, double gainVente, infoTraitement * infT);
void calculGainMinuteStopLoss(int anneeDebut, int anneeFin, infoTraitement * infT /*,double (*calculTp)(struct paramTp pTp) */);
void calculGainMinuteSLetVOL(int anneeDebut, int anneeFin, infoTraitement * infT);
void optimisation(int anneeDebut, int anneeFin,  infoTraitement * infT);
void affichageResultat(int anneeDebut, int anneeFin, infoTraitement *infT);
void affichageRisque(int anneeDebut, int anneeFin);
void affichageRisqueSL(int anneeDebut, int anneeFin);
void cumulGainPtf(int anneeDebut, int anneeFin, infoTraitement *infT);
void cumulGainPtfSL(int anneeDebut, int anneeFin, int loss, infoTraitement *infT);
void afficherResDebug();
void ecrireSLdansFichier(params p, int slMin, int slMax, infoTraitement *infT);
void verifierSiMeilleurSL(int l, int max, int min);
params getStrategy();
void writeDayResultWithSL(int loss, infoTraitement *infT );
void viderFichierResultat(int mF);
void setPathsInputOutputData(void);
void calculMeilleureMoyenne(int anneeDebut, int anneeFin, infoTraitement * infT);
//void randomizePivot(void);
//void pickRandomDataDay(int tab[]);
//void writeRandomDaysToFile(int tab[], infoTraitement* infT);
void generateRandomTestFile(infoTraitement* infT);
void ecrireSLAetSLVdansFichier(params p, infoTraitement *infT);
void calculGainSLetMOY(int anneeDebut, int anneeFin, infoTraitement * infT);
void optimiserTpAvecVolatilite(int anneeDebut, int anneeFin, int dP, infoTraitement * infT);
void ecritureGainsJour(int anneeDebut, int anneeFin, infoTraitement *infT);
void ecritureGainsJourVOL(int anneeDebut, int anneeFin, infoTraitement *infT);

/*
 * genere une valeur aleatoire pour le pivot (0 ou 10000)
 * permet de simuler des achat/vente aleatoire pour tester la 
 * strategie aléatoire contre notre strategie 
 * appelée dans gainsJour
 */
void randomizePivot()
{
	double pivotJournee = 0   ;
	dataMinute *donneeMinute ;
	for (int year = 0; year < ANNEE; year++)
	{
		for (int month = 0; month < MOIS; month++)
		{
			for (int day = 0; day < JOUR; day++)
			{
				tPivot[year][month][day] = round(rand() / (double)RAND_MAX)*10000;
				printf("%f\n", tPivot[year][month][day]);
			}
		}
	}
}

/*
 * genere 50 valeurs aleatoires 
 * dans un tableau fournit en parametre
 * appelée dans generateRandomTestFile()
 */
void pickRandomDataDay(int tab[])
{
	char listOfDays[2600] = {0};
	//int listSelectedDays[50] = {0};
	int randDay;
	int daysPicked = 0;

	while(daysPicked < 50)
	{
		randDay = rand() % (2575 + 1);
		//printf("randDay = %d\n",randDay);
		if(listOfDays[randDay] == 0 )
		{
			tab[daysPicked] = randDay;
			//printf("tab[%d]=%d\n",daysPicked, tab[daysPicked] );
			//listSelectedDays[daysPicked] = randDay;
			daysPicked++;
			listOfDays[randDay] = 1;
			
		}
	}
}

/*
 * prend en parametre un tableau de valeur (date exprimées en jour) 
 * et ecrit les valeurs OHLC des journées indiquées 
 * appelée dans generateRandomTestFile()
 */
void writeRandomDaysToFile(int tab[], infoTraitement* infT)
{
	dataMinute *donneeMinute;
	int year, month, day;
	char fname[500];
	
	sprintf(fname, "%srandomDraw_%s.csv", paramAppli.repertoireData, infT->devise);
	FILE* fd = fopen(fname,"w");
	if(fd == NULL) 
	{
		printf("Pas possible d'ouvrir le fichier %s\n", fname);
		return ;
	} 
	fprintf(fd, "date;minute;open;high;low;close\n");

	for(int i=0; i < 50; i++)
	{
		year 	=  tab[i] / 365 + ANNEE_DEB;
		month 	= (tab[i] % 365) / 30;
		day     = (tab[i] % 365) % 30;
		//printf("%d/%d/%d: tab[%d]=%d, tab[%d]/365 = %d\n", day+1, month+1, year, i, tab[i], i, tab[i]/365);
		int deltaMin = calculDeltaMin(year, month+1, day+1);

		for (int min = MINUTE_DEBUT + deltaMin; min <= minuteFin + deltaMin ; min++)
		{
			donneeMinute = &tableauDonnees[year][month][day][min];
			fprintf(fd, "%d/%d/%d;%d;%.5f;%.5f;%.5f;%.5f\n",day+1, month+1, year+2000, min, donneeMinute->open, donneeMinute->high, donneeMinute->low, donneeMinute->close );
		}
	}
	fclose(fd);
}

/*
 * permet de stocker la strategie utilisé dans une 
 * structure params (strategie, dp, tpa, tpv)
 * Pour toute nouvelle strategie on doit rajouter un "case" 
 * Retourne une structure params
 */
params getStrategy(void)
{
	params p;

	switch(STRATEGIE_SL) 
	{
		case 1 : 
			p.dp 		= gainTotal.deltaPivotMax;
			p.tpa 	= gainTotal.tpaGainMax;
			p.tpv 	= gainTotal.tpvGainMax;
			strcpy(p.strategie, "MAX");
			break;

		case 2 : 
			p.dp 		= gainTotal.deltaPivotMinAnneeNeg;
			p.tpa 	= gainTotal.tpaMinAnneeNeg;
			p.tpv 	= gainTotal.tpvMinAnneeNeg;
			strcpy(p.strategie, "MIN_ANNEE_NEG");
			break;

		case 3 : 
			p.dp 		= gainTotal.deltaPivotSommeAnneeNeg;
			p.tpa 	= gainTotal.tpaSommeAnneeNeg;
			p.tpv		= gainTotal.tpvSommeAnneeNeg;
			strcpy(p.strategie, "MIN_SOMME_ANNEE_NEG");
			break;
	}

	return p ; 
}

double calculTpCommeVol(sParamTp p)
{
	return p.open + p.tp * p.moyAjusteVol / 100.0;
}

double calculTpPure(sParamTp p)
{
	return p.open * (1.0 + p.tp / 10000.0);
}



int main(int argc, char *argv[])
{

	
	memset(tGainJourHeure,0,sizeof(tGainJourHeure));
	memset(tGainJourHeureSL,0,sizeof(tGainJourHeureSL));
	memset(&gainTotalHeure,0,sizeof(gainTotalHeure));
	
	setPathsInputOutputData();

	for(int m = MINUTE_FIN_MIN; m <= MINUTE_FIN_MAX; m+= 60 )
	{
		minuteFin = m ;	
		printf("%s && %s\n", paramAppli.repertoireData, paramAppli.repertoireRes );

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

			//generateRandomTestFile(&paramAppli.listeDevise[i]);
			//printf("apres lecture\n");
			ecritureBD(&paramAppli.listeDevise[i]);
			calculGains(&paramAppli.listeDevise[i]);
			optimisation(ANNEE_DEB, ANNEE_FIN, &paramAppli.listeDevise[i]);
			optimiserTpAvecVolatilite(ANNEE_DEB, ANNEE_FIN, gainTotal.deltaPivotMax, &paramAppli.listeDevise[0]);
			calculGainMinuteStopLoss(ANNEE_DEB, ANNEE_FIN, &paramAppli.listeDevise[i]);
			calculGainMinuteSLetVOL(ANNEE_DEB, ANNEE_FIN, &paramAppli.listeDevise[i]);
			calculMeilleureMoyenne(ANNEE_DEB, ANNEE_FIN, &paramAppli.listeDevise[i]);
			calculGainSLetMOY(ANNEE_DEB, ANNEE_FIN, &paramAppli.listeDevise[i]);
			affichageResultat(ANNEE_DEB, ANNEE_FIN, &paramAppli.listeDevise[i]);
			ecritureGainsJour(ANNEE_DEB, ANNEE_FIN, &paramAppli.listeDevise[i]);
			ecritureGainsJourVOL(ANNEE_DEB, ANNEE_FIN, &paramAppli.listeDevise[i]);
			cumulGainPtf(ANNEE_DEB, ANNEE_FIN, &paramAppli.listeDevise[i]);
			cumulGainPtfSL(ANNEE_DEB, ANNEE_FIN, gainTotalSL.maxSL/10, &paramAppli.listeDevise[i]);

		}
		affichageRisque(ANNEE_DEB, ANNEE_FIN);
		affichageRisqueSL(ANNEE_DEB, ANNEE_FIN);
		//afficherResDebug();
	}
    return 0;
}

/*
 * ecrit dans un fichier des journées d'une devise au hasard
 * ecrit une 50aine de journée
 *
 */
void generateRandomTestFile(infoTraitement* infT)
{
	int randomDates[50] = {0};

	pickRandomDataDay(randomDates);
	writeRandomDaysToFile(randomDates, infT);
}

/*
 * rendre le fichier de resultats généraux vierges
 * le parametre est actuellment inutilisé
 */
void viderFichierResultat(int mF)
{
	char filename[500] ;
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

/*
 * lit le fichier SimulateurParams.csv et
 * stocke toutes les infos dans paramAppli
 */
int lectureFichierParam(char *nomFichierParam)
{
	
	int nbDevises = 0;
	char **ligne, l[300];
	int size = 500;
	char titre[500] ;
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
    		paramAppli.listeDevise[nbDevises].devise 				= ligne[0];
    		paramAppli.listeDevise[nbDevises].nomFichierDevise 		= ligne[1];
    		paramAppli.listeDevise[nbDevises].valeurPip			= atoi(ligne[2]);
    		paramAppli.listeDevise[nbDevises].hDepart				= atoi(ligne[3]);
    		paramAppli.listeDevise[nbDevises].hFin					= atoi(ligne[4]);
    		paramAppli.listeDevise[nbDevises].dPivotMin 			= atoi(ligne[5]);
    		paramAppli.listeDevise[nbDevises].dPivotMax 			= atoi(ligne[6]);
    		paramAppli.listeDevise[nbDevises].tpaMin 				= atoi(ligne[7]);
    		paramAppli.listeDevise[nbDevises].tpaMax 				= atoi(ligne[8]);
    		paramAppli.listeDevise[nbDevises].tpvMin 				= atoi(ligne[9]);
    		paramAppli.listeDevise[nbDevises].tpvMax 				= atoi(ligne[10]);
    		paramAppli.listeDevise[nbDevises].slMin 				= atoi(ligne[11]);
    		paramAppli.listeDevise[nbDevises].slMax 				= atoi(ligne[12]);
    		paramAppli.listeDevise[nbDevises].departMoyMin 			= atoi(ligne[13]);
    		paramAppli.listeDevise[nbDevises].departMoyMax 			= atoi(ligne[14]);
    		paramAppli.listeDevise[nbDevises].relacherMoyMin 		= atoi(ligne[15]);
    		paramAppli.listeDevise[nbDevises].relacherMoyMax 		= atoi(ligne[16]);
    		paramAppli.listeDevise[nbDevises].ecartSSMin 			= atoi(ligne[17]);
    		paramAppli.listeDevise[nbDevises].ecartSSMax 			= atoi(ligne[18]);
    		paramAppli.listeDevise[nbDevises].creationBD 			= ligne[19];
    		paramAppli.listeDevise[nbDevises].optimiserGain			= ligne[20];
    		paramAppli.listeDevise[nbDevises].optimiserSL			= ligne[21];
    		paramAppli.listeDevise[nbDevises].optimiserMOY			= ligne[22];
    		paramAppli.listeDevise[nbDevises].optimiserSS 			= ligne[23];
    		paramAppli.listeDevise[nbDevises].optimiserMOYavecSL	= ligne[24];
    		paramAppli.listeDevise[nbDevises].afficher 	 			= ligne[25];
    		paramAppli.listeDevise[nbDevises].ecrireGainJ 	 		= ligne[26];

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

/*
 * lit le fichier configSimu.cfg et recupere 
 * les chemin de resultats et le chemin des données
 */
void setPathsInputOutputData(void)
{
	char l[200];
	FILE* fd = fopen("configSimu.cfg","r");
	if(fd != NULL)
	{
		while (! feof(fd))
    	{
    		fscanf(fd, "%s",l);
    		//int len = sizeof(tCor)/sizeof(tCor[0]);
			int i;

			for(i = 0; i < 2; ++i)
			{
			    if(!strcmp(tCor[i], l))
			    {
			        switch(i)
			        {
			        	case 0 : 
			        		fscanf(fd, "%s",l);
			        		strcpy(paramAppli.repertoireData, l);
			        		break;
			        	case 1 : 
			        		fscanf(fd, "%s",l);
			        		strcpy(paramAppli.repertoireRes, l);
			        		break;
			        	default:
			        		break;
			        }
			    }
			}
    	}
		fclose(fd);
	}


	// strcpy(paramAppli.repertoireData, "/Users/kant1_sahal/Desktop/QuentinCocoon/donnees/HistData/ptf/archives/");
	// strcpy(paramAppli.repertoireRes, "resultats/");
	sprintf(paramAppli.nomFichierRes, "%sresultats_9h_%dh.csv",paramAppli.repertoireRes,(minuteFin+360)/60);
}

void initialisation(infoTraitement *infT)
{
	strcpy(paramAppli.fichierAlire, paramAppli.repertoireData);
	strcat(paramAppli.fichierAlire, infT->nomFichierDevise) ;
	strcpy(paramAppli.fichierHE, paramAppli.repertoireData);
	strcat(paramAppli.fichierHE, "HeureEteHiver.csv") ;
	strcpy(paramAppli.fichierBD, paramAppli.repertoireData);
	strcat(paramAppli.fichierBD, infT->nomFichierDevise) ;

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
	memset(tGainJourVOL,0,sizeof(tGainJourVOL));
	
	//memset(gainJourPtf,0,sizeof(gainJourPtf));
	memset(&gainTotal,0,sizeof(gainTotal));
	memset(&gainTotalSL,0,sizeof(gainTotalSL));
	memset(&gainTotalSL_VOL,0,sizeof(gainTotalSL_VOL));

	memset(&gainTotalMOY,0,sizeof(gainTotalMOY));
	memset(&gainTotalVOL,0,sizeof(gainTotalVOL));
	memset(&gainTotalMOYetSL,0,sizeof(gainTotalMOYetSL));
	
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

/*
 * appelée dans comparerMeilleureHeure et comparer18Heure
 * permet de remplir une struct sGainJourHeure à partir de la structure sGainJour
 * 
 */
void remplirStructJour(int anneeDebut, int anneeFin, sGainJourHeure tabJ[ANNEE][MOIS][JOUR])
{
	int idx_dP = gainTotal.deltaPivotMax + IDX_PIVOT_MAX;
	int idx_tpv = gainTotal.tpvGainMax;
	int idx_tpa = gainTotal.tpaGainMax;

	for (int year = anneeDebut; year <= anneeFin ; year++)
	{
		for (int month = 0; month < MOIS ; month++)
		{
			for (int day = 0; day < JOUR ; day++)
			{
				if(tGainJour[year][month][day].achatOuVente[idx_dP] == 1) // achat
				{
					tabJ[year][month][day].gain = tGainJour[year][month][day].tGain[idx_dP][idx_tpa];
					tabJ[year][month][day].isTp = tGainJour[year][month][day].tpOuCloture[idx_dP][idx_tpa];
				}
				else 
				{
					tabJ[year][month][day].gain = tGainJour[year][month][day].tGain[idx_dP][idx_tpv];
					tabJ[year][month][day].isTp = tGainJour[year][month][day].tpOuCloture[idx_dP][idx_tpv];
				}
			}
		}
	}
}

/*
 * appelée dans comparerMeilleureHeureSL et comparer18HeureSL
 * permet de remplir une struct sGainJourHeureSL à partir de la structure sGainJourSL
 * 
 */
void remplirStructJourSL(int anneeDebut, int anneeFin, sGainJourHeureSL tabJ[ANNEE][MOIS][JOUR])
{
	int idx_sl = gainTotalSL.maxSL / 10;

	for (int year = anneeDebut; year <= anneeFin ; year++)
	{
		for (int month = 0; month < MOIS ; month++)
		{
			for (int day = 0; day < JOUR ; day++)
			{
				tabJ[year][month][day].gain = tGainJourSL[year][month][day].tGain[idx_sl];
				tabJ[year][month][day].isTp = tGainJourSL[year][month][day].isTp[idx_sl];
				tabJ[year][month][day].isSl = tGainJourSL[year][month][day].isSl[idx_sl];
			}
		}
	}
}

/*
 * appelée dans comparerMeilleureHeureVOL et comparer18HeureVOL
 * permet de remplir une struct sGainJourHeure à partir de la structure sGainJourVOL
 * 
 */
void remplirStructJour_VOL(int anneeDebut, int anneeFin, sGainJourHeure tabJ[ANNEE][MOIS][JOUR])
{
	int idx_tpv = gainTotalVOL.tpvMax;
	int idx_tpa = gainTotalVOL.tpaMax;

	for (int year = anneeDebut; year <= anneeFin ; year++)
	{
		for (int month = 0; month < MOIS ; month++)
		{
			for (int day = 0; day < JOUR ; day++)
			{

				if(tGainJourVOL[year][month][day].achatOuVente == 0) // achat
				{
					tabJ[year][month][day].gain = tGainJourVOL[year][month][day].tGain[idx_tpa];
					tabJ[year][month][day].isTp = tGainJourVOL[year][month][day].isTp[idx_tpa];
				}
				else 
				{
					tabJ[year][month][day].gain = tGainJourVOL[year][month][day].tGain[idx_tpv];
					tabJ[year][month][day].isTp = tGainJourVOL[year][month][day].isTp[idx_tpv];
				}
			}
		}
	}
}

void remplirStructJourSL_VOL(int anneeDebut, int anneeFin, sGainJourHeureSL tabJ[ANNEE][MOIS][JOUR])
{
	int idx_sl = gainTotalSL_VOL.maxSL / 10;

	for (int year = anneeDebut; year <= anneeFin ; year++)
	{
		for (int month = 0; month < MOIS ; month++)
		{
			for (int day = 0; day < JOUR ; day++)
			{
				tabJ[year][month][day].gain = tGainJourSL_VOL[year][month][day].tGain[idx_sl];
				tabJ[year][month][day].isTp = tGainJourSL_VOL[year][month][day].isTp[idx_sl];
				tabJ[year][month][day].isSl = tGainJourSL_VOL[year][month][day].isSl[idx_sl];
			}
		}
	}
}

/*
 * appelée dans calculGainJourRapide
 * Permet de remplir la structure tGainJourHeure  et gainTotalHeure
 * (heure fermeture et le gain total à cette heure de fermeture)
 * tGainJourHeure est le gain par jour de la meilleure heure testée de anneDebut à anneeFin
 * !!!! ne fonctionne pour l'instant qu'avec une seul devise à traiter !!!!!
 */
void comparerMeilleureHeure(int anneeDebut, int anneeFin, sGainJourHeure tabJ[ANNEE][MOIS][JOUR])
{
	if(gainTotal.gainTotalMax < gainTotalHeure.gainHeureMax) return;

	gainTotalHeure.heureF = minuteFin / 60;
	gainTotalHeure.gainHeureMax = gainTotal.gainTotalMax;

	remplirStructJour(anneeDebut, anneeFin, tabJ);
}

/*
 * appelée dans calculGainMinuteStopLoss
 * Permet de remplir la structure tGainJourHeureSL et gainTotalHeure
 * (heure fermeture et le gain total à cette heure de fermeture)
 * tGainJourHeure est le gain par jour de la meilleure heure testée de anneDebut à anneeFin
 * !!!! ne fonctionne pour l'instant qu'avec une seul devise à traiter !!!!!
 */
void comparerMeilleureHeureSL(int anneeDebut, int anneeFin, sGainJourHeureSL tabJ[ANNEE][MOIS][JOUR])
{
	if(gainTotalSL.meilleurGainSL < gainTotalHeure.gainHeureMaxSL) return;

	gainTotalHeure.heureFavecSL = minuteFin / 60;
	gainTotalHeure.gainHeureMaxSL = gainTotalSL.meilleurGainSL;

	remplirStructJourSL(anneeDebut, anneeFin, tabJ);
}

// void comparerMeilleureHeureSL_VOL(int anneeDebut, int anneeFin, sGainJourHeureSL tabJ[ANNEE][MOIS][JOUR])
// {
// 	if(gainTotalSL_VOL.meilleurGainSL < gainTotalHeureSL_VOL.gainHeureMaxSL) return;

// 	gainTotalHeure.heureFavecSL = minuteFin / 60;
// 	gainTotalHeure.gainHeureMaxSL = gainTotalSL.meilleurGainSL;

// 	remplirStructJourSL_VOL(anneeDebut, anneeFin, tabJ);
// }

/*
 * appelée dans calculGainMinuteStopLoss
 * Permet de remplir la structure tGainJourHeureSL et gainTotalHeure
 * (heure fermeture et le gain total à cette heure de fermeture)
 * tGainJourHeure est le gain par jour de la meilleure heure testée de anneDebut à anneeFin
 * !!!! ne fonctionne pour l'instant qu'avec une seul devise à traiter !!!!!
 */
void comparerMeilleureHeureVOL(int anneeDebut, int anneeFin, sGainJourHeure tabJ[ANNEE][MOIS][JOUR])
{
	if(gainTotalVOL.gainTotalMax < gainTotalHeureVOL.gainHeureMax) return;

	gainTotalHeureVOL.heureF = minuteFin / 60;
	gainTotalHeureVOL.gainHeureMax = gainTotalVOL.gainTotalMax;

	remplirStructJour_VOL(anneeDebut, anneeFin, tabJ);
}

/*
 * appelée dans calculGainJourRapide 
 * Permet de remplir la structure sGainJourHeure tGainJourH18
 * tGainJourH18 est le tabl. de gain, isTp par jour à 18h de anneDebut à anneeFin
 * !!!! ne fonctionne pour l'instant qu'avec une seul devise à traiter !!!!!
 */
void comparer18Heure(int anneeDebut, int anneeFin, sGainJourHeure tabJ[ANNEE][MOIS][JOUR])
{
	if(minuteFin / 60 != 18 - DCLGE_HOR) return;

	remplirStructJour(anneeDebut, anneeFin, tabJ);
}

/*
 * appelée dans calculGainMinuteStopLoss
 * Permet de remplir la structure sGainJourHeureSL tGainJourH18SL
 * tGainJourH18SL est le tabl. de gain, isTp, isSl par jour à 18h de anneDebut à anneeFin
 * !!!! ne fonctionne pour l'instant qu'avec une seul devise à traiter !!!!!
 */
void comparer18HeureSL(int anneeDebut, int anneeFin, sGainJourHeureSL tabJ[ANNEE][MOIS][JOUR])
{
	if(minuteFin / 60 != 18 - DCLGE_HOR) return;

	remplirStructJourSL(anneeDebut, anneeFin, tabJ);
}

/*
 * appelée dans optimiserTpAvecVolatilite
 * Permet de remplir la structure sGainJourHeure tGainJourH18_VOL
 * tGainJourH18_VOL est un tabl. de gain, isTp par jour à 18h de anneDebut à anneeFin
 * !!!! ne fonctionne pour l'instant qu'avec une seul devise à traiter !!!!!
 */
void comparer18HeureVOL(int anneeDebut, int anneeFin, sGainJourHeure tabJ[ANNEE][MOIS][JOUR])
{
	if(minuteFin / 60 != 18 - DCLGE_HOR) return;

	remplirStructJour_VOL(anneeDebut, anneeFin, tabJ);
}

void comparer18HeureSL_VOL(int anneeDebut, int anneeFin, sGainJourHeureSL tabJ[ANNEE][MOIS][JOUR])
{
	if(minuteFin / 60 != 18 - DCLGE_HOR) return;

	remplirStructJourSL_VOL(anneeDebut, anneeFin, tabJ);
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
					fprintf(fd, "%d/%d/%d;%.2f;%.2f;%c;%c;%c\n",day+1,month+1,year,tGainJourSL[year][month][day].tGain[80],tGainJourSL[year][month][day].tGain[loss], tGainJourSL[year][month][day].achatOuVente, tGainJourSL[year][month][day].isSl[loss], tGainJourSL[year][month][day].isTp[loss]);
				}
			}
		}
		fclose(fd);
	}
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

/*
 * Calcul du pivot et du high low close de la journée
 * Et calcul de la volatilité et de la moyenne (high+low)/2 de la journée
 */
void calculDuPivotEtVolatilite(int minuteDebut, int minuteFin)
{
	double pivotJournee = 0;
	double variance = 0;
	double dataMinuteMoy;
	dataMinute *donneeMinute;

	// Pour le calcul en deux temps
	double sumMoyenne;
	double sample;
	//

	for (int year = 0; year < ANNEE; year++)
	{
		for (int month = 0; month < MOIS; month++)
		{
			for (int day = 0; day < JOUR; day++)
			{
				int deltaMin = calculDeltaMin(year, month+1, day+1);
				double open = 0, close = 0, high = 0, low = 900;
				double moy = 0, prevMoy = 0;
				int n = 1;

				// Pour le calcul en deux temps
				double sumMoyenne = 0;
				int sample = 0;
				//

				for (int minute = minuteDebut + deltaMin; minute <= minuteFin + deltaMin; minute++)
				{
					donneeMinute = &tableauDonnees[year][month][day][minute];

					if(donneeMinute->high > 0 )
					{
						dataMinuteMoy = (donneeMinute->high + donneeMinute->low) / 2.0;
						moy = (prevMoy * (n - 1) + dataMinuteMoy) / (double)n;
						variance = ((n - 1) * pow(variance, 2) + (dataMinuteMoy - prevMoy) * (dataMinuteMoy - moy)) / (double)n;
						//printf("moy=%f, prevMoy=%f, n=%d, dataMinuteMoy=%f, !!variance:%f!!\n",moy, prevMoy, n, dataMinuteMoy, variance );
						prevMoy = moy;
						n++;

						// Variance en deux passe 
						sample++;
						sumMoyenne += dataMinuteMoy;
						//

						tPivot[year][month][day] = pivotJournee;

						if(donneeMinute->high  != 0 ) high  = MAX(high, donneeMinute->high);
						if(donneeMinute->low   != 0 ) low   = MIN(low, donneeMinute->low);
						if(donneeMinute->close != 0 ) close = donneeMinute->close;
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
					donneeMinute->open  = tableauDonnees[year][month][day][m].open;
					donneeMinute->high  = high;
					donneeMinute->low   = low;
					donneeMinute->close = close;
					//donneeMinute->pivot = pivotJournee;
					pivotJournee = ( high + low + close ) / 3 ;

					tVol[year][month][day] = sqrt(variance);
					
					// Variance en deux passe 
					tMoy[year][month][day] = sumMoyenne / (double)sample;
					//
				}

			}
		}
	}
}

void calculEcartTypeDeuxPasses(int minuteDebut, int minuteFin)
{
	dataMinute *donneeMinute ;
	double sumVariation, moyJournee, dataMin, ecart;
	int echantillon;
	for (int year = 0; year < ANNEE; year++)
	{
		for (int month = 0; month < MOIS; month++)
		{
			for (int day = 0; day < JOUR; day++)
			{
				sumVariation = 0;
				moyJournee = tMoy[year][month][day];
				echantillon = 0;
				int deltaMin = calculDeltaMin(year, month+1, day+1);
				for (int minute = minuteDebut + deltaMin; minute <= minuteFin + deltaMin; minute++)
				{
					donneeMinute = &tableauDonnees[year][month][day][minute];

					if(donneeMinute->high > 0 )
					{
						dataMin = (donneeMinute->high + donneeMinute->low) / 2.0;
						ecart = moyJournee - dataMin;
						sumVariation += pow(ecart, 2);
						echantillon++;

						// if(day == 4 && month == 9 && year == 7)
						// {
						// 	printf("%d;%.15E;%.15E\n", minute, pow(ecart, 2), sumVariation);
						// }
					}
				}
				ecart = sumVariation / (double)(echantillon - 1);
				tVolDeuxPasses[year][month][day] = sqrt(ecart);
				// if(day == 4 && month == 9 && year == 7)
				// {
				// 	printf("moyJournee: %.15E, tVolDeuxPasses[year][month][day]=%.15E\n", moyJournee, tVolDeuxPasses[year][month][day]);
				// }
			}
			
		}
	}
}

/*
 * est appelé dans optimiserTpAvecVolatilite
 * remplit la structure gainTotalVOL
 */
void recupererGainMax(struct sGainTotalVOL *s, infoTraitement *infT)
{
	s->gainAchatMax = s->gainVenteMax = -1000000;

	for (int tpa= infT->tpaMin; tpa <= infT->tpaMax; tpa++)
	{
		if(s->tGainA[tpa] > s->gainAchatMax)
		{
			s->gainAchatMax = s->tGainA[tpa];
			s->tpaMax = tpa;
		}
		//printf("%d;%.2f;%d\n", tpa, gainTotalVOL.tGainA[tpa], gainTotalVOL.nbTpa[tpa]);
	}
	for (int tpv= infT->tpvMin; tpv <= infT->tpvMax; tpv++)
	{
		if(s->tGainV[tpv] > s->gainVenteMax)
		{
			s->gainVenteMax = s->tGainV[tpv];
			s->tpvMax = tpv;
		}
		//printf("%d;%.2f;%d\n", tpv, gainTotalVOL.tGainV[tpv], gainTotalVOL.nbTpv[tpv]);
	}

	s->gainTotalMax = s->gainVenteMax + s->gainAchatMax;

	printf("gainMax total  (tpa=%d, tpv=%d, %dh): %.2f\n", s->tpaMax, s->tpvMax, minuteFin/60 + DCLGE_HOR, s->gainTotalMax);
}

/*
 * Ecrit dans le fichier %DEVISE%_%STRATEGIE%_SL.csv
 * Entre le gain tot, son sl, le gainA tot, son slA, le gainV tot, son slV
 * en fonction du stoploss.
 * Est appelée dans calculGainStopLoss()
 */
void ecrireSLdansFichier(params p, int slMin, int slMax, infoTraitement *infT)
{
	char filename[50] ;

	sprintf(filename, "resultats/SL_%s_%s_9h_%dh.csv", p.strategie, infT->devise, (minuteFin+360)/60);
	FILE *fichierResSL = fopen(filename,"w");
	if(fichierResSL != NULL){
		fprintf(fichierResSL, "strategie: %s\n", p.strategie);
		fprintf(fichierResSL, "dP= %d, tpa= %d, tpv=%d, slMin=%d, slMax=%d\n\n",p.dp, p.tpa, p.tpv, slMin, slMax);
		fprintf(fichierResSL, "gain total sans stoploss; gain total avec stoploss; meilleur sl;gain achat;slAchat; gain vente;slVente; perte max absolue\n");
		fprintf(fichierResSL, "%.2f;%.2f;%d(%.2f€);%.2f;%d(%.2f€);%.2f;%d(%.2f€);%.2f\n\n",gainTotal.gainTotalMax, gainTotalSL.meilleurGainSL, gainTotalSL.maxSL, gainTotalSL.maxSL*MONTANT/10000.0, gainTotalSL.meilleurGainSLAchat, gainTotalSL.maxSLAchat, gainTotalSL.maxSLAchat*MONTANT/10000.0, gainTotalSL.meilleurGainSLVente, gainTotalSL.maxSLVente, gainTotalSL.maxSLVente*MONTANT/10000.0, gainTotalSL.perteMax);
		fprintf(fichierResSL, "stoploss;gain;gain Achat; gain Vente\n");
		for(int i=0;i<STOP_LOSS;i++)
		{
			fprintf(fichierResSL, "%d;%.2f;%.2f;%.2f\n", i*ABS_STOP_LOSS_MAX, gainTotalSL.tGain[i], gainTotalSL.tGainAchat[i], gainTotalSL.tGainVente[i]);
		}
		fclose(fichierResSL);
	}
	else {
		printf("pas possible douvrir le fichier\n");
	}
}

void verifierSiMeilleurSL(int l, int min, int max)
{
	if( gainTotalSL.meilleurGainSL < gainTotalSL.tGain[l]){
		gainTotalSL.meilleurGainSL = gainTotalSL.tGain[l];
		gainTotalSL.maxSL     = ABS_STOP_LOSS_MAX*l;
		printf("on trouve un meilleur sl : %d qui donne un gain: %.2f\n",gainTotalSL.maxSL, gainTotalSL.meilleurGainSL);
	}
	
	if( gainTotalSL.meilleurGainSLAchat < gainTotalSL.tGainAchat[l]){
		gainTotalSL.meilleurGainSLAchat = gainTotalSL.tGainAchat[l];
		gainTotalSL.maxSLAchat     = ABS_STOP_LOSS_MAX*l;
		//printf("on trouve un meilleur slAchat : %d qui donne un gain: %.2f\n",gainTotalSL.maxSLAchat, gainTotalSL.meilleurGainSLAchat);
	}
	if( gainTotalSL.meilleurGainSLVente < gainTotalSL.tGainVente[l]){
		gainTotalSL.meilleurGainSLVente = gainTotalSL.tGainVente[l];
		gainTotalSL.maxSLVente     = ABS_STOP_LOSS_MAX*l;
		//printf("on trouve un meilleur slVente : %d qui donne un gain: %.2f\n",gainTotalSL.maxSLVente, gainTotalSL.meilleurGainSLVente);
	}
}

void verifierSiMeilleurSL_VOL(int l, int min, int max)
{
	if( gainTotalSL_VOL.meilleurGainSL < gainTotalSL_VOL.tGain[l]){
		gainTotalSL_VOL.meilleurGainSL = gainTotalSL_VOL.tGain[l];
		gainTotalSL_VOL.maxSL     = ABS_STOP_LOSS_MAX*l;
		printf("on trouve un meilleur sl_vol : %d qui donne un gain: %.2f\n",gainTotalSL_VOL.maxSL, gainTotalSL_VOL.meilleurGainSL);
	}
	
	if( gainTotalSL_VOL.meilleurGainSLAchat < gainTotalSL_VOL.tGainAchat[l]){
		gainTotalSL_VOL.meilleurGainSLAchat = gainTotalSL_VOL.tGainAchat[l];
		gainTotalSL_VOL.maxSLAchat     = ABS_STOP_LOSS_MAX*l;
		//printf("on trouve un meilleur slAchat : %d qui donne un gain: %.2f\n",gainTotalSL.maxSLAchat, gainTotalSL.meilleurGainSLAchat);
	}
	if( gainTotalSL_VOL.meilleurGainSLVente < gainTotalSL_VOL.tGainVente[l]){
		gainTotalSL_VOL.meilleurGainSLVente = gainTotalSL_VOL.tGainVente[l];
		gainTotalSL_VOL.maxSLVente     = ABS_STOP_LOSS_MAX*l;
		//printf("on trouve un meilleur slVente : %d qui donne un gain: %.2f\n",gainTotalSL.maxSLVente, gainTotalSL.meilleurGainSLVente);
	}
}

/*
 * est appelé dans optimiserTpAvecVolatilite
 * ecrit les resultats jour du meilleur tpa et tpv
 */
void ecrireDebugResVolDansFichier(int anneeDebut, int anneeFin, infoTraitement *infT)
{
	#ifdef DEBUG_VOL

		char fn[100]; 
		sprintf(fn,"%sgainJourVOL%s.csv", paramAppli.repertoireRes, infT->devise );
		FILE* fd = fopen(fn, "w");

		if(fd == NULL )
		{
			printf("Problemeeee... pas possible d'ouvrir le fichier %s", fn);
			return;
		}
		fprintf(fd, "date;gain;isTp\n");
		for (int year = anneeDebut; year <= anneeFin ; year++)
		{
			for (int month = 0; month < MOIS ; month++)
			{
				for (int day = 0; day < JOUR ; day++)
				{
					fprintf(fd, "%d/%d/%d;", day+1, month+1, year+2000);
					if(tGainJourVOL[year][month][day].achatOuVente == 0)
					{
						fprintf(fd, "%.5f;%d\n", tGainJourVOL[year][month][day].tGain[gainTotalVOL.tpaMax], tGainJourVOL[year][month][day].isTp[gainTotalVOL.tpaMax]);
					}
					else
					{
						fprintf(fd, "%.5f;%d\n", tGainJourVOL[year][month][day].tGain[gainTotalVOL.tpvMax], tGainJourVOL[year][month][day].isTp[gainTotalVOL.tpvMax] );
					}
				}
			}
		}

		fclose(fd);
	#endif
}

/*
 * Prend en param un tableau des 5 dernieres volatilitées
 * 
 */
double recupererMoy5dernieresVol(double tab[], double vol, int *idx_a_modifier)
{
	//printf("-----------------------------------\n");
	int ind = *idx_a_modifier % NB_VOL_REF;
	double moyenneTab, sumTab = 0, moyenneAjusteTab, maxCoeff = 1.25;
	tab[ind] = vol;

	// On evite ainsi de traiter les 5 premieres valeurs erronees
	
	if((*idx_a_modifier)++ < 5) return 100.0;

	for (int i = 0; i < NB_VOL_REF; i++)
	{
		sumTab += tab[i];
		//printf("tab[%d]= %f ;", i, tab[i]);
	}
	//printf("volatiliteJour=%f\n", vol );
	moyenneTab = sumTab / NB_VOL_REF;
	sumTab = 0;

	for (int i = 0; i < NB_VOL_REF; i++)
	{
		double volMaxTolere = maxCoeff * moyenneTab;
		tab[i] = (tab[i] > volMaxTolere) ? volMaxTolere : tab[i];
		//printf("tab[%d]= %f ;", i, tab[i]);
		sumTab += tab[i];
	}
	moyenneAjusteTab = sumTab / NB_VOL_REF;

	 ; 
	return moyenneAjusteTab;
}

/* 
 * CALCUL GAIN JOUR AVEC VOLATILITE 
 */
void optimiserTpAvecVolatilite(int anneeDebut, int anneeFin, int dP, infoTraitement * infT)
{
	printf("********************* CALCUL GAIN JOUR AVEC VOLATILITE *********************\n\n");

	double tLast5Vol[NB_VOL_REF] = {0};
	int idx_last = 0;
	dataMinute *donneeMinute ;
	double pivotAjuste, takeProfit, open, gain, gainMaxA, gainMaxV, moyAjusteVol, volJournee = 0;
	sParamTp paramTp;
	
	for (int year = anneeDebut; year <= anneeFin ; year++)
	{
		for (int month = 0; month < MOIS ; month++)
		{
			for (int day = 0; day < JOUR ; day++)
			{
				donneeMinute = &tableauDonnees[year][month][day][MINUTE -1];
				if (donneeMinute->high > 0)
				{
					moyAjusteVol = recupererMoy5dernieresVol(tLast5Vol, volJournee, &idx_last);

					//printf("%d/%d/%d;%.5f\n",day+1, month+1, year+2000, moyAjusteVol);
					pivotAjuste = tPivot[year][month][day] * (1.0 + dP / 10000.0);
					open = donneeMinute->open;

					if(open >= pivotAjuste)
					{
						tGainJourVOL[year][month][day].achatOuVente = 1;
						for (int tpv= infT->tpvMin; tpv<=infT->tpvMax; tpv++)
						{
							//takeProfit = open * (1.0 - tpv / 10000.0);
							takeProfit = open - tpv * moyAjusteVol / 100.0;


							if (donneeMinute->low <= takeProfit) 
							{
								gain = (open - takeProfit) * MONTANT / open;
								gainTotalVOL.nbTpv[tpv]++;
								tGainJourVOL[year][month][day].isTp[tpv] = 1;
							}
							else
							{
								gain = (open - donneeMinute->close) * MONTANT / open;
							}
							gainTotalVOL.tGainV[tpv] += gain;
							tGainJourVOL[year][month][day].tGain[tpv] = gain;
						}
					}
					else
					{
						tGainJourVOL[year][month][day].achatOuVente = 0;
						for (int tpa= infT->tpaMin; tpa <= infT->tpaMax; tpa++)
						{
							//takeProfit = open * (1.0 + tpa / 10000.0);
							takeProfit = open + tpa * moyAjusteVol / 100.0;

							if (donneeMinute->high >= takeProfit)
							{
								gain = (takeProfit - open ) * MONTANT / open;
								gainTotalVOL.nbTpa[tpa]++;
								tGainJourVOL[year][month][day].isTp[tpa] = 1;
							}
							else
							{
								gain = (donneeMinute->close - open ) * MONTANT / open;
							}
							gainTotalVOL.tGainA[tpa] += gain;
							tGainJourVOL[year][month][day].tGain[tpa] = gain;
						}
					}
					volJournee = tVolDeuxPasses[year][month][day];

					// FILE *fe = fopen("resulVOL.csv", "a");
					// fprintf(fe, "%d/%d/%d;%f\n",day+1, month+1,year+2000,tGainJour_VOL[year][month][day].tGain[loss]);
					// fclose(fe);
				}
			}
		}
	} // for year
	recupererGainMax(&gainTotalVOL, infT);
	ecrireDebugResVolDansFichier(anneeDebut, anneeFin, infT);

	if(infT->ecrireGainJ[0] == 'O')
	{
		comparerMeilleureHeureVOL(anneeDebut, anneeFin, tGainJourHeure_VOL);
		comparer18HeureVOL(anneeDebut, anneeFin, tGainJourH18_VOL);
	}
}

void calculGainMinuteSLetVOL(int anneeDebut, int anneeFin, infoTraitement * infT)
{
	printf("********************* CALCUL GAIN MINUTE SL ET VOL *********************\n\n");

	clock_t tDeb, tFin;
	tDeb = clock();

	gainTotalSL.meilleurGainSL = -10000000;

	double tLast5Vol[NB_VOL_REF] = {0};
	int idx_last = 0;

	dataMinute* donneeMinute, minuteCourante ;
	int dP, tpa, tpv, tpAtteint, slAtteint, stopMin = infT->slMin, stopMax = infT->slMax; 
	double pivotAjuste, takeProfit, takeLoss, gainVente, gainAchat, gainOrdre, moyAjusteVol, volJournee;
	char transaction, fn[100]; 

	params param ; 
	param = getStrategy();

	dP 	= param.dp ;
	tpa = gainTotalVOL.tpaMax;
	tpv = gainTotalVOL.tpvMax;

	
	
	#ifdef DEBUG_SL
		sprintf(fn,"%sgainJourSL_VOL%s.csv", paramAppli.repertoireRes, infT->devise );
		printf("nom du fichier:%s\n", fn );
		FILE* fd = fopen(fn, "w");

		if(fd == NULL )
		{
			printf("Problemeeee... pas possible d'ouvrir le fichier %s", fn);
			return;
		}
		fprintf(fd, "date;gain;minOuverture;minFermeture;isSl;isTp\n");
	#endif

	for (int loss = stopMin; loss <= stopMax; loss++)
	{
		for (int year = anneeDebut; year <= anneeFin ; year++)
		{
			for (int month = 0; month < MOIS ; month++)
			{
				for (int day = 0; day < JOUR ; day++)
				{
					donneeMinute = &tableauDonnees[year][month][day][MINUTE-1]  ;
					if(donneeMinute-> high <= 0) continue;

					tpAtteint = 0;
					slAtteint = 0;
					int deltaMin = calculDeltaMin(year, month+1, day+1);
					
					double open = donneeMinute->open;
					pivotAjuste = tPivot[year][month][day] * (1.0 + dP / 10000.0) ;
					moyAjusteVol = recupererMoy5dernieresVol(tLast5Vol, volJournee, &idx_last);

					//printf("%d/%d/%d : deltaMin = %d\n",day, month, year, deltaMin);
					for (int min = MINUTE_DEBUT + deltaMin; min <= minuteFin + deltaMin ; min++)
					{
						
						minuteCourante = tableauDonnees[year][month][day][min]  ;

						if(minuteCourante.high <= 0) continue;

						if(open >= pivotAjuste) // Cas d'une vente
						{
							tGainJourSL_VOL[year][month][day].achatOuVente = 0 ; 
							takeProfit = open - tpv * moyAjusteVol / 100.0;
							takeLoss = open * (1.0 + ABS_STOP_LOSS_MAX*loss / (10000.0) ); // On ajuste le pas
							
							if (minuteCourante.low < takeProfit )
							{
								gainOrdre = (open - takeProfit) * MONTANT / open;
								tGainJourSL_VOL[year][month][day].tGain[loss]  = gainOrdre;
								tGainJourSL_VOL[year][month][day].tGainV[loss] = gainOrdre;
								tGainJourSL_VOL[year][month][day].isTp[loss] = 1;
								tpAtteint = 1;
								#ifdef DEBUG_SL
									fprintf(fd, "%d/%d/%d;%.2f;0;%d;0;1\n", day+1, month+1, year, gainOrdre, min);
								#endif
								break;
							
							}
							else if(minuteCourante.high > takeLoss )
							{
								//printf("VENTE: on dépasse le stopLoss (%d), high= %f à la minute : %d, %d/%d/%d\n",loss,minuteCourante.low, min, day+1,month+1,year);
								// On gere le cas d'un stopLoss;
								gainOrdre = (open - takeLoss) * MONTANT / open;
								tGainJourSL_VOL[year][month][day].tGain[loss] = gainOrdre;
								tGainJourSL_VOL[year][month][day].tGainV[loss] = gainOrdre;
								tGainJourSL_VOL[year][month][day].isSl[loss] = 1;
								slAtteint = 1;
								#ifdef DEBUG_SL
									fprintf(fd, "%d/%d/%d;%.2f;0;%d;1;0\n", day+1, month+1, year, gainOrdre, min);
								#endif
								break;
							}

						}
						// A gerer de la meme maniere que que le cas d'une vente 
						else // cas d'un achat
						{
							gainAchat = tpa/100.0;
							takeProfit = open + tpa * moyAjusteVol / 100.0;
							tGainJourSL_VOL[year][month][day].achatOuVente = 1 ; 
							
							takeLoss = open * (1.0 - ABS_STOP_LOSS_MAX*loss / (10000.0) )  ; // On ajuste le pas
						
							if (minuteCourante.high > takeProfit )
							{
								gainOrdre = (takeProfit - open) * MONTANT / open;
								tGainJourSL_VOL[year][month][day].tGain[loss] = gainOrdre;
								tGainJourSL_VOL[year][month][day].tGainA[loss] = gainOrdre;
								tGainJourSL_VOL[year][month][day].isTp[loss] = 1;
								tpAtteint = 1;
								#ifdef DEBUG_SL
									fprintf(fd, "%d/%d/%d;%.2f;0;%d;0;1\n", day+1, month+1, year, gainOrdre, min);
								#endif
								break;
							}
							else if(minuteCourante.low < takeLoss )
							{
								// On gere le cas d'un stopLoss;
								gainOrdre = (takeLoss - open) * MONTANT / open;
								tGainJourSL_VOL[year][month][day].tGain[loss] = gainOrdre;
								tGainJourSL_VOL[year][month][day].tGainA[loss] = gainOrdre; 
								tGainJourSL_VOL[year][month][day].isSl[loss] = 1;
								slAtteint = 1;
								//printf("ACHAT: on dépasse le stoploss (%d), avec une perte de %.2f:, %d/%d/%d\n",loss, tGainJourSL[year][month][day].tGain[loss], day,month,year);
								#ifdef DEBUG_SL
									fprintf(fd, "%d/%d/%d;%.2f;0;%d;1;0\n", day+1, month+1, year, gainOrdre, min);
								#endif
								break;
							}
						}
					} // for min

					// On arrive en fin de journée , pas de sl ni tp
					if(slAtteint == 0 && tpAtteint == 0)
					{
						double diffOpenClose;
						tGainJourSL_VOL[year][month][day].isSl[loss] = 0;
						tGainJourSL_VOL[year][month][day].isTp[loss] = 0;

						if(tGainJourSL_VOL[year][month][day].achatOuVente == 1)
						{
							diffOpenClose = (donneeMinute->close - open);
							gainOrdre = diffOpenClose !=0 ? diffOpenClose * MONTANT / open : 0;
							tGainJourSL_VOL[year][month][day].tGainA[loss] = gainOrdre;
						}
						else
						{
							diffOpenClose = (open - donneeMinute->close);
							gainOrdre =  diffOpenClose != 0 ? diffOpenClose * MONTANT / open : 0;
							tGainJourSL_VOL[year][month][day].tGainV[loss] = gainOrdre;
						}

						tGainJourSL_VOL[year][month][day].tGain[loss] = gainOrdre;
				
						#ifdef DEBUG_SL
							fprintf(fd, "%d/%d/%d;%.2f;0;0;0;0\n", day+1, month+1, year, gainOrdre);
						#endif
					}
					gainTotalSL_VOL.tGainVente[loss] += tGainJourSL_VOL[year][month][day].tGainV[loss];
					gainTotalSL_VOL.tGainAchat[loss] += tGainJourSL_VOL[year][month][day].tGainA[loss];
					
					gainTotalSL_VOL.perteMax = MIN(gainTotalSL_VOL.perteMax, tGainJourSL_VOL[year][month][day].tGain[loss] );
					tGainMoisSL[year][month].tGain[loss] += tGainJourSL_VOL[year][month][day].tGain[loss] ;

					volJournee = tVolDeuxPasses[year][month][day];

					// FILE *fe = fopen("resulVOL.csv", "a");
					// fprintf(fe, "%d/%d/%d;%f\n",day+1, month+1,year+2000,tGainJourSL_VOL[year][month][day].tGain[loss]);
					// fclose(fe);
				} // for day
				tGainAnneeSL[year].tGain[loss] += tGainMoisSL[year][month].tGain[loss] ;
			} // for month
			gainTotalSL_VOL.tGain[loss] += tGainAnneeSL[year].tGain[loss];
		} // for year
		verifierSiMeilleurSL_VOL(loss, stopMin, stopMax);
		
	} // for loss
	tFin = clock() ;
	
	printf("temps du sl = %f\n", (double)(tFin - tDeb)/CLOCKS_PER_SEC );
	printf("*********************************************************************\n\n");
	ecrireSLdansFichier(param,stopMin,stopMax, infT);
	ecrireSLAetSLVdansFichier(param, infT);

		if(infT->ecrireGainJ[0] == 'O')
	{
		comparerMeilleureHeureSL(anneeDebut, anneeFin, tGainJourHeureSL_VOL);
		comparer18HeureSL_VOL(anneeDebut, anneeFin, tGainJourH18SL_VOL);
	}
}

void ecrireEcartTypeMethodes()
{
	char fn[500];
	double vol1, vol2;
	sprintf(fn, "%svariance.csv", paramAppli.repertoireRes);
	FILE* fd = fopen(fn, "w");
	if(fd == NULL) return;

	fprintf(fd, "date;variance 1 passe; variance 2 passes\n");

	for (int year = ANNEE_DEB; year <= ANNEE_FIN; year++)
	{
		for (int month = 0; month < MOIS; month++)
		{
			for (int day = 0; day < JOUR; day++)
			{
				vol1 = tVol[year][month][day];
				vol2 = tVolDeuxPasses[year][month][day];
				fprintf(fd, "%d/%d/%d;%f;%f;%f\n",day+1,month+1,year+2000, vol1, vol2, tMoy[year][month][day]);
			}
		}
	}

	fclose(fd);
}

void calculGains(infoTraitement * infT)
{
	//printf("avant calcul gain.\n");
	//if (infT->optimiserGain[0] == 'O' )
	//{
		printf("dans calcul gain : %c\n",infT->optimiserGain[0] );
		clock_t tDeb, tFin;

		tDeb = clock();

		calculDuPivotEtVolatilite(MINUTE_DEBUT, minuteFin);

		// methode ecart type deux passes
		calculEcartTypeDeuxPasses(MINUTE_DEBUT, minuteFin);
		ecrireEcartTypeMethodes();
		//

		//randomizePivot();

		tFin = clock();
		paramAppli.tempsCalculPivot = (double)(tFin - tDeb)/CLOCKS_PER_SEC;

		tDeb = tFin;
		gainTotal.gainTotalMax = -1000000; // Pq l'initialiser 2 fois ?? voir initialisation()
		for (int dP = infT->dPivotMin ; dP <= infT->dPivotMax  ; dP++)
		{
			deltaPivot = dP ; // modif kh avant deltaPivot = dP/100.0 ;
			calculGainJourRapide(ANNEE_DEB, ANNEE_FIN, deltaPivot, 0.0, 0.0, infT);
		}

		if(infT->ecrireGainJ[0] == 'O')
		{
			comparerMeilleureHeure(ANNEE_DEB, ANNEE_FIN, tGainJourHeure);
			comparer18Heure(ANNEE_DEB, ANNEE_FIN, tGainJourH18);
		}

		tFin = clock();
		paramAppli.tempsCalculGainJour = (double)(tFin - tDeb)/CLOCKS_PER_SEC;
	//} // if (infT->optimiserGain[0] == 'O' )
}



double calculGainJourRapide(int anneeDebut, int anneeFin, int deltaP, double gainAchat, double gainVente, infoTraitement * infT)
{
	dataMinute *donneeMinute ;
	double pivotAjuste, takeProfit, resYear, gainMaxA, gainMaxV;
	char transaction;
	int nbAchat=0, nbVente=0, tpaOpti, tpvOpti, dP = deltaP;

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
					pivotAjuste = tPivot[year][month][day] * (1.0 + dP / 10000.0) ;
					open = donneeMinute->open;
					//printf("dPivot: %f   gainTotV: %f   gainTotA: %f\n",dPivot, paramGainOpti.gainTotV, paramGainOpti.gainTotA );
					if(donneeMinute->open >= pivotAjuste)
					{
						nbVente++;
						tGainJour[year][month][day].achatOuVente[dP + IDX_PIVOT_MAX] = 0;
						for (int tpv= infT->tpvMin; tpv<=infT->tpvMax; tpv++)
						{
							gainVente = tpv/100.0;
							takeProfit = donneeMinute->open * (1.0 - gainVente / 100.0)  ;
							//donneeMinute->open = (takeProfit <= donneeMinute->high) ? (donneeMinute->open - takeProfit) * MONTANT / donneeMinute->open :  (donneeMinute->open - donneeMinute->close) * MONTANT / donneeMinute->open;
							if (donneeMinute->low <= takeProfit)
							{
								tGainJour[year][month][day].tGain[dP + IDX_PIVOT_MAX][tpv] = (donneeMinute->open - takeProfit) * MONTANT / donneeMinute->open ;
								tGainJour[year][month][day].tpOuCloture[dP + IDX_PIVOT_MAX][tpv] = 1;
							}

							else tGainJour[year][month][day].tGain[dP + IDX_PIVOT_MAX][tpv] = (donneeMinute->open - donneeMinute->close) * MONTANT / donneeMinute->open;
							//printf("avant - gainTpvDp[tpv][%d + 200]: %f\n",dP, gainTpvDp[tpv][dP + 200]);
							
							double gainJournee = tGainJour[year][month][day].tGain[dP + IDX_PIVOT_MAX][tpv];
							tGainMois[year][month].tGainV[dP + IDX_PIVOT_MAX][tpv] += gainJournee;
							tGainAnnee[year].tGainV[dP + IDX_PIVOT_MAX][tpv] += gainJournee;
							gainTotal.tGainV[dP + IDX_PIVOT_MAX][tpv] += gainJournee;
							//printf("après - gainTpvDp[tpv][%d + 200]: %f\n",dP, gainTpvDp[tpv][dP + 200]);
							//resYear += tGainJour[year][month][day].tGain[dP + 200][tpv];

							//printf("%d/%d/%d;%f\n", day+1, month+1,year+2000, gainJournee);

						}

					}
					else
					{
						nbAchat++;
						tGainJour[year][month][day].achatOuVente[dP + IDX_PIVOT_MAX] = 1;
						for (int tpa= infT->tpaMin; tpa <= infT->tpaMax; tpa++)
						{
							gainAchat = tpa/100.0;
							takeProfit = donneeMinute->open * (1.0 + gainAchat / 100.0)  ;
							//donneeMinute->open = (takeProfit >= donneeMinute->low) ? (takeProfit - donneeMinute->open ) * MONTANT / donneeMinute->open :  (donneeMinute->close - donneeMinute->open ) * MONTANT / donneeMinute->open;
							if (donneeMinute->high >= takeProfit)
							{
								tGainJour[year][month][day].tGain[dP + IDX_PIVOT_MAX][tpa] = (takeProfit - donneeMinute->open ) * MONTANT / donneeMinute->open;
								tGainJour[year][month][day].tpOuCloture[dP + IDX_PIVOT_MAX][tpa] = 1;
							}
							else tGainJour[year][month][day].tGain[dP + IDX_PIVOT_MAX][tpa] = (donneeMinute->close - donneeMinute->open ) * MONTANT / donneeMinute->open;
							
							
							// On remplit les structures du gain mois, année et totale
							double gainJournee = tGainJour[year][month][day].tGain[dP + IDX_PIVOT_MAX][tpa] ;
							tGainMois[year][month].tGainA[dP + IDX_PIVOT_MAX][tpa] += gainJournee;
							tGainAnnee[year].tGainA[dP + IDX_PIVOT_MAX][tpa] += gainJournee;
							gainTotal.tGainA[dP + IDX_PIVOT_MAX][tpa] += gainJournee;
							//printf("gainTpaDp[%d][%d + 200]: %f  gainTpvDp[%d][%d + 200]: %f\n",tpa, dP, gainTpaDp[tpa][dP + 200],97, dP, gainTpvDp[97][dP + 200]);
							//resYear += tGainJour[year][month][day].tGain[dP + 200][tpa];
							//printf("%d/%d/%d;%f\n", day+1, month+1,year+2000, gainJournee);
							

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

void calculGainMinuteStopLoss(int anneeDebut, int anneeFin, infoTraitement * infT /*,double (*calculTp)(paramTp pTp) */)
{
	printf("********************* CALCUL GAIN MINUTE STOP LOSS *********************\n\n");
	if(infT->optimiserSL[0] == 'N')
	{
		printf("\nOn optimise pas avec le SL\n");
		return; 
	}

	clock_t tDeb, tFin;
	tDeb = clock();

	gainTotalSL.meilleurGainSL = -10000000;

	dataMinute* donneeMinute, minuteCourante ;
	int dP, tpa, tpv, tpAtteint, slAtteint, stopMin = infT->slMin, stopMax = infT->slMax; 
	double pivotAjuste, takeProfit, takeLoss, gainVente, gainAchat, gainOrdre;
	char transaction, fn[100]; 

	params param ; 
	param = getStrategy();

	dP 	= param.dp ;
	tpa = param.tpa;
	tpv = param.tpv;

	sprintf(fn,"%sgainJourSL%s.csv", paramAppli.repertoireRes, infT->devise );
	
	#ifdef DEBUG_SL
		printf("nom du fichier:%s\n", fn );
		FILE* fd = fopen(fn, "w");

		if(fd == NULL )
		{
			printf("Problemeeee... pas possible d'ouvrir le fichier %s", fn);
			return;
		}
		fprintf(fd, "date;gain;minOuverture;minFermeture;isSl;isTp\n");
	#endif

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
					pivotAjuste = tPivot[year][month][day] * (1.0 + dP / 10000.0) ;

					//printf("%d/%d/%d : deltaMin = %d\n",day, month, year, deltaMin);
					for (int min = MINUTE_DEBUT + deltaMin; min <= minuteFin + deltaMin ; min++)
					{
						
						minuteCourante = tableauDonnees[year][month][day][min]  ;

						if(minuteCourante.high <= 0) continue;

						if(open >= pivotAjuste) // Cas d'une vente
						{
							tGainJourSL[year][month][day].achatOuVente = 0 ; 
							gainVente = tpv/100.0;
							takeProfit = open * (1.0 - gainVente / 100.0) ;
							takeLoss = open * (1.0 + ABS_STOP_LOSS_MAX*loss / (10000.0) ); // On ajuste le pas
							
							if (minuteCourante.low < takeProfit )
							{
								gainOrdre = (open - takeProfit) * MONTANT / open;
								tGainJourSL[year][month][day].tGain[loss]  = gainOrdre;
								tGainJourSL[year][month][day].tGainV[loss] = gainOrdre;
								tGainJourSL[year][month][day].isTp[loss] = 1;
								tpAtteint = 1;
								#ifdef DEBUG_SL
									fprintf(fd, "%d/%d/%d;%.2f;0;%d;0;1\n", day+1, month+1, year, gainOrdre, min);
								#endif
								break;
							
							}
							else if(minuteCourante.high > takeLoss )
							{
								//printf("VENTE: on dépasse le stopLoss (%d), high= %f à la minute : %d, %d/%d/%d\n",loss,minuteCourante.low, min, day+1,month+1,year);
								// On gere le cas d'un stopLoss;
								gainOrdre = (open - takeLoss) * MONTANT / open;
								tGainJourSL[year][month][day].tGain[loss] = gainOrdre;
								tGainJourSL[year][month][day].tGainV[loss] = gainOrdre;
								tGainJourSL[year][month][day].isSl[loss] = 1;
								tGainMoisSL[year][month].nbStopLoss[loss]++ ;
								tGainAnneeSL[year].nbStopLoss[loss]++;
								slAtteint = 1;
								#ifdef DEBUG_SL
									fprintf(fd, "%d/%d/%d;%.2f;0;%d;1;0\n", day+1, month+1, year, gainOrdre, min);
								#endif
								break;
							}

						}
						// A gerer de la meme maniere que que le cas d'une vente 
						else // cas d'un achat
						{
							gainAchat = tpa/100.0;
							takeProfit = open * (1.0 + gainAchat / 100.0)  ;
							tGainJourSL[year][month][day].achatOuVente = 1 ; 
							
							takeLoss = open * (1.0 - ABS_STOP_LOSS_MAX*loss / (10000.0) )  ; // On ajuste le pas
						
							if (minuteCourante.high > takeProfit )
							{
								gainOrdre = (takeProfit - open) * MONTANT / open;
								tGainJourSL[year][month][day].tGain[loss] = gainOrdre;
								tGainJourSL[year][month][day].tGainA[loss] = gainOrdre;
								tGainJourSL[year][month][day].isTp[loss] = 1;
								tpAtteint = 1;
								#ifdef DEBUG_SL
									fprintf(fd, "%d/%d/%d;%.2f;0;%d;0;1\n", day+1, month+1, year, gainOrdre, min);
								#endif
								break;
							}
							else if(minuteCourante.low < takeLoss )
							{
								// On gere le cas d'un stopLoss;
								gainOrdre = (takeLoss - open) * MONTANT / open;
								tGainJourSL[year][month][day].tGain[loss] = gainOrdre;
								tGainJourSL[year][month][day].tGainA[loss] = gainOrdre; 
								tGainJourSL[year][month][day].isSl[loss] = 1;
								tGainMoisSL[year][month].nbStopLoss[loss]++ ;
								tGainAnneeSL[year].nbStopLoss[loss]++;
								slAtteint = 1;
								//printf("ACHAT: on dépasse le stoploss (%d), avec une perte de %.2f:, %d/%d/%d\n",loss, tGainJourSL[year][month][day].tGain[loss], day,month,year);
								#ifdef DEBUG_SL
									fprintf(fd, "%d/%d/%d;%.2f;0;%d;1;0\n", day+1, month+1, year, gainOrdre, min);
								#endif
								break;
							}
						}
					} // for min

					// On arrive en fin de journée , pas de sl ni tp
					if(slAtteint == 0 && tpAtteint == 0)
					{
						double diffOpenClose;
						tGainJourSL[year][month][day].isSl[loss] = 0;
						tGainJourSL[year][month][day].isTp[loss] = 0;

						if(tGainJourSL[year][month][day].achatOuVente == 1)
						{
							diffOpenClose = (donneeMinute->close - open);
							gainOrdre = diffOpenClose !=0 ? diffOpenClose * MONTANT / open : 0;
							tGainJourSL[year][month][day].tGainA[loss] = gainOrdre;
						}
						else
						{
							diffOpenClose = (open - donneeMinute->close);
							gainOrdre =  diffOpenClose != 0 ? diffOpenClose * MONTANT / open : 0;
							tGainJourSL[year][month][day].tGainV[loss] = gainOrdre;
						}

						tGainJourSL[year][month][day].tGain[loss] = gainOrdre;
				
						#ifdef DEBUG_SL
							fprintf(fd, "%d/%d/%d;%.2f;0;0;0;0\n", day+1, month+1, year, gainOrdre);
						#endif
					}
					gainTotalSL.tGainVente[loss] += tGainJourSL[year][month][day].tGainV[loss];
					gainTotalSL.tGainAchat[loss] += tGainJourSL[year][month][day].tGainA[loss];
					
					gainTotalSL.perteMax = MIN(gainTotalSL.perteMax, tGainJourSL[year][month][day].tGain[loss] );
					tGainMoisSL[year][month].tGain[loss] += tGainJourSL[year][month][day].tGain[loss] ;

				} // for day
				tGainAnneeSL[year].tGain[loss] += tGainMoisSL[year][month].tGain[loss] ;
			} // for month
			gainTotalSL.tGain[loss] += tGainAnneeSL[year].tGain[loss];
		} // for year
		verifierSiMeilleurSL(loss, stopMin, stopMax);
	} // for loss

	if(infT->ecrireGainJ[0] == 'O')
	{
		comparerMeilleureHeureSL(anneeDebut, anneeFin, tGainJourHeureSL);
		comparer18HeureSL(anneeDebut, anneeFin, tGainJourH18SL);
	}
	
	ecrireSLdansFichier(param,stopMin,stopMax, infT);
	ecrireSLAetSLVdansFichier(param, infT);

	tFin = clock() ;
	
	printf("temps du sl = %f\n", (double)(tFin - tDeb)/CLOCKS_PER_SEC );
	printf("*********************************************************************\n\n");
} 

/*
 * Ecrit dans le fichier %DEVISE%_%STRATEGIE%_SL.csv
 * Entre le gain tot, son sl, le gainA tot, son slA, le gainV tot, son slV
 * en fonction du stoploss.
 * Est appelée dans calculGainStopLoss()
 */
void ecrireSLAetSLVdansFichier(params p, infoTraitement *infT)
{
	char fname[200] ;
	int slA, slV, indexSlA, indexSlV;
	double gainJour;
	sGainJourSL bilanJour;

	sprintf(fname, "%sSLA_SLV_%s_%s.csv", paramAppli.repertoireRes, p.strategie, infT->devise);
	FILE *fd = fopen(fname,"w");
	if(fd == NULL)
	{
		printf("impossible d'ouvrir le fichier %s\n", fname);
		return;
	}

	slA = gainTotalSL.maxSLAchat;
	slV = gainTotalSL.maxSLVente;
	indexSlA = slA / 10;
	indexSlV = slV / 10;

	fprintf(fd, "!!!! ATTENTION !!!! : La journée affiche par default une vente\n\n");
	fprintf(fd, "strategie: %s\n", p.strategie);
	fprintf(fd, "dP= %d, tpa= %d, tpv=%d\n\n", p.dp, p.tpa, p.tpv);
	fprintf(fd, "gain total;meilleur slA;meilleur slV\n");
	fprintf(fd, "%.2f;%d;%d\n\n", gainTotalSL.meilleurGainSLAchat+gainTotalSL.meilleurGainSLVente, slA, slV);
	fprintf(fd, "date;type;gain;isSl;isTp\n");
	for (int year = ANNEE_DEB; year <= ANNEE_FIN ; year++)
	{
		for (int month = 0; month < MOIS ; month++)
		{
			for (int day = 0; day < JOUR ; day++)
			{
				bilanJour = tGainJourSL[year][month][day] ;
				if(bilanJour.achatOuVente == 0){ // vente
					fprintf(fd, "%d/%d/%d;V;%.2f;%d;%d\n", day+1, month+1, year+2000, bilanJour.tGainV[indexSlV], bilanJour.isSl[indexSlV], bilanJour.isTp[indexSlV]);
				}
				else
				{
					fprintf(fd, "%d/%d/%d;A;%.2f;%d;%d\n", day+1, month+1, year+2000, bilanJour.tGainA[indexSlA], bilanJour.isSl[indexSlA], bilanJour.isTp[indexSlA]);
				}

			}
		}
	}
	fclose(fd);
}


void calculMeilleureMoyenne(int anneeDebut, int anneeFin, infoTraitement * infT)
{

	printf("******************** CALCUL MEILLEURE MOYENNE ********************\n\n");

	if(infT->optimiserMOY[0] == 'N' )
	{
		printf("\nOn n'optimise pas avec la moyenne...\n\n");
		return;
	} 

	clock_t tDeb, tFin;
	tDeb = clock();
	
	int dP, tpa, tpv, tpAtteint;
	dataMinute* donneeMinute, minuteCourante ;
	int departMoyMin  = infT->departMoyMin;
	int departMoyMax  = infT->departMoyMax;
	int relacheMoyMin = infT->relacherMoyMin;
	int relacheMoyMax = infT->relacherMoyMax;
	int valPip 		  = infT->valeurPip;
	int minOuverture;
	double pivotAjuste, takeProfit, takeMoy, releaseMoyPrice, gainVente, gainAchat, gainJournee ;
	char transaction, moyActive; 
	char fn[100];


	params param ; 
	param = getStrategy();

	dP 	= param.dp ;
	tpa = param.tpa;
	tpv = param.tpv;

	gainTotalMOY.meilleurGainMoy = -1000000;

	sprintf(fn,"%sgainJourMoy%s.csv",paramAppli.repertoireRes,infT->devise );

	#ifdef DEBUG_MOY
		FILE* fd = fopen(fn, "w");

		if(fd == NULL )
		{
			printf("Problemeeee... pas possible d'ouvrir le fichier gainJourMoy.csv");
			return;
		}
		fprintf(fd, "date;gain;minOuverture;minFermeture;isMoy;isTp\n");
	#endif

	for (int departMoy = departMoyMin; departMoy <= departMoyMax; departMoy+=10)
	{
		for (int relacherMoy = relacheMoyMin; relacherMoy <= MIN(departMoy,relacheMoyMax); relacherMoy+=1)
		{
			for (int year = anneeDebut; year <= anneeFin ; year++)
			{
				for (int month = 0; month < MOIS ; month++)
				{
					for (int day = 0; day < JOUR ; day++)
					{
						tpAtteint 			= 0;
						gainJournee 		= 0;
						moyActive   		= 0;

						int deltaMin 		= calculDeltaMin(year, month+1, day+1);
						donneeMinute 		= &tableauDonnees[year][month][day][MINUTE-1]  ;
						double open 		= donneeMinute->open;
						double gainOrdre 	= 0;
						pivotAjuste 		= tPivot[year][month][day] * (1.0 + dP/(double)valPip) ;

						for (int min = MINUTE_DEBUT + deltaMin; min <= minuteFin + deltaMin ; min++)
						{
							minuteCourante = tableauDonnees[year][month][day][min]  ;

							if(minuteCourante.high <= 0) continue;


							if(open >= pivotAjuste) // Cas d'une vente
							{
								transaction = 0; 
								gainVente 	= tpv/100.0;
								takeProfit 	= open * (1.0 - gainVente / 100.0) ;
								takeMoy 	= open + departMoy/(double)valPip ; // en pips
								releaseMoyPrice	= takeMoy - relacherMoy/(double)valPip;
								
								if (minuteCourante.low <= takeProfit )
								{
									gainOrdre = (open - takeProfit) * MONTANT / open - SPREAD_PRICE/2.0;
									gainJournee += gainOrdre;
									tpAtteint = 1;
									#ifdef DEBUG_MOY
										fprintf(fd, "%d/%d/%d;%.2f;0;%d;0;1\n", day+1, month+1, year, gainOrdre, min);
									#endif
									break;
								}
								else if(moyActive == 0 && minuteCourante.high >= takeMoy)
								{
									moyActive = 1;
									minOuverture = min;
								}

								if(moyActive == 1 && minuteCourante.low <= releaseMoyPrice)
								{
									gainOrdre = relacherMoy / (double)valPip * MONTANT / releaseMoyPrice - SPREAD_PRICE;
									gainJournee += gainOrdre;
									moyActive = 0;
									#ifdef DEBUG_MOY
										fprintf(fd, "%d/%d/%d;%.2f;%d;%d;1;0\n", day+1, month+1, year, gainOrdre, minOuverture, min);
									#endif
								}

							}
							else // cas d'un achat
							{
								transaction 	= 1 ;
								gainAchat 		= tpa/100.0;
								takeProfit 		= open * (1.0 + gainAchat / 100.0)  ;
								takeMoy 		= open - departMoy / (double)valPip ; // en pips
								releaseMoyPrice	= takeMoy + relacherMoy / (double)valPip;
								
							
								if (minuteCourante.high > takeProfit )
								{
									gainOrdre = (takeProfit - open) * MONTANT / donneeMinute->open - SPREAD_PRICE/2.0;
									gainJournee += gainOrdre;
									tpAtteint = 1;
									#ifdef DEBUG_MOY	
										fprintf(fd, "%d/%d/%d;%.2f;0;%d;0;1\n", day+1, month+1, year, gainOrdre, min);
									#endif

									break;
								}
								else if(moyActive == 0 && minuteCourante.low <= takeMoy)
								{
									moyActive = 1;
									minOuverture = min;
								}

								if(moyActive == 1 && minuteCourante.high >= releaseMoyPrice)
								{
									gainOrdre = (relacherMoy / (double)valPip) * MONTANT / releaseMoyPrice - SPREAD_PRICE;
									gainJournee += gainOrdre;
									moyActive = 0;
									#ifdef DEBUG_MOY
										fprintf(fd, "%d/%d/%d;%.2f;%d;%d;1;0\n", day+1, month+1, year, gainOrdre, minOuverture, min);
									#endif
								}
							}
						} // for min
						if(tpAtteint == 0) // Calcul du gain en fin de journee
						{

							double diffOpenClose = transaction == 1 ? (donneeMinute->close - open) : (open - donneeMinute->close);
							gainOrdre = (diffOpenClose != 0) ? diffOpenClose * MONTANT / open - SPREAD_PRICE : 0;
							gainJournee += gainOrdre ;
							#ifdef DEBUG_MOY
								fprintf(fd, "%d/%d/%d;%.2f;0;0;0;0\n", day+1, month+1, year, gainOrdre);
							#endif
						}
						if(moyActive == 1) // Cas d'une moyenne qui n'est pas gagnante
						{
							double diffOpenClose = (transaction == 1) ? (donneeMinute->close - takeMoy) : (takeMoy - donneeMinute->close);
							gainOrdre = diffOpenClose * MONTANT / donneeMinute->open - SPREAD_PRICE;
							gainJournee += gainOrdre;
							#ifdef DEBUG_MOY
							fprintf(fd, "%d/%d/%d;%.2f;%d;0;1;0\n", day+1, month+1, year, gainOrdre, minOuverture);
							#endif
							//printf("gainJournee quand moyenne pas gagnante:%.2f\n", gainJournee);
						}



						//printf("gainJournee:%.2f\n", gainJournee);
						gainTotalMOY.tGain[departMoy][relacherMoy] += gainJournee;
						//printf("%d/%d/%d,gainTotalMOY.tGain[%d][%d]= %.2f \n",day+1,month+1,year, departMoy,relacherMoy, gainTotalMOY.tGain[departMoy][relacherMoy] );
					} // for days
				} // for month

			} // for year
			//printf("gainTotalMOY.tGain[%d][%d]= %.2f \n",departMoy,relacherMoy, gainTotalMOY.tGain[departMoy][relacherMoy] );
			if(gainTotalMOY.tGain[departMoy][relacherMoy] > gainTotalMOY.meilleurGainMoy)
			{
				gainTotalMOY.meilleurGainMoy = gainTotalMOY.tGain[departMoy][relacherMoy];
				gainTotalMOY.meilleurDM = departMoy;
				gainTotalMOY.meilleurRM = relacherMoy;
				printf("departMoy=%d,relacherMoy=%d, gain:%.2f\n",departMoy,relacherMoy, gainTotalMOY.meilleurGainMoy );
			}
			
		} // for relacherMoy
	} // for departMoy
	#ifdef DEBUG_MOY
		fclose(fd);
	#endif

	tFin = clock() ;
	printf("temps de la moy = %f\n",(double)(tFin - tDeb)/CLOCKS_PER_SEC );

	printf("meilleur gain MOY: %.2f params (%d, %d)\n",gainTotalMOY.meilleurGainMoy, gainTotalMOY.meilleurDM, gainTotalMOY.meilleurRM);
}

/*
 * Prends en compte les spreads
 * Retirer le break du sl -> une moyenne peut etre en cours 
 * On veut relacher tous les ordres au sl
 * Mis en pause
 */
void calculGainSLetMOY(int anneeDebut, int anneeFin, infoTraitement * infT)
{
	printf("****************** CALCUL MEILLEURE MOYENNE ET SL ******************\n\n");
	if(infT->optimiserMOYavecSL[0] ==  'N')
	{
		printf("\nOn n'optimise pas la moyenne avec le SL...\n\n");
		return;
	} 

	clock_t tDeb, tFin;
	tDeb = clock();
	
	int dP, tpa, tpv, tpAtteint, slAtteint;
	dataMinute* donneeMinute, minuteCourante ;
	int departMoyMin = infT->departMoyMin;
	int departMoyMax = infT->departMoyMax;
	int relacheMoyMin = infT->relacherMoyMin;
	int relacheMoyMax = infT->relacherMoyMax;
	int valPip = infT->valeurPip;
	int stopMin = infT->slMin;
	int stopMax = infT->slMax;
	int minOuverture;
	double pivotAjuste, takeProfit, takeMoy, takeLoss, releaseMoyPrice, gainVente, gainAchat, gainJournee ;
	char transaction, moyActive; 
	char fn[100];


	params param ; 
	param = getStrategy();

	dP 	= param.dp ;
	tpa = param.tpa;
	tpv = param.tpv;

	gainTotalMOYetSL.meilleurGainMOYetSL = -1000000;

	#ifdef DEBUG_MOY_SL
		sprintf(fn,"%sgainJourMOYetSL%s.csv",paramAppli.repertoireRes,infT->devise );
		FILE* fd = fopen(fn, "w");

		if(fd == NULL )
		{
			printf("Problemeeee... pas possible d'ouvrir le fichier %s", fn);
			return;
		}
		fprintf(fd, "date;gain;minOuverture;minFermeture;isMoy;isSl;isTp\n");
	#endif

	for (int loss = stopMin; loss <= stopMax; loss++)
	{
		for (int departMoy = departMoyMin; departMoy <= MIN(loss*ABS_STOP_LOSS_MAX, departMoyMax); departMoy+=10)
		{
			for (int relacherMoy = relacheMoyMin; relacherMoy <= MIN(departMoy, relacheMoyMax); relacherMoy+=1)
			{
				for (int year = anneeDebut; year <= anneeFin ; year++)
				{
					for (int month = 0; month < MOIS ; month++)
					{
						for (int day = 0; day < JOUR ; day++)
						{
							tpAtteint 			= 0;
							slAtteint 			= 0;
							gainJournee 		= 0;
							moyActive   		= 0;

							int deltaMin 		= calculDeltaMin(year, month+1, day+1);
							donneeMinute 		= &tableauDonnees[year][month][day][MINUTE-1]  ;
							double open 		= donneeMinute->open;
							double gainOrdre 	= 0;
							pivotAjuste 		= tPivot[year][month][day] * (1.0 + dP / 10000.0) ;

							for (int min = MINUTE_DEBUT + deltaMin; min <= minuteFin + deltaMin ; min++)
							{
								minuteCourante = tableauDonnees[year][month][day][min]  ;

								if(minuteCourante.high <= 0) continue;


								if(open >= pivotAjuste) // Cas d'une vente
								{
									transaction 	= 0; 
									gainVente 		= tpv / 100.0;
									takeProfit 		= open * (1.0 - gainVente / 100.0);
									takeLoss		= open * (1.0 + ABS_STOP_LOSS_MAX*loss / (10000.0) );
									takeMoy 		= open + departMoy / (double)valPip ; // en pips
									releaseMoyPrice	= takeMoy - relacherMoy / (double)valPip;
									
									if (minuteCourante.low <= takeProfit )
									{
										gainOrdre = (open - takeProfit) * MONTANT / open - SPREAD_PRICE/2.0;
										gainJournee += gainOrdre;
										tpAtteint = 1;
										#ifdef DEBUG_MOY_SL
											fprintf(fd, "%d/%d/%d;%.2f;0;%d;0;0;1\n", day+1, month+1, year, gainOrdre, min);
										#endif
										break;
									}
									else if(minuteCourante.high >= takeLoss)
									{
										gainOrdre = (open - takeLoss) * MONTANT / open - SPREAD_PRICE/2.0;
										gainJournee += gainOrdre;
										// tGainJourSL[year][month][day].isSl[loss] = 1;
										// tGainMoisSL[year][month].nbStopLoss[loss]++ ;
										// tGainAnneeSL[year].nbStopLoss[loss]++;
										slAtteint = 1;
										#ifdef DEBUG_MOY_SL
											fprintf(fd, "%d/%d/%d;%.2f;0;%d;0;1;0\n", day+1, month+1, year, gainOrdre, min);
										#endif
										break;
									}
									else if(moyActive == 0 && minuteCourante.high >= takeMoy)
									{
										moyActive = 1;
										minOuverture = min;
									}

									if(moyActive == 1 && minuteCourante.low <= releaseMoyPrice)
									{
										gainOrdre = relacherMoy / (double)valPip * MONTANT / releaseMoyPrice - SPREAD_PRICE;
										gainJournee += gainOrdre;
										moyActive = 0;
										#ifdef DEBUG_MOY_SL
											fprintf(fd, "%d/%d/%d;%.2f;%d;%d;1;0;0\n", day+1, month+1, year, gainOrdre, minOuverture, min);
										#endif
									}

								}
								else // cas d'un achat
								{
									transaction 	= 1 ;
									gainAchat 		= tpa/100.0;
									takeProfit 		= open * (1.0 + gainAchat / 100.0)  ;
									takeMoy 		= open - departMoy / (double)valPip ; // en pips
									takeLoss		= open * (1.0 - ABS_STOP_LOSS_MAX*loss / (10000.0) );
									releaseMoyPrice	= takeMoy + relacherMoy / (double)valPip;
									
								
									if (minuteCourante.high > takeProfit )
									{
										gainOrdre = (takeProfit - open) * MONTANT / donneeMinute->open - SPREAD_PRICE/2.0;
										gainJournee += gainOrdre;
										tpAtteint = 1;
										#ifdef DEBUG_MOY_SL
											fprintf(fd, "%d/%d/%d;%.2f;0;%d;0;0;1\n", day+1, month+1, year, gainOrdre, min);
										#endif
										break;
									}
									else if(minuteCourante.high <= takeLoss)
									{
										gainOrdre = (takeLoss - open) * MONTANT / open - SPREAD_PRICE/2.0;
										gainJournee += gainOrdre;
										// tGainJourSL[year][month][day].isSl[loss] = 1;
										// tGainMoisSL[year][month].nbStopLoss[loss]++ ;
										// tGainAnneeSL[year].nbStopLoss[loss]++;
										slAtteint = 1;
										#ifdef DEBUG_MOY_SL
											fprintf(fd, "%d/%d/%d;%.2f;0;%d;0;1;0\n", day+1, month+1, year, gainOrdre, min);
										#endif
										break;
									}
									else if(moyActive == 0 && minuteCourante.low <= takeMoy)
									{
										moyActive = 1;
										minOuverture = min;
									}

									if(moyActive == 1 && minuteCourante.high >= releaseMoyPrice)
									{
										gainOrdre = (relacherMoy / (double)valPip) * MONTANT / releaseMoyPrice - SPREAD_PRICE;
										gainJournee += gainOrdre;
										moyActive = 0;
										#ifdef DEBUG_MOY_SL
											fprintf(fd, "%d/%d/%d;%.2f;%d;%d;1;0;0\n", day+1, month+1, year, gainOrdre, minOuverture, min);
										#endif
									}
								}
							} // for min
							if(tpAtteint == 0 && slAtteint == 0) // Calcul du gain en fin de journee
							{

								double diffOpenClose = transaction == 1 ? (donneeMinute->close - open) : (open - donneeMinute->close);
								gainOrdre = (diffOpenClose != 0) ? diffOpenClose * MONTANT / open - SPREAD_PRICE : 0;
								gainJournee += gainOrdre ;
								#ifdef DEBUG_MOY_SL
									fprintf(fd, "%d/%d/%d;%.2f;0;0;0;0;0\n", day+1, month+1, year, gainOrdre);
								#endif
							}
							if(moyActive == 1) // Cas d'une moyenne qui n'est pas gagnante
							{
								double diffOpenClose = (transaction == 1) ? (donneeMinute->close - takeMoy) : (takeMoy - donneeMinute->close);
								gainOrdre = diffOpenClose * MONTANT / donneeMinute->open - SPREAD_PRICE;
								gainJournee += gainOrdre;
								#ifdef DEBUG_MOY_SL
								fprintf(fd, "%d/%d/%d;%.2f;%d;0;1;0;0\n", day+1, month+1, year, gainOrdre, minOuverture);
								#endif
								//printf("gainJournee quand moyenne pas gagnante:%.2f\n", gainJournee);
							}
							//printf("gainJournee:%.2f\n", gainJournee);
							gainTotalMOYetSL.tGain[departMoy][relacherMoy][loss] += gainJournee;
							//printf("%d/%d/%d,gainTotalMOY.tGain[%d][%d]= %.2f \n",day+1,month+1,year, departMoy,relacherMoy, gainTotalMOY.tGain[departMoy][relacherMoy] );
						} // for days
					} // for month

				} // for year
				//printf("gainTotalMOY.tGain[%d][%d]= %.2f \n",departMoy,relacherMoy, gainTotalMOY.tGain[departMoy][relacherMoy] );
				if(gainTotalMOYetSL.tGain[departMoy][relacherMoy][loss] > gainTotalMOYetSL.meilleurGainMOYetSL)
				{
					gainTotalMOYetSL.meilleurGainMOYetSL = gainTotalMOYetSL.tGain[departMoy][relacherMoy][loss];
					gainTotalMOYetSL.meilleurDM = departMoy;
					gainTotalMOYetSL.meilleurRM = relacherMoy;
					gainTotalMOYetSL.maxSL = loss*ABS_STOP_LOSS_MAX;
					printf("departMoy=%d, relacherMoy=%d, loss=%d, gain:%.2f\n", departMoy, relacherMoy, loss, gainTotalMOYetSL.meilleurGainMOYetSL );
				}
				
			} // for relacherMoy
		} // for departMoy
		printf(" loss = %d terminé\n", loss);
	} // for loss
	#ifdef DEBUG_MOY_SL
		fclose(fd);
	#endif

	tFin = clock() ;
	printf("temps de la MOYetSL = %f\n",(double)(tFin - tDeb) / CLOCKS_PER_SEC );

	printf("meilleur gain MOY et SL: %.2f paramMOY (%d, %d) SL(%d)\n", gainTotalMOYetSL.meilleurGainMOYetSL, gainTotalMOYetSL.meilleurDM, gainTotalMOYetSL.meilleurRM, gainTotalMOYetSL.maxSL);
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

void printMemoireUtilise()
{
	printf("**********************   ESPACE MEMOIRE *********************************\n");
		printf("données minutes: %lu\npivot: %lu\nvolDeuxPasses: %lu\nmoy val: %lu\n", sizeof(tableauDonnees) , sizeof(tPivot) , sizeof(tVolDeuxPasses), sizeof(tMoy));
		printf("gain mois: %lu\ngain année: %lu\ngain total: %lu\n", sizeof(tGainMois) , sizeof(tGainAnnee) , sizeof(gainTotal));
		printf("gain jour: %lu\tgain jourSL: %lu\tgainJourSL_VOL:%lu\tgainJourVOL: %lu\n", sizeof(tGainJour), sizeof(tGainJourSL), sizeof(tGainJourSL_VOL), sizeof(tGainJourVOL));
		printf("gain jour H18-H18SL-H18VOL-H18SLVOL: %lu\n", 2 * sizeof(tGainJourH18) + 2 * sizeof(tGainJourH18SL));
		printf("gain jour (MH->meilleure heure) MH-MHSL-MHVOL-MHSLVOL: %lu\n", 2 * sizeof(tGainJourHeure) + 2 * sizeof(tGainJourHeureSL));
		printf("gain ptf :%lu\t, gain ptfSL: %lu\n", sizeof(tGainJourPtf), sizeof(tGainJourPtfSL));
		printf("gainTotal: %lu\tgainTotalSL: %lu\tgainTotalSL_VOL: %lu\tgainTotalMOY: %lu\tgainTotalMOYetSL: %lu\tgainTotalVOL: %lu\tgainTotalHeure: %lu\tgainTotalHeureVOL: %lu\n", sizeof(gainTotal), sizeof(gainTotalSL), sizeof(gainTotalSL_VOL), sizeof(gainTotalMOY), sizeof(gainTotalMOYetSL), sizeof(gainTotalVOL), sizeof(gainTotalHeure), sizeof(gainTotalHeureVOL));
		printf("Espace mémoire total alloué: %.2f Mo\n",( sizeof(tableauDonnees) + sizeof(tPivot) + sizeof(tVolDeuxPasses) + sizeof(tMoy) + sizeof(tGainMois) + sizeof(tGainAnnee) + sizeof(gainTotal) + sizeof(tGainJour) + sizeof(tGainJourSL) + sizeof(tGainJourSL_VOL) + sizeof(tGainJourVOL) + 4 * sizeof(tGainJourH18) + 4 * sizeof(tGainJourH18SL) + sizeof(tGainJourPtf) + sizeof(tGainJourPtfSL) + sizeof(gainTotal) + sizeof(gainTotalSL) + sizeof(gainTotalSL_VOL) + sizeof(gainTotalMOY) + sizeof(gainTotalMOYetSL) + sizeof(gainTotalVOL) + sizeof(gainTotalHeure) + sizeof(gainTotalHeureVOL) ) / 1000000.0);

		printf("**************************************************************************\n\n");
}

void printTempsExecution()
{
	printf("**********************   TEMPS EXECUTION *********************************\n");
	printf("temps de lecture des fichiers: %2.3f s\n\n", paramAppli.tempsLectureFichiers);
	//printf("temps d'écriture du fichier BD: %2.3f s\n", paramAppli.tempsEcritureBD);
	printf("Temps calcul pivot: %f   temps calcul gain jour:%f\n", paramAppli.tempsCalculPivot, paramAppli.tempsCalculGainJour );
	printf("Temps calcul opti min mois neg: %f   \n", paramAppli.tempsOpti );
	printf("**************************************************************************\n\n");
}

void affichageResultat(int anneeDebut, int anneeFin, infoTraitement *infT)
{
	if (infT->afficher[0] == 'O')
	{
		//printf("affichage résultat: %s\n",paramAppli.nomFichierRes);
		FILE *fichierResOpti = fopen(paramAppli.nomFichierRes,"a");
		if(fichierResOpti == NULL)
		{
			printf("Le fichier %s n'a pas pu etre ouvert \n", paramAppli.nomFichierRes);
			return; 
		} 

		printMemoireUtilise();

		printTempsExecution();

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
			printf("SL: %.2f€\n",gainTotalSL.maxSL * MONTANT / 10000.0);
			printf("--------------------------\n");
			printf("**************************************************************************\n\n");
		} // if (paramAppli.optimiserGain == 'O')
		fclose(fichierResOpti);
	} // if (paramAppli.afficher == 'O')
}

/*
 * est appelée dans le main, !!! ne fonctionne qu'en traitant les devises séparemment
 * structure du fichier csv (%NOM_REPERTOIRE%/gainJour%DEVISE%.csv) généré :
 * "date;gain (18h);isTp;gain sl;isTp;isSl;gain heure (%dh);isTp;gain heure SL (%dh);isTp;isSl"
 */
void ecritureGainsJour(int anneeDebut, int anneeFin, infoTraitement *infT)
{
	if(infT->ecrireGainJ[0] != 'O') return;

	char fname[400];
	sGainJourHeure resJ, resJH;
	sGainJourHeureSL resJSL, resJHSL;
	int idx_dP = gainTotal.deltaPivotMax + IDX_PIVOT_MAX;
	int idx_sl = gainTotalSL.maxSL/10;

	sprintf(fname, "%s/gainJour%s.csv", paramAppli.repertoireRes, infT->devise);

	FILE *fd = fopen(fname, "w");
	if(fd == NULL)
	{
		printf("Le fichier %s n'a pas pu etre ouvert \n", fname);
		return; 
	} 

	fprintf(fd, "date;gain (18h);isTp;gain sl;isTp;isSl;gain heure (%dh);isTp;gain heure SL (%dh);isTp;isSl\n", gainTotalHeure.heureF + DCLGE_HOR, gainTotalHeure.heureFavecSL + DCLGE_HOR);

	for (int year = anneeDebut; year <= anneeFin ; year++)
	{
		for (int month = 0; month < MOIS ; month++)
		{
			for (int day = 0; day < JOUR ; day++)
			{
				fprintf(fd, "%d/%d/%d;", day+1, month+1, year+2000);
				//printf("%f \n", tGainJourSL[year][month][day].tGain[gainTotalSL.maxSL/10]);
				resJ = tGainJourH18[year][month][day];
				resJSL = tGainJourH18SL[year][month][day];
				resJH = tGainJourHeure[year][month][day];
				resJHSL = tGainJourHeureSL[year][month][day];

				fprintf(fd, "%f;%d;", resJ.gain, resJ.isTp);
				fprintf(fd, "%f;%d;%d;", resJSL.gain, resJSL.isTp, resJSL.isSl);

				fprintf(fd, "%f;%d;", resJH.gain, resJH.isTp);
				fprintf(fd, "%f;%d;%d\n", resJHSL.gain, resJHSL.isTp, resJHSL.isSl);

			}
		}
	}

	fclose(fd);
}

/*
 * est appelée dans le main, !!! ne fonctionne qu'en traitant les devises séparemment
 * structure du fichier csv (%NOM_REPERTOIRE%/gainJour%DEVISE%.csv) généré :
 * "date;gain (18h);isTp;gain sl;isTp;isSl;gain heure (%dh);isTp;gain heure SL (%dh);isTp;isSl"
 */
void ecritureGainsJourVOL(int anneeDebut, int anneeFin, infoTraitement *infT)
{
	if(infT->ecrireGainJ[0] != 'O') return;

	char fname[400];
	sGainJourHeure resJ, resJH;
	sGainJourHeureSL resJSL, resJHSL;
	int idx_dP = gainTotal.deltaPivotMax + IDX_PIVOT_MAX; // parce qu'on garde le mm
	int idx_sl = gainTotalSL_VOL.maxSL/10;

	sprintf(fname, "%s/gainJourVOL_%s.csv", paramAppli.repertoireRes, infT->devise);

	FILE *fd = fopen(fname, "w");
	if(fd == NULL)
	{
		printf("Le fichier %s n'a pas pu etre ouvert \n", fname);
		return; 
	} 

	fprintf(fd, "tpa = %d, tpv=%d\n\n", gainTotalVOL.tpaMax, gainTotalVOL.tpvMax);
	fprintf(fd, "dP = %d, sl=%d\n\n\n", gainTotal.deltaPivotMax, gainTotalSL_VOL.maxSL);


	fprintf(fd, "date;gain (18h);isTp;gain sl;isTp;isSl;gain heure (%dh);isTp;gain heure SL (%dh);isTp;isSl\n", gainTotalHeure.heureF + DCLGE_HOR, gainTotalHeure.heureFavecSL + DCLGE_HOR);

	for (int year = anneeDebut; year <= anneeFin ; year++)
	{
		for (int month = 0; month < MOIS ; month++)
		{
			for (int day = 0; day < JOUR ; day++)
			{
				fprintf(fd, "%d/%d/%d;", day+1, month+1, year+2000);
				//printf("%f \n", tGainJourSL[year][month][day].tGain[gainTotalSL.maxSL/10]);
				resJ = tGainJourH18_VOL[year][month][day];
				resJSL = tGainJourH18SL_VOL[year][month][day];
				resJH = tGainJourHeure_VOL[year][month][day];
				resJHSL = tGainJourHeureSL_VOL[year][month][day];

				fprintf(fd, "%f;%d;", resJ.gain, resJ.isTp);
				fprintf(fd, "%f;%d;%d;", resJSL.gain, resJSL.isTp, resJSL.isSl);

				fprintf(fd, "%f;%d;", resJH.gain, resJH.isTp);
				fprintf(fd, "%f;%d;%d\n", resJHSL.gain, resJHSL.isTp, resJHSL.isSl);

			}
		}
	}

	fclose(fd);
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
	// printf("********************* CALCUL RISQUE AVEC SL *********************\n\n");
	// if(infT->optimiserSL[0] == 'N')
	// {
	// 	printf("\nOn optimise pas avec le SL\n");
	// 	return; 
	// }

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
	int idx_dP = gainTotal.deltaPivotMax + IDX_PIVOT_MAX;
	for (int year = anneeDebut; year <= anneeFin ; year++)
	{
		for (int month = 0; month < MOIS ; month++)
		{
			for (int day = 0; day < JOUR ; day++)
			{
				tp = tGainJour[year][month][day].achatOuVente[idx_dP] == 0 ? gainTotal.tpvGainMax : gainTotal.tpaGainMax;
				cumulCourant += tGainJour[year][month][day].tGain[idx_dP][tp];
				if (cumulCourant > 0) cumulCourant = 0;
				cumulMaxPerte = MIN(cumulCourant, cumulMaxPerte);
				tGainJourPtf[year][month][day] += tGainJour[year][month][day].tGain[idx_dP][tp];
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
	printf("\n********************* CALCUL CUMUL GAIN AVEC SL *********************\n\n");
	if(infT->optimiserSL[0] == 'N')
	{
		printf("\nOn optimise pas avec le SL\n");
		return; 
	}

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
				if(fd != NULL) fprintf(fd, "%d/%d/%d;%.2f;%.2f;%c;%c;%c\n",day+1,month+1,year,tGainJourSL[year][month][day].tGain[80],tGainJourSL[year][month][day].tGain[loss], tGainJourSL[year][month][day].achatOuVente, tGainJourSL[year][month][day].isSl[loss], tGainJourSL[year][month][day].isTp[loss]);

				cumulCourant += tGainJourSL[year][month][day].tGain[loss];
				if (cumulCourant > 0) cumulCourant = 0;
				cumulMaxPerte = MIN(cumulCourant, cumulMaxPerte);
				tGainJourPtfSL[year][month][day] += tGainJourSL[year][month][day].tGain[loss];
			}
		}
	}
	fclose(fd);

	FILE *fichierResOpti = fopen(paramAppli.nomFichierRes,"a");

	printf("Perte cumulée max(%s) pour un SL = %.2f€ : %f€\n", infT->devise,loss*ABS_STOP_LOSS_MAX*MONTANT / 10000.0, cumulMaxPerte);
	if(fichierResOpti != NULL )
	{
		fprintf(fichierResOpti, "\n\nPerte cumulée max (%s) (SL=%.2f€): %.2f€\n", infT->devise, loss*ABS_STOP_LOSS_MAX*MONTANT / 10000.0, cumulMaxPerte);
		fclose(fichierResOpti);
	}
	else printf("impossible d'ouvrir %s\n",paramAppli.nomFichierRes );
}



