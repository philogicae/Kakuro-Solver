#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "structures.h"

Csp init_csp(){ // Initialisation de la structure csp
  Csp csp;
  csp.nb_variables = 0;
  csp.nb_contraintes = 0;
  csp.variables = (Variable*) malloc (sizeof(Variable) * MAXVAR);
  csp.contraintes = (Contrainte*) malloc (sizeof(Contrainte) * MAXCONTR);
  return csp;
}

void add_variable(Csp * csp, int indice, int nb_valeurs, int * tab_valeurs){ // Ajout d'une variable au csp
  Variable *var = &csp->variables[csp->nb_variables];
  csp->nb_variables++;

  var->indice = indice;
  var->valeur = 0;
  var->nb_valeurs = nb_valeurs;
  var->domaine = (int*) malloc (sizeof(int) * nb_valeurs);
  memcpy(var->domaine, tab_valeurs, sizeof(int) * nb_valeurs); // Copie des valeurs
  var->nb_contraintes = 0;
  var->contrainte = (Liste) malloc (sizeof(struct Maillon)); // Déclaration de la liste chainée pour les contraintes
  var->contrainte->courante = NULL;
  var->contrainte->suivante = NULL;
}

void add_maillon(Variable * variable, Contrainte * contrainte){ // Ajout d'une contrainte à la liste chainée d'une variable
  Liste maillon = variable->contrainte;
  while(maillon->courante != NULL)
    maillon = maillon->suivante;
  maillon->courante = contrainte;
  maillon->suivante = (Liste) malloc (sizeof(struct Maillon));
  maillon->suivante->courante = NULL;
  maillon->suivante->suivante = NULL;
  variable->nb_contraintes++;
}

void add_contrainte(Csp * csp, enum TypeContrainte type, int somme, int arite, int * portee){ // Ajout d'une contrainte au csp
  Contrainte *contr = &csp->contraintes[csp->nb_contraintes];
  contr->indice = csp->nb_contraintes;
  csp->nb_contraintes++;

  contr->type = type;
  contr->somme = somme;
  contr->arite = arite;
  contr->portee = (Variable**) malloc (sizeof(Variable*) * arite);
  int i, j;
  for(i=0; i<arite; i++) // Remplissage du tableau portee, avec des pointeurs sur les variables affectées
    for(j=0; j<csp->nb_variables; j++)
      if(csp->variables[j].indice == portee[i])
        contr->portee[i] = &csp->variables[j]; // Pointage de la variable affectée

  for(i=0; i<contr->arite; i++) // Ajout de la contrainte dans les variables affectées
    add_maillon(contr->portee[i], &csp->contraintes[csp->nb_contraintes-1]);
}

void print_csp(Csp csp){ // Affichage du csp
  int i, j, k;
  Liste maillonTemp;

  printf("\n-----CSP-----\n\nVARIABLES :\n");
  for(i=0; i<csp.nb_variables; i++){
    printf(" X%d=%d  domaine=[ ", csp.variables[i].indice, csp.variables[i].valeur);
    k = 0;
    for(j=0; j<csp.variables[i].nb_valeurs; j++)
      if(csp.variables[i].domaine[j] == 0) k++;
      else printf("%d ", csp.variables[i].domaine[j]);
    printf("]  dom/deg = %d/%d  contraintes={ ", csp.variables[i].nb_valeurs - k, csp.variables[i].nb_contraintes);
    maillonTemp = csp.variables[i].contrainte;
    while(maillonTemp->courante != NULL){
      printf("%d ", maillonTemp->courante->indice);
      maillonTemp = maillonTemp->suivante;
    } printf("}\n");
  }

  printf("\nCONTRAINTES :\n");
  for(i=0; i<csp.nb_contraintes; i++){
    if(csp.contraintes[i].type == DIFFERENCE)
      printf(" %d) DIFFERENCE  arite=%d", csp.contraintes[i].indice, csp.contraintes[i].arite);
    else if(csp.contraintes[i].type == SOMME)
      printf(" %d) SOMME = %d  arite=%d", csp.contraintes[i].indice, csp.contraintes[i].somme, csp.contraintes[i].arite);
    else if(csp.contraintes[i].type == ALLDIFFERENT)
      printf(" %d) ALLDIFFERENT  arite=%d", csp.contraintes[i].indice, csp.contraintes[i].arite); // Non utilisé pour le kakuro
    printf("  portee=[ ");
    for(j=0; j<csp.contraintes[i].arite; j++){
      printf("%d ", csp.contraintes[i].portee[j]->indice);
    } printf("]\n");
  } printf("\n");
}
