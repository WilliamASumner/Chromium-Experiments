//Written by Will Sumner
//Under Dr. Vinicius Petrucci
/* Macro to allow affinity changes within a scope, only needs to be used once */
#ifndef CPU_UTILS_H
#define CPU_UTILS_H
#include <sched.h>
/* New Affinity scope */
#define INIT_AFFINITY  \
	cpu_set_t mask;   \
	CPU_ZERO(&mask); 
/* Macro to set affinity to all LITTLE cores */
#define SET_AFF_LITTLE    \
	CPU_ZERO(&mask);  \
	CPU_SET(0,&mask); \
	CPU_SET(1,&mask); \
	CPU_SET(2,&mask); \
	CPU_SET(3,&mask); \
	sched_setaffinity(0,sizeof(mask),&mask);

/* Macro to set affinity to all big cores */
#define SET_AFF_BIG       \
	CPU_ZERO(&mask);  \
	CPU_SET(4,&mask); \
	CPU_SET(5,&mask); \
	CPU_SET(6,&mask); \
	CPU_SET(7,&mask); \
	sched_setaffinity(0,sizeof(mask),&mask);

/* Macro to set affinity to all cores */
#define SET_AFF_ALL       \
	CPU_ZERO(&mask);  \
	CPU_SET(0,&mask); \
	CPU_SET(1,&mask); \
	CPU_SET(2,&mask); \
	CPU_SET(3,&mask); \
	CPU_SET(4,&mask); \
	CPU_SET(5,&mask); \
	CPU_SET(6,&mask); \
	CPU_SET(7,&mask); \
	sched_setaffinity(0,sizeof(mask),&mask);

void _set_affinity_little(void) {
	INIT_AFFINITY
	SET_AFF_LITTLE
}

void _set_affinity_big(void) {
	INIT_AFFINITY
	SET_AFF_BIG
}

void _set_affinity_all(void) {
	INIT_AFFINITY
	SET_AFF_ALL
}
#endif
