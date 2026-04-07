// 2024041 Πασχάλης Κούκος
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#define BUFFER_SIZE 1024
void error(const char *msg);
void print_message(int lines,int digits, int errors);
void line_change(char *buffer,int i,int flag_digits, int *lines, int *digits, int *errors);

int main(int argc, char *argv[]) { // argv[0] είναι το file name.c κι argv[1] πρέπει να είναι το αρχείο .log ( άρα σύνολο 2 )
    //πρέπει argc == 2
    if (argc != 2) {fprintf(stderr,"File not provided\n");exit(1);} // αν δε δώσει αρχείο -> μήνυμα σφάλματος κι έξοδος
    int fd = open(argv[1], O_RDONLY); //το ανοίγω μόνο για ανάγνωση
    if (fd==-1) {error("open"); exit(1);} //σφάλμα ανοίγματος

    char c;
    ssize_t b = read(fd, &c, 1); //διάβασμα ενός χαρακτήρα (αν υπάρχει) για έλεγχο

    if (b == 0) {
        // το αρχείο είναι κενό - δε διαβάστηκε χαρακτήρας
        puts("Empty file");
        close(fd); //κλείσιμο για ασφάλεια
        exit(2); //κενό αρχείο
    }
    if (b == -1) {
        // σφάλμα ανάγνωσης
        error("read");
        close(fd);
        exit(1);
    }
    lseek(fd, 0, SEEK_SET); // επιστροφή στην αρχή του αρχείου

    int lines = 0;int line_errors = 0;int line_with_digits = 0;
    char buffer[BUFFER_SIZE];int flag_digits = 0;int i =0;
    while (b > 0) { //οσο έχει μέσα bytes να διαβάσει
        if (isdigit(c)) {
            flag_digits=1;
        }

        if (i<BUFFER_SIZE-1 && c!='\n')buffer[i++]=c; // αν δεν είναι αλλαγή γραμμής και δεν ξεπερνά το μαξ μέγεθος προσθέτω
        //ώστε να ολοκληρώσει μια γραμμή
        if (c == '\n') { //αλλαγή γραμμής
            line_change(buffer,i,flag_digits,&lines,&line_with_digits,&line_errors);
            //επαναφορά για επόμενη επανάληψη
            flag_digits = 0;
            i=0;
        }
        b = read(fd, &c, 1); //διάβασμα επόμενου χαρακτήρα
    }
    if (i>0) { // για να μη μου ξεφύγει καμία γραμμή - έχει χαρακτήρες στο buffer
        // μπορεί μια γραμμή να μη τελειώνει σε \n αλλά πχ με EOF
        // αν υπάρχουν χαρακτήρες στο buffer τους επεξεργαζόμαστε ως γραμμή
        line_change(buffer,i,flag_digits,&lines,&line_with_digits,&line_errors);
    }
    print_message(lines,line_with_digits,line_errors);
    close(fd);
    return 0; // επιτυχία
}

void error(const char *msg) { //διαχείριση με p error
    perror(msg);
}

void print_message(const int lines,const int digits,const int errors) {
    // εμφάνιση μηνυμάτων στο τέλος
    printf("Total lines: %d\nTotal lines with word 'ERROR': %d\nTotal lines with digits: %d\n",lines, errors, digits);
}

void line_change(char *buffer,int i,int flag_digits, int *lines, int *digits, int *errors) { // συνάρτηση για την αλλαγή γραμμής
    buffer[i]='\0'; // Τέλος, γραμμής - string
    (*lines)++; //αυξάνω γραμμή
    if (flag_digits) {
        (*digits)++; //περιέχει ψηφίο
    }
    if (strstr(buffer, "ERROR")!=NULL) {
        (*errors)++; // το 'ERROR' είναι substring του ολόκληρου string-γραμμής
    }
}