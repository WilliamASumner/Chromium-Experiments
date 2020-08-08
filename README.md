# Chrome Interposition Experiments
## Project Overview
The goal of this project is to increase the energy efficiency of mobile page loads on heterogeneous platforms. We seek to interpose important functions in Chromium so that we can see the behavior of the system, adapt it to see the effect of certain core-configurations on performance/energy usage and ultimately develop a better method of assigning core configurations.

## What this repo is
- A way of keeping track of what we've tried
- A useful set of tools for running experiments that run by interposing important Chromium functions

## Prerequisites
### Things needed to run these experiments
- A Linux machine
- `make`
- ccurtsinger's [interposing header](https://github.com/ccurtsinger/interpose)
- [g3log](https://github.com/KjellKod/g3log) for data collection
- [PyChromeDevTools](https://github.com/marty90/PyChromeDevTools), see the repo for installation details<sup>1</sup> 

### Things that will make life easier
- A checked out chromium repo, see the [instructions](https://chromium.googlesource.com/chromium/src/+/master/docs/linux/build_instructions.md). This will help in creating the proper mangled symbols
- `nm` for reading the dynamic symbols in a binary
- [Zack's guide](https://docs.google.com/document/d/1TVIYvACQTvLrhdRw6EelifGGxvcSxwn_mU4oUGVymFE/edit) for cross-compiling to ARM

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
- `Makefile`: A makefile to simplify compiling/running experiments. `make run` will run an experiment, `make run-gui` will run an experiment with a graphical interface attached to chrome, etc. See additional targets in this file for more configurations.
- `README.md`: This file.
- `chrome-experimenter.py`: Uses PyChromeDevTools to time page loads. It closely (but not exactly) mirrors the usage of `run-chrome.sh`.
- `chrome_includes/v8/`: Includes for datatypes in v8, needed for interposing some v8 functions
- `chrome_intercept.cc`: Interposing functions, see the [table](https://github.com/WilliamASumner/Chromium-Experiments#interposed-functions) below
- `experiment/`
    - `cpu_utils.*`: Functions for setting affinity during experiments
    - `experimenter.*`: Functions for running/stopping experiments and logging
    - `interpose.hh`: From [ccurtsinger](https://github.com/ccurtsinger/interpose), for interposing \_libc\_start\_min
- `misc/`
    - `example`: Simple example illustrating how interposition works
    - `ipc-file.txt`: File layout for mmaped IPC (Python + Experiment Framework)
    - `list-procs.sh`: A script to list "interesting" attributes of running chrome processes. This is mainly used for debugging affinity and checking how many renderers are alive
    - `mapping.sh`: A script for recording memory mappings of loaded libraries
    - `odroid-port`: Old Odroid scripts that are being ported for this project
    - `web-performace-syms.txt`: Mangled symbols of potential interest
    - `wget-script.sh`: Unfinished script to download webpages programmatically
- `permutate.sh`: Generates core-configuration permutations for running experiments over multiple trials with randomized webpage/configuration ordering
- `plotting/`
    - `occupany.py`: Plot occupancy over a page load
    - `processing.py`: Process data logs into an organized numpy array
- `run-chrome.sh`: Bash script for running `chrome` (or `content_shell`) with some options that help with debugging
- `summarize.sh`: Generates the files `summary.log` and `func_latencies.log`. The former is a statistical overview of the function latencies and the latter is a concatenation of all the data logs from a single experiment (this is used to generate `summary.log`).
- `todo.txt`: A simple todo list for myself

### Interposed Functions
We've tracked down some interesting functions in each of the phases of a chrome page load.
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
To implement interposition, we use the [`LD_PRELOAD`](http://www.goldsborough.me/c/low-level/kernel/2016/08/29/16-48-53-the_-ld_preload-_trick/) trick. The idea is to get create a shared library whose function signatures match the desired interposition targets exactly, so that when `ld` looks for a dynamic function, your function gets loaded first. To call the original function as if nothing has happened, you can get a handle by using `dlsym(RTLD_NEXT,"binary_function_name")`. **It is important to note that for C++ functions the exact signature of the target needs to be replicated including namespace and arguments.** This is due to [name mangling](https://en.wikipedia.org/wiki/Name_mangling). C++ methods also require an explicit `this` argument to the original handle, unless you're interposing a method declared as `static`. See `chrome_intercept.cc` for examples of this.

---
## Running
To run an experiment, it's as easy as:
```
make run
```
This will run `permutate.sh` with some default arguments. To run with custom settings, see `Makefile` for usage or the ["more help"](#More-help section). 

### Changing log output
Use: `make run PERM_PREFIX=YOUR_PREFIX` to change the prefix for the log files

## More help
* For usage run:
```
./run-chrome.sh -h
./permutate.sh -h
python3 chrome-experimenter.py -h or ./chrome-experimenter.py -h
```
* For help with interposition check out this simple [example](https://github.com/WilliamASumner/Chromium-Experiments/tree/master/misc/example)
* For help with understanding the data produced see the [example data](https://github.com/WilliamASumner/Chromium-Experiments/tree/master/logs/example_data) that is produced by the framework and the [format spec](https://github.com/WilliamASumner/Chromium-Experiments/blob/master/logs/format.md)
* The [Chrome DevTools Protocol Docs](https://chromedevtools.github.io/devtools-protocol/) will help when working with `chrome-experimenter.py`
* [How Blink Works](https://docs.google.com/document/d/1aitSOucL0VHZa9Z2vbRJSyAIsAz24kX8LFByQ5xQnUg/edit) and the [Chromium Design Docs](https://www.chromium.org/developers/design-documents) are great for learning about Chromium's internals

---
1. At the time of writing, the PyChromeDevTools listed contain an error with the `wait_event` function that makes the recorded page load times much shorter than they normally would be. I've done a pull request to fix this issue, but if you're experiencing this issue check out [my version](https://github.com/WilliamASumner/PyChromeDevTools).

