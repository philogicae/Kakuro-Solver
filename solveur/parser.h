#ifndef PARSER_H
#define PARSER_H
#include "structures.h"



void parse (char * nom_fichier, Csp * csp);

/* fonctions à compléter pour remplir vos structures de données */

void Nouvelle_Variable (Csp * csp, int num);      /* fonction permettant la création d'une nouvelle variable ayant pour numéro num */
void Contrainte_Difference (Csp * csp, int var1, int var2);  /* fonction permettant la création d'une nouvelle contrainte binaire de différence entre les variables var1 et var2*/
void Contrainte_Somme (Csp * csp, int portee[], int arite, int val);  /* fonction permettant la création d'une nouvelle contrainte n-aire de somme portant sur les variables contenant
                                                              dans le tableau portee de taille arite et dont la valeur est val */


#endif
