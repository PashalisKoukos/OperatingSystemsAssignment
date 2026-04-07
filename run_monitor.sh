#!/bin/bash

DIR="$1" #δέχεται ως όρισμα έναν φάκελο logs (π.χ. raw/) (1)

if [ $# -ne 1 ] # αν το πλήθος των ορισμάτων != 1
then
	echo "Illegal number of arguments.Required: 1 , Given: $#"
	exit 1
fi # ελέγχει ότι έχει δοθεί 1 μόνο όρισμα , όπως πρέπει (2)

if [ ! -d "$DIR" ] 
then 
	echo "Directory '$1' does not exist"
	exit 1
fi #ελέγχει ότι ο κατάλογος που δόθηκε υπάρχει (3)

#εμφανίζει το όνομα του script και τον συνολικό αριθμό ορισμάτων (4)
echo "name of script is: '$0'" 
echo "number of arguments provided are: $#"

# παρακάτω ακολουθεί υλοποίηση του (5) κι του (6)
# καλεί το πρόγραμμα analyze_log για κάθε αρχείο στο directory
# συγκεντρώνει όλα τα αποτελέσματα σε ένα νέο αρχείο: reports/full_report.txt

FILE_TO_SAVE="reports/full_report.txt" # αρχείο που θα αποθηκεύσω το αποτέλεσμα
touch "$FILE_TO_SAVE" #δημιουργία αρχείου

echo -n "Do you want to start the process?"
read answer

while [ "$answer" = "y" ]
do
	echo "" > "$FILE_TO_SAVE"   # Δημιουργία / καθαρισμός report απο προηγούμενα πριν την επεξεργασία
	for file in "$DIR"/*.log # για κάθε αρχείο .log στον φάκελο που μας έδωσε
	do

    		# Categorization με case
    		case "$file" in
        		*system*)
            			category="SYSTEM"
            			;;
        		*network*)
            			category="NETWORK"
            			;;
        		*security*)
            			category="SECURITY"
            			;;
        		*)
        			category="$file" #ολόκληρο το όνομα πχ raw/one_report.log
            			;;
    		esac

    		# Κλήση analyze_log και ασφαλής ανάγνωση/διάσπαση γραμμών με IFS(5)
    		./analyze_log "$file" | while IFS= read -r string_line
    		do
        		echo "[$category] > $string_line" >> "$FILE_TO_SAVE" # κάνω append την γραμμή στο reports/full_report.txt (6)
    		done
	done
	echo "Report completed successfully at: $FILE_TO_SAVE" #επιτυχία
	echo -n "Do you want to restart the process?"
	read answer
done
