#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "parser.h"
#include "structures.h"
#include "prefiltrage.h"
#include "heuristique.h"
#include "backtracking.h"
#include "forwardchecking.h"
//#include "tri.h"

int print_solution = 1; // Utile lors des tests
int active_prefiltrage = 1; // Activation ou non du préfiltrage des sommes possibles
int active_print_csp = 0;   // Affiche le Csp lors des résultats

double what_time_is_it(){
  struct timespec now;
  clock_gettime(CLOCK_REALTIME, &now);
  return now.tv_sec + now.tv_nsec * 1e-9;
}

int main (int argc, char * argv[]){
  if (argc != 4){
    fprintf(stderr,"3 arguments needed : ./solver filename -algo =heuristic\n");
    exit(-1);
  }

  /* Choix de l'algorithme */
  void (*algo) (Csp source, enum TypeHeuristique type); // Création du pointeur sur fonction

  if(!strcmp(argv[2], "-bt"))
    algo = backtracking;
  else if(!strcmp(argv[2], "-fc"))
    algo = forwardchecking;
  else if(!strcmp(argv[2], "-fcfa"))
    algo = forwardchecking_FindAll;
  else{
    fprintf(stderr, "Invalid algorithm : %s\n", argv[2]);
    printf("-bt\t->\tBackTracking\n");
    printf("-fc\t->\tForwardChecking\n");
    printf("-fcfa\t->\tForwardChecking_FindAll\n");
    exit(-1);
  }

  /* Choix de l'heuristique */
  enum TypeHeuristique heuristic; // Création de la variable d'heuristique

  if (!strcmp(argv[3], "=def"))
    heuristic = DEFAULT;
  else if (!strcmp(argv[3], "=dod"))
    heuristic = DOMOVERDEG;
  else if (!strcmp(argv[3], "=ddod"))
    heuristic = DYNAMIC_DOMOVERDEG;
  else if (!strcmp(argv[3], "=cf"))
    heuristic = CROSSINGFIRST;
  else if (!strcmp(argv[3], "=dcf"))
    heuristic = DYNAMIC_CROSSINGFIRST;
  else if (!strcmp(argv[3], "=fs"))
    heuristic = FILLSUM;
  else{
    fprintf(stderr, "Invalid heuristic : %s\n", argv[3]);
    printf("=def\t->\tDefault\n");
    printf("=dod\t->\tDomOverDeg\n");
    printf("=ddod\t->\tDynamic_DomOverDeg\n");
    printf("=cf\t->\tCrossingFirst\n");
    printf("=dcf\t->\tDynamic_CrossingFirst\n");
    printf("=fs\t->\tFillSum\n");
    exit(-1);
  }

  double time = what_time_is_it(); // Top chrono !

  Csp solution = init_csp(); // Initialisation du csp
  parse(argv[1], &solution); // Lecture du fichier et remplissage du csp
  
  if(active_prefiltrage)
    prefiltrage_sommes_possibles(solution); // Filtrage préliminaire des valeurs
  if(active_print_csp)
    print_csp(solution); // Affichage du csp

  (*algo) (solution, heuristic); // Resolution avec l'algorithme et l'heuristique choisis

  printf("Time Taken %.6lf\n", what_time_is_it() - time); // Stop chrono !

  /* TEST DES TRIS
  int nb_variables = 9, first = 0;
  double valeurs[9] = {313,58,467,86,275,42,987,409,174};
  int indices[9] = {0,1,2,3,4,5,6,7,8};
  printf("AVANT\n"); for(int i=first; i<nb_variables; i++) printf("%d = %lf\n", indices[i], valeurs[i]);
  //tri_Par_Tas(nb_variables-first, &valeurs[first], &indices[first]);
  //tri_Rapide(nb_variables-first, &valeurs[first], &indices[first]);
  printf("\nAPRES\n"); for(int i=first; i<nb_variables; i++) printf("%d = %lf\n", indices[i], valeurs[i]);
  */
  return 0;
}
