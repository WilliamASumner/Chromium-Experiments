//Written by Will Sumner
//Under Dr. Vinicius Petrucci
#include "cpu_utils.hh"
#include <random> // better than rand()
#include <sched.h>

void _init_affinity() {
    return;
}

cpu_set_t set_affinity_little(void) {
    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(0,&mask);
    CPU_SET(1,&mask);
    CPU_SET(2,&mask);
    CPU_SET(3,&mask);
    sched_setaffinity(0,sizeof(mask),&mask);
    return mask;
}

cpu_set_t set_affinity_big(void) {
    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(4,&mask);
    CPU_SET(5,&mask);
    CPU_SET(6,&mask);
    CPU_SET(7,&mask);
    return mask;
}

cpu_set_t set_affinity_all(void) {
    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(0,&mask);
    CPU_SET(1,&mask);
    CPU_SET(2,&mask);
    CPU_SET(3,&mask);
    CPU_SET(4,&mask);
    CPU_SET(5,&mask);
    CPU_SET(6,&mask);
    CPU_SET(7,&mask);
    return mask;
}

cpu_set_t get_affinity_permute(int bigs, int lils, std::mt19937 mt) {
    static std::uniform_int_distribution<int> dist(0,3);

    int choice;
    cpu_set_t mask;
    CPU_ZERO(&mask);

    while (bigs > 0) {
        choice  = dist(mt) + 4;
        if (CPU_ISSET(choice,&mask)) {
            continue;
        }
        CPU_SET(choice,&mask);
        bigs--;
    }

    while (lils > 0) {
        choice  = dist(mt);
        if (CPU_ISSET(choice,&mask)) {
            continue;
        }
        CPU_SET(choice,&mask);
        lils--;
    }
    return mask;
}

void set_affinity_with_mask(cpu_set_t mask) {
    sched_setaffinity(0,sizeof(mask),&mask);
}

unsigned int get_curr_cpu() {
    return sched_getcpu();
}
