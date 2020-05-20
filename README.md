# Chrome Interposition Experiments
## What this repo is
- A way of keeping track of what we've tried
- A useful set of tools for running experiments that run by interposing important Chromium functions

### Interposed Functions
We've tracked down some interesting functions in the main phases of a chrome page load.
These phases can happen out of this order in some cases, but this is the general flow.
1. Parsing HTML
2. Parsing CSS
3. Layout
4. Paint
5. Javacript

---
Here's a specific list of the functions we've interposed so far:

*Function* | *Phase*
|------|----:|
blink::HTMLParser::PumpPendingSpeculations | HTML
blink::HTMLParser::PumpTokenizer | HTML
blink::CSSParser::ParseSheet| CSS
blink::Document::UpdateStyleAndLayout| CSS
blink::LocalFrameView::PerformLayout | Layout
blink::LocalFrameView::UpdateLifecyclePhasesInternal | Paint
blink::ScriptController::EvaluateScriptInMainWorld | Javascript
blink::ScriptController::EvaluateScriptInIsolatedWorld | Javascript
blink::v8ScriptRunner::CallFunction | Javascript
---
## Interposition
To implement interposition, we use the `LD_PRELOAD` trick. The idea is to get create a shared library whose function signatures match the desired interposition
targets exactly, so that when `ld` looks for a dynamic function, your function gets loaded first. To call the original function as if nothing has happened,
you can get a handle by using `dlsym(RTLD_NEXT,"name_as_in_binary"` You can learn more [here](www.goldsborough.me/c/low-level/kernel/2016/08/29/16-48-53-the_-ld_preload-_trick/). It is important to note that for C++ interposition special attention to generate the exact same mangled needs to be given, and unless your interposing a method declared as `static`, you will need to explicitly pass the `this` parameter so that the original function can access its own data.
---
## Running
```
make
./run.sh
```
or
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

