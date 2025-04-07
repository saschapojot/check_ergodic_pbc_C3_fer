import numpy as np
from datetime import datetime
import statsmodels.api as sm
import sys
import re
import warnings


from scipy.stats import ks_2samp
import glob

import os
import json
import pickle



#This script checks if U values reach equilibrium and writes summary file of U
#This file checks pkl files
argErrCode=2
sameErrCode=3
missingErrCode=4
eps_for_auto_corr=1e-2

jsonFromSummaryLast=json.loads(sys.argv[1])
jsonDataFromConf=json.loads(sys.argv[2])

startingFileIndSuggest=int(sys.argv[3])
TDirRoot=jsonFromSummaryLast["TDirRoot"]
U_dipole_dataDir=jsonFromSummaryLast["U_dipole_dataDir"]
effective_data_num_required=int(jsonDataFromConf["effective_data_num_required"])


N=int(jsonDataFromConf["N"])
sweep_to_write=int(jsonDataFromConf["sweep_to_write"])
summary_U_File=TDirRoot+"/summary_U.txt"


def sort_data_files_by_flushEnd(oneDir):
    dataFilesAll=[]
    flushEndAll=[]
    for oneDataFile in glob.glob(oneDir+"/flushEnd*.pkl"):
        dataFilesAll.append(oneDataFile)
        matchEnd=re.search(r"flushEnd(\d+)",oneDataFile)
        if matchEnd:
            indTmp=int(matchEnd.group(1))
            flushEndAll.append(indTmp)

    endInds=np.argsort(flushEndAll)
    sortedDataFiles=[dataFilesAll[i] for i in endInds]
    return sortedDataFiles


def parseSummary_U():
    startingFileInd=-1
    summaryFileExists=os.path.isfile(summary_U_File)
    if summaryFileExists==False:
        return startingFileInd
    with open(summary_U_File,"r") as fptr:
        lines=fptr.readlines()
    for oneLine in lines:
        #match startingFileInd
        matchStartingFileInd=re.search(r"startingFileInd=(\d+)",oneLine)
        if matchStartingFileInd:
            startingFileInd=int(matchStartingFileInd.group(1))
    return startingFileInd




def auto_corrForOneVec(vec):
    """

    :param colVec: a vector of data
    :return:
    """
    same=False
    NLags=int(len(vec)*3/4)
    with warnings.catch_warnings():
        warnings.filterwarnings("error")
    try:
        acfOfVec=sm.tsa.acf(vec,nlags=NLags)
    except Warning as w:
        same=True

    acfOfVecAbs=np.abs(acfOfVec)
    minAutc=np.min(acfOfVecAbs)
    lagVal=-1
    if minAutc<=eps_for_auto_corr:
        lagVal=np.where(acfOfVecAbs<=eps_for_auto_corr)[0][0]

    return same,lagVal


def ksTestOneColumn(vec,lag):
    """

    :param vec: a vector of data
    :param lag: auto-correlation length
    :return:
    """
    vecSelected=vec[::lag]
    lengthTmp=len(vecSelected)
    if lengthTmp%2==1:
        lengthTmp-=1

    lenPart=int(lengthTmp/2)
    vecToCompute=vecSelected[-lengthTmp:]

    #ks test
    selectedVecPart0=vecToCompute[:lenPart]
    selectedVecPart1=vecToCompute[lenPart:]
    result=ks_2samp(selectedVecPart0,selectedVecPart1)

    return result.pvalue,result.statistic, lenPart*2



def checkUDataFilesForOneT(UData_dir):
    U_sortedDataFilesToRead=sort_data_files_by_flushEnd(UData_dir)
    if len(U_sortedDataFilesToRead)==0:
        print("no data for U.")
        exit(0)

    startingFileInd=parseSummary_U()
    if startingFileInd<0:
        #we guess that the equilibrium starts at this file
        startingFileInd=startingFileIndSuggest

    startingFileName=U_sortedDataFilesToRead[startingFileInd]
    with open(startingFileName,"rb") as fptr:
        inArrStart=pickle.load(fptr)

    U_arr=inArrStart
    #read the rest of the pkl files
    for pkl_file in U_sortedDataFilesToRead[(startingFileInd+1):]:
        with open(pkl_file,"rb") as fptr:
            inArr=pickle.load(fptr)
        U_arr=np.append(U_arr,inArr)

    sameUTmp,lagUTmp=auto_corrForOneVec(U_arr)

    if sameUTmp==True or lagUTmp==-1:
        return [sameUTmp,lagUTmp,-1,-1,-1,-1]

    pUTmp,statUTmp,lengthUTmp=ksTestOneColumn(U_arr,lagUTmp)
    numDataPoints=lengthUTmp

    return [sameUTmp,lagUTmp,pUTmp,statUTmp,numDataPoints,startingFileInd]


UDataDir=U_dipole_dataDir+"/U/"
sameVec=[]
lagVec=[]
pVec=[]
statVec=[]
numDataVec=[]
print("checking U")
sameUTmp,lagUTmp,pUTmp,statUTmp,numDataPointsU,startingFileInd=checkUDataFilesForOneT(UDataDir)
sameVec.append(sameUTmp)
lagVec.append(lagUTmp)
pVec.append(pUTmp)
statVec.append(statUTmp)
numDataVec.append(numDataPointsU)
print("lagU="+str(lagUTmp))

lagVecAll=[lagUTmp]
lagMax=np.max(lagVecAll)
numDataPoints=np.min([numDataPointsU])

print("lagMax="+str(lagMax))
print("numDataPoints="+str(numDataPoints))
print(f"pVec={pVec}")
print(f"statVec={statVec}")
statThreshhold=0.1
if pVec[0]==True:
    with open(summary_U_File,"w+") as fptr:
        msg="error: same\n"
        fptr.writelines(msg)
        exit(sameErrCode)


if numDataPoints<0:
    msg="high correlation"
    with open(summary_U_File,"w+") as fptr:
        fptr.writelines(msg)
    exit(0)

if (np.min(pVec)>=0.01 or np.max(statVec)<=statThreshhold) and numDataPoints>=200:
    if numDataPoints>=effective_data_num_required:
        newDataPointNum=0
    else:
        newDataPointNum=effective_data_num_required-numDataPoints

    msg="equilibrium\n" \
        +"lag="+str(lagMax)+"\n" \
        +"numDataPoints="+str(numDataPoints)+"\n" \
        +"startingFileInd="+str(startingFileInd)+"\n" \
        +"newDataPointNum="+str(newDataPointNum)+"\n" \
        +"sweep_to_write="+str(sweep_to_write)+"\n"

    print(msg)
    with open(summary_U_File,"w+") as fptr:
        fptr.writelines(msg)
    exit(0)


#continue
continueMsg="continue\n"
if not (np.min(pVec)>=0.01 or np.max(statVec)<=statThreshhold):
    continueMsg+="stat value: "+str(np.max(statVec))+"\n"
    continueMsg+="p value: "+str(np.min(pVec))+"\n"


if numDataPoints<200:
    #not enough data number

    continueMsg+="numDataPoints="+str(numDataPoints)+" too low\n"
    continueMsg+="lag="+str(lagMax)+"\n"
print(continueMsg)
with open(summary_U_File,"w+") as fptr:
    fptr.writelines(continueMsg)
exit(0)