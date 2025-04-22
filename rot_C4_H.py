import numpy as np

N0=12
N1=12
def double_ind_flattened(n0,n1):
    return n0*N1+n1


J=50
a=1
theta=np.pi/2
np.random.seed(37)

p_all=np.random.uniform(-1,1,N0*N1*2).reshape((-1,2))

# p_all=np.zeros(N0*N1*2).reshape((-1,2))
# n0=0
# n1=1
#
# m0=1
# m1=0
#
# a0=1
# a1=1
# p_all[double_ind_flattened(n0,n1),:]=np.array([7,8])
# p_all[double_ind_flattened(m0,m1),:]=np.array([9,10])
# p_all[double_ind_flattened(a0,a1),:]=np.array([15,16])
# print(f"p_all[double_ind_flattened(1,0),:]={p_all[double_ind_flattened(1,0),:]}")
# print(f"p_all[double_ind_flattened(1,1),:]={p_all[double_ind_flattened(1,1),:]}")


def H3(pi,pj,ri,rj):
    rij=rj-ri
    term1=J/np.linalg.norm(rij,ord=2)**2*np.dot(pi,pj)

    term2=-2*J/np.linalg.norm(rij,ord=2)**4 \
          *np.dot(pi,rij)*np.dot(pj,rij)
    # print(f"term1={term1}, term2={term2}")
    return term1+term2

def On01(n0,n1):
    return np.array([n0,n1])*a


U4=np.array([
    [np.cos(theta),-np.sin(theta)],
    [np.sin(theta),np.cos(theta)]
])
U4T=U4.T
print(U4)
C=np.array([0,0])


def C4_next(n0,n1):
    # O_tmp=On01(n0,n1)
    # O_next=C+U4@(O_tmp-C)
    # return int(O_next[0]/a),int(O_next[1]/a)
    # return -n1,n0 #C=0
    return 1-n1,n0

def C4_prev(n0,n1):
    # O_tmp=On01(n0,n1)
    # O_prev=C+U4T@(O_tmp-C)
    # return int(O_prev[0]/a), int(O_prev[1]/a)
    # return n1,-n0# center 0
    return n1,1-n0
#print rows of p_all
for ind, row in enumerate(p_all):
    if np.linalg.norm(row,2)>0:
        print(f"ind={ind}, p_all[ind,:]={p_all[ind,:]}, row={row}")
        n1=ind%N1
        n0=(ind-n1)//N1
        print(f"ind={ind}, n0={n0}, n1={n1}")

        print("********************")

pU_all=np.zeros(N0*N1*2).reshape((-1,2))
#iterate over indices of pU_all
# prev_set=set()
# for n0 in range(0,N0):
#     for n1 in range(0,N1):
#         n0_prev,n1_prev=C4_prev(n0,n1)
#         prev_set.add((n0_prev,n1_prev))
#
#
# print(len(prev_set))

def p_2_p_tilde(n0,n1):
    n0_tilde,n1_tilde=C4_prev(n0,n1)
    flat_ind=double_ind_flattened(n0_tilde%N0,n1_tilde%N1)
    p_tmp=p_all[flat_ind,:]
    # if np.linalg.norm(p_tmp)!=0:
    #     print(f"n0={n0}, n1={n1}, n0_tilde={n0_tilde}, n1_tilde={n1_tilde},  flat_ind={flat_ind}, p_tmp={p_tmp}")
    return U4@p_tmp




for n0 in range(0,N0):
    for n1 in range(0,N1):
        flat_ind=double_ind_flattened(n0,n1)
        row_tmp=p_2_p_tilde(n0,n1)
        if np.linalg.norm(row_tmp,2)>0:
            print(f"n0={n0}, n1={n1}, flat_ind={flat_ind}, row={row_tmp}")
        pU_all[flat_ind,:]=row_tmp



e1=0
width=2

for n0 in range(0,N0):
    for n1 in range(0,N1):
        for m0 in range(n0-width,n0+width+1):
            for m1 in range(n1-width,n1+width+1):
                if m0==n0 and m1==n1:
                    continue
                else:
                    rn0n1=On01(n0,n1)
                    rm0m1=On01(m0,m1)


                    flat_ind_left=double_ind_flattened(n0,n1)
                    flat_ind_right=double_ind_flattened(m0%N0,m1%N1)

                    p_left=p_all[flat_ind_left,:]
                    p_right=p_all[flat_ind_right,:]

                    e1+=H3(p_left,p_right,rn0n1,rm0m1)/2


e1U=0
for n0 in range(0,N0):
    for n1 in range(0,N1):
        for m0 in range(n0-width,n0+width+1):
            for m1 in range(n1-width,n1+width+1):
                if m0==n0 and m1==n1:
                    continue
                else:
                    rn0n1=On01(n0,n1)
                    rm0m1=On01(m0,m1)


                    flat_ind_left=double_ind_flattened(n0,n1)
                    flat_ind_right=double_ind_flattened(m0%N0,m1%N1)

                    pU_left=pU_all[flat_ind_left,:]
                    pU_right=pU_all[flat_ind_right,:]

                    e1U+=H3(pU_left,pU_right,rn0n1,rm0m1)/2


print(e1)
print(e1U)