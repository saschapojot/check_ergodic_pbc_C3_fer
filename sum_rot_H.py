import numpy as np



theta=2*np.pi/3


J=50
np.random.seed(31)
def H3(pi,pj,ri,rj):
    rij=rj-ri
    term1=J/np.linalg.norm(rij,ord=2)**2*np.dot(pi,pj)

    term2=-2*J/np.linalg.norm(rij,ord=2)**4\
            *np.dot(pi,rij)*np.dot(pj,rij)
    # print(f"term1={term1}, term2={term2}")
    return term1+term2

U3=np.array([
    [np.cos(theta),-np.sin(theta)],
    [np.sin(theta),np.cos(theta)]
])
U3T=U3.T

N0=6
N1=6
a=1
# p_all=np.random.uniform(-1,1,N0*N1*2).reshape((-1,2))
p_all=np.zeros(N0*N1*2).reshape((-1,2))

def double_ind_flattened(n0,n1):
    return n0*N1+n1
def On0n1(n0,n1):
    return np.array([a*n0-1/2*a*n1,np.sqrt(3)/2*a*n1])
def C(n0,n1):
    n0Next=-n1+1
    n1Next=n0-n1
    return n0Next,n1Next

def C_inv(n0,n1):
    n0Next=-n0+n1+1
    n1Next=1-n0
    return n0Next,n1Next
def p_2_p_tilde(n0,n1):
    """


    :param n0:
    :param n1:
    :return:
    """
    n0_tilde=-n0+n1+1

    n1_tilde=1-n0

    flat_ind=double_ind_flattened(n0_tilde%N0,n1_tilde%N1)

    p_tmp=p_all[flat_ind,:]

    return U3@p_tmp

n0=0
n1=1
m0=2
m1=0
p_all[double_ind_flattened(n0,n1),:]=np.array([7,8])
p_all[double_ind_flattened(m0,m1),:]=np.array([9,10])
n0_tilde=-n0+n1+1
n1_tilde=1-n0
print(f"n0={n0}, n1={n1}")
print(f"n0_tilde={n0_tilde}, n1_tilde={n1_tilde}")
m0_tilde=-m0+m1+1
m1_tilde=1-m0
print(f"m0={m0}, m1={m1}")
print(f"m0_tilde={m0_tilde}, m1_tilde={m1_tilde}")

print(f"p_all[double_ind_flattened(n0,n1),:]={p_all[double_ind_flattened(n0,n1),:]}")
print(f"p_all[double_ind_flattened(m0,m1),:]={p_all[double_ind_flattened(m0,m1),:]}")


pU_all=np.zeros(N0*N1*2).reshape((-1,2))
for n0 in range(0,N0):
    for n1 in range(0,N1):

        n0_tilde=-n0+n1+1
        n1_tilde=1-n0
        flat_ind=double_ind_flattened(n0,n1)
        flat_indU=double_ind_flattened(n0_tilde%N0,n1_tilde%N1)
        pU_all[flat_ind,:]=U3@p_all[flat_indU,:]



for ind, row in enumerate(pU_all):
    if np.linalg.norm(row,2)>0:
        print(f"ind={ind}, pU_all[ind,:]={pU_all[ind,:]}, U3T@row={U3T@row}")
        n1=ind%N1
        n0=(ind-n1)//N1
        print(f"ind={ind}, n0={n0}, n1={n1}")

n0_prev,n1_prev=C_inv(0,5)

flat_prev=double_ind_flattened(n0_prev%N0,n1_prev%N1)

O1=On0n1(0,1)
O2=On0n1(2,0)
r12=np.linalg.norm(O2-O1,2)

O1_tilde=On0n1(0,-1)
O2_tilde=On0n1(1,2)
r12_tilde=np.linalg.norm(O2_tilde-O1_tilde,2)

print(f"r12={r12}")
print(f"r12_tilde={r12_tilde}")