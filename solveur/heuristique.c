#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "structures.h"
#include "tri.h"
#include "heuristique.h"
#define INFINI 99999

Ordre * choix_heuristique(Csp * csp, enum TypeHeuristique type){ // Renvoie un tableau ordonnée d'indices de variables pour une heuristique
  if(type == DOMOVERDEG || type == DYNAMIC_DOMOVERDEG) return init_DomOverDeg(csp, type);
  else if(type == CROSSINGFIRST || type == DYNAMIC_CROSSINGFIRST) return init_CrossingFirst(csp, type);
  else if(type == FILLSUM) return init_FillSum(csp, type);
  /* Ajout nouvelle heuristique ici */
  else /* DEFAULT */ return init_Default(csp);
}

const char * print_heuristique(enum TypeHeuristique type){ // Renvoie le nom de l'heuristique
  if(type == DOMOVERDEG) return "DomOverDeg";
  else if(type == DYNAMIC_DOMOVERDEG) return "Dynamic_DomOverDeg";
  else if(type == CROSSINGFIRST) return "CrossingFirst";
  else if(type == DYNAMIC_CROSSINGFIRST) return "Dynamic_CrossingFirst";
  else if(type == FILLSUM) return "FillSum";
  /* Ajout nouvelle heuristique ici */
  else /* DEFAULT */ return "Default";
}

Ordre * init_Default(Csp * csp){ // Initialisation avec l'heuristique d'ordre par défaut
  Ordre * ordre = malloc (sizeof(Ordre));
  ordre->csp = csp;
  ordre->type = DEFAULT;
  ordre->curseur = -1;
  ordre->indices = (int*) malloc (sizeof(int) * csp->nb_variables);
  for(int i=0; i<csp->nb_variables; i++) ordre->indices[i] = i;
  return ordre;
}

void print_ordre(Ordre * ordre){ // Affiche l'ordre des indices
  for(int i=0; i<ordre->csp->nb_variables; i++)
    printf("%d ", ordre->indices[i]);
  printf("\n");
}

int precedente(Ordre * ordre){ // Renvoie l'indice précédent dans la liste ordonnée
  ordre->curseur--;
  return ordre->indices[ordre->curseur];
}

int suivante(Ordre * ordre){ // Renvoie l'indice suivant dans la liste ordonnée
  if(ordre->type == DYNAMIC_DOMOVERDEG) update_DomOverDeg(ordre);
  else if(ordre->type == DYNAMIC_CROSSINGFIRST) update_CrossingFirst(ordre);
  /* Ajout nouvelle heuristique ici */
  ordre->curseur++;
  return ordre->indices[ordre->curseur];
}


/* Heuristique Dom/Deg, version statique ou dynamique */

Ordre * init_DomOverDeg(Csp * csp, enum TypeHeuristique type){ // Initialisation avec Dom/Deg
  Ordre * ordre = init_Default(csp); // Initialisation par défaut
  ordre->type = type;                // Changement du type d'heuristique
  int valeurs_restantes = 0;
  double * ratio = malloc (sizeof(double) * csp->nb_variables);
  for(int i=0; i<csp->nb_variables; i++){ // Calcul du ratio dom/deg pour chaque variable
    valeurs_restantes = csp->variables[i].nb_valeurs;
    for (int j=0; j<csp->variables[i].nb_valeurs; j++) // Calcul de la taille du domaine actualisée (valeurs restantes)
      if(csp->variables[i].domaine[j] == 0)
        valeurs_restantes--;
    ratio[i] = (double) valeurs_restantes / csp->variables[i].nb_contraintes; // calcul Dom/Deg
  }
  tri_Par_Tas(csp->nb_variables, ratio, ordre->indices); // Tri des variables, du plus petit ratio au plus grand
  free(ratio);
  return ordre; // On renvoie la structure contenant la liste ordonnée des indices
}

void update_DomOverDeg(Ordre * ordre){ // Mise à jour de Dom/Deg
  int firstElement = ordre->curseur+1;
  int valeurs_restantes = 0, min = 999, best = -1;
  double * ratio = malloc (sizeof(double) * ordre->csp->nb_variables);
  for(int i=firstElement; i<ordre->csp->nb_variables; i++){ // On se préoccupe seulement des variables à traiter ensuite
    valeurs_restantes = ordre->csp->variables[ordre->indices[i]].nb_valeurs;
    for (int j=0; j<ordre->csp->variables[ordre->indices[i]].nb_valeurs; j++)
      if(ordre->csp->variables[ordre->indices[i]].domaine[j] == 0)
        valeurs_restantes--;
    ratio[i] = (double) valeurs_restantes / ordre->csp->variables[ordre->indices[i]].nb_contraintes; // Nouveau calcul des ratios
    if(min > ratio[i]){ // Conservaton du meilleur ratio, inutile de tout retrier
      min = ratio[i];
      best = i;
    }
  } free(ratio);
  if(firstElement != best){ // Si on a trouvé un meilleur ratio, on intervertit les variables
    int temp = ordre->indices[firstElement];
    ordre->indices[firstElement] = ordre->indices[best];
    ordre->indices[best] = temp;
  }
}


/* Heuristique favorisant les variables aux intersections des sommes, version statique ou dynamique */

Ordre * init_CrossingFirst(Csp * csp, enum TypeHeuristique type){ // Initialisation avec CrossingFirst
  Ordre * ordre = init_Default(csp); // Initialisation par défaut
  ordre->type = type;                // Changement du type d'heuristique
  int valeurs_restantes = 0, counterSum = 0, counterDiff = 0;
  double * ratio = malloc (sizeof(double) * csp->nb_variables);
  for(int i=0; i<csp->nb_variables; i++){ // Calcul des ratios des variables
    valeurs_restantes = csp->variables[i].nb_valeurs;
    for (int j=0; j<csp->variables[i].nb_valeurs; j++) // Calcul de la taille du domaine actualisée (valeurs restantes)
      if(csp->variables[i].domaine[j] == 0)
        valeurs_restantes--;
    counterSum = 0, counterDiff = 0;
    Liste maillon = csp->variables[i].contrainte;
    while(maillon->courante != NULL){
      if(maillon->courante->type == SOMME) // Comptage des contraintes de SOMME
        counterSum++;
      else if(maillon->courante->type == DIFFERENCE) // Comptage des contraintes de DIFFERENCE
        counterDiff++;
      maillon = maillon->suivante;
    }
    /* Calcul du ratio comme Dom/Deg, mais en donnant plus de poids aux variables aux croisements de 2 sommes */
    ratio[i] = (double) valeurs_restantes / (10 * counterSum + counterDiff);
  }
  tri_Par_Tas(csp->nb_variables, ratio, ordre->indices); // Tri des variables, du plus petit ratio au plus grand
  free(ratio);
  return ordre; // On renvoie la structure contenant la liste ordonnée des indices
}

void update_CrossingFirst(Ordre * ordre){ // Mise à jour de CrossingFirst
  int firstElement = ordre->curseur+1;
  int valeurs_restantes = 0, counterSum = 0, counterDiff = 0, min = 999, best = -1;
  double * ratio = malloc (sizeof(double) * ordre->csp->nb_variables);
  for(int i=firstElement; i<ordre->csp->nb_variables; i++){ // On se préoccupe seulement des variables à traiter ensuite
    valeurs_restantes = ordre->csp->variables[ordre->indices[i]].nb_valeurs;
    for (int j=0; j<ordre->csp->variables[ordre->indices[i]].nb_valeurs; j++)
      if(ordre->csp->variables[ordre->indices[i]].domaine[j] == 0)
        valeurs_restantes--;
    counterSum = 0, counterDiff = 0;
    Liste maillon = ordre->csp->variables[ordre->indices[i]].contrainte;
    while(maillon->courante != NULL){
      if(maillon->courante->type == SOMME)
        counterSum++;
      else if(maillon->courante->type == DIFFERENCE)
        counterDiff++;
      maillon = maillon->suivante;
    }
    ratio[i] = (double) valeurs_restantes / (10 * counterSum + counterDiff); // Nouveau calcul des ratios
    if(min > ratio[i]){ // Conservaton du meilleur ratio, inutile de tout retrier
      min = ratio[i];
      best = i;
    }
  } free(ratio);
  if(firstElement != best){ // Si on a trouvé un meilleur ratio, on intervertit les variables
    int temp = ordre->indices[firstElement];
    ordre->indices[firstElement] = ordre->indices[best];
    ordre->indices[best] = temp;
  }
}

/* Heuristique favorisant la complétion des sommes, d'abord sommes à petite arité et variables à petit domaine */

Ordre * init_FillSum(Csp * csp, enum TypeHeuristique type){ // Initialisation avec FillSum
  Ordre * ordre = init_Default(csp); // Initialisation par défaut
  ordre->type = type;                // Changement du type d'heuristique
  int variables_restantes = csp->nb_variables;
  int * indices = malloc (sizeof(int) * csp->nb_variables);
  int * contraintes_restantes = malloc (sizeof(int) * csp->nb_contraintes);
  for(int i=0; i<csp->nb_contraintes; i++) // Tableau de booléens des contraintes restantes
    contraintes_restantes[i] = 1;

  while(variables_restantes != 0){ // Tant qu'on a pas listé toutes les variables
    int min_arite = 10, select = -1;
    for(int i=0; i<csp->nb_contraintes; i++) // Sélection de la contraite restante
      if(contraintes_restantes[i] != 0)
        /* Selection de la contrainte de somme à plus petit arité */
        if (csp->contraintes[i].type == SOMME && csp->contraintes[i].arite < min_arite){
          min_arite = csp->contraintes[i].arite;
          select = i;
        }
    contraintes_restantes[select] = 0;

    int * tab_var = malloc (sizeof(int) * min_arite); // Récupération des variables
    double * tab_dom = malloc (sizeof(double) * min_arite);
    for(int i=0; i<min_arite; i++){
      Variable *temp = csp->contraintes[select].portee[i];
      int valeurs_restantes = temp->nb_valeurs;
      for (int j=0; j<temp->nb_valeurs; j++)
        if(temp->domaine[j] == 0)
          valeurs_restantes--;
      tab_var[i] = temp->indice;
      tab_dom[i] = (double) valeurs_restantes;
    }
    tri_Rapide(min_arite, tab_dom, tab_var); // Tri par taille de domaine (+ petit au + grand)

    for(int i=0; i<min_arite; i++){
      int already = 0;
      for(int j=0; j<csp->nb_variables-variables_restantes; j++) // Variable déjà présente ?
        if(indices[j] == tab_var[i])
          already = 1;
      if(!already){ // Ajout de la variable aux indices
        indices[csp->nb_variables-variables_restantes] = tab_var[i];
        variables_restantes--;
      }
    }
  }
  memcpy(ordre->indices, indices, sizeof(int) * csp->nb_variables);
  return ordre; // On renvoie la structure contenant la liste ordonnée des indices
}
