//Written by Will Sumner
//Under Dr. Vinicius Petrucci
#include "cpu_utils.hh"
#include <random> // better than rand()
#include <sched.h>
#include <string>

void set_affinity_little(cpu_set_t* mask) {
    CPU_ZERO(mask);
    CPU_SET(0,mask);
    CPU_SET(1,mask);
    CPU_SET(2,mask);
    CPU_SET(3,mask);
    sched_setaffinity(0,sizeof(cpu_set_t),mask);
}

void set_affinity_big(cpu_set_t* mask) {
    CPU_ZERO(mask);
    CPU_SET(4,mask);
    CPU_SET(5,mask);
    CPU_SET(6,mask);
    CPU_SET(7,mask);
    sched_setaffinity(0,sizeof(cpu_set_t),mask);
}

void set_affinity_all(cpu_set_t* mask) {
    CPU_ZERO(mask);
    CPU_SET(0,mask);
    CPU_SET(1,mask);
    CPU_SET(2,mask);
    CPU_SET(3,mask);
    CPU_SET(4,mask);
    CPU_SET(5,mask);
    CPU_SET(6,mask);
    CPU_SET(7,mask);
    sched_setaffinity(0,sizeof(cpu_set_t),mask);
}

void set_affinity_permute(cpu_set_t* mask, std::mt19937 rng, int bigs, int lils) {
    static std::uniform_int_distribution<int> dist(0,3);

    int choice;
    CPU_ZERO(mask);

    while (bigs > 0) {
        choice  = dist(rng) + 4;
        if (CPU_ISSET(choice,mask)) {
            continue;
        }
        CPU_SET(choice,mask);
        bigs--;
    }
    while (lils > 0) {
        choice  = dist(rng);
        if (CPU_ISSET(choice,mask)) {
            continue;
        }
        CPU_SET(choice,mask);
        lils--;
    }
    sched_setaffinity(0,sizeof(cpu_set_t),mask);
}

void set_affinity_with_mask(cpu_set_t* mask) {
    // giving PID 0 uses calling process
    sched_setaffinity(0,sizeof(cpu_set_t),mask);
}

void print_mask(cpu_set_t* mask, int cpus) {
    int end = cpus > 0 && cpus < sizeof(cpu_set_t) ? cpus : sizeof(cpu_set_t);
    for (int i = 0; i < sizeof(cpu_set_t) && i < end; i++) {
        if (CPU_ISSET(i,mask))
            printf("1");
        else
            printf("0");
    }
}

cpu_set_t mask_from_str(std::string& s) {
    cpu_set_t mask;
    CPU_ZERO(&mask);
    for (int i = 0; i < s.size(); i++) {
        if (s[i] == '1')
            CPU_SET(i,&mask);
    }
    return mask;
}

unsigned int get_curr_cpu() {
    return sched_getcpu();
}
