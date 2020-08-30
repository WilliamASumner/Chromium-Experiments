import random
from .convenience import maskFromConfig
class FunctionSet:
    """
    A set of functions that are associated with specific entry and exit configurations.

    Attributes
    ----------
    entryConfig : str
        The configuration to be used when a function in the set is entered.

    exitConfig : str
        The configuration to be used when a function in the set is exited.

    functions : [str, ... ,str]
        The function names in the set.

    """

    def __init__(self,entryConfig,exitConfig,functions):
        self._entryConfig = entryConfig
        self._exitConfig = exitConfig
        self.functions = set(functions)
        self._offset = None # update when written out

    def __len__(self):
        size = len(entryConfig) + len(exitConfig) + 2 # newlines
        size += len(str(len(functions))) + 1
        for f in functions:
            size += len(f) + 1

    def getOffset(self):
        if (self._offset is None):
            raise ValueError("Offset was not set")
        return self._offset

    def setOffset(self,offset):
        if (offset is None or type(offset) != int or offset < 0):
            raise ValueError("Offset must be an integer > 0")
        if self._offset is None:
            self._offset = offset
        else:
            raise ValueError("Offset is already set")

    def bytes(self):
        entryMask = maskFromConfig(self._entryConfig)
        exitMask  = maskFromConfig(self._exitConfig)
        nl = '\n'
        string = f"{entryMask}\n{exitMask}\n{len(self.functions)}\n{nl.join([f for f in self.functions])}\n"
        return bytes(string,"UTF-8")

    def __len__(self):
        return len(self.bytes())

    def getEntryMask(self):
        return maskFromConfig(self._entryConfig)

    def getExitMask(self):
        return maskFromConfig(self._exitConfig)

class ExperimentInterface:
    """
    A list of functionSets and an mmap used for use as an interface for IPC with the C++ experimental framework.

    Attributes
    ----------
    functionSetList : [FunctionSet,...,FunctionSet]
        List of FunctionSets to control affinity settings for a given experiment.

    globalSet : set(str,...,str)
        The global union of all FunctionSets, this is used to ensure each function is assigned to at most one FunctionSet.

    ipcFile : File
        File to be used for the ipc.

    ipcWritable : boolean
        True if `initIpc` method has been called.

    semaphore : posix_ipc.Semaphore
        A system-wide semaphore for IPC synchronization
    """

    def __init__(self):
        """ Create new ExperimentInterface """
        self.functionSetList = []
        self.globalSet = set()
        self.ipcFile = None
        self.ipcWritable = False
        self.semaphore = None

    def __del__(self):
        if self.semaphore is not None:
            self.semaphore.close()

    def setIpcFile(self,mFile):
        """ Assign file to be used for IPC """
        if self.ipcFile is not None:
            raise RuntimeError("Error: cannot change existing IPC file of ExperimentalInterface")
        self.ipcFile = mFile

    def addSet(self,fSet):
        """ Public function for adding a FunctionSet to the interface """
        if self.ipcWritable:
            raise RuntimeError("Error: cannot add sets after IPC is initialized")

        if type(fSet) == list:
            for entry in fSet:
                if type(entry) != FunctionSet:
                    raise TypeError("Invalid type" + str(type(entry)) + " expected FunctionSet")
                self._addSet(entry)
        elif type(fSet) == FunctionSet:
            self._addSet(fSet)
        else:
            raise TypeError("Invalid type" + str(type(fSet)) + " expected FunctionSet")

    def _addSet(self,fSet):
        if self.globalSet.intersection(fSet.functions) != set():
            raise ValueError(f"Error: values {self.globalSet.intersection(fSet.functions)} already assigned to another set")

        self.functionSetList.append(fSet)
        self.globalSet = self.globalSet.union(fSet.functions)

    def initIpc(self,ipcFile,semName = "/sem-chrome-ipc"):
        """ Initialize mmap with functionSets """
        if self.ipcWritable:
            raise RuntimeError("Error: cannot re-initialize IPC for an ExperimentalInterface")
        self.ipcFile = ipcFile
        self.ipcWritable = True

        #self.semaphore = posix_ipc.Semaphore(semName) Using semaphore
        #self.semaphore.acquire()
        self.ipcFile.seek(0)
        self.ipcFile.write(bytes(str(len(self.functionSetList)) + "\n","UTF-8"))
        for fs in self.functionSetList:
            self.ipcFile.write(fs.bytes())
            fs.setOffset(self.ipcFile.tell())
        #self.semaphore.release()

    def getSize(self):
        return sum([len(fSet) for fSet in self.functionSetList]) + len(bytes(str(len(self.functionSetList)),"UTF-8")) + 1 # include EOF

    def randomizeMasks(self):
        """ Randomize affinity masks to be used by interposition framework"""
        if not self.ipcWritable:
            raise RuntimeError("Error: ExperimentalInterface IPC must be initialized first")
        offset = len(str(len(self.functionSetList))) + 1
        for fs in self.functionSetList:
            entryMask = fs.getEntryMask()
            exitMask  = fs.getExitMask()
            self.ipcFile.seek(offset)
            self.ipcFile.write(bytes(entryMask + "\n","UTF-8"))
            self.ipcFile.write(bytes(exitMask + "\n","UTF-8"))
            offset = fs.getOffset()

    # Decorators for experiment customization

# Phases and function names we're working with
html   = ["PumpPendingSpeculations","ResumeParsingAfterYield"]
css    = ["ParseSheet","UpdateStyleAndLayoutTree"]
layout = ["PerformLayout"]
paint  = ["UpdateLifecyclePhasesInternal"]
js     = ["ExecuteScriptInMainWorld","ExecuteScriptInIsolatedWorld","CallFunction"]

# Sample FunctionSets
littleToAll = FunctionSet((4,0),(4,4),html + css + layout)
bigToAll    = FunctionSet((0,4),(4,4), paint + js)
other       = FunctionSet((0,2),(1,2), ["MyRandomFunction","MyOtherFunction"])
otherTwo    = FunctionSet((2,2),(4,2), ["FunctionUno","FunctionDos"])

# Simple test
if __name__ == "__main__":
    with open("test.txt","wb",buffering=0) as f:
        print("Writing to test.txt")
        e = ExperimentInterface()
        print(f"Total size is {e.getSize()}")

        try:
            e.initIpc()
        except Exception as exc: # cannot init IPC without file
            pass

        print(f"Adding littleToAll set (size {len(littleToAll)})")
        e.addSet(littleToAll)
        print(f"Adding bigToAll set (size {len(bigToAll)})")
        e.addSet(bigToAll)
        print(f"Adding other set (size {len(other)})")
        e.addSet(other)
        print(f"Total size is {e.getSize()}")

        try:
            e.addSet(bigToAll)
        except ValueError as v: # cannot add sets with same function names
            pass

        e.initIpc(f)

        try:
            e.addSet(otherTwo)
        except RuntimeError as r: # cannot add sets after initialization
            pass


        input("Check values and press enter to continue") # wait so we can verify results in test.txt are changing
        e.randomizeMasks()

        print("\nCheck that values changed")
