#include <stdio.h>
#include <stdlib.h>
#include "tri.h"

void permuterDouble(double *t, int a, int b){ double temp = t[a];	t[a] = t[b]; t[b] = temp; }
void permuterInt(int *t, int a, int b){ int temp = t[a];	t[a] = t[b]; t[b] = temp; }

/* TRI PAR TAS */
int filsG(int i){ return 2*i+1; }
int filsD(int i){ return 2*i+2; }

void entasser(int n, double *valeurs, int *indices, int i){
	int iMax = i;
	if(filsG(i) < n && valeurs[filsG(i)] > valeurs[iMax])
		iMax = filsG(i);
	if(filsD(i) < n && valeurs[filsD(i)] > valeurs[iMax])
		iMax = filsD(i);
	if(iMax != i){
		permuterDouble(valeurs, i, iMax);
		permuterInt(indices, i, iMax);
		entasser(n, valeurs, indices, iMax);
	}
}

void tri_Par_Tas(int n, double *valeurs, int *indices){
	for(int i=n/2-1; i>=0; i--)
		entasser(n, valeurs, indices, i);
	for(int i=n-1; i>=1; i--){
		permuterDouble(valeurs, 0, i);
		permuterInt(indices, 0, i);
		entasser(i, valeurs, indices, 0);
	}
}

/* TRI RAPIDE */
int partitionner(int first, int last, double *valeurs, int *indices){
    int pivot = valeurs[first], i = first-1, j = last+1;
    while(1){
        do j--; while (valeurs[j] > pivot);
        do i++; while (valeurs[i] < pivot);
        if(i < j){
					permuterDouble(valeurs, i, j);
					permuterInt(indices, i, j);
				} else return j;
    }
}

void quick_Sort(int first, int last, double *valeurs, int *indices){
    if(first < last){
        int part = partitionner(first, last, valeurs, indices);
        quick_Sort(first, part, valeurs, indices);
        quick_Sort(part+1, last, valeurs, indices);
    }
}

void tri_Rapide(int n, double *valeurs, int *indices){ quick_Sort(0, n-1, valeurs, indices); }
