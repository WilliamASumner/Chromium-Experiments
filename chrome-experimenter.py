#!/usr/bin/env python3
import PyChromeDevTools # interacting with chrome
import subprocess       # running chrome
import os               # environ, getcwd
import sys              # exit
import requests         # requests.exceptions.ConnectionError
import uuid             # unique ids
import random           # core config permutations
import time             # pageLoad timing
import mmap             # shared file
import argparse         # argument parsing

def genUniqueId():
    return str(uuid.uuid4()).replace("-","")[0:8]

def findProcess(procName):
    p = subprocess.Popen(['pgrep','Xvfb'],stdout=subprocess.PIPE)
    output, _ = p.communicate()
    output = str(output,'UTF-8').rstrip().split("\n")
    return output

def procIsAlive(procName):
    return len(findProcess(procName)) > 0

def fileExists(filename):
    return os.path.exists(filename)

def printv(msg,verbose):
    if verbose: print(msg)

def printe(msg):
    print(f"{sys.argv[0]}: " + msg)
    sys.exit(1)

# Arg parsing
parser = argparse.ArgumentParser()
parser.add_argument("-v","--verbose",help="output everything this script is doing",action='store_true')
parser.add_argument("-p","--pause-gdb",help="instruct renderers to wait for gdb",action='store_true')
parser.add_argument("-g","--gui",help="run chromium without Xvfb",action='store_true')
parser.add_argument("-i","--renderer-info",help="increase chrome renderer verbosity",action='store_true')
parser.add_argument("-n","--no-cache",help="disable cache across page loads",action='store_true')
parser.add_argument("-s","--single-process",help="run chromium in a single process",action='store_true')
parser.add_argument("-r","--rng-seed",help="run chromium in a single process",nargs=1,type=int)
parser.add_argument("-b","--binary",help="specify binary file to run",nargs=1)
parser.add_argument("-l","--ldpreload",help="change LD_PRELOAD value. 'None' unsets LD_PRELOAD",nargs=1)
parser.add_argument("-w","--website",help="specify default website to be loaded",nargs=1)
parser.add_argument("-f","--chrome-flags",help="append to default or override flags passed to Chromium. Prefix given value with 'clear:' to override completely. Note using 'None' will also override the '-i' and 'p' flags",nargs=1)


# Run time variables
pwd = os.getcwd()
iterations=1
interceptLibName = "libintercept.so"
mFilename= "/tmp/chrome_exp_mmap"
mmapSizeBytes = len("testing") # TODO update this with a better size
expectedProcesses = 2 # normally browser proc and at least one renderer
chromeBin = "/home/vagrant/chromium/src/out/x64Linux/chrome"

expDirName = f"./logs/exp-{genUniqueId()}" # give experiment it's own unique dir
while os.path.exists(expDirName):
    expDirName = f"{pwd}/logs/exp-{genUniqueId()}"
os.mkdir(expDirName)

filePrefix = f"{expDirName}/PyChrome" # label with tool that generated the data
chromeStdLog = f'{filePrefix}-chrome-log.txt'
chromeErrLog = f'{filePrefix}-chrome-err-log.txt'

# Environment variables
os.environ['TIMING'] = "external" # don't use internal timer (otherwise the exp lib will kill chrome)
os.environ['IPC'] = "on" # use IPC to communicate dynamically rather than env vars
os.environ['MMAP_FILE'] = mFilename # set mmap file to be used
os.environ['LD_PRELOAD'] = f"{pwd}/{interceptLibName}"
os.environ['LOG_FILE'] = f"{expDirName}/data"
# os.environ['LD_LIBRARY_PATH'] = f"/usr/local/lib:{os.environ['LD_LIBRARY_PATH']}" # not sure why run-chrome has this

# Chromium flag options
chromeFlags = "--no-zygote --no-sandbox"
debugPort = 9222
chromeFlags += f" --remote-debugging-port={debugPort}"
#url = f"-w {startUrl}"
url = ""


# Experiment variables
functions   = []
sites       = ["example.com","amazon.com","twitter.com","wikipedia.org","facebook.com"]
coreConfigs = [(4,4),(2,2),(0,4),(4,0)] # (lil,big)
littles     = [0,1,2,3]
bigs        = [4,5,6,7]

# Start processing arguments

args = parser.parse_args()
if args.pause_gdb:
    chromeFlags += " --renderer-startup-dialog"
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

if args.single_process:
    chromeFlags += " --single-process"
    expectedProcesses = 1 # chrome will only be running as 1 process

if args.rng_seed:
    random.seed(args.rng_seed)
    os.environ['RNG_SEED']=str(rng_seed) # let the experiment framework know too

if args.binary:
    args.binary = args.binary[0]
    chromeBin = args.binary

if args.ldpreload:
    args.ldpreload = args.ldpreload[0]
    if args.ldpreload == 'None':
        printv("Unsetting LD_PRELOAD",args.verbose)
        os.environ['LD_PRELOAD'] = ''
    else:
        if not fileExists(args.ldpreload):
            printe(f"Invalid LD_PRELOAD library {args.ldpreload}")
        printv(f"Setting LD_PRELOAD={args.ldpreload}",args.verbose)
        os.environ['LD_PRELOAD'] = args.ldpreload
else:
    printv(f"LD_PRELOAD set to '{pwd}/{interceptLibName}'",args.verbose)

if args.website:
    args.website = args.website[0]
    chromeFlags += f" -w {args.website}"

if args.chrome_flags:
    args.chrome_flags = args.chrome_flags[0]
    if 'clear:' in args.flags:
        printv(f"Overriding chrome flags with {args.chrome_flags}",args.verbose)
        chromeFlags = args.flags.replace("clear:","")
    else:
        printv(f"Appending flags {args.chrome_flags}",args.verbose)
        chromeFlags += args.flags

# Create final chrome command
chromeCmd = [chromeBin] + chromeFlags.split(" ")
printv(chromeCmd,args.verbose)

# Preallocate mmap file
if not os.path.exists(mFilename):
    printv(f"Creating mmap file {mFilename}",args.verbose)
    with open(mFilename,"wb") as mfile:
        mfile.write(b'\x00' * mmapSizeBytes)
else:
    printv(f"Found mmap file {mFilename}",args.verbose)

# Open mmap and logs
with open(mFilename, "r+b")  as mfile, \
     open(chromeStdLog,'w') as log, \
     open(chromeErrLog,'w') as errlog:

    printv(f"Opening {chromeStdLog} for stdout and {chromeErrLog} for stderr",args.verbose)
    mm = mmap.mmap(mfile.fileno(), mmapSizeBytes, flags=mmap.MAP_SHARED)

    # Start the chrome process
    printv(chromeCmd,args.verbose)
    try:
        process = subprocess.Popen(chromeCmd)#,stdout=log,stderr=errlog)
    except FileNotFoundError:
        printe("No file " + str(chromeCmd[0]) + " found")

    time.sleep(5)

    retries = 10
    while True:
        try: 
            chrome = PyChromeDevTools.ChromeInterface(port=debugPort)
            break
        except requests.exceptions.ConnectionError: # Chrome not available yet
            if retries <= 0:
                printe(f"Error connecting to Chromium on port {debugPort}")
            elif len(findProcess("chrome")) < expectedProcesses: # missing procs means error
                printe("chrome process unable to start")
            printv("Couldn't connect to chrome, retrying",args.verbose)
            time.sleep(2)
            continue
        except Exception as e: # other error
            print(e)
            sys.exit(1)

    chrome.Network.enable()
    chrome.Network.setCacheDisabled(cacheDisabled=args.no_cache)
    chrome.Page.enable()
    #chrome.Performance.enable() # TODO check out using performance metrics

    # Run experiments
    loadTimes = dict() # id =>> pageload time
    for iteration in range(iterations):
        printv(f"On iteration {iteration}",args.verbose)
        for page in random.sample(sites,len(sites)):
            printv("\n\nOn page: " + page,args.verbose)
            if "http://www." not in page:
                page = "http://www." + page # chrome dev tools require a http:// starting address
            pageLoadId = genUniqueId()

            # Instruct chrome to navigate to page
            chrome.Page.navigate(url=page)

            # TODO - see if this is a good metric for page load time
            chrome.wait_event("Page.frameNavigated",timeout=120) # as soon as page is navigated
            pageStart = time.time()
            chrome.wait_event("Page.loadEventFired",timeout=120) # as soon as page is loaded
            pageEnd = time.time()

            # Record pageload times in ms
            loadTimes[pageLoadId] = (page,pageLoadId,pageStart, pageEnd, (pageEnd - pageStart)*1000)
            try:
                chrome.Page.stopLoading() # in case it is still loading, stop it so we can change mmap file
            except:
                pass


    printv("Waiting for chrome to finish",args.verbose)
    try:
        process.communicate(timeout=10) # wait for chrome to complete, or timeout
    except subprocess.TimeoutExpired:
        pass
    except Exception as e:
        printe(e)

    printv("Closing chrome",args.verbose)
    chrome.Browser.close()

# Write out page load times
with open(f'{expDirName}/pageloads.log','w') as results:
    for pageLoadId,pageLoadTime in loadTimes.items():
        results.write(str(loadTimes[pageLoadId]))
        results.write("\n")
