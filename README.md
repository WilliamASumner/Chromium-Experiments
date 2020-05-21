# Chrome Interposition Experiments
## Project Overview
The goal of this project is to increase the energy efficiency of mobile page loads on heterogeneous platforms. We seek to interpose important functions in Chromium so that we can see the behavior of the system, adapt it to see the effect of certain core-configurations on performance/energy usage and ultimately develop a better method of assigning core configurations.

## What this repo is
- A way of keeping track of what we've tried
- A useful set of tools for running experiments that run by interposing important Chromium functions

## Prerequisites
### Things you'll need to run these experiments
- A Linux machine
- `make`
- ccurtsinger's [interposing header](https://github.com/ccurtsinger/interpose)
- The [g3log](https://github.com/KjellKod/g3log) logger for data collection

### Things that will make your life easier
- A checked out chromium repo, see the [instructions](https://chromium.googlesource.com/chromium/src/+/master/docs/linux/build_instructions.md). This will help in creating the proper mangled symbols
- `nm` for reading the dynamic symbols in a binary
- Zack's [guide](https://docs.google.com/document/d/1TVIYvACQTvLrhdRw6EelifGGxvcSxwn_mU4oUGVymFE/edit) for cross-compiling to ARM

### g3log Installation
To install g3log for use with this project, here's a quick intro.
Start off by cloning [g3log](https://github.com/KjellKod/g3log) and then running the following commands:
```
cd g3log
mkdir build
cd build
cmake ..
make
sudo make install
```

After that, using g3log in a project should be as easy as `#include`ing the right files and linking with `-lg3logger`. I'm not sure this is the proper way to do things since there was no mention of this in the repo, but it worked for this project.

## Explanation of files
- `chrome_intercept.cc`: Interposing functions, see the [table](https://github.com/WilliamASumner/Chromium-Experiments#interposed-functions) below
- `chrome_includes/v8`: Includes for datatypes in v8, needed for interposing some v8 functions
- `experiment`
    - `cpu_utils`: Utility functions for setting affinity while running experiments
    - `experimenter`: Functions for running/stopping experiments and logging
    - `interpose.hh`: Credits to ccurtsinger, for interposing \_libc\_start\_min
    - `mapping.sh`: A script for recording memory mappings of loaded libraries
- `misc`
    - `example`: Simple interposition example illustrating how interposing works
    - `odroid-port`: Old Odroid scripts that are being ported for this project
    - `thoughts.txt`: Collection of my thoughts as I've been working on this project
    - `web-performace-syms.txt`: Mangled symbols of potential interest
- `permutate.sh`: Script that generates core-configuration permutations for running experiments over a bunch of trials
- `process.py`: Python script for parsing/analyzing output logs
- `run.sh`: Bash script for running `chrome` (or `content\_shell`) with some options that help in debugging

### Interposed Functions
We've tracked down some interesting functions in the main phases of a chrome page load.
These phases can happen out of this order in some cases, but this is the general flow.
1. Parsing HTML
2. Parsing CSS
3. Layout
4. Paint
5. JavaScript

## Currently Interposed Functions

*Function* | *Phase*
|------|----:|
blink::HTMLParser::PumpPendingSpeculations | HTML
blink::HTMLParser::PumpTokenizer | HTML
blink::CSSParser::ParseSheet| CSS
blink::Document::UpdateStyleAndLayout| CSS
blink::LocalFrameView::PerformLayout | Layout
blink::LocalFrameView::UpdateLifecyclePhasesInternal | Paint
blink::ScriptController::EvaluateScriptInMainWorld | JavaScript
blink::ScriptController::EvaluateScriptInIsolatedWorld | JavaScript
blink::v8ScriptRunner::CallFunction | JavaScript
---

## Interposition
To implement interposition, we use the [`LD_PRELOAD`](http://www.goldsborough.me/c/low-level/kernel/2016/08/29/16-48-53-the_-ld_preload-_trick/) trick. The idea is to get create a shared library whose function signatures match the desired interposition targets exactly, so that when `ld` looks for a dynamic function, your function gets loaded first. To call the original function as if nothing has happened, you can get a handle by using `dlsym(RTLD_NEXT,"binary_function_name")`. It is important to note that for C++ interposition special attention to generate the exact same mangled needs to be given, and unless your interposing a method declared as `static`, you will need to explicitly pass the `this` parameter so that the original method can access its own data.

---
## Running
To run this file, it's as easy as:
```
make run
```

## Changing log output
Use: `make run PERM_PREFIX=YOUR_PREFIX` to change the prefix for the log files

## More help
```
./run.sh -h
```
or
```
./permutate.sh -h
```

