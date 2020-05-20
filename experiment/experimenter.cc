#include <stdio.h> // printf
#include <stdlib.h> // getenv
#include <iostream>
#include <mutex>

#include <unistd.h> // tid
#include <sys/syscall.h> // get tid
#include <time.h> 
#include <signal.h>
#include <random>

#include"cpu_utils.hh" // affinity functions
#include "experimenter.hh"

#include <g3log/g3log.hpp> // logger
#include <g3log/logworker.hpp>


const int timeout_s = 45;
std::mutex mut;
bool did_start = false;

std::unique_ptr<g3::LogWorker> worker = nullptr;
std::unique_ptr<g3::FileSinkHandle> handle = nullptr;

void sigalrm_handler( int sig) {
    experiment_stop();
}

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

void experiment_init(const char *exec_name) {
    mut.lock();
    if (did_start) {
        printf("Already started\n");
        mut.unlock();
        return;
    }

    struct sigaction sact;
    sigemptyset(&sact.sa_mask);
    sact.sa_flags = 0;
    sact.sa_handler = sigalrm_handler;
    sigaction(SIGALRM, &sact, NULL);

    alarm(timeout_s);
    fprintf(stderr,"Initializing experiment");

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

    /*std::random_device rd;
    std::mt19937 mt(rd());

    // generate unique file id, so render processes don't mess eachother up
    std::string str("0123456789abcdefghijklmnopqrstuvwxyz");
    std::shuffle(str.begin(), str.end(),mt);
    std::string id = str.substr(0,8);*/

    worker=g3::LogWorker::createLogWorker();
    handle=worker->addDefaultLogger(logfile,logdir);

    g3::initializeLogging(worker.get());

    did_start = true; // done initializing, all threads can go now
    mut.unlock();
}

void experiment_stop() {
    fprintf(stderr,"\nProgram exceeded %d s limit\n",timeout_s);
    g3::internal::shutDownLogging();
    exit(0);
}


void experiment_fentry(const char* func_name) {
    unsigned int tid = syscall(SYS_gettid);
    cpu_set_t mask = _set_affinity_little();
    LOG(INFO) << tid << ": Entering function: " << func_name << " mask: " << mask_to_str(mask);

    clock_gettime(CLOCK_MONOTONIC,&timeStart);
}

void experiment_fexit(const char* func_name) {
    clock_gettime(CLOCK_MONOTONIC,&timeEnd);
    double latency = ((double)timeEnd.tv_sec*1000 + (double)timeEnd.tv_nsec/ns_to_ms)
                        - ((double)timeStart.tv_sec*1000 + (double)timeStart.tv_nsec/ns_to_ms);

    unsigned int tid = syscall(SYS_gettid);
    cpu_set_t mask = _set_affinity_all();
    LOG(INFO) << tid << ": Exiting function: " << func_name << " mask: " << mask_to_str(mask) << " latency: " << latency;
}
