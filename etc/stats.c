#include "stats.h"

// Function to create a CPU_Stats struct pointer, allocate memory and initialize the fields
struct CPU_Stats* createCPUStats(char* name, unsigned long long user, unsigned long long nice,
                                unsigned long long system, unsigned long long idle,
                                unsigned long long iowait, unsigned long long irq,
                                unsigned long long softirq, unsigned long long steal,
                                unsigned long long guest, unsigned long long guest_nice) {
    struct CPU_Stats* stats = (struct CPU_Stats*)malloc(sizeof(struct CPU_Stats));
    if (stats == NULL) {
        return NULL; // Failed to allocate memory
    }

    // Allocate memory for the name and copy the provided string
    stats->name = (char*)malloc(strlen(name) + 1);
    if (stats->name == NULL) {
        free(stats);
        return NULL; // Failed to allocate memory
    }
    strcpy(stats->name, name);

    // Set the remaining fields
    stats->user = user;
    stats->nice = nice;
    stats->system = system;
    stats->idle = idle;
    stats->iowait = iowait;
    stats->irq = irq;
    stats->softirq = softirq;
    stats->steal = steal;
    stats->guest = guest;
    stats->guest_nice = guest_nice;

    return stats;
}

// Function to delete a CPU_Struct and free the memory
void deleteCPUStats(struct CPU_Stats *Stats) {
    if (Stats != NULL) {
        free(Stats->name);
        free(Stats);
    }
}