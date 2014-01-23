#!/bin/bash
# Tests : Effectue une comparaison entre résultats attendus et obtenus

test="TestFact_Jeu"
sa="Sortie_attendue"
so="Sortie_obtenue"

mi="ModInterm_"
nbmi=2 
pgcdf="PGCDFinal"

pi="PInterm_"
pf="PFinal"
nbpi=1

i=1
nbJeux=4

for ((i=1; $nbJeux + 1 -$i ; i++ ))
do
	echo ""
	echo "#### Test courant : Jeu $i ####"

	#Convertir les fichiers clairs en GMP
	#	./convertGMP $test$i/$sa/$pf
	#	./convertGMP $test$i/$sa/$pgcdf

	#Comparatif produit final 
	if diff $test$i/$sa/$pf $test$i/$so/$pf > /dev/null ; then
		echo "$test$i $pf PASSED" 
	else
		echo "$test$i $pf NOT PASSED" 
	fi
	
	#Comparatif PGCD final 
	if diff $test$i/$sa/$pgcdf $test$i/$so/$pgcdf > /dev/null ; then
		echo "$test$i $pgcdf PASSED" 
	else
		echo "$test$i $pgcdf NOT PASSED" 
	fi

	# Comparatif produits intermédiaires
	for ((j=1; $nbpi + 1 -$j ; j++ ))
	do
	#	./convertGMP $test$i/$sa/$pi$j

		if diff $test$i/$sa/$pi$j $test$i/$so/$pi$j > /dev/null ; then
			echo "$test$i $pi$j PASSED" 
		else
			echo "$test$i $pi$j NOT PASSED" 
		fi
	done 

	# Comparatif des modulos intermédiares
	for ((j=1; $nbmi + 1 -$j ; j++ ))
	do

	#	./convertGMP $test$i/$sa/$mi$j

		if diff $test$i/$sa/$mi$j $test$i/$so/$mi$j > /dev/null ; then
			echo "$test$i $mi$j PASSED" 
		else
			echo "$test$i $mi$j NOT PASSED" 
		fi
	done 

done
