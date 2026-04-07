// 2024041 Πασχάλης Κούκος
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <ctype.h>
#define FILESIZE 256
#define BUFFER_SIZE 1024

typedef struct {
    char filename[FILESIZE];   // ποιο αρχείο θα αναλύσει
    int lines;            // πόσες γραμμές
    int errors;           // πόσα errors
    int digits;           //πόσες γραμμές περιέχουν digit
} file_thread;

void line_change(char *buffer,int i,int flag_digits, int *lines, int *digits, int *errors);
void error(const char *msg);
void* analyze(void* arg);
void print_each_log_stats(file_thread *file,int *lines,int *errors,int *digits);

int main(int argc, char *argv[]) {
    if (argc < 2) {fprintf(stderr,"File not provided\n");exit(1);} //σφάλμα, κανένα όρισμα
    file_thread file_threads[argc-1]; //πίνακας με τα αρχεία μεγέθους ίσου με τα ορίσματα που μας έδωσε, δηλαδή τα αρχεία
    pthread_t thread[argc-1]; // πίνακας με τα threads μας

    for (int i=0; i<argc-1; i++) {
        strcpy(file_threads[i].filename,argv[i+1]); // βάζω το όνομα του αρχείου
        file_threads[i].digits=0; file_threads[i].errors=0; file_threads[i].lines=0; //αρχικοποίηση σε 0
        if (pthread_create(&thread[i], NULL, analyze, &file_threads[i]) != 0) { //δημιουργία thread για κάθε αρχείο
            //έλεγχος
            error("pthread_create");
            exit(1);
        }
    }

    for (int i = 0; i < argc - 1; i++) {
        if (pthread_join(thread[i], NULL) != 0) { //για κάθε αρχείο κάνει join, ολοκληρώνεται η δουλειά και συνεχίζει με το επόμενο
            //Έλεγχος
            error("pthread_join");
            exit(1);
        }
    }

    int total_lines = 0, total_errors = 0, total_digits = 0; //μετρητές για σύνολο
    for (int i = 0; i < argc - 1; i++) {
        print_each_log_stats(&file_threads[i],&total_lines,&total_errors,&total_digits); //εκτυπώνει stats για 1 αρχείο
        //και προσθέτει για το σύνολο
    }
    //εκτύπωση συνολικών στατιστικών
    printf("    TOTAL LINES: %d\n   TOTAL ERRORS: %d\n   TOTAL DIGIT-LINES: %d\n",total_lines,total_errors,total_digits);
    return 0;
}

void* analyze(void* arg) { //συνάρτηση που αναλύει κάθε αρχείο
    file_thread *thread = (file_thread *)arg; //cast
    int fd = open(thread->filename,O_RDONLY); //το ανοίγω μόνο για ανάγνωση
    if (fd==-1) {error("open"); pthread_exit(NULL);} //σφάλμα ανοίγματος

    char c;
    ssize_t b = read(fd, &c, 1); //διάβασμα ενός χαρακτήρα (αν υπάρχει) για έλεγχο

    if (b == 0) {
        // το αρχείο είναι κενό - δε διαβάστηκε χαρακτήρας
        printf("File %s is empty\n", thread->filename);
        close(fd); //κλείσιμο για ασφάλεια
        pthread_exit(NULL);
    }
    if (b == -1) {
        // σφάλμα ανάγνωσης
        error("read");
        close(fd);
        pthread_exit(NULL);
    }
    lseek(fd, 0, SEEK_SET); // επιστροφή στην αρχή του αρχείου

    int lines = 0;int line_errors = 0;int line_with_digits = 0;
    int flag_digits = 0;int i =0;

    int bufs=BUFFER_SIZE; //για να επεκτείνω μέγεθος δυναμικά
    char *buffer = malloc(BUFFER_SIZE); // δυναμικός πίνακας
    if (!buffer) { error("malloc"); close(fd); pthread_exit(NULL); } //έλεγχος για μη επιτυχή malloc
    while (b > 0) { //οσο έχει μέσα bytes να διαβάσει
        if (isdigit(c)) flag_digits = 1; // αν ο χαρακτήρας είναι ψηφίο

        if (i>=bufs-1) { // αν φτάσει στο όριο
            bufs*=2; //διπλασιασμός προκαθορισμένου μεγέθους
            char *temp = realloc(buffer,bufs); // μεγαλύτερο μέγεθος με realloc σε temp για ασφάλεια
            if (!temp) { error("realloc"); free(buffer); close(fd); pthread_exit(NULL); } //έλεγχος για realloc
            buffer = temp; //θέτω τον πίνακά μας ίσο με το temp - η realloc διατηρεί τα προηγούμενα δεδομένα στον καινούργιο-μεγαλύτερο buffer
        }
        if (c != '\n') { //αν δεν είναι αλλαγή γραμμής βάζω χαρακτήρα στο buffer
            buffer[i++]=c;
        }

        if (c == '\n') { //αλλαγή γραμμής
            line_change(buffer,i,flag_digits,&lines,&line_with_digits,&line_errors);
            //επαναφορά για επόμενη επανάληψη
            flag_digits = 0;
            i=0;
            bufs=BUFFER_SIZE;
        }
        b = read(fd, &c, 1); //διάβασμα επόμενου χαρακτήρα
    }
    if (i>0) { // για να μη μου ξεφύγει καμία γραμμή - έχει χαρακτήρες στο buffer
        // μπορεί μια γραμμή να μη τελειώνει σε \n αλλά πχ με EOF
        // αν υπάρχουν χαρακτήρες στο buffer τους επεξεργαζόμαστε ως γραμμή
        line_change(buffer,i,flag_digits,&lines,&line_with_digits,&line_errors);
    }
    free(buffer); // ελευθέρωση για να μην έχω memory leaks
    close(fd); //κλείσιμο αρχείου
    // θέτω τα δεδομένα στο τρέχων αρχείο μας μέσω του struct
    thread->lines=lines;
    thread->errors=line_errors;
    thread->digits=line_with_digits;
    pthread_exit(NULL); // Τέλος
}

void error(const char *msg) { //διαχείριση με p error
    perror(msg);
}

void line_change(char *buffer,const int i,const int flag_digits, int *lines, int *digits, int *errors) { // συνάρτηση για την αλλαγή γραμμής
    buffer[i]='\0'; // Tέλος γραμμής - string
    (*lines)++; //αυξάνω γραμμή
    if (flag_digits) {
        (*digits)++; //περιέχει ψηφίο η γραμμή
    }
    if (strstr(buffer, "ERROR")!=NULL) {
        (*errors)++; // το 'ERROR' είναι substring του ολόκληρου string-γραμμής
    }
}

void print_each_log_stats(file_thread *file,int *lines,int *errors,int *digits) {
    //συνάρτηση που εκτυπώνει για ένα αρχείο τα αποτελέσματα και παράλληλα
    //αυξάνει τους συνολικούς μετρητές με τα τρέχοντα αποτελέσματα του αρχείου για τα συνολικά στατιστικά στο τέλος
    printf("File: %s | Lines: %d | Errors: %d | Lines with digit: %d\n",
        file->filename, file->lines, file->errors, file->digits);
    //αύξηση μετρητών
    (*lines) += file->lines;
    (*errors) += file->errors;
    (*digits) += file->digits;
}
