//Written by Will Sumner
//Under Dr. Vinicius Petrucci
#include "cpu_utils.hh"
#include <random> // better than rand()
#include <sched.h>

void _init_affinity() {
    return;
}

cpu_set_t _set_affinity_little(void) {
    cpu_set_t mask;
    CPU_ZERO(&mask);
    SET_AFF_LITTLE
    return mask;
}

cpu_set_t _set_affinity_big(void) {
    cpu_set_t mask;
    CPU_ZERO(&mask);
    SET_AFF_BIG
    return mask;
}

cpu_set_t _set_affinity_all(void) {
    cpu_set_t mask;
    CPU_ZERO(&mask);
    SET_AFF_ALL
    return mask;
}

cpu_set_t _set_affinity_permute(int bigs, int lils, std::mt19937 mt) {
    static std::uniform_int_distribution<int> dist(0,3);

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

    int choice;

    while (bigs > 0) {
        choice  = dist(mt) + 4;
        if (!CPU_ISSET(choice,&mask)) {
            continue;
        }
        CPU_CLR(choice,&mask);
        bigs--;
    }

    while (lils > 0) {
        choice  = dist(mt);
        if (!CPU_ISSET(choice,&mask)) {
            continue;
        }
        CPU_CLR(choice,&mask);
        bigs--;
    }
    return mask;
}
