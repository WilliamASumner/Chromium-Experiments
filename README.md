# Chrome Interposition Experiments
## What this repo is
- A way of keeping track of what we've tried
- A useful set of tools for running experiments that run by interposing important Chromium functions

### Interposed Functions
We've tracked down some interesting functions in the main phases of a chrome page load:
1. Parsing HTML
2. Parsing CSS
3. Layout
4. Paint
5. Javacript
These phases can happen out of this order in some cases, but this is the general flow.
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

