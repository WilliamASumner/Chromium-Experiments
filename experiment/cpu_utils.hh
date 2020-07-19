//Written by Will Sumner
//Under Dr. Vinicius Petrucci
/* Macro to allow affinity changes within a scope, only needs to be used once */

#ifndef CPU_UTILS_H
#define CPU_UTILS_H
#include <sched.h>
#include <random>

void set_affinity_little(cpu_set_t* mask);

void set_affinity_big(cpu_set_t* mask);

void set_affinity_all(cpu_set_t* mask);

void set_affinity_permute(cpu_set_t* mask, std::mt19937 rng, int bigs, int lils);

void set_affinity_with_mask(cpu_set_t* mask);

void print_mask(cpu_set_t* mask, int cpus);

unsigned int get_curr_cpu();

#endif
