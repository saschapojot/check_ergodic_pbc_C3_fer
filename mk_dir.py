from pathlib import Path
from decimal import Decimal, getcontext
from math import factorial



def format_using_decimal(value, precision=15):
    # Set the precision higher to ensure correct conversion
    getcontext().prec = precision + 2
    # Convert the float to a Decimal with exact precision
    decimal_value = Decimal(str(value))
    # Normalize to remove trailing zeros
    formatted_value = decimal_value.quantize(Decimal(1)) if decimal_value == decimal_value.to_integral() else decimal_value.normalize()
    return str(formatted_value)



N=10 #unit cell number
N0=N
N1=N
if N%2==0:
    Nx=(N-1)//2
    Ny=(N-1)//2

else:
    Nx=N//2
    Ny=N//2

Nx-=2
Ny-=2

TVals=[5.8]
default_flush_num=60
#lattice const
a=2
#charge
q=2
dataRoot="./dataAll/"


dataOutDir=dataRoot
effective_data_num_required=5000
#alpha1: 1/(1!2!)
const_multiple=1000
sweep_to_write=500
sweep_multiple=700
alpha1_coef=1/(factorial(1)*factorial(2))
alpha1=alpha1_coef*const_multiple
#alpha2: 1/(1!2!)
alpha2_coef=1/(factorial(1)*factorial(2))
alpha2=alpha2_coef*const_multiple
#alpha3: 1/(2!4!)
alpha3_coef=1/(factorial(2)*factorial(4))
alpha3=alpha3_coef*const_multiple
#alpha4: 1/(1!1!2!2!)
alpha4_coef=1/(factorial(1)*factorial(1)*factorial(2)*factorial(2))
alpha4=alpha4_coef*const_multiple
#alpha5: 1/2!

alpha5_coef=1/factorial(2)
alpha5=alpha5_coef*const_multiple
#alpha6: 1/4!
alpha6_coef=1/factorial(4)
alpha6=alpha6_coef*const_multiple
#alpha7: 1/6!
alpha7_coef=1/factorial(6)
alpha7=alpha7_coef*const_multiple

J=1/5*alpha5

print(f"alpha1={alpha1}, alpha2={alpha2}, alpha3={alpha3}, alpha4={alpha4}, alpha5={alpha5}, alpha6={alpha6}, alpha7={alpha7}")
print(f"J={J}")
print(f"Nx={Nx}")
print(f"Ny={Ny}")
NStr=format_using_decimal(N)
aStr=format_using_decimal(a)
qStr=format_using_decimal(q)
alpha1_Str=format_using_decimal(alpha1)
alpha2_Str=format_using_decimal(alpha2)
alpha3_Str=format_using_decimal(alpha3)
alpha4_Str=format_using_decimal(alpha4)
alpha5_Str=format_using_decimal(alpha5)
alpha6_Str=format_using_decimal(alpha6)
alpha7_Str=format_using_decimal(alpha7)
init_path_tot=100

J_Str=format_using_decimal(J)
h=0.05
TDirsAll=[]
TStrAll=[]

for k in range(0,len(TVals)):
    T=TVals[k]
    # print(T)

    TStr=format_using_decimal(T)
    TStrAll.append(TStr)


def contents_to_conf(k,which_init_ind):
    contents=[
        "#This is the configuration file for mc computations\n",
        "#System has C3\n",
        "\n" ,
        "#parameters\n",
        "\n",
        "#Temperature\n",
        "T="+TStrAll[k]+"\n",
        "#which init path\n",
        f"init_path={which_init_ind}\n",
        "\n",
        f"alpha1={alpha1_Str}\n",
        "\n",
        f"alpha2={alpha2_Str}\n",
        "\n",
        f"alpha3={alpha3_Str}\n",
        "\n",
        f"alpha4={alpha4_Str}\n",
        "\n",
        f"alpha5={alpha5_Str}\n",
        "\n",
        f"alpha6={alpha6_Str}\n",
        "\n",
        f"alpha7={alpha7_Str}\n",
        "\n",
        f"J={J_Str}\n",
        "\n",
        f"N={NStr}\n",
        "\n",
        f"N_half_side={Nx}\n",
        "\n",
        f"a={aStr}\n",
        "\n",
        f"q={qStr}\n",
        "\n",
        "erase_data_if_exist=False\n",
        "\n",
        "search_and_read_summary_file=True\n"
        "\n",
        "#For the observable name, only digits 0-9, letters a-zA-Z, underscore _ are allowed\n",
        "\n",
        "observable_name=U_dipole\n",
        "\n",
        f"effective_data_num_required={effective_data_num_required}\n",
        "\n",
        "#this is the data number in each pkl file, i.e., in each flush\n"
        f"sweep_to_write={sweep_to_write}\n",
        "\n",
        "#within each flush,  sweep_to_write*sweep_multiple mc computations are executed\n",
        "\n",
        f"default_flush_num={default_flush_num}\n",
        "\n",
        f"h={h}\n",
        "\n",
        "#the configurations of the system are saved to file if the sweep number is a multiple of sweep_multiple\n",
        "\n",
        f"sweep_multiple={sweep_multiple}\n",
        ]
    outDir=dataOutDir+f"/N{NStr}/T{TStrAll[k]}/init_path{which_init_ind}/"
    Path(outDir).mkdir(exist_ok=True,parents=True)
    outConfName=outDir+f"/run_T{TStrAll[k]}_init_path{which_init_ind}.mc.conf"
    with open(outConfName,"w+") as fptr:
        fptr.writelines(contents)


for k in range(0,len(TVals)):
    for j in range(0,init_path_tot):
        contents_to_conf(k,j)
