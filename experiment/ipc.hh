#include <map>
#include <string>
#include <sched.h>

typedef std::map< std::string, std::pair< cpu_set_t, cpu_set_t> > FuncMapType;

void ipc_open_mmap(const char* filename);

void ipc_update_funcmap(FuncMapType& funcMap);

void ipc_close_mmap(void);
