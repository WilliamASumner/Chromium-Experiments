# Experimenter
This is the experimental framework that is used when interposing Chromium. It defines a number of functions for changing chrome process affinities and recording the latencies of functions.

## Design
### `cpu_utils`
These functions allow for easy adaptation using the `sched_affinity` API. They're mostly convenience wrappers, except for `set_affinity_permute` which generates a random permutation of a specified core configuration.
### `ipc`
These functions allow for communication with `chrome-experimenter.py`. The basic idea is that a python ExperimentalInterface object acts as the interface between the script and the C++ framework. The ExperimentalInterface uses a series of FunctionSets to generate masks that are then read by the framework at runtime, and these masks can be randomized (e.g. for a config like 2l-0b on a machine with 4l cores, it will randomly choose 2 of the 4). The interface operates off an mmaped file `/tmp/chrome_exp_mmap`. This file is read in whenever the framework receives a SIGCONT signal. That way we can update masks when we know the targeted functions are not running. The parsing takes up to 1ms in some of the benchmarks I've used, so it shouldn't be too much of a concern for longer-lived functions, however to be safe these updates are handled once `chrome-experimenter.py` has called the stopLoading function from ChromeDevTools. A map of std::string -> (cpu\_set\_t,cpu\_set\_t) is then used to find which mask needs to be used on entry/exit if IPC mode is enabled.

### `experimenter`
This is the meat of the experimental framework. It provides initialization of an experiment (setting signal handlers, optionally setting an alarm, reading in env vars, initializing the logger(s), etc.). It provides functions to mark a page load time (though their use is discouraged now). Most importantly it provides the fentry and fexit wrappers that time functions and sets their affinity based on the string name passed to them.

#### Environment Variables
The behavior of the framework is configurable through several environment variables:
- `RNG_SEED` : `int` - Sets a seed for RNG operations to make an experiment repeatable. The value can be any `int`.
- `TIMING` : `string` - Determines whether an internal timer is set to stop the browser. Values can be "external" to prevent an internal timer from being set or unset/any other value to default to internal timers.
- `CORE_CONFIG`: `string` - Determines what singular core configuration to run an experiment with. This is used for `run-chrome.sh` invocation and is not used when `IPC` is set to "on".
- `IPC` : `string` - Determines whether an mmaped IPC file is checked during execution. Values can be "on" for using the IPC file, or unset/any other value to use the `CORE_CONFIG` variable.
- `MMAP_FILE`: `filename` - The name of the ipc file to use if `IPC` is set to "on".
- `LOG_FILE` : `filename` - Determines the logfile name. Can either be a filename or a filename with a directory. If no directory precedes the filename, the directory `home/vagrant/research/interpose/logs/` is used. Note you may need to change this default value in `experimenter.cc` for this behavior to work on your system.
