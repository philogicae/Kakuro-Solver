#ifndef TRI_H
#define TRI_H

void permuterDouble(double *t, int a, int b);
void permuterInt(int *t, int a, int b);

int filsG(int i);
int filsD(int i);
void entasser(int n, double *valeurs, int *indices, int i);
void tri_Par_Tas(int n, double *valeurs, int *indices);

int partitionner(int first, int last, double *valeurs, int *indices);
void quick_Sort(int first, int last, double *valeurs, int *indices);
void tri_Rapide(int n, double *valeurs, int *indices);

#endif
