import numpy as np

import matplotlib.pyplot as plt


#this script plots C3 neighbors


Nx=2

Ny=2

S_O=[(n0,n1) for n0 in range(-Nx,Nx+1) for n1 in range(-Ny,Ny+1)]
S_D=[(n0,n1) for n0 in range(-Nx,Nx+1) for n1 in range(-Ny,Ny+1)]
def iter_O_U6(n0,n1):
    # n0,n1=n0n1

    n0Next=n0-n1
    n1Next=n0
    return (n0Next,n1Next)
def iter_O_U3(n0,n1):
    n0Next=-n1-1
    n1Next=n0-n1-1

    return n0Next,n1Next



R_O=set()
for n0n1 in S_O:
    n0Next,n1Next=n0n1
    for _ in range(0,6):
        n0Next,n1Next=iter_O_U6(n0Next,n1Next)
        R_O.add((n0Next,n1Next))


R_D=set()
for n0n1 in S_D:
    n0Next,n1Next=n0n1
    for _ in range(0,3):
        n0Next,n1Next=iter_O_U3( n0Next,n1Next)
        R_D.add((n0Next,n1Next))

a=1
def On0n1(n0,n1):
    return np.array([a*n0-1/2*a*n1,np.sqrt(3)/2*a*n1])

def Dn0n1(n0,n1):
    return np.array([a*n0-1/2*a*n1,np.sqrt(3)/2*a*n1+np.sqrt(3)/3*a])
plt.figure()
for point in R_O:
    n0,n1=point
    O_tmp=On0n1(n0,n1)
    plt.scatter(O_tmp[0],O_tmp[1],color="red",marker="o",s=5)

plt.savefig("O.png")
plt.close()

plt.figure()
plt.scatter(0,0,color="red",marker="o",s=5)
for point in R_D:
    n0,n1=point
    D_tmp=Dn0n1(n0,n1)
    plt.scatter(D_tmp[0],D_tmp[1],color="green",marker="o",s=5)
plt.savefig("D.png")
plt.close()



S_O2=[(n0,n1) for n0 in range(-Nx,Nx+1) for n1 in range(-Ny,Ny+1)]
S_D2=[(n0,n1) for n0 in range(-Nx,Nx+1) for n1 in range(-Ny,Ny+1)]

def iter_U3_around_D00(n0,n1):
    n0Next=-n1+1
    n1Next=n0-n1+1
    return n0Next,n1Next

def iter_U6_around_D00(n0,n1):
    n0Next=n0-n1
    n1Next=n0
    return n0Next,n1Next
R_O2=set()
for point in S_O2:
    n0Next,n1Next=point
    for _ in range(0,3):
        n0Next,n1Next=iter_U3_around_D00(n0Next,n1Next)
        R_O2.add((n0Next,n1Next))


D00=Dn0n1(0,0)
plt.figure()
plt.scatter(D00[0],D00[1],color="red",marker="o",s=5)
for point in R_O2:
    n0,n1=point
    O_tmp=On0n1(n0,n1)
    plt.scatter(O_tmp[0],O_tmp[1],color="green",marker="o",s=5)
plt.savefig("O_round_D00.png")
plt.close()


R_D2=set()
for point in S_D2:
    n0Next,n1Next=point
    for _ in range(0,6):
        n0Next,n1Next=iter_U6_around_D00(n0Next,n1Next)
        R_D2.add(( n0Next,n1Next))

plt.figure()

for point in R_D2:
    n0,n1=point
    D_tmp=Dn0n1(n0,n1)
    plt.scatter(D_tmp[0],D_tmp[1],color="blue",marker="o",s=5)
plt.scatter(D00[0],D00[1],color="red",marker="o",s=5)
plt.savefig("D_round_D00.png")
plt.close()
