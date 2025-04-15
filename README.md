#this project checks ergodic condition of a C3 ferroelectric system under PBC


python mk_dir.py, to set coefficients, T, and directories

##########################################
To manually perform each step of computations for U
1. python launch_one_run_U.py ./path/to/mc.conf
2. make run_mc
3. ./run_mc ./path/to/cppIn.txt
4. python check_after_one_run_U.py ./path/to/mc.conf  startingFileIndSuggest
5. go to 1, until no more data points are needed


##########################################
To manually perform each step of computations for dipole 
1. python launch_one_run_dipole.py ./path/to/mc.conf
2. make run_mc
3. ./run_mc ./path/to/cppIn.txt
4. python check_after_one_run_dipole.py ./path/to/mc.conf  startingFileIndSuggest
5. go to 1, until no more data points are needed

#########################################
To run 1 pass of mc with checking statistics of U
1. cmake .
2. make run_mc
3. python exec_checking_U.py T N startingFileIndSuggest
4. run 3 until equilibrium
5. python exec_noChecking.py T N

###################################
#plot dipole 
1. cd separate_pltLattice, python oneT_one_path_lattice_dipole.py N T init_path, to compute average Px, Py, Qx, Qy, polarization
2. to call oneT_one_path_lattice_dipole for each computation  path: python batch_lattice_dipole_separate.py N T
3. python load_csv_one_T_one_path_dipole.py N T init_path, it plots lattice and dipole for one T ,one path_name
4. to call load_csv_one_T_one_path_dipole for all paths for 1 T , use: python batch_plt_dipoles_one_T_all_paths.py N T