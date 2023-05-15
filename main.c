#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>

#define NUM_THREADS 5
#define MAX_STRLEN 1024
#define MAX_CPUS 8

#ifdef __APPLE__
    #include "demo.c"
    #define PROC_STAT_FILE "proc/stat"
#elif __linux__
    #include "etc/linked_list.c"
    #define PROC_STAT_FILE "/proc/stat"
#endif

bool flag_read = true;
bool flag_analyze = false;
bool flag_print = false;

bool signal_watchdog_read = false;
bool signal_watchdog_analyze = false;
bool signal_watchdog_print = false;
bool signal_watchdog_log = false;

double usage = 0.0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t cond_read = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_analyze = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_printer = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_watchdog = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_logger = PTHREAD_COND_INITIALIZER;

struct LinkedList *List;

pthread_t threads[NUM_THREADS];

double calculate_cpu_usage(long long unsigned totalDiff, long long unsigned idleDiff) {
    return totalDiff != 0 ? ((double)(totalDiff - idleDiff) / totalDiff * 100.0) : 0;
}

void *thread_reader(void *arg) {
    char buffer[MAX_STRLEN];
    FILE *file;

    while (true) {
        pthread_mutex_lock(&mutex);
        while (!flag_read) pthread_cond_wait(&cond_analyze, &mutex);
        flag_read = false;

        file = fopen(PROC_STAT_FILE, "r");
        if (file == NULL) {
            printf("Error opening file\n");
            pthread_exit(NULL);
        }

        if (List) freeLinkedList(List);
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

        sleep(1);

        flag_analyze = true;
        signal_watchdog_read = true;
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
        while (!flag_analyze) pthread_cond_wait(&cond_read, &mutex);
        flag_analyze = false;

        stats = getAtPosition(List, 0)->data;
        
        currIdle = stats.idle + stats.iowait;
        currNonIdle = stats.user + stats.nice + stats.system + stats.irq + stats.softirq;
        currTotal = currIdle + currNonIdle;

        totalDiff = currTotal - prevTotal;
        idleDiff = currIdle - prevIdle;

        if (idleDiff != totalDiff) usage = (usage + calculate_cpu_usage(totalDiff, idleDiff)) / 2;

        prevIdle = currIdle;
        prevNonIdle = currNonIdle;
        prevTotal = currTotal;

        #ifdef __APPLE__
            alternate_data(List);
        #endif 

        flag_read = true;
        signal_watchdog_analyze = true;
        pthread_cond_signal(&cond_analyze);
        pthread_mutex_unlock(&mutex);
    }

    pthread_exit(NULL);
}

void *thread_printer(void *arg) {
    while (true) {
        printf("CPU Usage: %.2f%%\n", usage);
        sleep(1);

        signal_watchdog_print = true;
        flag_print = true;
    }

    pthread_exit(NULL);
}

void *thread_watchdog(void *arg) {
    while (true) {
        sleep(2);
        if (!signal_watchdog_read) {
            printf("Error: Thread Reader did not send message within 2 seconds.\n");
            break;
        }
        if (!signal_watchdog_analyze) {
            printf("Error: Thread Analyzer did not send message within 2 seconds.\n");
            break;
        }
        if (!signal_watchdog_print) {
            printf("Error: Thread Printer did not send message within 2 seconds.\n");
            break;
        }
        if (!signal_watchdog_log) {
            printf("Error: Thread Logger did not send message within 2 seconds.\n");
            break;
        }

        signal_watchdog_read = false;
        signal_watchdog_analyze = false;
        signal_watchdog_print = false;
        signal_watchdog_log = false;
    }

    exit(EXIT_FAILURE);
    pthread_exit(NULL);
}

void *thread_logger(void *arg) {
    FILE* file = NULL;
    time_t currentTime;
    struct tm* timeInfo = NULL;

    file = fopen("log.txt", "w");
    if (file == NULL) {
        printf("Error opening log file.\n");
        pthread_exit(NULL);
    }
    fclose(file);

    while (true) {
        currentTime = time(NULL);
        timeInfo = localtime(&currentTime);

        char timestamp[20];
        strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", timeInfo);

        file = fopen("log.txt", "a");
        if (file == NULL) {
            printf("Error opening log file.\n");
            pthread_exit(NULL);
        }

        while (!flag_read);
        fprintf(file, "[%s] %s\n", timestamp, " - Reader signal");
        signal_watchdog_log = true;

        while (!flag_analyze);
        fprintf(file, "[%s] %s\n", timestamp, " - Analyzer signal");
        signal_watchdog_log = true;

        while (!flag_print);
        flag_print = false;
        fprintf(file, "[%s] %s\n", timestamp, " - Printer signal");
        signal_watchdog_log = true;

        fclose(file);
    }

    pthread_exit(NULL);
}

void sigterm_handler(int s) {
    printf("Received SIGTERM signal. Terminating...\n");
    
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

    freeLinkedList(List);
    free(List);

    exit(0);
}

int main() {
    signal(SIGTERM, sigterm_handler);

    srand(time(NULL));

    List = NULL;

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

    while (true);

    // for (int i = 0; i < NUM_THREADS; i++) {
    //     if (pthread_join(threads[i], NULL) != 0) {
    //         printf("Error joining thread %d!\n", i);
    //         exit(-1);
    //     }
    // }

    // pthread_mutex_destroy(&mutex);

    // pthread_cond_destroy(&cond_read);
    // pthread_cond_destroy(&cond_analyze);
    // pthread_cond_destroy(&cond_printer);
    // pthread_cond_destroy(&cond_watchdog);
    // pthread_cond_destroy(&cond_logger);

    // freeLinkedList(List);
    // free(List);

    return 0;
}
