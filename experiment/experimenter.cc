#include <stdio.h> // printf
#include <stdlib.h> // getenv
#include <string.h>
#include <iostream>
#include <mutex>
#include <string> // TODO update all refs to this
#include <map> // map function names

#include <unistd.h> // tid
#include <sys/syscall.h> // get tid
#include <time.h> 
#include <signal.h>
#include <random>

#include"cpu_utils.hh" // affinity functions
#include "experimenter.hh"

#include <g3log/g3log.hpp> // logger
#include <g3log/logworker.hpp>


std::mutex log_mut, time_mut, config_mut, start_mut;
std::atomic<bool> did_start(false), page_loaded(false), config_set(false), page_started(false), external_timing(false);
std::atomic<int> timeout_s(45);

static int pgid = 0;

// TODO add queue for function timing (in case one is called inside of another)

// TODO add function map

std::unique_ptr<g3::LogWorker> worker = nullptr;
std::unique_ptr<g3::FileSinkHandle> handle = nullptr;

thread_local struct timespec time_start,time_end;
thread_local std::random_device rdev;
thread_local std::mt19937 rng;

const unsigned int ns_to_ms = 1000000;

struct timespec page_start, page_end;

struct config_t {
    int nbigs;
    int nlils;
} experiment_config;

void sigalrm_handler( int sig) {
    experiment_stop();

    int result = killpg(pgid,SIGINT);
    if (result != 0) {
        fprintf(stderr,"experimenter.cc: ");
        fprintf(stderr,"Error killing process group: %d;%d",pgid,result);
    }
}

void sigint_handler(int sig) {
    experiment_stop();
}

void set_config(const char* config) {
    //config is in form '4l-4b'
    int bigs = config[0] - '0';
    int lils = config[3] - '0';
    if (bigs > 4 || bigs < 0 || lils > 4 || lils < 0) {
        fprintf(stderr,"experimenter.cc: ");
        fprintf(stderr,"Error: invalid CORE_CONFIG '%s'",config);
    }

    const std::lock_guard<std::mutex> lock(config_mut);
    if (!config_set) {
        experiment_config.nbigs = bigs;
        experiment_config.nlils = lils;
        config_set = true;
    }
}

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

void set_sigint_hndlr() {
    struct sigaction sact;
    sigemptyset(&sact.sa_mask);
    sact.sa_flags = 0;
    sact.sa_handler = sigint_handler;
    sigaction(SIGINT, &sact, NULL);
}

void experiment_start_timer() {
    struct sigaction sact;
    sigemptyset(&sact.sa_mask);
    sact.sa_flags = 0;
    sact.sa_handler = sigalrm_handler;
    sigaction(SIGALRM, &sact, NULL);

    if (!external_timing) {
        alarm(timeout_s); // start timeout
    }
}

void experiment_init(const char *exec_name) {
    const std::lock_guard<std::mutex> lock(start_mut);
    if (did_start) {
        return;
    }

    fprintf(stderr,"experimenter.cc: ");
    fprintf(stderr,"Initializing experiment\n");

    // Init RNG
    char* env_seed = getenv("RNG_SEED");
    if (env_seed != nullptr && atoi(env_seed) != 0) {
        rng.seed(atoi(env_seed));
    } else {
        rng.seed(rdev());
    }

    char* env_timing = getenv("TIMING");
    if (env_timing != nullptr && strncmp(env_timing,"external",9) == 0) { // default to internal timing
        external_timing = true;
    } else if (env_timing != nullptr) {
        int timing_s = atoi(env_timing);
        if (timing_s > 0)
            timeout_s = timing_s; // update time to wait
        else {
            fprintf(stderr,"experimenter.cc: ");
            fprintf(stderr,"Error using env TIMING variable\n");
        }
    }

    if (external_timing) {
        const std::lock_guard<std::mutex> lock(time_mut);
        clock_gettime(CLOCK_MONOTONIC,&page_start);
        set_sigint_hndlr();
    }

    char* ipc = getenv("IPC");
    if (ipc != nullptr && strncmp(ipc,"on",3) == 0) { // default to non-IPC
        // setup IPC
        //do_ipc = true;

        char* pipe_file = getenv("PIPE_FILE");
        if (pipe_file == nullptr) {
            fprintf(stderr,"experimenter.cc: ");
            fprintf(stderr,"Error reading PIPE_FILE variable\n");
            exit(1);
        }

        g3::initializeLogging(worker.get());
        did_start = true; // done initializing, all threads can go now
        return;
    }


    char* env_log = getenv("LOG_FILE");
    if(env_log == nullptr) {
        fprintf(stderr,"experimenter.cc: ");
        fprintf(stderr,"Error: no LOG_FILE defined\n");
        exit(1);
    }

    char* env_config = getenv("CORE_CONFIG");
    if(env_config == nullptr) {
        fprintf(stderr,"experimenter.cc: ");
        fprintf(stderr,"Error: no CORE_CONFIG defined\n");
        exit(1);
    }
    set_config(env_config);
    int size_mb = 2;

    std::string logdir = "/home/vagrant/research/interpose/logs/";
    std::string logfile(env_log);

    worker=g3::LogWorker::createLogWorker();
    handle=worker->addDefaultLogger(logfile,logdir);

    g3::initializeLogging(worker.get());

    did_start = true; // done initializing, all threads can go now
}

void experiment_stop() {
    if (did_start) {
        //fprintf(stderr,"experimenter.cc: ");
        //fprintf(stderr,"\nProgram exceeded %d s limit\n",timeout_s);
        g3::internal::shutDownLogging();
    }
}

void experiment_mark_page_start() {
    if (!page_started && !external_timing) {
        clock_gettime(CLOCK_MONOTONIC,&page_start);
    }
}

void experiment_mark_page_loaded() {
    if (!page_loaded && !external_timing) {
        clock_gettime(CLOCK_MONOTONIC,&page_end);
        double page_load = -1.0;
        {
            const std::lock_guard<std::mutex> lock(time_mut);
            page_load = ((double)page_end.tv_sec*1000 + (double)page_end.tv_nsec/ns_to_ms)
                - ((double)page_start.tv_sec*1000 + (double)page_start.tv_nsec/ns_to_ms);
            page_loaded = true;
        }

        unsigned int tid = syscall(SYS_gettid);
        {
            const std::lock_guard<std::mutex> lock(log_mut);
            LOG(INFO)<< tid << ":\t" << "PageLoadTime\t" << page_load;
        }

    }
}

void experiment_fentry(std::string func_name) {
    unsigned int tid = syscall(SYS_gettid);
    cpu_set_t mask;
    set_affinity_little(&mask);
    {
        const std::lock_guard<std::mutex> lock(log_mut);
        LOG(INFO) << tid << ":\t" << func_name << "\t" << mask_to_str(mask) << "\t" << get_curr_cpu();
    }

    clock_gettime(CLOCK_MONOTONIC,&time_start);
}

void experiment_fexit(std::string func_name) {
    clock_gettime(CLOCK_MONOTONIC,&time_end);
    double latency = ((double)time_end.tv_sec*1000 + (double)time_end.tv_nsec/ns_to_ms)
                        - ((double)time_start.tv_sec*1000 + (double)time_start.tv_nsec/ns_to_ms);

    unsigned int tid = syscall(SYS_gettid);
    cpu_set_t mask;
    set_affinity_all(&mask);
    {
        const std::lock_guard<std::mutex> lock(log_mut);
        LOG(INFO) << tid << ":\t" << func_name << "\t" << mask_to_str(mask) << "\t" << get_curr_cpu() << "\t" << latency;
    }
}
