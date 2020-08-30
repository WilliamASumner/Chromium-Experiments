#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <chrono>
#include <math.h>
#include <map>
#include <random>
#include <functional>

#include "ipc.h"

int bin(double val, int nbins, double min, double max) {
    double range = max - min;
    double binsize = range/nbins;

    for (int i = 0; i < nbins; i++) {
        if ((i*binsize + min) >= val)
            return i;
    }
    return nbins-1;
}

inline void timed_function() {
    FuncMapType f;
    ipc_update_funcmap(f);
}

int main(void) {
    double bins[20] = { 0.0 };
    int num_trials = 1000000, num_trials_done = 0, num_bins = 20;
    double average = 0.0, min_time = 1000000.0, max_time = 0.0;
    //double min = 1.8E-5, max = 3E-5; // range for read_mmap
    double min = 2.8E-5, max = 7.2E-5; // range for parse_mmap
    double binsize = (max-min)/num_bins;
    std::cout << "binsize is : " << binsize << "s" << std::endl;

    ipc_open_mmap("/tmp/chrome_exp_mmap");

    std::chrono::duration<double> elapsed_seconds;
    for(int i = 0; i < num_trials; i++) {
        auto start = std::chrono::steady_clock::now();
        timed_function();
        auto end = std::chrono::steady_clock::now();

        elapsed_seconds = end-start;
        if (elapsed_seconds.count() < min_time)
            min_time = elapsed_seconds.count();
        else if (elapsed_seconds.count() > max_time)
            max_time = elapsed_seconds.count();
        average += elapsed_seconds.count();
        bins[bin(elapsed_seconds.count(),num_bins,min,max)] += 1;
        num_trials_done++;
    }
    average /= num_trials_done;
    std::cout << "average: " << average << "s for " << num_trials_done << " trials.\n";
    std::cout << "max: " << max_time << "s for " << num_trials_done << " trials.\n";
    std::cout << "min: " << min_time << "s for " << num_trials_done << " trials.\n";

    std::cout << "bins: \n";
    for (int i = 0; i < num_bins; i++) {
        printf("%*.1e ",7,(binsize*i) + min);
    }
    std::cout << "\n";
    for (int i = 0; i < num_bins; i++) {
        printf("%*.0f ",7,bins[i]);
    }
    std::cout << "\n";

    ipc_close_mmap();

    return 0;
}
