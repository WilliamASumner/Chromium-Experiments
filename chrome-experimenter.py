import PyChromeDevTools # interacting with chrome
import subprocess       # running chrome
import os               # environment vars
import uuid             # unique ids
import random           # core config permutations
import time             # pageLoad timing


### Program Layout
"""
Iteration - one run through all sites with specified core configurations
Experiment - collection of all samples

1) Start chrome process
2) Generate a sample -> (site list and core config list)
3) Record results and redo 2 until all iterations are complete

"""
###

#Runtime vars
iterations=1

# Environment variables
filePrefix = "PyChrome" # label with which tool generated the data
os.environ['TIMING'] = "external" # don't use internal timer (we'll kill chrome)
os.environ['IPC'] = "on" # use mmap file to communicate dynamically rather than env vars
os.environ['LD_PRELOAD'] = f"{filePrefix}-{genUniqueID()}" # TODOTODOTODO
os.environ['LD_LIBRARY_PATH'] = f"{filePrefix}-{genUniqueID()}"
chromeDir = "/home/vagrant/chromium/src/out/x64Linux"
chromeFlags = "--no-zygote --no-sandbox"

# Flag options
verbose = ""
info = ""
gui = ""
debugPort = 9222
remoteDebug = f"--remote-debugging-port={debugPort}"
url= f"-w {url}"
chromeCmd = ["./chrome",info,verbose,gui,remoteDebug,url]
chromeCmd = ["sleep", "3"]

#Config and Sites
sites       = ["cnn.com","amazon.com","twitter.com","wikipedia.org"]
coreConfigs = [(4,4),(2,2),(0,4),(4,0)] # (lil,big)
littles     = [0,1,2,3]
bigs        = [4,5,6,7]

def genUniqueId():
    return str(uuid.uuid4()).replace("-","")[0:8]


with open(f'{filePrefix}-chrome-err-log.txt','a') as errlog, \
     open(f'{filePrefix}-chrome-log.txt','a') as log:
    print(chromeCmd)

    process = subprocess.Popen(chromeCmd)#,stdout=log,stderr=errlog) # Start the chrome process

    chrome = PyChromeDevTools.ChromeInterface(port=debugPort)
    chrome.Network.enable()
    chrome.Page.enable()

    loadTimes = new dict() # id =>> pageload time

    for iteration in range(iterations):
        for page in random.sample(sites,len(sites)):
            pageLoadId = genUniqueId()
            os.environ['LOG_FILE'] = f"{filePrefix}-{pageLoadId}"

            # Actual navigation
            chrome.Page.navigate(url=url)
            pageStart = time.time()
            chrome.wait_event("Page.loadEventFired",timeout=60)
            pageEnd = time.time()

            # Record pageload times
            loadTimes[pageLoadId] = (pageEnd,pageEnd - pageStart)

with open(f'{filePrefix}-experiment-{experimentId}.log','w') as results:
    for pageLoadTime,pageLoadId in enumerate(loadTimes)
        results.write(loadTimes[pageLoadId])
        results.write("\n")
