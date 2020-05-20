#include <stdio.h> // printf
#include <stdlib.h> // getenv
#include <iostream>

#include <unistd.h> // tid
#include <sys/syscall.h> // get tid
#include <time.h> 

#include"NanoLog/NanoLog.hpp" // fast logger
#include"cpu_utils.hh" // affinity functions
#include "experimenter.hh"

struct config_t {
    int bigs;
    int lils;
};

struct config_t experiment_config;
thread_local struct timespec timeStart,timeEnd;
const unsigned int ns_to_ms = 1000000;

void experiment_set_config(const char* config) {
    //config is in form '4l-4b'
    int bigs = config[0] - '0';
    int lils = config[3] - '0';
    if (bigs > 4 || bigs < 0 || lils > 4 || lils < 0) {
        fprintf(stderr,"Error: invalid CORE_CONFIG '%s'",config);
    }
}

//TODO make hex
std::string mask_to_str(cpu_set_t mask) {
    int big = 0, little = 0;
    std::string result = "XXXXXXXX";
    for (int i = 0; i < 8; i++) {
        if (CPU_ISSET(i,&mask)) {
            result[i] = '1';
        } else {
            result[i] = '0';
        }
    }
    return result;
}

void experiment_init() {
    char* env_log = getenv("LOG_FILE");
    if(env_log == NULL) {
        fprintf(stderr,"Error: no LOG_FILE defined\n");
        exit(1);
    }

    char* env_config = getenv("CORE_CONFIG");
    if(env_config == NULL) {
        fprintf(stderr,"Error: no CORE_CONFIG defined\n");
        exit(1);
    }
    experiment_set_config(env_config);
    int size_mb = 2;

    std::string logdir = "/home/vagrant/research/interpose/logs/";
    std::string logfile(env_log);
    nanolog::initialize(nanolog::GuaranteedLogger(),logdir, logfile,size_mb);
}


void experiment_fentry(const char* func_name) {
    unsigned int tid = syscall(SYS_gettid);
    LOG_INFO << tid << ": Entering function: " << func_name << "\n";
    cpu_set_t mask = _set_affinity_little();
    LOG_INFO << tid << ": Using mask: " << mask_to_str(mask) << "\n";

    clock_gettime(CLOCK_MONOTONIC,&timeStart);
}

void experiment_fexit(const char* func_name) {
    clock_gettime(CLOCK_MONOTONIC,&timeEnd);
    double latency = ((double)timeEnd.tv_sec*1000 + (double)timeEnd.tv_nsec*ns_to_ms)
                        - ((double)timeStart.tv_sec*1000 + (double)timeStart.tv_nsec*ns_to_ms);

    unsigned int tid = syscall(SYS_gettid);
    LOG_INFO << tid << ": Exiting function: " << func_name << "\n";
    cpu_set_t mask = _set_affinity_all();
    LOG_INFO << tid << ": Resetting mask: " << mask_to_str(mask) << "\n";
    LOG_INFO << tid << ": Latency: " << latency << "\n";
}
