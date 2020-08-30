#!/usr/local/bin/python3
from os import getenv
from glob import glob
from time import mktime, strptime
import numpy as np


def process_line(line,verbose=False):
    if (len(line) < 8 or len(line) > 10 or line[6] == "PageLoadTime"):
        return [None,0]

    date = line.pop(0)
    time = line.pop(0)
    date_str = date + " " + time

    try:
        ms = mktime(strptime(date_str,"%Y/%m/%d %H:%M:%S")) * 1000 + float(line.pop(0))
    except Exception as e: # unable to parse float means invalid line
        if verbose:
            print(e)
        return [None,0]

    line.pop(0) # remove INFO
    line.pop(0) # remove file

    tid = line.pop(0).replace(':','')

    func = line.pop(0)

    mask = line.pop(0)

    core = line.pop(0)

    try:
        test = int(mask)
        test = int(core)
        if ("." in func or "/" in func or "[" in func or "]" in func):
            raise Exception("Invalid function name: " + func)
    except Exception as e:
        if verbose:
            print(e)
        return [None, 0]
    return [(ms,tid,func,mask,core),len(func)]

def gen_data_matrix(trimLatency=False):
    func_name_length = 0
    data_mat = []
    filenames = getenv("DATA_FILES")
    if filenames is None:
        raise Exception("DATA_FILES not specified")

    elif type(filenames) is str and "*" in filenames: # unexpanded glob
        glob_pattern = filenames
        filenames = list(glob(glob_pattern))

        if (len(filenames) == 0):
            raise Exception("DATA_FILES contains empty pattern: " + glob_pattern)

    elif type(filenames) is str:
        filenames = filenames.split()

    for filename in filenames:
        print(filename)
        with open(filename) as f:
            for i,line in enumerate(f):
                if (i < 4): # skip first 4 lines, header info
                    continue

                line = line.split() # split on all whitespace
                if trimLatency and (len(line)) == 9:
                    line.pop() # remove latency

                line,f_len = process_line(line)
                if line is not None:
                    data_mat.append(line)
                func_name_length = max(f_len,func_name_length)

    entrytype = np.dtype({'names':['time_ms','tid','func_name','mask','core'],
                         'formats':[np.float64,np.uint16,'S'+str(func_name_length),'S8',np.uint8]})
    data_np = np.array(data_mat,dtype=entrytype)
    data_np = data_np[data_np['time_ms'].argsort()]
    return data_np

if __name__ == "__main__":
    data = gen_data_matrix()
