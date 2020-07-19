//Written by Will Sumner
//Under Dr. Vinicius Petrucci
/* Macro to allow affinity changes within a scope, only needs to be used once */

#ifndef CPU_UTILS_H
#define CPU_UTILS_H

void _init_affinity();

cpu_set_t set_affinity_little(void);

cpu_set_t set_affinity_big(void);

cpu_set_t set_affinity_all(void);

cpu_set_t get_affinity_permute(int bigs, int lils, std::mt19937 mt);

void set_affinity_with_mask(cpu_set_t mask);

unsigned int get_curr_cpu();

#endif
