#include <string>

void experiment_start_timer();

void experiment_init(const char*);

void experiment_stop();

void experiment_mark_page_start();

void experiment_mark_page_loaded();

void experiment_fentry(std::string func_name);

void experiment_fexit(std::string func_name);
