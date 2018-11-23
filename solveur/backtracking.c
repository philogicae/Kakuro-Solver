#include <stdio.h>
#include <stdlib.h>
#include "structures.h"
#include "heuristique.h"
#include "backtracking.h"

int nb_noeud; // Variable globabe utile pour comparer les heuristiques
int nb_test_contrainte; // Variable globabe utile pour comparer les heuristiques
extern int print_solution;

static void reset_variables(Csp * csp){ // Remet les valeurs de chaque variable à 0
  for(int i=0; i<csp->nb_variables; i++)
    csp->variables[i].valeur = 0;
}

static void print_variables(Csp * csp){ // Affiche les valeurs de chaque variable
  for(int i=0; i<csp->nb_variables; i++)
    printf(" X%d = %d\n", csp->variables[i].indice, csp->variables[i].valeur);
}

/* Change la valeur de la variable en prenant la prochaine disponible, renvoie 1 si succès, ou 0 en cas d'échec */
static int next_value(Variable * variableCourante){
  int i = 0;
  if(variableCourante->valeur != 0){ // Si ce n'est pas la première affectation, on se place au bon emplacement du tableau
    while(variableCourante->valeur != variableCourante->domaine[i])
      i++;
    i++;
  }
  while(i < variableCourante->nb_valeurs && variableCourante->domaine[i] == 0) // On ignore les valeurs exclues (0)
    i++;
  if(i < variableCourante->nb_valeurs){ // On vérifie qu'on est toujours dans le domaine
    variableCourante->valeur = variableCourante->domaine[i];
    return 1; // retour avec succès, on a trouvé une valeur suivante
  }
  return 0; // retour avec échec, il n'y a plus de valeur suivante disponible
}

static int nombre_triangulaire(int x){ return (x * (x + 1)) / 2; } // Calcul du total des valeurs min des autres variables d'une somme

static int test_contrainte(Contrainte * contrainte){ // Teste la contrainte, renvoie 1 si satisfaite, sinon 0
  nb_test_contrainte++;
  if(contrainte->type == DIFFERENCE && contrainte->portee[0]->valeur == contrainte->portee[1]->valeur) // test contrainte de DIFFERENCE
    return 0;
  else if(contrainte->type == SOMME){ // test contrainte de SOMME
    int somme = 0, valnul = 0;
    for(int i=0; i<contrainte->arite; i++){ // Erreur si valeur courante + valeurs min des autres variables  >= somme
      if(contrainte->portee[i]->valeur > contrainte->somme - nombre_triangulaire(contrainte->arite-1)) return 0;
      else if(contrainte->portee[i]->valeur == 0) valnul = 1; // Prise en compte s'il reste des variables non-définies
      somme += contrainte->portee[i]->valeur;
    }
    if(!valnul && somme != contrainte->somme) return 0; // Si une variable est non-définie, on ignore le résultat du test
  }
  else if(contrainte->type == ALLDIFFERENT) // Non utilisé pour le kakuro
    for(int i=0; i<contrainte->arite; i++)
      for(int j=i+1; j<contrainte->arite; j++)
        if(contrainte->portee[i]->valeur != 0 && contrainte->portee[i]->valeur == contrainte->portee[j]->valeur) return 0;
  return 1;
}

static int check_contraintes(Variable * variableCourante){ // Vérifie que chaque contrainte portant sur la variable est satisfaite
  nb_noeud++;
  Liste maillon = variableCourante->contrainte;
  while(maillon->courante != NULL) // Parcours de la liste chainée gérant les contraintes
    if(test_contrainte(maillon->courante)) maillon = maillon->suivante;
    else return 0; // Retour avec échec si une contrainte n'est pas satisfaite
  return 1; // Retour avec succès, les contraintes sont toutes satisfaites
}

static void check_solution(Csp * csp){ // Vérifie si la solution trouvée satisfait bien toutes les contraintes
  printf("VERIFICATION DE LA SOLUTION : ");
  for(int i=0; i<csp->nb_variables; i++)
    if(csp->variables[i].valeur == 0){ printf("ECHEC (Cause : certaines variables non-définies)\n"); return; }
  for(int i=0; i<csp->nb_contraintes; i++)
    if(!test_contrainte(&csp->contraintes[i])){ printf("ECHEC (Cause : contraintes non-satisfaites)\n"); return; }
  printf("OK !\n\n");
}

void backtracking(Csp source, enum TypeHeuristique type){ // Algorithme de résolution, demandant un csp et une heuristique
  nb_noeud = 0;
  nb_test_contrainte = 0;
  Csp *csp = &source;                           // Pointeur sur le csp en argument
  reset_variables(csp);                         // Réinitialisation des valeurs des variables, par précaution
  Ordre *indice = choix_heuristique(csp, type); // Initialisation de l'ordre de traitement des variables, selon l'heuristique
  printf("Algorithme : Backtracking\nHeuristique : %s\n", print_heuristique(type)); fflush(stdout);
  Variable * variableCourante = &csp->variables[suivante(indice)]; // Chargement de la première variable
  next_value(variableCourante); // Chargement de sa première valeur

  int depth = 0; // On commence au niveau 0
  while(0 <= depth && depth < csp->nb_variables){ // Boucle de parcours de l'arbre
    if(check_contraintes(variableCourante)){ // Si l'affectation de la variable courante satisfait les contraintes
      depth++; // On descend dans l'arbre
      if(depth < csp->nb_variables){ // Si on a pas encore atteint le fond
        variableCourante = &csp->variables[suivante(indice)]; // On charge la prochaine variable
        next_value(variableCourante); // On charge sa première valeur
      }
    }
    else{ // Si une contrainte n'est pas satisfaite
      while(!next_value(variableCourante) && 0 <= depth){ // Tant qu'il n'y a pas de valeur suivante et qu'on peut remonter
        variableCourante->valeur = 0; depth--; // On remonte dans l'arbre
        if(0 <= depth)
          variableCourante = &csp->variables[precedente(indice)]; // On charge la variable précédente
      }
    }
  }
  if(depth == csp->nb_variables){ // Si on a trouvé une solution
    printf("SOLUTION TROUVEE: [%d noeuds] [%d tests]\n", nb_noeud, nb_test_contrainte);
    if(print_solution){
      print_variables(csp); check_solution(csp);
    }
    else printf("\n");
  }
  else printf("AUCUNE SOLUTION.\n");
}
