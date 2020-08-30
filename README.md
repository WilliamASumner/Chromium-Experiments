# Chrome Interposition Experiments
## Project Overview
The goal of this project is to increase the energy efficiency of mobile page loads on heterogeneous platforms. We seek to interpose important functions in Chromium so that we can monitor the behavior of the system, adapt it to see the effect of certain core-configurations on performance/energy usage and ultimately develop a better method of assigning core configurations.

## What this repo is
- A way of keeping track of what we've tried
- A useful set of tools for running experiments that work by interposing important Chromium functions

## Prerequisites
### Things needed to run these experiments
- A Linux machine
- `make`
- ccurtsinger's [interposing header](https://github.com/ccurtsinger/interpose) (included in this repo)
- [g3log](https://github.com/KjellKod/g3log) for data collection
- [PyChromeDevTools](https://github.com/marty90/PyChromeDevTools), see the repo for installation details<sup>1</sup> 

### Things that will make life easier
- A checked out chromium repo, see the [instructions](https://chromium.googlesource.com/chromium/src/+/master/docs/linux/build_instructions.md). This will help in creating the proper mangled symbols. The version I'm testing with is Chromium 82.0.4063.0
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

## Running
To run an experiment, it's as easy as:
```
make run-pychrome
```
This will run `chrome-experimenter.py` with some default options. The preferred (and most flexible) way to run experiments is with `./chrome-experimenter.py [OPTIONS]`. See `./chrome-experimenter.py -h` for help.
```
make run-sh
```
This will run `permutate.sh` with some default arguments. To run with custom settings, see `Makefile` for usage or the ["more help"](#More-help) section. 

### Changing log output
Use: `make run PERM_PREFIX=YOUR_PREFIX` to change the prefix for the log files

## Terminology
* *experiment* - a single invocation of a script to produce data from 1+ page loads in Chromium
* *affinity* - the set of cores that a process can be scheduled on
* *(core) configuration* - same as *affinity*
* *adaptation* - an alteration to the affinity of a process or several processes
* *phase* - a period in program execution during which a process performs related tasks. An example could be the Layout phase of the browser in which offsets for various elements on a page are calculated.
* *page load time* - Can have multiple meanings. The high-level definition is the time from when a page load starts to when it ends.  For `chrome-experimenter.py` it is the time from when a page is navigated to (i.e. the `navigationStart` event) to when the `loadEventEnd` event occurs. For `run-chrome.sh` or any experiment using the internal timing mode of the framework, the definition is the time from when the `experiment_mark_page_start` function is called to when the `experiment_mark_page_loaded` function is called, which is from the entry into `main` until `Document::SetReadyState` is called with an argument of `kComplete`. This definition is not preferred because 1) it creates multiple page load times due to subprocesses spawned during the page load (see [Site Isolation](http://www.chromium.org/Home/chromium-security/site-isolation)) and 2) is ad-hoc compared to the performance timing API.

## Explanation of files
- `Makefile`: A makefile to simplify compiling/running experiments.
- `README.md`: This file.
- `chrome-experimenter.py`: Uses PyChromeDevTools to time page loads. It closely (but not exactly) mirrors the usage of `run-chrome.sh`.
- `src/chrome_includes/v8/`: Includes for datatypes in v8, needed for interposing some v8 functions
- `src/chrome_interpose.cc`: Interposing functions, see the [table](https://github.com/WilliamASumner/Chromium-Experiments#interposed-functions) below
- `src/experiment/`: Everything related to the experiment framework itself
    - `cpu_utils.*`: Functions for setting affinity during experiments
    - `experimenter.*`: Functions for running/stopping experiments and logging
    - `interpose.hh`: From [ccurtsinger](https://github.com/ccurtsinger/interpose), for interposing \_libc\_start\_min
    - `README.md`: Information on the design and use of the framework
- `src/python`: Helper python scripts for `chrome-experimenter.py`
    - `convenience.py`: Various convenience functions
    - `ipc.py`: All IPC related code
    - `plotting/`: Scripts to process and plot the data of an experiment
        - `occupany.py`: Plot occupancy over a page load
        - `processing.py`: Process data logs into an organized numpy array
- `src/scripts/`: All bash scripts used in this project
    - `list-procs.sh`: List "interesting" attributes of running chrome processes. This is mainly used for debugging affinity and checking how many renderers are alive
    - `mapping.sh`: Record memory mappings of loaded libraries
    - `permutate.sh`: Generates core-configuration permutations for running experiments over multiple trials with randomized webpage/configuration ordering
    - `run-chrome.sh`: Bash script for running `chrome` (or `content_shell`) with some options that help with debugging
    - `summarize.sh`: Generates the files `summary.log` and `func_latencies.log`. The former is a statistical overview of the function latencies and the latter is a concatenation of all the data logs from a single experiment (this is used to generate `summary.log`).
    - `wget-script.sh`: Unfinished script to download webpages programmatically

- `logs/`: All data from experiments is output here
    - `example_data/`: An example of the output create for an experiment
    - `format.md`: A description of all the log files produced
    - `exp-*`: An experiment output
- `misc/`: Items that don't fit anywhere else
    - `example`: Simple example illustrating how interposition works
    - `ipc-file.txt`: File layout for mmaped IPC (Python + Experiment Framework)
    - `web-performace-syms.txt`: Mangled symbols of potential interest
- `todo.txt`: A todo list for myself

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
To implement interposition, we use the [`LD_PRELOAD`](http://www.goldsborough.me/c/low-level/kernel/2016/08/29/16-48-53-the_-ld_preload-_trick/) trick. The idea is to get create a shared library whose function signatures match the desired interposition targets exactly, so that when `ld` looks for a dynamic function, your function gets loaded first. To call the original function as if nothing has happened, you can get a handle by using `dlsym(RTLD_NEXT,"binary_function_name")`. **It is important to note that for C++ functions the exact signature of the target needs to be replicated including namespace and arguments.** This is due to [name mangling](https://en.wikipedia.org/wiki/Name_mangling). C++ methods also require an explicit `this` argument to the original handle, unless you're interposing a method declared as `static`. See `chrome_interpose.cc` for examples of this.

---
## More help
* For usage run:
```
python3 chrome-experimenter.py -h or ./chrome-experimenter.py -h
./run-chrome.sh -h
./permutate.sh -h
```
* For help with interposition check out this simple [example](https://github.com/WilliamASumner/Chromium-Experiments/tree/master/misc/example)
* For help with understanding the data produced see the [example data](https://github.com/WilliamASumner/Chromium-Experiments/tree/master/logs/example_data) and the [format description](https://github.com/WilliamASumner/Chromium-Experiments/blob/master/logs/format.md)
* The [Chrome DevTools Protocol Docs](https://chromedevtools.github.io/devtools-protocol/) will help when working with `chrome-experimenter.py`
* [How Blink Works](https://docs.google.com/document/d/1aitSOucL0VHZa9Z2vbRJSyAIsAz24kX8LFByQ5xQnUg/edit) and the [Chromium Design Docs](https://www.chromium.org/developers/design-documents) are great for learning about Chromium's internals
* [Debugging in Linux](https://chromium.googlesource.com/chromium/src/+/master/docs/linux/debugging.md) is a great resource for figuring out why Chromium is crashing

---
1. At the time of writing, the PyChromeDevTools listed contain an error with the `wait_event` function that makes the recorded page load times much shorter than they normally would be. I've done a pull request to fix this issue, but if you're experiencing this issue check out [my version](https://github.com/WilliamASumner/PyChromeDevTools).

