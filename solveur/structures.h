#ifndef STRUCTURES_H
#define STRUCTURES_H
#define MAXVAR 1000
#define MAXCONTR 1000

enum TypeContrainte {DIFFERENCE, SOMME, ALLDIFFERENT}; // ALLDIFFERENT : non utilisé pour le kakuro

typedef struct Contrainte Contrainte;
typedef struct Maillon *Liste;
typedef struct Variable Variable;
typedef struct Csp Csp;

struct Contrainte{
  int indice; // Indice de la contrainte
  enum TypeContrainte type; // Son type (DIFFERENCE, SOMMME, ...)
  int somme; // Valeur de la somme, si elle est de type SOMME
  int arite; // Nombre de variables affectées par la contrainte
  Variable **portee; // Tableaux des pointeurs sur les variables affectées
};

struct Maillon{ // Structure utile pour créer les listes chainées de contraintes
  Contrainte *courante;  // Pointeur sur une contrainte
  Liste suivante; // Maillon suivant
};

struct Variable{
  int indice; // Indice de la variable
  int valeur; // Valeur courante de la variable
  int nb_valeurs; // Taille du domaine
  int *domaine; // Tableau des valeurs possibles
  int nb_contraintes; // Nombre de contraintes portant sur la variable
  Liste contrainte; // Liste chainée permettant l'accès aux contraintes affectantes
};

struct Csp{
  int nb_variables; // Nombre de variables existantes
  int nb_contraintes; // Nombre de contraintes existantes
  Variable *variables; // Tableau de stockage des variables
  Contrainte *contraintes; // Tableau de stockage des contraintes
};

Csp init_csp();
void add_variable(Csp * csp, int indice, int nb_valeurs, int * tab_valeurs);
void add_maillon(Variable * variable, Contrainte * contrainte);
void add_contrainte(Csp * csp, enum TypeContrainte type, int somme, int arite, int * portee);
void print_csp(Csp csp);

#endif
