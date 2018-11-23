#ifndef HEURISTIQUE_H
#define HEURISTIQUE_H

enum TypeHeuristique { // Les différents types d'heuristique conçus
  DEFAULT,
  DOMOVERDEG, DYNAMIC_DOMOVERDEG,
  CROSSINGFIRST, DYNAMIC_CROSSINGFIRST,
  FILLSUM
};

typedef struct Ordre{
  Csp * csp; // Pointeur sur le Csp étudié
  enum TypeHeuristique type; // Type de l'heuristique
  int curseur; // Indice de la Variable courante
  int * indices; // Tableau ordonné des indices de variables
} Ordre;

Ordre * choix_heuristique(Csp * csp, enum TypeHeuristique type); // Renvoie un tableau ordonnée d'indices de variables pour une heuristique
const char * print_heuristique(enum TypeHeuristique type); // Renvoie le nom de l'heuristique

Ordre * init_Default(Csp * csp); // Initialise avec l'heuristique par défaut
void print_ordre(Ordre * ordre); // Affiche l'ordre des indices
int precedente(Ordre * ordre); // Renvoie l'indice précédent dans la liste ordonnée
int suivante(Ordre *ordre);    // Renvoie l'indice suivant dans la liste ordonnée

Ordre * init_DomOverDeg(Csp * csp, enum TypeHeuristique type); // Initialise avec l'heuristique Dom/Deg
void update_DomOverDeg(Ordre * ordre); // Met à jour l'ordre selon Dom/Deg

Ordre * init_CrossingFirst(Csp *csp, enum TypeHeuristique type); // Initialise avec l'heuristique CrossingFirst
void update_CrossingFirst(Ordre * ordre); // Met à jour l'ordre selon CrossingFirst

Ordre * init_FillSum(Csp *csp, enum TypeHeuristique type); // Initialise avec l'heuristique FillSum

#endif
