#include <stdio.h>
#include <time.h>

#include "etc/linked_list.c"

#define PROC_STAT_FILE "proc/stat"

// This function alternates data in '/proc/stat' file as the program is run on Mac device
// On Mac the program can be used only for demo
void alternate_data(struct LinkedList *List) {
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