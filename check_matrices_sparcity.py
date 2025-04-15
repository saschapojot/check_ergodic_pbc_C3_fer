import numpy as np
import matplotlib.pyplot as plt
from pathlib import Path

#this script checks how sparse the interaction matrices are

N=20 #unit cell number
N0=N
N1=N
if N%2==0:
    Nx=(N-1)//2
    Ny=(N-1)//2

else:
    Nx=N//2
    Ny=N//2
Nx-=4
Ny-=4
print(f"Nx={Nx}")
print(f"Ny={Ny}")

out_pic_dir=f"./matrix_sparcity/N{N}/"
Path(out_pic_dir).mkdir(exist_ok=True,parents=True)
def double_ind_2_flat_ind(n0,n1):
    """

    :param n0:
    :param n1:
    :return: flattened index for interaction matrix
    """
    return n0 * N1 + n1

def iter_O_U6(n0,n1):
    # n0,n1=n0n1

    n0Next=n0-n1
    n1Next=n0
    return n0Next,n1Next


def iter_O_U3(n0,n1):
    n0Next=-n1-1
    n1Next=n0-n1-1

    return n0Next,n1Next

S_O1=[(n0,n1) for n0 in range(-Nx,Nx+1) for n1 in range(-Ny,Ny+1)]
S_D1=[(n0,n1) for n0 in range(-Nx,Nx+1) for n1 in range(-Ny,Ny+1)]

R_O1=set()
for point in S_O1:
    n0Next,n1Next=point
    for _ in range(0,6):
        n0Next,n1Next=iter_O_U6(n0Next,n1Next)
        R_O1.add((n0Next,n1Next))

R_D1=set()
for point in S_D1:
    n0Next,n1Next=point
    for _ in range(0,3):
        n0Next,n1Next=iter_O_U3(n0Next,n1Next)
        R_D1.add((n0Next,n1Next))




#construct A matrix

A=np.zeros((N0*N1,N0*N1))
for n0 in range(0,N0):
    for n1 in range(0,N1):
        for point in R_O1:
            n_row,n_col=point
            m0=n0+n_row

            m1=n1+n_col

            flat_ind_row=double_ind_2_flat_ind(n0,n1)
            flat_ind_col=double_ind_2_flat_ind(m0%N0,m1%N1)

            if flat_ind_row==flat_ind_col:
                continue
            else:
                A[flat_ind_row,flat_ind_col]=1/((m0-n0)**2-(m0-n0)*(m1-n1)+(m1-n1)**2)







zero_count = np.sum(A == 0)
total_elements = A.size
percentage_zeros = (zero_count / total_elements) * 100

print("Number of zero elements in A:", zero_count)
print("Percentage of zero elements in A: {:.2f}%".format(percentage_zeros))

plt.imshow(A,cmap='viridis')
plt.colorbar()  # this adds a colorbar to the side of the plot indicating the scale
plt.title("Color Plot of A")
plt.xlabel("Column Index")
plt.ylabel("Row Index")
plt.savefig(out_pic_dir+f"A_N{N}.png")
plt.close()

# #construct B matrix
B=np.zeros((N0*N1,N0*N1))
for n0 in range(0,N0):
    for n1 in range(0,N1):
        for point in R_O1:
            n_row,n_col=point
            m0=n0+n_row

            m1=n1+n_col

            flat_ind_row=double_ind_2_flat_ind(n0,n1)
            flat_ind_col=double_ind_2_flat_ind(m0%N0,m1%N1)


            if flat_ind_row==flat_ind_col:
                continue
            else:
                B[flat_ind_row,flat_ind_col]= \
                    (m0-n0-1/2*m1+1/2*n1)**2/((m0-n0)**2-(m0-n0)*(m1-n1)+(m1-n1)**2)**2



zero_count_B = np.sum(B == 0)
total_elements_B = B.size
percentage_zeros_B = (zero_count_B / total_elements_B) * 100

print("Number of zero elements in B:", zero_count_B)
print("Percentage of zero elements in B: {:.2f}%".format(percentage_zeros_B))

plt.imshow(B,cmap='viridis')
plt.colorbar()  # this adds a colorbar to the side of the plot indicating the scale
plt.title("Color Plot of B")
plt.xlabel("Column Index")
plt.ylabel("Row Index")
plt.savefig(out_pic_dir+f"B_N{N}.png")
plt.close()

#construct C matrix
C=np.zeros((N0*N1,N0*N1))
for n0 in range(0,N0):
    for n1 in range(0,N1):
        for point in R_O1:
            n_row,n_col=point
            m0=n0+n_row

            m1=n1+n_col

            flat_ind_row=double_ind_2_flat_ind(n0,n1)
            flat_ind_col=double_ind_2_flat_ind(m0%N0,m1%N1)

            if flat_ind_row==flat_ind_col:
                continue
            else:
                C[flat_ind_row,flat_ind_col]= \
                    (m0-n0-1/2*m1+1/2*n1)*(m1-n1)/((m0-n0)**2-(m0-n0)*(m1-n1)+(m1-n1)**2)**2


zero_count_C = np.sum(C == 0)
total_elements_C = C.size
percentage_zeros_C = (zero_count_C / total_elements_C) * 100

print("Number of zero elements in C:", zero_count_C)
print("Percentage of zero elements in C: {:.2f}%".format(percentage_zeros_C))

plt.imshow(C,cmap='viridis')
plt.colorbar()  # this adds a colorbar to the side of the plot indicating the scale
plt.title("Color Plot of C")
plt.xlabel("Column Index")
plt.ylabel("Row Index")
plt.savefig(out_pic_dir+f"C_N{N}.png")
plt.close()


#construct G matrix

G=np.zeros((N0*N1,N0*N1))

for n0 in range(0,N0):
    for n1 in range(0,N1):
        for point in R_O1:
            n_row,n_col=point
            m0=n0+n_row

            m1=n1+n_col

            flat_ind_row=double_ind_2_flat_ind(n0,n1)
            flat_ind_col=double_ind_2_flat_ind(m0%N0,m1%N1)

            if flat_ind_row==flat_ind_col:
                continue
            else:
                G[flat_ind_row,flat_ind_col]=(m1-n1)**2/((m0-n0)**2-(m0-n0)*(m1-n1)+(m1-n1)**2)**2


zero_count_G = np.sum(G == 0)
total_elements_G = G.size
percentage_zeros_G = (zero_count_G / total_elements_G) * 100

print("Number of zero elements in G:", zero_count_G)
print("Percentage of zero elements in G: {:.2f}%".format(percentage_zeros_G))

plt.imshow(G,cmap='viridis')
plt.colorbar()  # this adds a colorbar to the side of the plot indicating the scale
plt.title("Color Plot of G")
plt.xlabel("Column Index")
plt.ylabel("Row Index")
plt.savefig(out_pic_dir+f"G_N{N}.png")
plt.close()

#construct R matrix
R=np.zeros((N0*N1,N0*N1))

for n0 in range(0,N0):
    for n1 in range(0,N1):
        for point in R_D1:
            n_row,n_col=point
            m0=n0+n_row

            m1=n1+n_col

            flat_ind_row=double_ind_2_flat_ind(n0,n1)
            flat_ind_col=double_ind_2_flat_ind(m0%N0,m1%N1)
            R[flat_ind_row,flat_ind_col]=1/ \
                                         ((m0-n0)**2-(m0-n0)*(m1-n1)+(m1-n1)**2+m1-n1+1/3)


zero_count_R = np.sum(R == 0)
total_elements_R = R.size
percentage_zeros_R = (zero_count_R / total_elements_R) * 100

print("Number of zero elements in R:", zero_count_R)
print("Percentage of zero elements in R: {:.2f}%".format(percentage_zeros_R))

plt.imshow(R,cmap='viridis')
plt.colorbar()  # this adds a colorbar to the side of the plot indicating the scale
plt.title("Color Plot of R")
plt.xlabel("Column Index")
plt.ylabel("Row Index")
plt.savefig(out_pic_dir+f"R_N{N}.png")
plt.close()


Gamma=np.zeros((N0*N1,N0*N1))

for n0 in range(0,N0):
    for n1 in range(0,N1):
        for point in R_D1:
            n_row,n_col=point
            m0=n0+n_row

            m1=n1+n_col

            flat_ind_row=double_ind_2_flat_ind(n0,n1)
            flat_ind_col=double_ind_2_flat_ind(m0%N0,m1%N1)
            Gamma[flat_ind_row,flat_ind_col]=(m0-n0-1/2*m1+1/2*n1)**2/ \
                                             ((m0-n0)**2-(m0-n0)*(m1-n1)+(m1-n1)**2+m1-n1+1/3)**2


zero_count_Gamma = np.sum(Gamma == 0)
total_elements_Gamma = Gamma.size
percentage_zeros_Gamma = (zero_count_Gamma / total_elements_Gamma) * 100

print("Number of zero elements in Gamma:", zero_count_Gamma)
print("Percentage of zero elements in Gamma: {:.2f}%".format(percentage_zeros_Gamma))

plt.imshow(Gamma,cmap='viridis')
plt.colorbar()  # this adds a colorbar to the side of the plot indicating the scale
plt.title("Color Plot of Gamma")
plt.xlabel("Column Index")
plt.ylabel("Row Index")
plt.savefig(out_pic_dir+f"Gamma_N{N}.png")
plt.close()


Theta=np.zeros((N0*N1,N0*N1))
for n0 in range(0,N0):
    for n1 in range(0,N1):
        for point in R_D1:
            n_row,n_col=point
            m0=n0+n_row

            m1=n1+n_col

            flat_ind_row=double_ind_2_flat_ind(n0,n1)
            flat_ind_col=double_ind_2_flat_ind(m0%N0,m1%N1)
            Theta[flat_ind_row,flat_ind_col]=(m0-n0-1/2*m1+1/2*n1)*(np.sqrt(3)/2*m1-np.sqrt(3)/2*n1+np.sqrt(3)/2)/ \
                                             ((m0-n0)**2-(m0-n0)*(m1-n1)+(m1-n1)**2+m1-n1+1/3)**2



zero_count_Theta = np.sum(Theta == 0)
total_elements_Theta = Theta.size
percentage_zeros_Theta = (zero_count_Theta / total_elements_Theta) * 100

print("Number of zero elements in Theta:", zero_count_Theta)
print("Percentage of zero elements in Theta: {:.2f}%".format(percentage_zeros_Theta))

plt.imshow(Theta,cmap='viridis')
plt.colorbar()  # this adds a colorbar to the side of the plot indicating the scale
plt.title("Color Plot of Theta")
plt.xlabel("Column Index")
plt.ylabel("Row Index")
plt.savefig(out_pic_dir+f"Theta_N{N}.png")
plt.close()


Lambda=np.zeros((N0*N1,N0*N1))
for n0 in range(0,N0):
    for n1 in range(0,N1):
        for point in R_D1:
            n_row,n_col=point
            m0=n0+n_row

            m1=n1+n_col

            flat_ind_row=double_ind_2_flat_ind(n0,n1)
            flat_ind_col=double_ind_2_flat_ind(m0%N0,m1%N1)

            Lambda[flat_ind_row,flat_ind_col]=(np.sqrt(3)/2*m1-np.sqrt(3)/2*n1+np.sqrt(3)/3)**2/ \
                                              ((m0-n0)**2-(m0-n0)*(m1-n1)+(m1-n1)**2+m1-n1+1/3)**2


zero_count_Lambda = np.sum(Lambda == 0)
total_elements_Lambda = Lambda.size
percentage_zeros_Lambda = (zero_count_Lambda / total_elements_Lambda) * 100

print("Number of zero elements in Lambda:", zero_count_Lambda)
print("Percentage of zero elements in Lambda: {:.2f}%".format(percentage_zeros_Lambda))

plt.imshow(Lambda,cmap='viridis')
plt.colorbar()  # this adds a colorbar to the side of the plot indicating the scale
plt.title("Color Plot of Lambda")
plt.xlabel("Column Index")
plt.ylabel("Row Index")
plt.savefig(out_pic_dir+f"Lambda{N}.png")
plt.close()
