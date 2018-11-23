#ifndef PREFILTRAGE_H
#define PREFILTRAGE_H

typedef struct Combinaisons{ // Structure servant de mémoire pour les combinaisons calculées
  int * nbSaved; // Tableau du nombre de combinaisons possibles par arité
  int *** tab; // Tableau des combinaisons possibles (en 3D : arité/combinaison/valeur)
} Combinaisons;

Combinaisons * init_combinaisons();
void print_allcombinaisons(Combinaisons * combi);
void prefiltrage_sommes_possibles(Csp source);

#endif
