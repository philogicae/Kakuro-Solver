#!/bin/bash

stdout="$4"
path="../grilles"
pathresult="../solutions"
mkdir -p $pathresult

if [ "$1" = "help" ]; then # Affichage de la notice
	cat notice_easy-solver.txt
	exit 0
elif [ "$1" = "list" ]; then # Affichage du répertoire des grilles
	echo "Grilles disponibles :"
	i=0
	for grid in $(ls $path); do
		i=`expr $i + 1`
		echo "$i) $grid"
	done
	exit 0
elif [ $# -lt 3 ] || [ $# -gt 4 ]; then
	echo "3 arguments demandés : ./easy-solver.sh numgrid -algo =heuristic"
	echo "Plus de détails dans la notice : ./easy-solver.sh help"
	exit 1
fi

function resolution {
	echo -n "$4$2$3__$1 : "
	solution=`./solver $path/$1 $2 $3`
	echo "$solution" > $pathresult/$4$2$3__$1
	if [ "$stdout" = "print" ]; then
		echo "$solution"
	else
		echo "OK"
	fi
}

function heuristique {
	heuristics=("=def" "=dod" "=ddod" "=cf" "=dcf" "=fs")
	for heuristic in "${heuristics[@]}"; do
    	if [ "$3" = "$heuristic" ] || [ "$3" = "=all" ]; then
			resolution $1 $2 $heuristic $4
		fi
	done
}

function algorithme {
	algos=("-bt" "-fc" "-fcfa")
	for algo in "${algos[@]}"; do
    	if [ "$2" = "$algo" ] || [ "$2" = "-all" ]; then
			heuristique $1 $algo $3 $4
		fi
	done
}

make
i=0
for grid in $(ls $path); do
	i=`expr $i + 1`
	if [ "$1" = "$i" ] || [ "$1" = "all" ]; then
		algorithme $grid $2 $3 $i
	fi
done
exit 0