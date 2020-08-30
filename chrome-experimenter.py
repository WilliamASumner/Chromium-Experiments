#!/usr/bin/env python3
import PyChromeDevTools   # interacting with chrome
import subprocess         # running chrome
import signal             # Sending signals
import os                 # environ, getcwd
import sys                # exit
import requests           # requests.exceptions.ConnectionError
import random             # core config permutations
import time               # pageLoad timing
import mmap               # shared file
import argparse           # argument parsing
import shlex              # shell arg parsing


from convenience import * # convenience functions
import ipc                # IPC interface

# Experiment Setup
expInt = ipc.ExperimentInterface()

# Create function sets
bigToAll = ipc.FunctionSet((2,2),(1,1),ipc.layout + ipc.paint + ipc.js)
littleToAll = ipc.FunctionSet((0,4),(1,1),ipc.css + ipc.html)

# Add sets to experiment and init
expInt.addSet(bigToAll)
expInt.addSet(littleToAll)

# Arg parsing
parser = argparse.ArgumentParser()
parser.add_argument("-v","--verbose",help="output everything this script is doing",action="store_true")
parser.add_argument("-p","--pause-gdb",help="instruct renderers to wait for gdb and disable hang monitor",action="store_true")
parser.add_argument("-g","--gui",help="run chromium without Xvfb",action="store_true")
parser.add_argument("-i","--renderer-info",help="increase chrome renderer verbosity",action="store_true")
parser.add_argument("-n","--no-logs",help="disable all output logs",action="store_true")
parser.add_argument("-d","--direct-to-term",help="direct chromium output to stdout instead of a log file. The --no-logs flag will not prevent this output.",action="store_true")
parser.add_argument("-s","--single-process",help="run chromium in a single process",action="store_true")
parser.add_argument("--disable-cache",help="disable cache across page loads",action="store_true")
parser.add_argument("--interactive",help="wait for user input to continue",action="store_true")
parser.add_argument("--just-mmap",help="only create mmap file then exit",action="store_true")
parser.add_argument("-c","--renderer-cmd-prefix",help="prefix to be inserted before renderer process creation",nargs=1, type=str)
parser.add_argument("-r","--rng-seed",help="set a RNG seed for repeatable results",nargs=1,type=int)
parser.add_argument("-t","--timeout",help="set timeout for page loads",nargs=1,type=int)
parser.add_argument("-b","--binary",help="specify binary file to run",nargs=1,type=fileType)
parser.add_argument("-l","--ldpreload",help="change LD_PRELOAD value. 'None' unsets LD_PRELOAD",nargs=1,type=ldPreloadFile)
parser.add_argument("-w","--websites",help="specify list of websites to be loaded", type=str, nargs="+")
parser.add_argument("-f","--chrome-flags",help="append to default or override flags passed to Chromium. Prefix given value with 'clear:' to override completely. Using 'clear:' will override the '--renderer-info' and '--pause-gdb' flags",nargs=1)

# Runtime variables default values
pwd = os.getcwd()
iterations=1
interceptLibName = "libintercept.so"
mFilename= "/tmp/chrome_exp_mmap"
mmapSizeBytes = expInt.getSize()
expectedProcesses = 2 # normally browser proc and at least one renderer
chromeBin = "/home/vagrant/chromium/src/out/x64Linux/chrome"

expDirName = f"./logs/exp-{genUniqueId()}" # give experiment it's own unique dir
while os.path.exists(expDirName):
    expDirName = f"{pwd}/logs/exp-{genUniqueId()}"

filePrefix = f"{expDirName}/PyChrome" # label with tool that generated the data
chromeStdLog = f'{filePrefix}-chrome.log'
chromeErrLog = f'{filePrefix}-chrome-err.log'
pageLoadLog = f'{expDirName}/pageloads.log'

# Environment variables default values
os.environ['TIMING'] = "external" # don't use internal timer (otherwise the exp lib will kill chrome)
os.environ['IPC'] = "on" # use IPC to communicate dynamically rather than env vars
os.environ['MMAP_FILE'] = mFilename # set mmap file to be used
if os.path.exists(f"{pwd}/{interceptLibName}"):
    os.environ['LD_PRELOAD'] = f"{pwd}/{interceptLibName}"
else:
    raise FileNotFoundError(f"No library {pwd}/{interceptLibName}")
os.environ['LOG_FILE'] = f"{expDirName}/data"
os.environ['LD_LIBRARY_PATH'] = f"/usr/local/lib:{os.environ['LD_LIBRARY_PATH'] if 'LD_LIBRARY_PATH' in os.environ else ''}" # not sure why run-chrome has this

# Chromium default flag options
chromeFlags = "--no-zygote --no-sandbox"
debugPort = 9222
chromeFlags += f" --remote-debugging-port={debugPort}"
url = ""

# Experiment variables
functions   = [
        "PumpPendingSpeculations","ResumeParsingAfterYield","ParseSheet",
        "UpdateStyleAndLayoutTree","UpdateLifeCyclePhasesInternal","PerformLayout",
        "ExecuteScriptInMainWorld","ExecuteScriptInIsolatedWorld","CallFunction"
        ]
sites       = ["amazon.com","wikipedia.org","facebook.com"]
coreConfigs = [(4,4),(2,2),(0,4),(4,0)] # (lil,big)
littles     = [0,1,2,3]
bigs        = [4,5,6,7]

# Start processing arguments
args = parser.parse_args()
if args.pause_gdb:
    chromeFlags += " --renderer-startup-dialog --disable-hang-monitor"

if args.gui:
    printv("Running without Xvfb",args.verbose)
    os.environ['DISPLAY'] = ':0'
else:
    os.environ['DISPLAY'] = ':99'
    if procIsAlive("Xvfb"):
        printv(f"Connecting to existing Xvfb [{findProcess('Xvfb')[0]}]",args.verbose)
    else:
        printv("Starting Xvfb",args.verbose)
        subprocess.Popen(['Xvfb',':99','-screen','0','800x600x16'])

if args.renderer_info:
    chromeFlags += " --enable-logging=stderr --v=1"

if args.renderer_cmd_prefix:
    chromeFlags += f" --renderer-cmd-prefix='{' '.join(args.renderer_cmd_prefix)}'"

if args.no_logs or args.direct_to_term:
    chromeStdLog = f"/dev/null"
    chromeErrLog = f"/dev/null"

if args.no_logs:
    pageLoadLog = f'/dev/null'
    os.environ['LOG_FILE'] = '/tmp/trash' # just write to the tmp dir
else:
    printv(f"Creating experiment directory: {expDirName}",args.verbose)
    os.mkdir(expDirName)

if args.single_process:
    chromeFlags += " --single-process"
    expectedProcesses = 1 # chrome will only be running as 1 process

if args.rng_seed:
    random.seed(args.rng_seed[0])
    os.environ['RNG_SEED']=str(args.rng_seed[0]) # let the experiment framework know too
    printv(f"Setting random seed to {args.rng_seed[0]}",args.verbose)

if not args.timeout:
    args.timeout = 120

if args.binary:
    args.binary = args.binary[0]
    chromeBin = args.binary

if args.ldpreload:
    args.ldpreload = args.ldpreload[0]
    if args.ldpreload == 'None':
        printv("Unsetting LD_PRELOAD",args.verbose)
        os.environ['LD_PRELOAD'] = ''
    else:
        if not os.path.exists(args.ldpreload):
            printe(f"Invalid LD_PRELOAD library {args.ldpreload}")
        printv(f"Setting LD_PRELOAD={args.ldpreload}",args.verbose)
        os.environ['LD_PRELOAD'] = args.ldpreload
else:
    printv(f"LD_PRELOAD set to '{pwd}/{interceptLibName}'",args.verbose)

if args.websites is not None:
    printv(f"Adding sites {str(args.websites)}", args.verbose)
    sites = args.websites
    #chromeFlags += f" -w {args.website[0]}" # old implementation, for now just open new tab

if args.chrome_flags:
    args.chrome_flags = args.chrome_flags[0]
    if 'clear:' in args.chrome_flags:
        printv(f"Overriding chrome flags with {args.chrome_flags}",args.verbose)
        chromeFlags = args.chrome_flags.replace("clear:","")
    else:
        printv(f"Appending flags {args.chrome_flags}",args.verbose)
        chromeFlags += args.chrome_flags

# Create final chrome command
chromeCmd = [chromeBin] + shlex.split(chromeFlags)

# Preallocate mmap file
if not os.path.exists(mFilename) or os.path.getsize(mFilename) != mmapSizeBytes:
    printv(f"Creating file {mFilename}",args.verbose)
    with open(mFilename,"wb") as mfile:
        mfile.write(b'\x00' * mmapSizeBytes)
else:
    printv(f"Found file {mFilename}",args.verbose)


# Open mmap and logs
with open(mFilename, "r+b")  as mfile, \
     open(chromeStdLog,'w') as log, \
     open(chromeErrLog,'w') as errlog:

    # Create mmap and experiment interface
    printv(f"Creating mmap from file {mFilename} ({mmapSizeBytes} bytes)",args.verbose)
    mm = mmap.mmap(mfile.fileno(), mmapSizeBytes, flags=mmap.MAP_SHARED)

    expInt.initIpc(mm)
    if args.just_mmap:
        sys.exit(0)

    # Start the chrome process
    printv(chromeCmd,args.verbose)
    try:
        if args.direct_to_term:
            printv("Disabled stdout/stderr redirection",args.verbose)
            process = subprocess.Popen(chromeCmd)
        else:
            printv(f"Opening {log} for stdout", args.verbose)
            printv(f"Opening {errlog} for stderr",args.verbose)
            process = subprocess.Popen(chromeCmd,stdout=log,stderr=errlog)
        printv("Started chrome process",args.verbose)

    except FileNotFoundError:
        printe("No file " + str(chromeCmd[0]) + " found")

    time.sleep(5) # wait for chrome to start up

    if args.interactive: # chrome dev tools won't connect if no processes are running thanks to gdb
        waitForInput("Starting chrome dev tools")

    retries = 10
    while True:
        try: 
            chrome = PyChromeDevTools.ChromeInterface(port=debugPort)
            break
        except requests.exceptions.ConnectionError: # Chrome not available yet
            if retries <= 0:
                printe(f"Error connecting to Chromium on port {debugPort}")
            elif len(findProcess("chrome")) < expectedProcesses: # missing procs means error
                printe("chrome process unable to start properly")
            printv("Couldn't connect to chrome, retrying",args.verbose)
            time.sleep(2)
            retries -= 1
            continue
        except Exception as e: # other error
            printe(e)

    chrome.Network.enable()
    printv("Enabled chrome networking tools",args.verbose)
    chrome.Network.setCacheDisabled(cacheDisabled=args.disable_cache)
    printv(f"{'Disabled' if args.disable_cache else 'Enabled'} network cache",args.verbose)
    #chrome.Network.emulateNetworkConditions(args.net_conds) # Could be interesting to see what the effects of a slow network are
    #printv(f"Emulating network conditions {args.net_conds}",args.verbose)
    chrome.Page.enable()
    printv("Enabled chrome page tools",args.verbose)
    chrome.Runtime.enable()
    printv("Enabled chrome runtime tools",args.verbose)

    if args.pause_gdb or args.interactive:
        waitForInput("Starting experiment")
    os.killpg(os.getpgid(process.pid),signal.SIGCONT) # tell the C++ framework to update function maps

    # Run experiments
    loadTimes = dict() # id =>> pageload time
    for iteration in range(iterations):
        printv(f"On iteration {iteration}",args.verbose)
        for page in random.sample(sites,len(sites)):
            if "http://www." not in page:
                page = "http://www." + page # address needs to be http://www.*

            # Generate ID to keep track of page load (may be multiple per page)
            pageLoadId = genUniqueId()
            printv(f"On page ({pageLoadId}): " + page,args.verbose)

            # Instruct chrome to navigate to page
            chrome.Page.navigate(url=page)

            chrome.wait_event("Page.loadEventFired",timeout=args.timeout)

            # Data is in the format [navigationStart timestamp, duration in ms]
            result = [None,None]
            try:
                result[0] = chrome.Runtime.evaluate(expression="performance.timing.navigationStart")['result']['result']['value'] # there's probably a more pythonic way to do this...
                result[1] = chrome.Runtime.evaluate(expression="performance.getEntriesByType('navigation')[0].duration")['result']['result']['value']
            except:
                pass
            while 0 in result or None in result:
                printv("Waiting for result",args.verbose)
                time.sleep(5) # wait 5 seconds then retry
                try:
                    result[0] = chrome.Runtime.evaluate(expression="performance.timing.navigationStart")['result']['result']['value']
                    result[1] = chrome.Runtime.evaluate(expression="performance.getEntriesByType('navigation')[0].duration")['result']['result']['value']
                except:
                    pass
            printv(f"Runtime result: {result}",args.verbose)

            # Store recorded data with associated pageLoadId
            loadTimes[pageLoadId] = result
            try:
                chrome.Page.stopLoading() # in case it is still loading, stop it so we can change mmap file
            except Exception as e:
                printe(e)

            if args.interactive:
                waitForInput("Loading next page")

            expInt.randomizeMasks()
            os.killpg(os.getpgid(process.pid),signal.SIGCONT)
            printv("Randomized masks for next run",args.verbose)

    printv("Waiting for chrome to finish",args.verbose)
    try:
        process.communicate(timeout=5) # wait for chrome to complete, or timeout
    except subprocess.TimeoutExpired:
        pass
    except Exception as e:
        printe(e)

    try:
        printv("Closing chrome",args.verbose)
        chrome.Browser.close()
        time.sleep(3)
        process.kill()
    except:
        pass

# Write out page load times csv-style
with open(pageLoadLog,'w') as pageLoadsLog:
    for pageLoadId,pageLoadData in loadTimes.items():
        pageLoadsLog.write(",".join([str(item) for item in pageLoadData]))
