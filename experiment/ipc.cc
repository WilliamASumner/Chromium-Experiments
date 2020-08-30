#include <sys/mman.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <assert.h>

#include "ipc.hh"
#include "cpu_utils.hh"
#include <sched.h>
#include <string>
#include <vector>
#include <sstream>

#ifdef TEST_IPC
#include <iostream>
#endif

static void* mmapData = nullptr;
static bool initializedMmap = false;
static int fd = (int)NULL;
static size_t fsize = -1;

size_t get_filesize(const char* filename) {
    struct stat st;
    stat(filename, &st);
    return st.st_size;
}

std::string get_mmap_str() {
    assert(initializedMmap == true && mmapData != nullptr);
    return std::string(static_cast<char*>(mmapData));
}

void tokenize(std::string const &str, const char delim, std::vector<std::string> &out) {
    std::stringstream ss(str);
    std::string s;
    while (std::getline(ss,s,delim)) {
        out.push_back(s);
    }
}

void ipc_update_funcmap(FuncMapType& funcMap) {
    funcMap.clear(); // TODO find a more efficient way of updating masks
    std::vector<std::string> fileLines;
    tokenize(get_mmap_str(),'\n',fileLines);
    int numEntries = std::stoi(fileLines[0]);
    int i = 0, line = 1;
    while (i < numEntries){
        std::string fentryMask = fileLines[line++];
        std::string fexitMask = fileLines[line++];
        int numFuncs = std::stoi(fileLines[line++]);
        int j = 0;
        for (j = 0; j < numFuncs; j++) {
            funcMap[fileLines[line+j]] =
                std::make_pair(mask_from_str(fentryMask),mask_from_str(fexitMask));
        }
        line += j;
        i++;
    }
}

void ipc_open_mmap(const char* filename) {
    assert(fd == (int)NULL && mmapData == nullptr);

    fsize = get_filesize(filename);
    assert(fsize != -1);
    fd = open(filename, O_RDONLY, 0);
    assert(fd != (int)NULL);

    mmapData = mmap(NULL,fsize, PROT_READ, MAP_SHARED, fd, 0);

    assert(mmapData != MAP_FAILED);
    initializedMmap = true;
}

void ipc_close_mmap() {
    assert(initializedMmap == true);
    int val = munmap(mmapData, fsize);
    assert(val == 0);
    initializedMmap = false;
    close(fd);
}

#ifdef TEST_IPC
int main(void) {

   ipc_open_mmap("/tmp/chrome_exp_mmap");
   FuncMapType f;
   ipc_update_funcmap(f);

   std::cout << "CallFunction Mapping is: ";
   print_mask(&f["CallFunction"].first,8);
   std::cout << " + ";
   print_mask(&f["CallFunction"].second,8);
   std::cout << std::endl;

   int x = 0;
   std::cout << "Enter a number to continue: ";
   std::cin >> x;

   ipc_update_funcmap(f);
   std::cout << "CallFunction Mapping is: ";
   print_mask(&f["CallFunction"].first,8);
   std::cout << " + ";
   print_mask(&f["CallFunction"].second,8);
   std::cout << std::endl;
   return 0;
}
#endif
