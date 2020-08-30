# Experimenter
This is the experimental framework that is used when interposing Chromium. It defines a number of functions for changing chrome process affinities and recording the latencies of functions.

## Design
### `cpu_utils`
These functions allow for easy adaptation using the `sched_affinity` API. They're mostly convenience wrappers, except for `set_affinity_permute` which generates a random permutation of a specified core configuration.

### `experimenter`
This is the meat of the experimental framework. It provides initialization of an experiment (setting a timer, reading in env vars, initializing the logger, etc.).

#### Environment Variables
The behavior of the framework is configurable through several environment variables:
- `RNG_SEED` : `int` - Sets a seed for RNG operations to make an experiment repeatable. The value can be any `int`.
- `TIMING` : `string` - Determines whether an internal timer is set to stop the browser. Values can be "external" to prevent an internal timer from being set or unset/any other value to default to internal timers.
- `CORE_CONFIG`: `string` - Determines what singular core configuration to run an experiment with. This is used for `run-chrome.sh` invocation and is not used when `IPC` is set to "on".
- `IPC` : `string` - Determines whether an mmaped IPC file is checked during execution. Values can be "on" for using the IPC file, or unset/any other value to use the `CORE_CONFIG` variable.
- `MMAP_FILE`: `filename` - The name of the ipc file to use if `IPC` is set to "on".
- `LOG_FILE` : `filename` - Determines the logfile name. Can either be a filename or a filename with a directory. If no directory precedes the filename, the directory `home/vagrant/research/interpose/logs/` is used. Note you may need to change this value in `experimenter.cc` for this behavior to work.


