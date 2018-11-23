#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "structures.h"
#include "heuristique.h"
#include "forwardchecking.h"

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

static int *** init_memory(Csp * csp){ // Initialisation du tableau sauvegardant les états du Csp
  int var = csp->nb_variables, max_val = 0;
  for(int i=0; i<csp->nb_variables; i++)
    if(csp->variables[i].nb_valeurs > max_val)
      max_val = csp->variables[i].nb_valeurs;

  int *** memory = malloc (sizeof(int**) * var); // Tableau en 3 dimmentions : état/domaine/valeur
  for(int i=0; i<var; i++){ // Pour chaque profondeur (état utile) possible
    memory[i] = malloc (sizeof(int*) * var);
    for(int j=0; j<var; j++) // Pour chaque domaine de variable
      memory[i][j] = malloc (sizeof(int) * max_val);
  } return memory;
}

static void save_values(Csp * csp, int *** memory, int level){ // Sauvegarde d'un état du Csp à une profondeur (level)
  for(int var=0; var<csp->nb_variables; var++)
    memcpy(memory[level][var], csp->variables[var].domaine, sizeof(int)*csp->variables[var].nb_valeurs);
}

static void restore_values(Csp * csp, int *** memory, int level){ // Restauration d'un état antérieur (level) du Csp
  for(int var=0; var<csp->nb_variables; var++)
    memcpy(csp->variables[var].domaine, memory[level][var], sizeof(int)*csp->variables[var].nb_valeurs);
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

static void delete_value(int valeur, Variable * variable){ // Annule cette valeur dans le domaine de la variable
  for(int i=0; i<variable->nb_valeurs; i++)
    if(variable->domaine[i] == valeur)
      variable->domaine[i] = 0;
}

static void delete_bigger_values(int valeur, Variable * variable){ // Annule les valeurs plus grandes dans le domaine de la variable
  for(int i=0; i<variable->nb_valeurs; i++)
    if(variable->domaine[i] > valeur)
      variable->domaine[i] = 0;
}

static void delete_differents_values(int valeur, Variable * variable){ // Annule les valeurs différentes dans le domaine de la variable
  for(int i=0; i<variable->nb_valeurs; i++)
    if(variable->domaine[i] != valeur)
      variable->domaine[i] = 0;
}

static int nombre_triangulaire(int x){ return (x * (x + 1)) / 2; } // Calcul du total des valeurs min des autres variables d'une somme

static void filtre_contrainte(Csp * csp, Contrainte * contrainte){ // Réalise un filtrage des valeurs du Csp par rapport à une contrainte
  nb_test_contrainte++;
  if(contrainte->type == DIFFERENCE){ // Filtrage d'une contrainte de DIFFERENCE
    if(contrainte->portee[0]->valeur != 0 && contrainte->portee[1]->valeur == 0) // Si la première variable est affectée, filtre la seconde
      delete_value(contrainte->portee[0]->valeur, contrainte->portee[1]);
    else if(contrainte->portee[1]->valeur != 0 && contrainte->portee[0]->valeur == 0) // Fait l'inverse
      delete_value(contrainte->portee[1]->valeur, contrainte->portee[0]);
  }
  else if(contrainte->type == SOMME){ // Filtrage d'une contrainte de SOMME
    int somme = 0, compteur = 0;
    for(int i=0; i<contrainte->arite; i++)
      if(contrainte->portee[i]->valeur != 0){
        somme += contrainte->portee[i]->valeur; // Calcul la somme actuelle des variables affectées par la contrainte
        compteur++;
      }
    int max = contrainte->somme - somme; // Nombre manquant pour atteindre le total de la somme
    for(int i=0; i<contrainte->arite; i++)
      if(contrainte->portee[i]->valeur == 0){ // Pour chaque variable encore non-définie
        if(contrainte->arite - compteur > 1) // S'il reste plus d'une variable non-définie
          /* Suppression des valeurs trop élevées, relativement à ce qui manque et aux valeurs possibles selon le nombre de variables */
          delete_bigger_values(max - nombre_triangulaire(contrainte->arite - compteur - 1), contrainte->portee[i]);
        else delete_differents_values(max, contrainte->portee[i]); // s'il ne reste qu'une variable non-définie, une seule possibilité
      }
  }
  else if(contrainte->type == ALLDIFFERENT) // Non utilisé pour le kakuro
    for(int i=0; i<contrainte->arite; i++)
      for(int j=i+1; j<contrainte->arite; j++)
        if(contrainte->portee[i]->valeur != 0 && contrainte->portee[j]->valeur == 0)
          delete_value(contrainte->portee[i]->valeur, contrainte->portee[j]);
}

static int domaine_non_vide(Variable * variable){ // Renvoie 1 si le domaine de la variable est encore non vide, sinon 0
  for(int i=0; i<variable->nb_valeurs; i++)
    if(variable->domaine[i] > 0) return 1;
  return 0;
}

static int filtrage_valeurs(Csp * csp, Variable * variableCourante){ // Filtrage des valeurs du Csp, après l'affection de la variable courante
  nb_noeud++;
  Liste maillon = variableCourante->contrainte;
  while(maillon->courante != NULL){ // Parcours et filtrage par contraintes liées à la variable courante
    filtre_contrainte(csp, maillon->courante);
    maillon = maillon->suivante;
  }
  for(int i=0; i<csp->nb_variables; i++) // Verifie si un domaine de variable est désormais vide
    if(!domaine_non_vide(&csp->variables[i])) return 0; // Si c'est le cas, échec
  return 1; // Sinon succès
}

static int test_contrainte(Contrainte * contrainte){ // Teste la contrainte, renvoie 1 si satisfaite, sinon 0
  if (contrainte->type == DIFFERENCE && contrainte->portee[0]->valeur == contrainte->portee[1]->valeur) // test contrainte de DIFFERENCE
    return 0;
  else if(contrainte->type == SOMME){ // test contrainte de SOMME
    int somme = 0;
    for(int i=0; i<contrainte->arite; i++)
      somme += contrainte->portee[i]->valeur;
    if(somme != contrainte->somme) return 0;
  }
  else if(contrainte->type == ALLDIFFERENT) // Non utilisé pour le kakuro
    for(int i=0; i<contrainte->arite; i++) // Ignore si une valeur est non définie
      for(int j=i+1; j<contrainte->arite; j++)
        if(contrainte->portee[i]->valeur == contrainte->portee[j]->valeur) return 0;
  return 1;
}

static void check_solution(Csp * csp){ // Vérifie si la solution trouvée satisfait bien toutes les contraintes
  printf("VERIFICATION DE LA SOLUTION : ");
  for(int i=0; i<csp->nb_variables; i++)
    if(csp->variables[i].valeur == 0){ printf("ECHEC (Cause : certaines variables non-définies)\n"); return; }
  for(int i=0; i<csp->nb_contraintes; i++)
    if(!test_contrainte(&csp->contraintes[i])){ printf("ECHEC (Cause : contraintes non-satisfaites)\n"); return; }
  printf("OK !\n\n");
}

void forwardchecking(Csp source, enum TypeHeuristique type){ // Algorithme de résolution, demandant un csp et une heuristique
  nb_noeud = 0;
  nb_test_contrainte = 0;
  Csp *csp = &source;                           // Pointeur sur le csp en argument
  reset_variables(csp);                         // Réinitialisation des valeurs des variables, par précaution
  Ordre *indice = choix_heuristique(csp, type); // Initialisation de l'ordre de traitement des variables, selon l'heuristique
  printf("Algorithme : ForwardChecking\nHeuristique : %s\n", print_heuristique(type)); fflush(stdout);
  Variable *variableCourante = &csp->variables[suivante(indice)]; // Chargement de la première variable
  int *** memory = init_memory(csp); // Initialisation de la pile mémoire
  save_values(csp, memory, 0); // Sauvegarde de l'état initial
  next_value(variableCourante); // Chargement de la première valeur de la variable courante

  int depth = 0; // On commence au niveau 0
  while(0 <= depth && depth < csp->nb_variables){ // Boucle de parcours de l'arbre
    if(filtrage_valeurs(csp, variableCourante)){ // Si le filtrage s'est bien déroulé sans rendre un domaine vide
      depth++; // On descend dans l'arbre
      if(depth < csp->nb_variables){ // Si on a pas encore atteint le fond
        variableCourante = &csp->variables[suivante(indice)]; // On charge la prochaine variable
        save_values(csp, memory, depth); // Sauvegarde de l'état du Csp
        next_value(variableCourante); // On charge sa prochaine valeur
      }
    }
    else{ // Le filtrage a rendu un domaine vide, on doit revenir en arrière
      restore_values(csp, memory, depth); // Restauration de l'état précédent du Csp
      while(!next_value(variableCourante) && 0 <= depth){ // Tant qu'il n'y a pas de valeur suivante et qu'on peut remonter
        variableCourante->valeur = 0; depth--; // On remonte dans l'arbre
        if(0 <= depth){
          variableCourante = &csp->variables[precedente(indice)]; // On charge la variable précédente
          restore_values(csp, memory, depth);                     // Restauration de l'état précédent du Csp
        }
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
  restore_values(csp, memory, 0); // Annulation des changements
}

static void print_result(Csp * csp, int check){ // Affichage d'un résultat de forwardchecking_FindAll
  for(int var=0; var<csp->nb_variables; var++)
    printf(" %d", csp->variables[var].valeur);
  if(check){
    for(int i=0; i<csp->nb_variables; i++)
      if(csp->variables[i].valeur == 0){
        printf("\tECHEC (Cause : certaines variables non-définies)\n");
        return;
      }
    for(int i=0; i<csp->nb_contraintes; i++)
      if(!test_contrainte(&csp->contraintes[i])){
        printf("\tECHEC (Cause : contraintes non-satisfaites)\n");
        return;
      }
    printf("\tOK ! [%d noeuds] [%d tests]\n", nb_noeud, nb_test_contrainte);
  } fflush(stdout);
}

void forwardchecking_FindAll(Csp source, enum TypeHeuristique type){ // Algorithme ForwardChecking trouvant toutes les solutions (HORS PROJET)
  nb_noeud = 0;
  nb_test_contrainte = 0;
  Csp *csp = &source;
  reset_variables(csp);
  Ordre * indice = choix_heuristique(csp, type);
  printf("Algorithme : ForwardChecking_FindAll\nHeuristique : %s\n", print_heuristique(type)); fflush(stdout);
  Variable * variableCourante = &csp->variables[suivante(indice)];
  int *** memory = init_memory(csp);
  save_values(csp, memory, 0);
  next_value(variableCourante);

  int depth = 0;
  while(0 <= depth){
    if(filtrage_valeurs(csp, variableCourante)){
      depth++;
      if(depth < csp->nb_variables){
        variableCourante = &csp->variables[suivante(indice)];
        save_values(csp, memory, depth);
        next_value(variableCourante);
      }
      else{ // Une solution a été trouvée
        if(print_solution){
          print_result(csp, 1); fflush(stdout);
        }
        depth--; restore_values(csp, memory, depth);
        while(!next_value(variableCourante) && 0 <= depth){
          variableCourante->valeur = 0; depth--;
          if(0 <= depth){
            variableCourante = &csp->variables[precedente(indice)];
            restore_values(csp, memory, depth);
          }
        }
      }
    }
    else{
      restore_values(csp, memory, depth);
      while(!next_value(variableCourante) && 0 <= depth){
        variableCourante->valeur = 0; depth--;
        if(0 <= depth){
          variableCourante = &csp->variables[precedente(indice)];
          restore_values(csp, memory, depth);
        }
      }
    }
  } printf("TOTAL : [%d noeuds] [%d tests]\n\n", nb_noeud, nb_test_contrainte);
}
