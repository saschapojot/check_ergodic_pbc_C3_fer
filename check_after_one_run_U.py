import re
import subprocess
import sys

import json
argErrCode=2

if (len(sys.argv)!=3):
    print("wrong number of arguments")
    print("example: python check_after_one_run_U.py ./path/to/mc.conf startingFileIndSuggest")
    exit(argErrCode)


confFileName=str(sys.argv[1])

startingFileIndSuggest=int(sys.argv[2])
print(f"startingFileIndSuggest={startingFileIndSuggest}")

invalidValueErrCode=1
summaryErrCode=2
loadErrCode=3
confErrCode=4

#################################################
#parse conf, get jsonDataFromConf
confResult=subprocess.run(["python3", "./init_run_scripts/parseConf.py", confFileName], capture_output=True, text=True)
confJsonStr2stdout=confResult.stdout
# print(confJsonStr2stdout)
if confResult.returncode !=0:
    print("Error running parseConf.py with code "+str(confResult.returncode))
    # print(confResult.stderr)
    exit(confErrCode)


match_confJson=re.match(r"jsonDataFromConf=(.+)$",confJsonStr2stdout)
if match_confJson:
    jsonDataFromConf=json.loads(match_confJson.group(1))
else:
    print("jsonDataFromConf missing.")
    exit(confErrCode)
# print(jsonDataFromConf)
################################################

##################################################
#read summary file, get jsonFromSummary
parseSummaryResult=subprocess.run(["python3","./init_run_scripts/search_and_read_summary.py", json.dumps(jsonDataFromConf)],capture_output=True, text=True)
# print(parseSummaryResult.stdout)
if parseSummaryResult.returncode!=0:
    print("Error in parsing summary with code "+str(parseSummaryResult.returncode))
    # print(parseSummaryResult.stdout)
    # print(parseSummaryResult.stderr)
    exit(summaryErrCode)

match_summaryJson=re.match(r"jsonFromSummary=(.+)$",parseSummaryResult.stdout)
if match_summaryJson:
    jsonFromSummary=json.loads(match_summaryJson.group(1))
# print(jsonFromSummary)
##################################################

###############################################
#load previous data
#get loadedJsonData
loadResult=subprocess.run(["python3","./init_run_scripts/load_previous_data.py", json.dumps(jsonDataFromConf), json.dumps(jsonFromSummary)],capture_output=True, text=True)
if loadResult.returncode!=0:
    print("Error in loading with code "+str(loadResult.returncode))
    exit(loadErrCode)
# print("entering")
match_loadJson=re.match(r"loadedJsonData=(.+)$",loadResult.stdout)
if match_loadJson:
    loadedJsonData=json.loads(match_loadJson.group(1))
else:
    print("loadedJsonData missing.")
    exit(loadErrCode)
###############################################

##########################################################
#statistics
checkU_dipole_ErrCode = 5
# print("entering statistics")
# Start the subprocess
# print("jsonFromSummary="+json.dumps(jsonFromSummary))
# print("jsonDataFromConf="+json.dumps(jsonDataFromConf))
checkU_dipole_Process = subprocess.Popen(
    ["python3", "-u", "./oneTCheckObservables/check_U_OneT_pkl.py",
     json.dumps(jsonFromSummary), json.dumps(jsonDataFromConf),str(startingFileIndSuggest)],
    stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True
)
# return_code = checkU_dipole_Process.poll()
# if return_code is not None:
#     print(f"Process exited immediately with return code: {return_code}")

# Read output in real-time
while True:
    output = checkU_dipole_Process.stdout.readline()
    if output == '' and checkU_dipole_Process.poll() is not None:
        break
    if output:
        print(output.strip())

# Collect remaining output and error messages
stdout, stderr = checkU_dipole_Process.communicate()
# Check if the process was killed
if checkU_dipole_Process.returncode is not None:
    if checkU_dipole_Process.returncode < 0:
        # Process was killed by a signal
        print(f"checkU_dipole_Process was killed by signal: {-checkU_dipole_Process.returncode}")
    else:
        # Process exited normally
        print(f"checkU_dipole_Process exited with return code: {checkU_dipole_Process.returncode}")
else:
    print("checkU_dipole_Process is still running")
# Print any remaining standard output
if stdout:
    print(stdout.strip())

# Handle errors and print the return code if there was an error
if stderr:
    print(f"checkU_dipole_Process return code={checkU_dipole_Process.returncode}")
    print(stderr.strip())

##########################################################
