import subprocess
import sys
import os
import uuid
import argparse
import random

def genUniqueId():
    """Generate a unique 8 character id string"""
    return str(uuid.uuid4()).replace("-","")[0:8]

def findProcess(procName):
    """Return a list of active PIDs for a given process name"""
    p = subprocess.Popen(['pgrep',procName],stdout=subprocess.PIPE)
    output, _ = p.communicate()
    output = str(output,'UTF-8').rstrip().split("\n")
    return output

def procIsAlive(procName):
    """Return whether or not a process with the name procName exists"""
    return len(findProcess(procName)) > 0

def fileType(filename):
    """Check whether a file exists (for use with argparse)"""
    if os.path.exists(filename):
        return filename
    raise argparse.ArgumentTypeError(f"invalid file '{filename}'")

def ldPreloadFile(filename):
    """Check whether a file exists or is None (for use with argparse)"""
    if os.path.exists(filename) or filename == "None":
        return filename
    raise argparse.ArgumentTypeError(f"invalid file '{filename}'")

def waitForInput(msg):
    """Print a message and wait for the user to continue execution"""
    print(msg)
    response = input("'y' to continue, 'q' to quit: ").lower()
    while  response != 'y' and response != 'q':
        response = input("'y' to start chrome dev tools, 'q' to quit: ").lower()
    if response == 'q':
        sys.exit(0)

def printv(msg,verbose):
    """Print a message if verbose is True"""
    if verbose:
        print(f"{sys.argv[0]}: " + str(msg))

def printe(msg):
    """Print an error and exit"""
    print(f"{sys.argv[0]}: Encountered an error:  " + str(msg))
    sys.exit(1)

def maskFromConfig(config):
    """
    Converts a config into a mask for use with sched_setaffinity. 

    Parameters
    ----------
    config : (little,big)
        The number of little and big cores active (1) in the mask.


    Returns
    -------
    str
        A mask of the form 'XXXXXXXX' with all X's being either 1 or 0.
    """

    lils = [ "1" for x in range(config[0]) ] + [ "0" for x in range(4 - config[0]) ]
    bigs = [ "1" for x in range(config[1]) ] + [ "0" for x in range(4 - config[1]) ]
    random.shuffle(lils)
    random.shuffle(bigs)
    return "".join(lils) + "".join(bigs)

