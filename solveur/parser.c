#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "parser.h"
#include "structures.h"

/* utiles pour le kakuro : */
static int nb_valeurs = 9; // nombre de valeurs possibles pour une case
static int tab_valeurs[9] = {1, 2, 3, 4, 5, 6, 7, 8, 9}; // ces valeurs

enum couleur {BLANCHE, NOIRE};   /* les deux couleurs possibles pour les cases */

typedef struct
{
  enum couleur coul;      /* la nature de la case */
  int num;                /* le numero de la case s'il s'agit d'une case noire */
  int somme_horizontale;  /* la valeur de la somme horizontale le cas écheant, -1 sinon */
  int somme_verticale;    /* la valeur de la somme verticale le cas écheant, -1 sinon */
} Case;


void parse (char * nom_fichier, Csp * csp)
{
  FILE * f;         /* le descripteur du fichier à lire */
  int c;            /* le caractère courant */
  int nb_lignes;    /* le nombre de lignes de la grille */
  int nb_colonnes;  /* le nombre de colonnes de la grille */
  int num_ligne;    /* le numero de la ligne courante */
  int num_colonne;  /* le numero de la colonne courante */
  int nb_variables; /* le nombre de variables déjà trouvées */
  int somme;        /* une somme */

  Case ** grille;   /* la grille de jeu */
  int * portee;     /* la portee d'une contrainte */
  int arite;        /* l'arité d'une contrainte */
  int i,j;          /* des compteurs de boucle */

  f = fopen (nom_fichier,"r");

  if (f == NULL)
  {
    fprintf (stderr,"Fichier %s non trouve\n",nom_fichier);
    exit (-1);
  }

  /* on calcule la taille de la grille */

  nb_lignes = 0;
  nb_colonnes = 0;

  c = fgetc(f);

  while (! feof (f))
  {
    if (c == '\n')
      nb_lignes++;
    else
      if ((nb_lignes == 0) && ((c == '.') || (c == '\\')))
        nb_colonnes++;

    c = fgetc(f);
  }
  fclose (f);

  //printf ("Taille : %d x %d\n",nb_lignes,nb_colonnes);


  /* remplissage de la grille */

  grille = (Case **) malloc (sizeof(Case *) * nb_lignes);
  for (num_ligne = 0; num_ligne < nb_lignes; num_ligne++)
    grille[num_ligne] = (Case *) malloc (sizeof(Case) * nb_colonnes);

  f = fopen (nom_fichier,"r");

  if (f == NULL)
  {
    fprintf (stderr,"Fichier %s non trouve\n",nom_fichier);
    exit (-1);
  }

  num_ligne = 0;
  num_colonne = 0;
  nb_variables = 0;

  c = fgetc(f);

  while (! feof (f))
  {
    if (c == '\n')
    {
      num_ligne++;
      num_colonne = 0;
    }
    else
      if (c == ' ')
        num_colonne++;
      else
        if (c == '.')     /* case blanche */
        {
          grille[num_ligne][num_colonne].coul = BLANCHE;
          grille[num_ligne][num_colonne].num = nb_variables;
          grille[num_ligne][num_colonne].somme_horizontale = -1;
          grille[num_ligne][num_colonne].somme_verticale = -1;

          Nouvelle_Variable(csp, nb_variables);

          nb_variables++;
        }
        else
          if (c == '\\')    /* case noire de la forme \y ou \ */
          {
            grille[num_ligne][num_colonne].coul = NOIRE;
            grille[num_ligne][num_colonne].num = -1;
            grille[num_ligne][num_colonne].somme_verticale = -1;

            c = fgetc(f);

            if (isdigit(c))    /* case noire de la forme \y */
            {
              ungetc (c,f);
              fscanf (f,"%d",&somme);
              grille[num_ligne][num_colonne].somme_horizontale = somme;
            }
            else  /* case noire de la forme \ */
            {
              grille[num_ligne][num_colonne].somme_horizontale = -1;

              ungetc (c,f);
            }
          }
          else
            if (isdigit(c))    /* case noire de la forme x\ ou x\y */
            {
              grille[num_ligne][num_colonne].coul = NOIRE;
              grille[num_ligne][num_colonne].num = -1;

              ungetc (c,f);
              fscanf (f,"%d",&somme);
              grille[num_ligne][num_colonne].somme_verticale = somme;

              c = fgetc(f);   /* on lit le caractère \ */

              c = fgetc(f);

              if (isdigit(c))  /* case noire de la forme x\y */
              {
                ungetc (c,f);
                fscanf (f,"%d",&somme);
                grille[num_ligne][num_colonne].somme_horizontale = somme;
              }
              else   /* case noire de la forme x\ */
              {
                grille[num_ligne][num_colonne].somme_horizontale = -1;

                ungetc (c,f);
              }
            }

    c = fgetc(f);
  }
  fclose (f);

  //printf ("Nombre de variables trouvées : %d\n",nb_variables);


  /* on crée les contraintes */

  if (nb_lignes < nb_colonnes)
    portee = (int *) malloc (sizeof(int) * nb_colonnes);
  else portee = (int *) malloc (sizeof(int) * nb_lignes);

  for (num_ligne = 0; num_ligne < nb_lignes; num_ligne++)
    for (num_colonne = 0; num_colonne < nb_colonnes; num_colonne++)
      if (grille[num_ligne][num_colonne].coul == NOIRE)
      {
        if (grille[num_ligne][num_colonne].somme_horizontale != -1)
        {
          arite = 0;
          i = num_colonne+1;
          while ((i < nb_colonnes) && (grille[num_ligne][i].coul == BLANCHE))
          {
            portee[arite] = grille[num_ligne][i].num;
            arite++;

            j = i+1;
            while ((j < nb_colonnes) && (grille[num_ligne][j].coul == BLANCHE))
            {
              Contrainte_Difference (csp, grille[num_ligne][i].num,grille[num_ligne][j].num);
              j++;
            }
            i++;
          }

          Contrainte_Somme (csp, portee,arite,grille[num_ligne][num_colonne].somme_horizontale);
        }

        if (grille[num_ligne][num_colonne].somme_verticale != -1)
        {
          arite = 0;
          i = num_ligne+1;
          while ((i < nb_lignes) && (grille[i][num_colonne].coul == BLANCHE))
          {
            portee[arite] = grille[i][num_colonne].num;
            arite++;

            j = i+1;
            while ((j < nb_lignes) && (grille[j][num_colonne].coul == BLANCHE))
            {
              Contrainte_Difference (csp, grille[i][num_colonne].num,grille[j][num_colonne].num);
              j++;
            }
            i++;
          }

          Contrainte_Somme (csp, portee,arite,grille[num_ligne][num_colonne].somme_verticale);
        }
      }


  /* liberation de la mémoire allouée pour la grille et la portée des contraintes */
  free (portee);

  for (num_ligne = 0; num_ligne < nb_lignes; num_ligne++)
    free (grille[num_ligne]);
  free (grille);
  //printf ("\n");
}


/* fonctions à compléter pour remplir vos structures de données */


void Nouvelle_Variable (Csp * csp, int num)
/* fonction permettant la création d'une nouvelle variable ayant pour numéro num */
{
  add_variable(csp, num, nb_valeurs, tab_valeurs);
  //printf ("Nouvelle Variable %d\n",num);
}


void Contrainte_Difference (Csp * csp, int var1, int var2)
/* fonction permettant la création d'une nouvelle contrainte binaire de différence entre les variables var1 et var2*/
{
  int portee[2] = {var1, var2};
  add_contrainte(csp, DIFFERENCE, 0, 2, portee);
  //printf ("Contrainte binaire de difference entre %d et %d\n",var1,var2);
}


void Contrainte_Somme (Csp * csp, int portee [], int arite, int val)
/* fonction permettant la création d'une nouvelle contrainte n-aire de somme portant sur les variables contenant dans le tableau portee de taille arite et dont la valeur est val */
{
  add_contrainte(csp, SOMME, val, arite, portee);
  //printf ("Contrainte n-aire de somme portant sur");
  //for (int i = 0; i < arite; i++)
    //printf (" %d",portee[i]);
  //printf (" et de valeur %d\n",val);
}
