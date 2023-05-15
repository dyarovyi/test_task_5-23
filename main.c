#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <time.h>

#include "etc/linked_list.c"

#define NUM_THREADS 6
#define MAX_STRLEN 1024
#define MAX_CPUS 16

#ifdef __APPLE__
    #define PROC_STAT_FILE "proc/stat"
#elif __linux__
    #define PROC_STAT_FILE "/proc/stat"
#endif

bool launched = false;

// int inc = 10;
// bool odd = false;
double usage = 0.0;

pthread_mutex_t mutex;

pthread_cond_t cond_read;
pthread_cond_t cond_analyze;
pthread_cond_t cond_printer;
pthread_cond_t cond_watchdog;
pthread_cond_t cond_logger;
pthread_cond_t cond_sigterm;

struct LinkedList *List;
struct LinkedList *PreviousList;

// This function alternates data in '/proc/stat' file as the program is run on Mac device
// On Mac the program can be used only for demo
void alternate_data() {
    FILE *file = fopen(PROC_STAT_FILE, "w");
    if (file == NULL) {
        printf("Error opening file\n");
        pthread_exit(NULL);
    }

    if (List->head == NULL) {
        printf("Linked list is empty.\n");
        return;
    }
    struct Node *CurrentNode = List->head;

    // Generate a random offset between -10 and 10
    int inc = rand() % 21 - 10;

    struct CPU_Stats stats;
    stats = getAtPosition(List, 0)->data;
    fprintf(file, "%s %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu\n",
            stats.name, stats.user+(abs(inc)*8 < stats.user? inc*8 : stats.user),
            stats.nice+(abs(inc)*8 < stats.nice? inc*8 : stats.nice), 
            stats.system+(abs(inc)*8 < stats.system? inc*8 : stats.system), 
            stats.idle+(abs(inc)*8 < stats.idle? inc*8 : stats.idle), 
            stats.iowait+(abs(inc)*8 < stats.iowait? inc*8 : stats.iowait), 
            stats.irq, 
            stats.softirq+(abs(inc)*8 < stats.softirq? inc*8 : stats.softirq), 
            stats.steal, stats.guest, 
            stats.guest_nice);
    for (size_t i = 1; i < List->size-1; i++) {
        stats = getAtPosition(List, i)->data;
        fprintf(file, "%s %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu\n",
            stats.name, stats.user+(abs(inc) < stats.user? inc : stats.user),
            stats.nice+(abs(inc) < stats.nice? inc : stats.nice), 
            stats.system+(abs(inc) < stats.system? inc : stats.system), 
            stats.idle+(abs(inc) < stats.idle? inc : stats.idle), 
            stats.iowait+(abs(inc) < stats.iowait? inc : stats.iowait), 
            stats.irq, 
            stats.softirq+(abs(inc) < stats.softirq? inc : stats.softirq), 
            stats.steal, stats.guest, 
            stats.guest_nice);
    }
    stats = getAtPosition(List, List->size-1)->data;
    fprintf(file, "%s %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu\n",
            stats.name, stats.user, stats.nice, stats.system, stats.idle, stats.iowait, stats.irq, stats.softirq, stats.steal, stats.guest, stats.guest_nice);

    fclose(file);
}

void *thread_reader(void *arg) {
    printf("Thread reader\n");

    char buffer[MAX_STRLEN];
    FILE *file;

    while (true) {
        pthread_mutex_lock(&mutex);
        if (launched) {
            //pthread_cond_wait(&cond_printer, &mutex);
            pthread_cond_wait(&cond_analyze, &mutex);
            printf("Cond printer signal received\n");
            sleep(1);
        } else launched = true;

        file = fopen(PROC_STAT_FILE, "r");
        if (file == NULL) {
            printf("Error opening file\n");
            pthread_exit(NULL);
        }

        if (List) {
            PreviousList = copyLinkedList(List);
            freeLinkedList(List);
        }
        List = (struct LinkedList*)malloc(sizeof(struct LinkedList));
        initLinkedList(List);

        char line[MAX_STRLEN];
        while (fgets(line, sizeof(line), file)) {
            struct CPU_Stats NewStats;
            char name[16];
            sscanf(line, "%s %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu",
                name,
                &(NewStats.user),
                &(NewStats.nice),
                &(NewStats.system),
                &(NewStats.idle),
                &(NewStats.iowait),
                &(NewStats.irq),
                &(NewStats.softirq),
                &(NewStats.steal),
                &(NewStats.guest),
                &(NewStats.guest_nice));

            NewStats.name = strdup(name);
            pushBack(List, NewStats);
        }

        fclose(file);

        #ifdef __APPLE__
            alternate_data();
        #endif 

        pthread_cond_signal(&cond_read);
        pthread_mutex_unlock(&mutex);
    }

    pthread_exit(NULL);
}

void *thread_analyzer(void *arg) {
    struct CPU_Stats stats; 

    unsigned long long prevIdle = 0;
    unsigned long long currIdle = 0;

    unsigned long long prevNonIdle = 0;
    unsigned long long currNonIdle = 0;

    unsigned long long prevTotal = 0;
    unsigned long long currTotal = 0;

    unsigned long long totalDiff = 0;
    unsigned long long idleDiff = 0;

    while (true) {
        pthread_mutex_lock(&mutex);
        
        pthread_cond_wait(&cond_read, &mutex);

        printf("Cond read signal received\n");

        stats = getAtPosition(List, 0)->data;
        
        currIdle = stats.idle + stats.iowait;
        currNonIdle = stats.user + stats.nice + stats.system + stats.irq + stats.softirq;
        currTotal = currIdle + currNonIdle;

        totalDiff = currTotal - prevTotal;
        idleDiff = currIdle - prevIdle;

        usage = (usage + ((double)(totalDiff - idleDiff) / totalDiff * 100.0)) / 2;

        prevIdle = currIdle;
        prevNonIdle = currNonIdle;
        prevTotal = currTotal;

        pthread_cond_signal(&cond_analyze);
        pthread_mutex_unlock(&mutex);
    }

    pthread_exit(NULL);
}

void *thread_printer(void *arg) {
    while (true) {
        printf("Cond analyze signal recieved\n");
        printf("CPU Usage: %.2f%%\n", usage);

        if (List) printLinkedList(List);

        sleep(2);
    }

    pthread_exit(NULL);
}

void *thread_watchdog(void *arg) {
    printf("Thread watchdog\n");
    pthread_exit(NULL);
}

void *thread_logger(void *arg) {
    printf("Thread logger\n");
    pthread_exit(NULL);
}

void *thread_sigterm_handler(void *arg) {
    printf("Thread handler\n");
    pthread_exit(NULL);
}

int main() {
    srand(time(NULL));
    
    pthread_mutex_init(&mutex, NULL);

    pthread_cond_init(&cond_read, NULL);
    pthread_cond_init(&cond_analyze, NULL);
    pthread_cond_init(&cond_printer, NULL);
    pthread_cond_init(&cond_watchdog, NULL);
    pthread_cond_init(&cond_logger, NULL);
    pthread_cond_init(&cond_sigterm, NULL);

    pthread_t threads[NUM_THREADS];
    List = NULL;
    PreviousList = NULL;

    if (pthread_create(&threads[0], NULL, thread_reader, NULL) != 0) {
        printf("Error creating thread reader!\n");
        exit(-1);
    }

    if (pthread_create(&threads[1], NULL, thread_analyzer, NULL) != 0) {
        printf("Error creating thread analyzer!\n");
        exit(-1);
    }

    if (pthread_create(&threads[2], NULL, thread_printer, NULL) != 0) {
        printf("Error creating thread printer!\n");
        exit(-1);
    }

    if (pthread_create(&threads[3], NULL, thread_watchdog, NULL) != 0) {
        printf("Error creating thread watchdog!\n");
        exit(-1);
    }

    if (pthread_create(&threads[4], NULL, thread_logger, NULL) != 0) {
        printf("Error creating thread logger!\n");
        exit(-1);
    }

    if (pthread_create(&threads[5], NULL, thread_sigterm_handler, NULL) != 0) {
        printf("Error creating thread sigterm handler!\n");
        exit(-1);
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        if (pthread_join(threads[i], NULL) != 0) {
            printf("Error joining thread %d!\n", i);
            exit(-1);
        }
    }

    pthread_mutex_destroy(&mutex);

    pthread_cond_destroy(&cond_read);
    pthread_cond_destroy(&cond_analyze);
    pthread_cond_destroy(&cond_printer);
    pthread_cond_destroy(&cond_watchdog);
    pthread_cond_destroy(&cond_logger);
    pthread_cond_destroy(&cond_sigterm);
    return 0;
}
