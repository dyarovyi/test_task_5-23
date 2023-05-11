#include <stdlib.h>
#include <string.h>

struct CPU_Stats {
    char *name;
    unsigned long long user;
    unsigned long long nice;
    unsigned long long system;
    unsigned long long idle;
    unsigned long long iowait;
    unsigned long long irq;
    unsigned long long softirq;
    unsigned long long steal;
    unsigned long long guest;
    unsigned long long guest_nice;
};

struct CPU_Stats* createCPUStats(char *name, unsigned long long user, unsigned long long nice,
                                unsigned long long system, unsigned long long idle,
                                unsigned long long iowait, unsigned long long irq,
                                unsigned long long softirq, unsigned long long steal,
                                unsigned long long guest, unsigned long long guest_nice);

void deleteCPUStats(struct CPU_Stats *Stats);