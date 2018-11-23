#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "structures.h"
#include "prefiltrage.h"

/* Filtrage des valeurs possibles des variables affectées par des sommes */

static void add_combinaison(Combinaisons * combi, int arite, int * sequence){ // Sauvegarde d'une nouvelle combinaison
  memcpy(combi->tab[arite][combi->nbSaved[arite]], sequence, sizeof(int) * arite);
  combi->nbSaved[arite]++;
}

/* Fonction construisant récursivement les combinaisons possibles pour une arité donnée */
static void calcul_combinaisons(Combinaisons * combi, int arite, int restant, int * sequence, int min){
  for(int i=min+1; i<=9; i++){
    sequence[arite-restant] = i;
    if(restant == 1) add_combinaison(combi, arite, sequence); // Ajout de la nouvelle combinaison trouvée à la structure mémoire
    else calcul_combinaisons(combi, arite, restant-1, sequence, i);
  }
}

/* Utile au calcul du coefficient binomial */
static int factoriel(int x){ int y=1; for(; x>1; --x) y*=x; return y; }
/* Calcul du nombre de combinaisons non-redondantes : merci les mathématiques combinatoires */
static int coeff_binomial(int k){ return factoriel(9) / (factoriel(k) * factoriel(9 - k)); }

Combinaisons * init_combinaisons(){ // Calcul des combinaisons
  Combinaisons * combi = malloc (sizeof(Combinaisons));
  combi->nbSaved = (int*) malloc (sizeof(int) * 10); // Tableau du nombre de combinaisons pour chaque arité (2 à 9)
  combi->tab = (int***) malloc (sizeof(int**) * 10); // Tableau des combinaisons possibles
  int nb_combi = 0;
  for(int arite=2; arite<=9; arite++){ // Pour les arités de 2 à 9
    combi->nbSaved[arite] = 0;
    nb_combi = coeff_binomial(arite); // On calcule combien il existe de combinaisons non-redondantes grâce au coefficient binomial
    combi->tab[arite] = malloc (sizeof(int*) * nb_combi);
    for(int i=0; i<nb_combi; i++) 
      combi->tab[arite][i] = malloc (sizeof(int) * arite);
    calcul_combinaisons(combi, arite, arite, malloc(sizeof(int) * arite), 0); // Puis on calcule les combinaisons en question
  } return combi; // On retourne la structure remplie
}

void print_allcombinaisons(Combinaisons * combi){ // Affichage des combinaisons calculées
  for(int arite=2; arite<=9; arite++){
    for(int i=0; i<combi->nbSaved[arite]; i++){
      for (int j=0; j<arite; j++)
        printf("%d ", combi->tab[arite][i][j]);
      printf("\n");
    } printf("\n");
  }
}

static void filtrage_valeurs(Combinaisons * combi, Contrainte * contrainte){ // Restriction des domaines des variables d'une SOMME 
  int arite = contrainte->arite, somme = 0;
  int * autorisees = malloc(sizeof(int) * 9); // Création d'un nouveau domaine
  for(int i=0; i<9; i++) autorisees[i] = 0; // Initialisation

  for(int i=0; i<combi->nbSaved[arite]; i++){ // On parcourt toutes les combinaisons calculées préalablement pour cette arité (nb de var)
    somme = 0;
    for (int j=0; j<arite; j++)
      somme += combi->tab[arite][i][j]; // Calcul de la somme des valeurs de la combinaison courante
    if(somme == contrainte->somme) // Si total = valeur de notre contrainte SOMME, alors on a la liste des valeurs autorisées
      for(int j=0; j<arite; j++)
        if(autorisees[combi->tab[arite][i][j]-1] == 0) // Recopiage des valeurs autorisées dans le nouveau domaine
          autorisees[combi->tab[arite][i][j]-1] = combi->tab[arite][i][j];
  }
  for(int var=0; var<arite; var++) // Mise à jour des domaines des variables 
    memcpy(contrainte->portee[var]->domaine, autorisees, sizeof(int) * 9);
}

void prefiltrage_sommes_possibles(Csp source){ // Filtrage des valeurs impossibles pour les variables dans les sommes d'un csp
  Csp * csp = &source;
  Combinaisons * combi = init_combinaisons(); // Calcul de tous les cas de figure de sommes
  //print_allcombinaisons(combi);
  for(int i=0; i<csp->nb_contraintes; i++) // Parcours des contraintes de SOMME
    if(csp->contraintes[i].type == SOMME)
      filtrage_valeurs(combi, &csp->contraintes[i]); // Réalisation du filtrage des valeurs, par rapport aux combinaisons calculées
  printf("\nPre-filtrage : OK\n");
}
