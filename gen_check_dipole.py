from pathlib import Path
from decimal import Decimal, getcontext
import shutil
import numpy as np
import pandas as pd
import os

#this script creates slurm bash files for check_after_one_run_dipole.py

def format_using_decimal(value, precision=15):
    # Set the precision higher to ensure correct conversion
    getcontext().prec = precision + 2
    # Convert the float to a Decimal with exact precision
    decimal_value = Decimal(str(value))
    # Normalize to remove trailing zeros
    formatted_value = decimal_value.quantize(Decimal(1)) if decimal_value == decimal_value.to_integral() else decimal_value.normalize()
    return str(formatted_value)


outPath="./bashFiles_checking_dipole/"

if os.path.isdir(outPath):
    shutil.rmtree(outPath)
Path(outPath).mkdir(exist_ok=True,parents=True)

N=10 #unit cell number
init_path_tot=30
startingFileIndSuggest=3
TVals=[5.7]


def contents_to_bash(T_ind,j):
    TStr=TVals[T_ind]
    out_sub_dir=outPath+f"/T{TStr}/"
    Path(out_sub_dir).mkdir(exist_ok=True,parents=True)
    outBashName=out_sub_dir+f"/check_after_one_run_dipole_T{TStr}_init_path{j}.sh"
    confFileName=f"./dataAll/N{N}/T{TStr}/init_path{j}/run_T{TStr}_init_path{j}.mc.conf"
    contents=[
        "#!/bin/bash\n",
        "#SBATCH -n 5\n",
        "#SBATCH -N 1\n",
        "#SBATCH -t 0-60:00\n",
        "#SBATCH -p hebhcnormal01\n"
        "#SBATCH --mem=10GB\n",
        f"#SBATCH -o out_check_dipole_T{TStr}_path{j}.out\n",
        f"#SBATCH -e out_check_dipole_T{TStr}_path{j}.err\n",
        "cd /public/home/hkust_jwliu_1/liuxi/Document/cppCode/check_ergodic_pbc_C3_fer\n",
        f"python3 -u check_after_one_run_dipole.py {confFileName} {startingFileIndSuggest}"
        ]

    with open(outBashName,"w+") as fptr:
        fptr.writelines(contents)



for T_ind in range(0,len(TVals)):
    for j in range(0,init_path_tot):
        contents_to_bash(T_ind,j)