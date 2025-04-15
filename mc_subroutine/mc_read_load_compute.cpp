//
// Created by adada on 20/3/2025.
//

# include "mc_read_load_compute.hpp"


void mc_computation::init_Px_Py_Qx_Qy()
{
    std::string name;

    std::string Px_inFileName, Py_inFileName, Qx_inFileName, Qy_inFileName;
    if (this->flushLastFile == -1)
    {
        name = "init";

        Px_inFileName = out_Px_path + "/Px_" + name + ".pkl";

        Py_inFileName = out_Py_path + "/Py_" + name + ".pkl";

        Qx_inFileName = out_Qx_path + "/Qx_" + name + ".pkl";

        Qy_inFileName = out_Qy_path + "/Qy_" + name + ".pkl";

        this->load_pickle_data(Px_inFileName, Px_init, N0 * N1);
        this->load_pickle_data(Py_inFileName, Py_init, N0 * N1);
        this->load_pickle_data(Qx_inFileName, Qx_init, N0 * N1);
        this->load_pickle_data(Qy_inFileName, Qy_init, N0 * N1);
    } //end flushLastFile==-1
    else
    {
        name="flushEnd"+std::to_string(this->flushLastFile);
        Px_inFileName=out_Px_path+"/"+name+".Px.pkl";

        Py_inFileName=out_Py_path+"/"+name+".Py.pkl";

        Qx_inFileName=out_Qx_path+"/"+name+".Qx.pkl";

        Qy_inFileName=out_Qy_path+"/"+name+".Qy.pkl";

        //load Px
        this->load_pickle_data(Px_inFileName,Px_all_ptr,sweepToWrite * N0 * N1);
        //copy last N0*N1 elements of to Px_init
        std::memcpy(Px_init.get(),Px_all_ptr.get()+N0*N1*(sweepToWrite-1),
            N0*N1*sizeof(double));

        //load Py
        this->load_pickle_data(Py_inFileName,Py_all_ptr,sweepToWrite * N0 * N1);
        //copy last N0*N1 elements of to Py_init
        std::memcpy(Py_init.get(),Py_all_ptr.get()+N0*N1*(sweepToWrite-1),
            N0*N1*sizeof(double));


        //load Qx
        this->load_pickle_data(Qx_inFileName,Qx_all_ptr,sweepToWrite * N0 * N1);
        //copy last N0*N1 elements of to Qx_init
        std::memcpy(Qx_init.get(),Qx_all_ptr.get()+N0*N1*(sweepToWrite-1),
            N0*N1*sizeof(double));

        //load Qy
        this->load_pickle_data(Qy_inFileName,Qy_all_ptr,sweepToWrite * N0 * N1);
        //copy last N0*N1 elements of to Qy_init
        std::memcpy(Qy_init.get(),Qy_all_ptr.get()+N0*N1*(sweepToWrite-1),
            N0*N1*sizeof(double));
    }
}


void mc_computation::load_pickle_data(const std::string& filename, std::shared_ptr<double[]>& data_ptr,
                                      std::size_t size)
{
    // Initialize Python and NumPy
    Py_Initialize();
    np::initialize();


    try
    {
        // Use Python's 'io' module to open the file directly in binary mode
        py::object io_module = py::import("io");
        py::object file = io_module.attr("open")(filename, "rb"); // Open file in binary mode

        // Import the 'pickle' module
        py::object pickle_module = py::import("pickle");

        // Use pickle.load to deserialize from the Python file object
        py::object loaded_data = pickle_module.attr("load")(file);

        // Close the file
        file.attr("close")();

        // Check if the loaded object is a NumPy array
        if (py::extract<np::ndarray>(loaded_data).check())
        {
            np::ndarray np_array = py::extract<np::ndarray>(loaded_data);

            // Convert the NumPy array to a Python list using tolist()
            py::object py_list = np_array.attr("tolist")();

            // Ensure the list size matches the expected size
            ssize_t list_size = py::len(py_list);
            if (static_cast<std::size_t>(list_size) > size)
            {
                throw std::runtime_error("The provided shared_ptr array size is smaller than the list size.");
            }

            // Copy the data from the Python list to the shared_ptr array
            for (ssize_t i = 0; i < list_size; ++i)
            {
                data_ptr[i] = py::extract<double>(py_list[i]);
            }
        }
        else
        {
            throw std::runtime_error("Loaded data is not a NumPy array.");
        }
    }
    catch (py::error_already_set&)
    {
        PyErr_Print();
        throw std::runtime_error("Python error occurred.");
    }
}


void mc_computation::save_array_to_pickle(const std::shared_ptr<double[]>& ptr, int size, const std::string& filename)
{
    using namespace boost::python;
    namespace np = boost::python::numpy;

    // Initialize Python interpreter if not already initialized
    if (!Py_IsInitialized())
    {
        Py_Initialize();
        if (!Py_IsInitialized())
        {
            throw std::runtime_error("Failed to initialize Python interpreter");
        }
        np::initialize(); // Initialize NumPy
    }

    try
    {
        // Import the pickle module
        object pickle = import("pickle");
        object pickle_dumps = pickle.attr("dumps");

        // Convert C++ array to NumPy array using shared_ptr
        np::ndarray numpy_array = np::from_data(
            ptr.get(), // Use shared_ptr's raw pointer
            np::dtype::get_builtin<double>(), // NumPy data type (double)
            boost::python::make_tuple(size), // Shape of the array (1D array)
            boost::python::make_tuple(sizeof(double)), // Strides
            object() // Optional base object
        );

        // Serialize the NumPy array using pickle.dumps
        object serialized_array = pickle_dumps(numpy_array);

        // Extract the serialized data as a string
        std::string serialized_str = extract<std::string>(serialized_array);

        // Write the serialized data to a file
        std::ofstream file(filename, std::ios::binary);
        if (!file)
        {
            throw std::runtime_error("Failed to open file for writing");
        }
        file.write(serialized_str.data(), serialized_str.size());
        file.close();

        // Debug output (optional)
        // std::cout << "Array serialized and written to file successfully." << std::endl;
    }
    catch (const error_already_set&)
    {
        PyErr_Print();
        std::cerr << "Boost.Python error occurred." << std::endl;
    } catch (const std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
}
///
/// @param n0
/// @param n1
/// @return flatenned index
int mc_computation::double_ind_to_flat_ind(const int& n0, const int& n1)
{
    return n0 * N1 + n1;
}

int mc_computation::mod_direction0(const int&m0)
{

return ((m0 % N0) + N0) % N0;

}

int mc_computation::mod_direction1(const int&m1)
{return ((m1 % N1) + N1) % N1;
}
void mc_computation::init_mats_C3()
{
    ////////////////////////////////////////////////////////////////////////
    //init A_T
    this->A=arma::sp_dmat (N0*N1,N0*N1);
    for (int n0=0;n0<N0;n0++)
    {
        for (int n1=0;n1<N1;n1++)
        {
        for (const auto& vec: this->R_O1)
        {
            int n_row=vec[0];
            int n_col=vec[1];
            int m0=n0+n_row;
            int m1=n1+n_col;
            int flat_ind_row=double_ind_to_flat_ind(n0,n1);
            int m0_mod=mod_direction0(m0);
            int m1_mod=mod_direction1(m1);
            double tmp = std::pow(m0 - n0, 2.0) - (m0 - n0) * (m1 - n1) + std::pow(m1 - n1, 2.0);
            int flat_ind_col=double_ind_to_flat_ind(m0_mod,m1_mod);
            if (flat_ind_row==flat_ind_col)
            {
                continue;
            }else
            {
                A(flat_ind_row,flat_ind_col)=1.0 / tmp;
            }//end if else

        }//end for vec
        }//end for n1
    }//end for n0
    // A.print("A:");
    this->A_T=A.t();
    // arma::dmat A_dense(A_T);
    // double* raw_ptr = A_dense.memptr();
    // std::shared_ptr<double[]> sptr(raw_ptr, [](double* p) {
    // /* do nothing */
    // });
    // std::string outA="./A.pkl";
    // this->save_array_to_pickle(sptr,N0*N1*N0*N1,outA);
    //
    //
    // std::cout<<"A saved"<<std::endl;
    //end A_T init
    ////////////////////////////////////////////////////////////////////////
    ///
    ///////////////////////////////////////////////////////////////////////////
    //init B_T
    this-> B=arma::sp_dmat (N0*N1,N0*N1);
    for (int n0=0;n0<N0;n0++)
    {
        for (int n1=0;n1<N1;n1++)
        {
            for (const auto& vec: this->R_O1)
            {
                int n_row=vec[0];
                int n_col=vec[1];
                int m0=n0+n_row;
                int m1=n1+n_col;
                int m0_mod=mod_direction0(m0);
                int m1_mod=mod_direction1(m1);
                int flat_ind_row=double_ind_to_flat_ind(n0,n1);
                int flat_ind_col=double_ind_to_flat_ind(m0_mod,m1_mod);
                if (flat_ind_row==flat_ind_col)
                {
                    continue;
                }else
                {
                    double up = std::pow(m0 - n0 - 0.5 * m1 + 0.5 * n1, 2.0);
                    double down_tmp = std::pow(m0 - n0, 2.0) - (m0 - n0) * (m1 - n1) + std::pow(m1 - n1, 2.0);
                    double down = std::pow(down_tmp, 2.0);
                    B(flat_ind_row,flat_ind_col)=up / down;
                }//end if else
            }//end for vec

        }//end for n1
    }//end for n0

    // B.print("B:");
    this->B_T=B.t();
    //save and check
    // arma::dmat B_dense(B_T);
    // double* raw_ptr = B_dense.memptr();
    // std::shared_ptr<double[]> sptr(raw_ptr, [](double* p) {
    // /* do nothing */
    // });
    // std::string outB="./B.pkl";
    // this->save_array_to_pickle(sptr,N0*N1*N0*N1,outB);
    // std::cout<<"B saved"<<std::endl;
    //end init B_T
    ///////////////////////////////////////////////////////////////////////////
    ///
    //////////////////////////////////////////////////////////////////////////////
    /// init C_T
    this->C=arma::sp_dmat (N0*N1,N0*N1);
    for (int n0=0;n0<N0;n0++)
    {
        for (int n1=0;n1<N1;n1++)
        {
            for (const auto& vec: this->R_O1)
            {
                int n_row=vec[0];
                int n_col=vec[1];
                int m0=n0+n_row;
                int m1=n1+n_col;
                int flat_ind_row=double_ind_to_flat_ind(n0,n1);
                int m0_mod=mod_direction0(m0);
                int m1_mod=mod_direction1(m1);
                int flat_ind_col=double_ind_to_flat_ind(m0_mod,m1_mod);
                if (flat_ind_row==flat_ind_col)
                {
                    continue;
                }else
                {
                    double up = (m0 - n0 - 0.5 * m1 + 0.5 * n1) * (m1 - n1);
                    double down_tmp = std::pow(m0 - n0, 2.0) - (m0 - n0) * (m1 - n1) + std::pow(m1 - n1, 2.0);
                    double down = std::pow(down_tmp, 2.0);

                    C(flat_ind_row,flat_ind_col)=up / down;
                }//end if else
            }//end for vec

        }//end for n1
    }//end for n0
    // C.print("C:");
    this->C_T=C.t();
    //save and check
    // arma::dmat C_dense(C_T);
    // double* raw_ptr = C_dense.memptr();
    // std::shared_ptr<double[]> sptr(raw_ptr, [](double* p) {
    // /* do nothing */
    // });
    // std::string outC="./C.pkl";
    // this->save_array_to_pickle(sptr,N0*N1*N0*N1,outC);
    // std::cout<<"C saved"<<std::endl;
    //end init C_T
    //////////////////////////////////////////////////////////////////////////////
    /// init G_T
    this-> G=arma::sp_dmat (N0*N1,N0*N1);
    for (int n0=0;n0<N0;n0++)
    {

        for (int n1=0;n1<N1;n1++)
        {
            for (const auto& vec: this->R_O1)
            {
                int n_row=vec[0];
                int n_col=vec[1];
                int m0=n0+n_row;
                int m1=n1+n_col;
                int flat_ind_row=double_ind_to_flat_ind(n0,n1);
                int m0_mod=mod_direction0(m0);
                int m1_mod=mod_direction1(m1);
                int flat_ind_col=double_ind_to_flat_ind(m0_mod,m1_mod);

                if (flat_ind_row==flat_ind_col)
                {
                    continue;
                }else
                {
                    double up = std::pow(m1 - n1, 2.0);
                    double down_tmp = std::pow(m0 - n0, 2.0) - (m0 - n0) * (m1 - n1) + std::pow(m1 - n1, 2.0);
                    double down = std::pow(down_tmp, 2.0);
                    G(flat_ind_row,flat_ind_col)=up / down;
                }//end if else

            }//end for vec
        }//end for n1

    }//end for n0
    this->G_T=G.t();

    // G.print("G:");
    // arma::dmat G_dense(G_T);
    // double* raw_ptr = G_dense.memptr();
    // std::shared_ptr<double[]> sptr(raw_ptr, [](double* p) {
    // /* do nothing */
    // });
    // std::string outG="./G.pkl";
    // this->save_array_to_pickle(sptr,N0*N1*N0*N1,outG);
    // std::cout<<"G saved"<<std::endl;
    /// end init G_T
    /// /////////////////////////////////////////////////////////////////////////////////
    ///
    this->R = arma::sp_dmat(N0 * N1, N0 * N1);
    for (int n0=0;n0<N0;n0++)
    {
        for (int n1=0;n1<N1;n1++)
        {
            for (const auto & vec:this->R_D1)
            {
                int n_row=vec[0];
                int n_col=vec[1];
                int m0=n0+n_row;
                int m1=n1+n_col;
                int flat_ind_row=double_ind_to_flat_ind(n0,n1);
                int m0_mod=mod_direction0(m0);
                int m1_mod=mod_direction1(m1);
                int flat_ind_col=double_ind_to_flat_ind(m0_mod,m1_mod);
                double down = std::pow(m0 - n0, 2.0) - (m0 - n0) * (m1 - n1) + std::pow(m1 - n1, 2.0) + m1 - n1 +
                                               1.0 / 3.0;
                R(flat_ind_row,flat_ind_col)=1.0 / down;
            }//end for vec
        }//end for n1
    }//end for n0
    this->R_T=R.t();
    //save and check
    // arma::dmat R_dense(R_T);
    // double* raw_ptr = R_dense.memptr();
    // std::shared_ptr<double[]> sptr(raw_ptr, [](double* p) {
    // /* do nothing */
    // });
    // std::string outR="./R.pkl";
    // this->save_array_to_pickle(sptr,N0*N1*N0*N1,outR);
    // std::cout<<"R saved"<<std::endl;
    ///end init R
    /////////////////////////////////////////////////////////////////////////////////
    ///init Gamma
    this->Gamma=arma::sp_dmat(N0*N1,N0*N1);
    for (int n0=0;n0<N0;n0++)
    {
        for (int n1=0;n1<N1;n1++)
        {
            for (const auto & vec:this->R_D1)
            {
                int n_row=vec[0];
                int n_col=vec[1];
                int m0=n0+n_row;
                int m1=n1+n_col;
                int flat_ind_row=double_ind_to_flat_ind(n0,n1);
                int m0_mod=mod_direction0(m0);
                int m1_mod=mod_direction1(m1);
                int flat_ind_col=double_ind_to_flat_ind(m0_mod,m1_mod);
                double up = std::pow(m0 - n0 - 0.5 * m1 + 0.5 * n1, 2.0);
                double down_tmp = std::pow(m0 - n0, 2.0) - (m0 - n0) * (m1 - n1) + std::pow(m1 - n1, 2.0) + m1 - n1
                    + 1.0 / 3.0;
                double down = std::pow(down_tmp, 2.0);
                this->Gamma(flat_ind_row, flat_ind_col) = up / down;
            }//end for vec
        }//end for n1
    }//end for n0
    this->Gamma_T=Gamma.t();
    // Gamma_T.print("Gamma_T:");
    // arma::dmat Gamma_dense(Gamma);
    // double* raw_ptr = Gamma_dense.memptr();
    // std::shared_ptr<double[]> sptr(raw_ptr, [](double* p) {
    // /* do nothing */
    // });
    // std::string outGamma="./Gamma.pkl";
    // this->save_array_to_pickle(sptr,N0*N1*N0*N1,outGamma);
    // std::cout<<"Gamma saved"<<std::endl;
    ///end init Gamma
    ///////////////////////////////////////////////////////////////////////////////////
    ///init Theta
    this->Theta=arma::sp_dmat(N0*N1,N0*N1);
    for (int n0=0;n0<N0;n0++)
    {
        for (int n1=0;n1<N1;n1++)
        {
            for (const auto & vec:this->R_D1)
            {
                int n_row=vec[0];
                int n_col=vec[1];
                int m0=n0+n_row;
                int m1=n1+n_col;
                int flat_ind_row=double_ind_to_flat_ind(n0,n1);
                int m0_mod=mod_direction0(m0);
                int m1_mod=mod_direction1(m1);
                int flat_ind_col=double_ind_to_flat_ind(m0_mod,m1_mod);
                double up = (m0 - n0 - 0.5 * m1 + 0.5 * n1) * (std::sqrt(3.0) / 2.0 * m1 - std::sqrt(3.0) / 2.0 * n1
                       + std::sqrt(3.0) / 2.0);

                double down_tmp = std::pow(m0 - n0, 2.0) - (m0 - n0) * (m1 - n1) + std::pow(m1 - n1, 2.0) + m1 - n1
                    + 1.0 / 3.0;

                double down = std::pow(down_tmp, 2.0);
                this->Theta(flat_ind_row, flat_ind_col) = up / down;
            }//end for vec
        }//end for n1
    }//end for n0
    this->Theta_T=Theta.t();
    // arma::dmat Theta_dense(Theta);
    // double* raw_ptr = Theta_dense.memptr();
    // std::shared_ptr<double[]> sptr(raw_ptr, [](double* p) {
    // /* do nothing */
    // });
    // std::string outTheta="./Theta.pkl";
    // this->save_array_to_pickle(sptr,N0*N1*N0*N1,outTheta);
    // std::cout<<"Theta saved"<<std::endl;
    ///end init Theta
    ///////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////
    /// init Lambda
    this->Lambda=arma::sp_dmat(N0*N1,N0*N1);
    for (int n0=0;n0<N0;n0++)
    {
        for (int n1=0;n1<N1;n1++)
        {
            for (const auto & vec:this->R_D1)
            {
                int n_row=vec[0];
                int n_col=vec[1];
                int m0=n0+n_row;
                int m1=n1+n_col;
                int flat_ind_row=double_ind_to_flat_ind(n0,n1);
                int m0_mod=mod_direction0(m0);
                int m1_mod=mod_direction1(m1);
                int flat_ind_col=double_ind_to_flat_ind(m0_mod,m1_mod);
                double up = std::pow(std::sqrt(3.0) / 2.0 * m1 - std::sqrt(3.0) / 2.0 * n1 + std::sqrt(3.0) / 3.0,
                                       2.0);

                double down_tmp = std::pow(m0 - n0, 2.0) - (m0 - n0) * (m1 - n1) + std::pow(m1 - n1, 2.0) + m1 - n1
                    + 1.0 / 3.0;
                double down = std::pow(down_tmp, 2.0);

                this->Lambda(flat_ind_row, flat_ind_col) = up / down;
            }//end for vec
        }//end for n1
    }//end for n0
    this->Lambda_T=Lambda.t();
    // arma::dmat Lambda_dense(Lambda);
    // double* raw_ptr = Lambda_dense.memptr();
    // std::shared_ptr<double[]> sptr(raw_ptr, [](double* p) {
    // /* do nothing */
    // });
    // std::string outLambda="./Lambda.pkl";
    // this->save_array_to_pickle(sptr,N0*N1*N0*N1,outLambda);
    // std::cout<<"Lambda saved"<<std::endl;

    /// end init Lambda
    ///////////////////////////////////////////////////////////////////////////////////
}


void mc_computation::compute_seed_vecs()
{
for (int n0=-Nx;n0<=Nx;n0++)
{
    for (int n1=-Ny;n1<=Ny;n1++)
    {
        this->S_O1.push_back({n0,n1});
        this->S_D1.push_back({n0,n1});
    }//end for n1
}//end for n0
}
void mc_computation::iter_U6(const int&n0, const int &n1, int &n0Next, int &n1Next)
{
    n0Next=n0-n1;
    n1Next=n0;
}

void mc_computation::iter_U3(const int&n0, const int &n1, int &n0Next, int &n1Next)
{
    n0Next=-n1-1;
    n1Next=n0-n1-1;
}
void mc_computation::compute_rotated_sets()
{
    //R_O1
    for (const auto & vec: this->S_O1)
    {
        int n0Curr=vec[0];
        int n1Curr=vec[1];
        int n0Next,n1Next;
        for (int j=0;j<6;j++)
        {
            this->iter_U6(n0Curr,n1Curr,n0Next,n1Next);
            n0Curr=n0Next;
            n1Curr=n1Next;
            R_O1.insert({n0Curr,n1Curr});
        }//end for j

    }//end for
// for (const auto & vec:R_O1)
// {
//     std::cout<<vec[0]<<", "<<vec[1]<<std::endl;
// }//print
//     std::cout<<"size ="<<R_O1.size()<<std::endl;

    //R_D1
    for (const auto & vec: this->S_D1)
    {
        int n0Curr=vec[0];
        int n1Curr=vec[1];
        int n0Next,n1Next;
        for (int j=0;j<3;j++)
        {
        this->iter_U3(n0Curr,n1Curr,n0Next,n1Next);
            n0Curr=n0Next;
            n1Curr=n1Next;
            this->R_D1.insert({n0Curr,n1Curr});
        }//end for j
    }//end for
// for (const auto & vec: R_D1)
// {
//     std::cout<<vec[0]<<", "<<vec[1]<<std::endl;
// }
//     std::cout<<"size="<<R_D1.size()<<std::endl;

}


void mc_computation::init_mats()
{
    ////////////////////////////////////////////////////////////////////////
    //init A_T
   this->A=arma::sp_dmat (N0*N1,N0*N1);
    for (int n0=0;n0<N0;n0++)
    {
        for (int n1=0;n1<N1;n1++)
        {
            for (int m0=n0-Nx;m0<n0+Nx+1;m0++)
            {
                for (int m1=n1-Ny;m1<n1+Ny+1;m1++)
                {
                    int flat_ind_row=double_ind_to_flat_ind(n0,n1);
                    int m0_mod=mod_direction0(m0);
                    int m1_mod=mod_direction1(m1);
                    double tmp = std::pow(m0 - n0, 2.0) - (m0 - n0) * (m1 - n1) + std::pow(m1 - n1, 2.0);
                    int flat_ind_col=double_ind_to_flat_ind(m0_mod,m1_mod);
                    if (flat_ind_row==flat_ind_col)
                    {
                        continue;
                    }else
                    {
                        A(flat_ind_row,flat_ind_col)=1.0 / tmp;
                    }//end if else
                }//end m1
            }//end m0
        }//end n1
    }//end n0
    // A.print("A:");
    this->A_T=A.t();
    // save and check
    // arma::dmat A_dense(A_T);
    // double* raw_ptr = A_dense.memptr();
    // std::shared_ptr<double[]> sptr(raw_ptr, [](double* p) {
    // /* do nothing */
    // });
    // std::string outA="./A.pkl";
    // this->save_array_to_pickle(sptr,N0*N1*N0*N1,outA);
    //
    //
    // std::cout<<"A saved"<<std::endl;
    //end A_T init
    ////////////////////////////////////////////////////////////////////////
    ///
    ///////////////////////////////////////////////////////////////////////////
    //init B_T
    this-> B=arma::sp_dmat (N0*N1,N0*N1);
    for (int n0=0;n0<N0;n0++)
    {
        for (int n1=0;n1<N1;n1++)
        {
            for (int m0=n0-Nx;m0<n0+Nx+1;m0++)
            {
                for (int m1=n1-Ny;m1<n1+Ny+1;m1++)
                {
                    int flat_ind_row=double_ind_to_flat_ind(n0,n1);
                    int m0_mod=mod_direction0(m0);
                    int m1_mod=mod_direction1(m1);
                    int flat_ind_col=double_ind_to_flat_ind(m0_mod,m1_mod);
                    if (flat_ind_row==flat_ind_col)
                    {
                        continue;
                    }else
                    {
                        double up = std::pow(m0 - n0 - 0.5 * m1 + 0.5 * n1, 2.0);
                        double down_tmp = std::pow(m0 - n0, 2.0) - (m0 - n0) * (m1 - n1) + std::pow(m1 - n1, 2.0);
                        double down = std::pow(down_tmp, 2.0);
                        B(flat_ind_row,flat_ind_col)=up / down;
                    }//end if else


                }//end m1
            }//end m0
        }//end n1
    }//end n0
// B.print("B:");
    this->B_T=B.t();
    //save and check
    // arma::dmat B_dense(B_T);
    // double* raw_ptr = B_dense.memptr();
    // std::shared_ptr<double[]> sptr(raw_ptr, [](double* p) {
    // /* do nothing */
    // });
    // std::string outB="./B.pkl";
    // this->save_array_to_pickle(sptr,N0*N1*N0*N1,outB);
    // std::cout<<"B saved"<<std::endl;
    //end init B_T
    ///////////////////////////////////////////////////////////////////////////
    ///
    //////////////////////////////////////////////////////////////////////////////
    /// init C_T
    this->C=arma::sp_dmat (N0*N1,N0*N1);

    ///end init C_T
    for (int n0=0;n0<N0;n0++)
    {
        for (int n1=0;n1<N1;n1++)
        {
            for (int m0=n0-Nx;m0<n0+Nx+1;m0++)
            {
                for (int m1=n1-Ny;m1<n1+Ny+1;m1++)
                {
                    int flat_ind_row=double_ind_to_flat_ind(n0,n1);
                    int m0_mod=mod_direction0(m0);
                    int m1_mod=mod_direction1(m1);
                    int flat_ind_col=double_ind_to_flat_ind(m0_mod,m1_mod);
                    if (flat_ind_row==flat_ind_col)
                    {
                        continue;
                    }else
                    {
                        double up = (m0 - n0 - 0.5 * m1 + 0.5 * n1) * (m1 - n1);
                        double down_tmp = std::pow(m0 - n0, 2.0) - (m0 - n0) * (m1 - n1) + std::pow(m1 - n1, 2.0);
                        double down = std::pow(down_tmp, 2.0);

                        C(flat_ind_row,flat_ind_col)=up / down;

                    }//end if else

                }//end m1
            }//end m0
        }//end n1
    }//end n0
    // C.print("C:");
    this->C_T=C.t();
    //save and check
    // arma::dmat C_dense(C_T);
    // double* raw_ptr = C_dense.memptr();
    // std::shared_ptr<double[]> sptr(raw_ptr, [](double* p) {
    // /* do nothing */
    // });
    // std::string outC="./C.pkl";
    // this->save_array_to_pickle(sptr,N0*N1*N0*N1,outC);
    // std::cout<<"C saved"<<std::endl;
    //end init C_T
    //////////////////////////////////////////////////////////////////////////////
    /// init G_T
    this-> G=arma::sp_dmat (N0*N1,N0*N1);
    for (int n0=0;n0<N0;n0++)
    {
        for (int n1=0;n1<N1;n1++)
        {
            for (int m0=n0-Nx;m0<n0+Nx+1;m0++)
            {
                for (int m1=n1-Ny;m1<n1+Ny+1;m1++)
                {

                    int flat_ind_row=double_ind_to_flat_ind(n0,n1);
                    int m0_mod=mod_direction0(m0);
                    int m1_mod=mod_direction1(m1);
                    int flat_ind_col=double_ind_to_flat_ind(m0_mod,m1_mod);
                    if (flat_ind_row==flat_ind_col)
                    {
                        continue;
                    }else
                    {
                        double up = std::pow(m1 - n1, 2.0);
                        double down_tmp = std::pow(m0 - n0, 2.0) - (m0 - n0) * (m1 - n1) + std::pow(m1 - n1, 2.0);
                        double down = std::pow(down_tmp, 2.0);
                        G(flat_ind_row,flat_ind_col)=up / down;
                    }//end if else

                }//end m1
            }//end m0
        }//end n1
    }//end n0
    this->G_T=G.t();

    // G.print("G:");
    // arma::dmat G_dense(G_T);
    // double* raw_ptr = G_dense.memptr();
    // std::shared_ptr<double[]> sptr(raw_ptr, [](double* p) {
    // /* do nothing */
    // });
    // std::string outG="./G.pkl";
    // this->save_array_to_pickle(sptr,N0*N1*N0*N1,outG);
    // std::cout<<"G saved"<<std::endl;

    /// end init G_T
    /////////////////////////////////////////////////////////////////////////////////
    ///
    this->R=arma::sp_dmat(N0*N1,N0*N1);
    for (int n0=0;n0<N0;n0++)
    {
        for (int n1=0;n1<N1;n1++)
        {
            for (int m0=n0-Nx;m0<n0+Nx+1;m0++)
            {
                for (int m1=n1-Ny;m1<n1+Ny+1;m1++)
                {
                    int flat_ind_row=double_ind_to_flat_ind(n0,n1);
                    int m0_mod=mod_direction0(m0);
                    int m1_mod=mod_direction1(m1);
                    int flat_ind_col=double_ind_to_flat_ind(m0_mod,m1_mod);
                    if (flat_ind_row==flat_ind_col)
                    {
                        continue;
                    }else
                    {
                        double down = std::pow(m0 - n0, 2.0) - (m0 - n0) * (m1 - n1) + std::pow(m1 - n1, 2.0) + m1 - n1 +
                                                1.0 / 3.0;
                        R(flat_ind_row,flat_ind_col)=1.0 / down;
                    }//end if else
                }//end m1
            }//end m0
        }//end n1
    }//end n0
    this->R_T=R.t();
    //save and check
    // arma::dmat R_dense(R_T);
    // double* raw_ptr = R_dense.memptr();
    // std::shared_ptr<double[]> sptr(raw_ptr, [](double* p) {
    // /* do nothing */
    // });
    // std::string outR="./R.pkl";
    // this->save_array_to_pickle(sptr,N0*N1*N0*N1,outR);
    // std::cout<<"R saved"<<std::endl;
    ///end init R
    /////////////////////////////////////////////////////////////////////////////////
    ///init Gamma
    this->Gamma=arma::sp_dmat(N0*N1,N0*N1);
    for (int n0=0;n0<N0;n0++)
    {
        for (int n1=0;n1<N1;n1++)
        {
            for (int m0=n0-Nx;m0<n0+Nx+1;m0++)
            {
                for (int m1=n1-Ny;m1<n1+Ny+1;m1++)
                {
                    int flat_ind_row=double_ind_to_flat_ind(n0,n1);
                    int m0_mod=mod_direction0(m0);
                    int m1_mod=mod_direction1(m1);
                    int flat_ind_col=double_ind_to_flat_ind(m0_mod,m1_mod);
                    if (flat_ind_row==flat_ind_col)
                    {
                        continue;
                    }else
                    {
                        double up = std::pow(m0 - n0 - 0.5 * m1 + 0.5 * n1, 2.0);
                        double down_tmp = std::pow(m0 - n0, 2.0) - (m0 - n0) * (m1 - n1) + std::pow(m1 - n1, 2.0) + m1 - n1
                            + 1.0 / 3.0;
                        double down = std::pow(down_tmp, 2.0);
                        this->Gamma(flat_ind_row, flat_ind_col) = up / down;
                    }//end if else
                }//end m1
            }//end m0
        }//end n1
    }//end n0
    this->Gamma_T=Gamma.t();
    // Gamma_T.print("Gamma_T:");
    // arma::dmat Gamma_dense(Gamma);
    // double* raw_ptr = Gamma_dense.memptr();
    // std::shared_ptr<double[]> sptr(raw_ptr, [](double* p) {
    // /* do nothing */
    // });
    // std::string outGamma="./Gamma.pkl";
    // this->save_array_to_pickle(sptr,N0*N1*N0*N1,outGamma);
    // std::cout<<"Gamma saved"<<std::endl;
    ///end init Gamma
    ///////////////////////////////////////////////////////////////////////////////////
    ///init Theta
    this->Theta=arma::sp_dmat(N0*N1,N0*N1);
    for (int n0=0;n0<N0;n0++)
    {
        for (int n1=0;n1<N1;n1++)
        {
            for (int m0=n0-Nx;m0<n0+Nx+1;m0++)
            {
                for (int m1=n1-Ny;m1<n1+Ny+1;m1++)
                {
                    int flat_ind_row=double_ind_to_flat_ind(n0,n1);
                    int m0_mod=mod_direction0(m0);
                    int m1_mod=mod_direction1(m1);
                    int flat_ind_col=double_ind_to_flat_ind(m0_mod,m1_mod);
                    if (flat_ind_row==flat_ind_col)
                    {
                        continue;
                    }else
                    {
                        double up = (m0 - n0 - 0.5 * m1 + 0.5 * n1) * (std::sqrt(3.0) / 2.0 * m1 - std::sqrt(3.0) / 2.0 * n1
                        + std::sqrt(3.0) / 2.0);

                        double down_tmp = std::pow(m0 - n0, 2.0) - (m0 - n0) * (m1 - n1) + std::pow(m1 - n1, 2.0) + m1 - n1
                            + 1.0 / 3.0;

                        double down = std::pow(down_tmp, 2.0);
                        this->Theta(flat_ind_row, flat_ind_col) = up / down;

                    }//end if else
                }//end m1
            }//end m0
        }//end n1
    }//end n0
    this->Theta_T=Theta.t();
    // arma::dmat Theta_dense(Theta);
    // double* raw_ptr = Theta_dense.memptr();
    // std::shared_ptr<double[]> sptr(raw_ptr, [](double* p) {
    // /* do nothing */
    // });
    // std::string outTheta="./Theta.pkl";
    // this->save_array_to_pickle(sptr,N0*N1*N0*N1,outTheta);
    // std::cout<<"Theta saved"<<std::endl;
    ///end init Theta
    ///////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////
    /// init Lambda
    this->Lambda=arma::sp_dmat(N0*N1,N0*N1);
    for (int n0=0;n0<N0;n0++)
    {
        for (int n1=0;n1<N1;n1++)
        {
            for (int m0=n0-Nx;m0<n0+Nx+1;m0++)
            {
                for (int m1=n1-Ny;m1<n1+Ny+1;m1++)
                {

                    int flat_ind_row=double_ind_to_flat_ind(n0,n1);
                    int m0_mod=mod_direction0(m0);
                    int m1_mod=mod_direction1(m1);
                    int flat_ind_col=double_ind_to_flat_ind(m0_mod,m1_mod);
                    if (flat_ind_row==flat_ind_col)
                    {
                        continue;
                    }else
                    {
                        double up = std::pow(std::sqrt(3.0) / 2.0 * m1 - std::sqrt(3.0) / 2.0 * n1 + std::sqrt(3.0) / 3.0,
                                         2.0);

                        double down_tmp = std::pow(m0 - n0, 2.0) - (m0 - n0) * (m1 - n1) + std::pow(m1 - n1, 2.0) + m1 - n1
                            + 1.0 / 3.0;
                        double down = std::pow(down_tmp, 2.0);

                        this->Lambda(flat_ind_row, flat_ind_col) = up / down;

                    }//end if else
                }//end m1
            }//end m0
        }//end n1
    }//end n0
    this->Lambda_T=Lambda.t();
    // arma::dmat Lambda_dense(Lambda);
    // double* raw_ptr = Lambda_dense.memptr();
    // std::shared_ptr<double[]> sptr(raw_ptr, [](double* p) {
    // /* do nothing */
    // });
    // std::string outLambda="./Lambda.pkl";
    // this->save_array_to_pickle(sptr,N0*N1*N0*N1,outLambda);
    // std::cout<<"Lambda saved"<<std::endl;

    /// end init Lambda
    ///////////////////////////////////////////////////////////////////////////////////
}

///
/// @param x proposed value
/// @param y current value
/// @param a left end of interval
/// @param b right end of interval
/// @param epsilon half length
/// @return proposal probability S(x|y)
double mc_computation::S_uni(const double& x, const double& y, const double& a, const double& b, const double& epsilon)
{
    if (a < y and y < a + epsilon)
    {
        return 1.0 / (y - a + epsilon);
    }
    else if (a + epsilon <= y and y < b - epsilon)
    {
        return 1.0 / (2.0 * epsilon);
    }
    else if (b - epsilon <= y and y < b)
    {
        return 1.0 / (b - y + epsilon);
    }
    else
    {
        std::cerr << "value out of range." << std::endl;
        std::exit(10);
    }
}

///
/// @param x
/// @param leftEnd
/// @param rightEnd
/// @param eps
/// @return return a value within distance eps from x, on the open interval (leftEnd, rightEnd)
double mc_computation::generate_uni_open_interval(const double& x, const double& leftEnd, const double& rightEnd,
                                                  const double& eps)
{
    double xMinusEps = x - eps;
    double xPlusEps = x + eps;

    double unif_left_end = xMinusEps < leftEnd ? leftEnd : xMinusEps;
    double unif_right_end = xPlusEps > rightEnd ? rightEnd : xPlusEps;

    //    std::random_device rd;
    //    std::ranlux24_base e2(rd());

    double unif_left_end_double_on_the_right = std::nextafter(unif_left_end, std::numeric_limits<double>::infinity());


    std::uniform_real_distribution<> distUnif(unif_left_end_double_on_the_right, unif_right_end);
    //(unif_left_end_double_on_the_right, unif_right_end)

    double xNext = distUnif(e2);
    return xNext;
}
void mc_computation::proposal_uni(const arma::dvec& arma_vec_curr, arma::dvec& arma_vec_next,
                                  const int& flattened_ind)
{
    double dp_val_new = this->generate_uni_open_interval(arma_vec_curr(flattened_ind), dipole_lower_bound,
                                                         dipole_upper_bound, h);
    arma_vec_next = arma_vec_curr;
    arma_vec_next(flattened_ind) = dp_val_new;


}

/// @param Px_arma_vec Px
/// @param Py_arma_vec Py
/// @return self energy H1
double mc_computation::H1(const int& flattened_ind, const arma::dvec& Px_arma_vec, const arma::dvec& Py_arma_vec)
{
    // int flat_ind = double_ind_to_flat_ind(n0, n1);

    double px_n0n1 = Px_arma_vec(flattened_ind);

    double py_n0n1 = Py_arma_vec(flattened_ind);


    double squared_px_n0n1 = std::pow(px_n0n1, 2.0);
    double squared_py_n0n1 = std::pow(py_n0n1, 2.0);

    double part1 = alpha1 * px_n0n1 * (squared_px_n0n1 - 3.0 * squared_py_n0n1);

    double part2 = alpha2 * py_n0n1 * (3.0 * squared_px_n0n1 - squared_py_n0n1);

    double part3 = alpha3 * (
        squared_px_n0n1 * std::pow(squared_px_n0n1 - 3.0 * squared_py_n0n1, 2.0)
        - squared_py_n0n1 * std::pow(3.0 * squared_px_n0n1 - squared_py_n0n1, 2.0)
    );

    double part4 = alpha4 * px_n0n1 * py_n0n1 * (squared_px_n0n1 - 3.0 * squared_py_n0n1)
        * (3.0 * squared_px_n0n1 - squared_py_n0n1);

    double part5 = alpha5 * (squared_px_n0n1 + squared_py_n0n1);

    double part6 = alpha6 * std::pow(squared_px_n0n1 + squared_py_n0n1, 2.0);

    double part7 = alpha7 * std::pow(squared_px_n0n1 + squared_py_n0n1, 3.0);

    return part1 + part2 + part3 + part4 + part5 + part6 + part7;
}


/// @param Qx_arma_vec Qx
/// @param Qy_arma_vec Qy
/// @return self energy H2
double mc_computation::H2(const int& flattened_ind, const arma::dvec& Qx_arma_vec, const arma::dvec& Qy_arma_vec)
{
    // int flat_ind = double_ind_to_flat_ind(n0, n1);

    double qx_n0n1 = Qx_arma_vec(flattened_ind);

    double qy_n0n1 = Qy_arma_vec(flattened_ind);

    double squared_qx_n0n1 = std::pow(qx_n0n1, 2.0);

    double squared_qy_n0n1 = std::pow(qy_n0n1, 2.0);

    double part1 = alpha1 * qx_n0n1 * (squared_qx_n0n1 - 3.0 * squared_qy_n0n1);

    double part2 = alpha2 * qy_n0n1 * (3.0 * squared_qx_n0n1 - squared_qy_n0n1);

    double part3 = alpha3 * (
        squared_qx_n0n1 * std::pow(squared_qx_n0n1 - 3.0 * squared_qy_n0n1, 2.0)
        - squared_qy_n0n1 * std::pow(3.0 * squared_qx_n0n1 - squared_qy_n0n1, 2.0)
    );

    double part4 = alpha4 * qx_n0n1 * qy_n0n1 * (squared_qx_n0n1 - 3.0 * squared_qy_n0n1)
        * (3.0 * squared_qx_n0n1 - squared_qy_n0n1);

    double part5 = alpha5 * (squared_qx_n0n1 + squared_qy_n0n1);

    double part6 = alpha6 * std::pow(squared_qx_n0n1 + squared_qy_n0n1, 2.0);

    double part7 = alpha7 * std::pow(squared_qx_n0n1 + squared_qy_n0n1, 3.0);

    return part1 + part2 + part3 + part4 + part5 + part6 + part7;
}


double mc_computation::acceptanceRatio_uni(const arma::dvec& arma_vec_curr,
                                           const arma::dvec& arma_vec_next, const int& flattened_ind,
                                           const double& UCurr, const double& UNext)
{
    double numerator = -this->beta * UNext;
    double denominator = -this->beta * UCurr;
    double R = std::exp(numerator - denominator);

    double S_curr_next = S_uni(arma_vec_curr(flattened_ind), arma_vec_next(flattened_ind),
                               dipole_lower_bound, dipole_upper_bound, h);

    double S_next_curr = S_uni(arma_vec_next(flattened_ind), arma_vec_curr(flattened_ind),
                               dipole_lower_bound, dipole_upper_bound, h);

    double ratio = S_curr_next / S_next_curr;

    if (std::fetestexcept(FE_DIVBYZERO))
    {
        std::cout << "Division by zero exception caught." << std::endl;
        std::exit(15);
    }
    if (std::isnan(ratio))
    {
        std::cout << "The result is NaN." << std::endl;
        std::exit(15);
    }
    R *= ratio;

    return std::min(1.0, R);
}
void mc_computation::HPx_update_colForm(const int& flattened_ind, const arma::dvec& Px_arma_vec_curr,
                    const arma::dvec& Px_arma_vec_next,
                    const arma::dvec& Py_arma_vec_curr,
                    const arma::dvec& Qx_arma_vec_curr,
                    const arma::dvec& Qy_arma_vec_curr,
                    double& UCurr, double& UNext)
{

    double H1_self_curr = this->H1(flattened_ind, Px_arma_vec_curr, Py_arma_vec_curr);
    double H1_self_next = this->H1(flattened_ind, Px_arma_vec_next, Py_arma_vec_curr);


    double left_factor1=J_over_a_squared*arma::dot(A_T.col(flattened_ind),Px_arma_vec_curr);
    // std::cout<<"left_factor1="<<left_factor1<<std::endl;
    double left_factor2=-2.0*J_over_a_squared*arma::dot(B_T.col(flattened_ind),Px_arma_vec_curr);

    // std::cout<<"left_factor2="<<left_factor2<<std::endl;

    double left_factor3=-std::sqrt(3.0) * J_over_a_squared*arma::dot(C_T.col(flattened_ind),Py_arma_vec_curr);


    // std::cout<<"left_factor3="<<left_factor3<<std::endl;

    double left_factor4=J_over_a_squared*arma::dot(R_T.col(flattened_ind),Qx_arma_vec_curr);

    // std::cout<<"left_factor4="<<left_factor4<<std::endl;

    double left_factor5=-2.0 * J_over_a_squared * arma::dot(Gamma_T.col(flattened_ind),Qx_arma_vec_curr);



    // std::cout<<"left_factor5="<<left_factor5<<std::endl;

    double left_factor6 = -2.0 * J_over_a_squared *arma::dot(Theta_T.col(flattened_ind),Qy_arma_vec_curr);

    // std::cout<<"left_factor6="<<left_factor6<<std::endl;

    double left_factor=left_factor1+left_factor2+left_factor3\
                    + left_factor4+left_factor5+left_factor6;

    double E_int_curr = Px_arma_vec_curr(flattened_ind) *left_factor;

    double E_int_next = Px_arma_vec_next(flattened_ind) *left_factor;

    UCurr = H1_self_curr + E_int_curr;

    UNext = H1_self_next + E_int_next;
    // std::cout<<"UCurr="<<UCurr<<std::endl;
    // std::cout<<"UNext="<<UNext<<std::endl;



}


///
/// @param flattened_ind
/// @param Py_arma_vec_curr
/// @param Py_arma_vec_next
/// @param Px_arma_vec_curr
/// @param Qx_arma_vec_curr
/// @param Qy_arma_vec_curr
/// @param UCurr
/// @param UNext
void mc_computation::HPy_update_colForm(const int& flattened_ind,
                const arma::dvec& Py_arma_vec_curr, const arma::dvec& Py_arma_vec_next,
                const arma::dvec& Px_arma_vec_curr, const arma::dvec& Qx_arma_vec_curr,
                const arma::dvec& Qy_arma_vec_curr,
                double& UCurr, double& UNext)
{
    double H1_self_curr = this->H1(flattened_ind, Px_arma_vec_curr, Py_arma_vec_curr);

    double H1_self_next = this->H1(flattened_ind, Px_arma_vec_curr, Py_arma_vec_next);

    double left_factor1=J_over_a_squared*arma::dot(A_T.col(flattened_ind),Py_arma_vec_curr);

// std::cout<<"left_factor1="<<left_factor1<<std::endl;

    double left_factor2=-std::sqrt(3.0) * J_over_a_squared * arma::dot(C_T.col(flattened_ind),Px_arma_vec_curr);
    // std::cout<<"left_factor2="<<left_factor2<<std::endl;


    double left_factor3=-3.0 / 2.0 * J_over_a_squared * arma::dot(G_T.col(flattened_ind),Py_arma_vec_curr);

    // std::cout<<"left_factor3="<<left_factor3<<std::endl;

    double left_factor4=J_over_a_squared * arma::dot(R_T.col(flattened_ind),Qy_arma_vec_curr);


    // std::cout<<"left_factor4="<<left_factor4<<std::endl;

    double left_factor5=-2.0 * J_over_a_squared * arma::dot(Theta_T.col(flattened_ind),Qx_arma_vec_curr);

    // std::cout<<"left_factor5="<<left_factor5<<std::endl;

    double left_factor6=-2.0 * J_over_a_squared * arma::dot(Lambda_T.col(flattened_ind),Qy_arma_vec_curr);

    // std::cout<<"left_factor6="<<left_factor6<<std::endl;

    double left_factor=left_factor1+left_factor2+left_factor3\
                    + left_factor4+left_factor5+left_factor6;

    double E_int_curr = Py_arma_vec_curr(flattened_ind) *left_factor;

    double E_int_next = Py_arma_vec_next(flattened_ind) *left_factor;

    UCurr = H1_self_curr + E_int_curr;

    UNext = H1_self_next + E_int_next;
    // std::cout<<"UCurr="<<UCurr<<std::endl;

    // std::cout<<"UNext="<<UNext<<std::endl;

}

void mc_computation::HQx_update_colForm(const int& flattened_ind,
                    const arma::dvec& Qx_arma_vec_curr, const arma::dvec& Qx_arma_vec_next,
                    const arma::dvec& Px_arma_vec_curr,
                    const arma::dvec& Py_arma_vec_curr,
                    const arma::dvec& Qy_arma_vec_curr,
                    double& UCurr, double& UNext)
{

    double H2_self_curr = this->H2(flattened_ind, Qx_arma_vec_curr, Qy_arma_vec_curr);
    double H2_self_next = this->H2(flattened_ind, Qx_arma_vec_next, Qy_arma_vec_curr);

double left_factor1= J_over_a_squared * arma::dot(R.col(flattened_ind),Px_arma_vec_curr);
    // std::cout<<"left_factor1="<<left_factor1<<std::endl;

    double left_factor2=-2.0 * J_over_a_squared * arma::dot(Gamma.col(flattened_ind),Px_arma_vec_curr);
    // std::cout<<"left_factor2="<<left_factor2<<std::endl;

    double left_factor3=-2.0 * J_over_a_squared * arma::dot(Theta.col(flattened_ind),Py_arma_vec_curr);

    // std::cout<<"left_factor3="<<left_factor3<<std::endl;

    double left_factor4=J_over_a_squared * arma::dot(A_T.col(flattened_ind),Qx_arma_vec_curr);

    // std::cout<<"left_factor4="<<left_factor4<<std::endl;

    double left_factor5=-2.0 * J_over_a_squared * arma::dot(B_T.col(flattened_ind),Qx_arma_vec_curr);

    // std::cout<<"left_factor5="<<left_factor5<<std::endl;

    double left_factor6=-std::sqrt(3.0) * J_over_a_squared*arma::dot(C_T.col(flattened_ind),Qy_arma_vec_curr);

    // std::cout<<"left_factor6="<<left_factor6<<std::endl;

    double left_factor=left_factor1+left_factor2+left_factor3\
                        +left_factor4+left_factor5+left_factor6;
    double E_int_curr = Qx_arma_vec_curr(flattened_ind) *left_factor;
    double E_int_next = Qx_arma_vec_next(flattened_ind) *left_factor;

     UCurr = H2_self_curr + E_int_curr;

    UNext = H2_self_next + E_int_next;


    // std::cout<<"UCurr="<<UCurr<<std::endl;
// std::cout<<"UNext="<<UNext<<std::endl;


}

void mc_computation::HQy_update_colForm(const int& flattened_ind,
                    const arma::dvec& Qy_arma_vec_curr, const arma::dvec& Qy_arma_vec_next,
                    const arma::dvec& Px_arma_vec_curr,
                    const arma::dvec& Py_arma_vec_curr,
                    const arma::dvec& Qx_arma_vec_curr,
                    double& UCurr, double& UNext)
{
    double H2_self_curr = this->H2(flattened_ind, Qx_arma_vec_curr, Qy_arma_vec_curr);

    double H2_self_next = this->H2(flattened_ind, Qx_arma_vec_curr, Qy_arma_vec_next);


    double left_factor1=J_over_a_squared * arma::dot(R.col(flattened_ind),Py_arma_vec_curr);

    // std::cout<<"left_factor1="<<left_factor1<<std::endl;

    double left_factor2=-2.0 * J_over_a_squared * arma::dot(Theta.col(flattened_ind),Px_arma_vec_curr);

    // std::cout<<"left_factor2="<<left_factor2<<std::endl;

    double left_factor3=-2.0 * J_over_a_squared * arma::dot(Lambda.col(flattened_ind),Py_arma_vec_curr);

    // std::cout<<"left_factor3="<<left_factor3<<std::endl;


    double left_factor4=J_over_a_squared * arma::dot(A_T.col(flattened_ind),Qy_arma_vec_curr);

    // std::cout<<"left_factor4="<<left_factor4<<std::endl;

    double left_factor5=-std::sqrt(3.0) * J_over_a_squared * arma::dot(C_T.col(flattened_ind),Qx_arma_vec_curr);

    // std::cout<<"left_factor5="<<left_factor5<<std::endl;

    double left_factor6=-3.0 / 2.0 * J_over_a_squared * arma::dot(G_T.col(flattened_ind),Qy_arma_vec_curr);

    // std::cout<<"left_factor6="<<left_factor6<<std::endl;

    double left_factor=left_factor1+left_factor2+left_factor3\
                    +left_factor4+left_factor5+left_factor6;
    double E_int_curr = Qy_arma_vec_curr(flattened_ind) *left_factor;

    double E_int_next = Qy_arma_vec_next(flattened_ind) *left_factor;
    UCurr = H2_self_curr + E_int_curr;

    UNext = H2_self_next + E_int_next;
    // std::cout<<"UCurr="<<UCurr<<std::endl;
    // std::cout<<"UNext="<<UNext<<std::endl;












}


void mc_computation::execute_mc_one_sweep(arma::dvec& Px_arma_vec_curr,
                              arma::dvec& Py_arma_vec_curr,
                              arma::dvec& Qx_arma_vec_curr,
                              arma::dvec& Qy_arma_vec_curr,
                              double& U_base_value,
                              arma::dvec& Px_arma_vec_next,
                              arma::dvec& Py_arma_vec_next,
                              arma::dvec& Qx_arma_vec_next,
                              arma::dvec& Qy_arma_vec_next)
{
    double UCurr=0;
    double UNext = 0;

    // time count for base U
    double base_U_time=0;
    std::chrono::steady_clock::time_point t_U_base_Start{std::chrono::steady_clock::now()};
    U_base_value=this->H_total(Px_arma_vec_curr,Py_arma_vec_curr,Qx_arma_vec_curr,Qy_arma_vec_curr);
    std::chrono::steady_clock::time_point t_U_base_End{std::chrono::steady_clock::now()};
    std::chrono::duration<double> elapsed_secondsAll_U_base{t_U_base_End - t_U_base_Start};
    base_U_time=elapsed_secondsAll_U_base.count();

    //time count for computing energy
    double energy_time=0;
    std::chrono::steady_clock::time_point t_compute_energy_start;
    std::chrono::steady_clock::time_point t_compute_energy_end;
    std::chrono::duration<double> elapsed_secondsAll_compute_energy_time;

    //time for accept-reject
    double acc_reject_time=0;
    std::chrono::steady_clock::time_point t_acc_reject_start;
    std::chrono::steady_clock::time_point t_acc_reject_end;
    std::chrono::duration<double> elapsed_secondsAll_acc_rejecty_time;

    //update Px
    for (int i = 0; i < N0 * N1; i++)
    {
        //end updating Px
        int flattened_ind = unif_in_0_N0N1(e2);
        // std::cout<<"flattened_ind="<<flattened_ind<<std::endl;

        this->proposal_uni(Px_arma_vec_curr, Px_arma_vec_next, flattened_ind);

        t_compute_energy_start= std::chrono::steady_clock::now();
        this->HPx_update_colForm(flattened_ind, Px_arma_vec_curr, Px_arma_vec_next,
                         Py_arma_vec_curr, Qx_arma_vec_curr, Qy_arma_vec_curr, UCurr, UNext);
        t_compute_energy_end=std::chrono::steady_clock::now();

        elapsed_secondsAll_compute_energy_time=t_compute_energy_end-t_compute_energy_start;
        energy_time+=elapsed_secondsAll_compute_energy_time.count();


        t_acc_reject_start=std::chrono::steady_clock::now();
        double r = this->acceptanceRatio_uni(Px_arma_vec_curr, Px_arma_vec_next,
                                             flattened_ind, UCurr, UNext);
        double u = distUnif01(e2);

        if (u <= r)
        {
            Px_arma_vec_curr = Px_arma_vec_next;
            // UCurr = UNext;
            U_base_value+=UNext-UCurr;
        } //end of accept-reject
        t_acc_reject_end=std::chrono::steady_clock::now();
        elapsed_secondsAll_acc_rejecty_time=t_acc_reject_end-t_acc_reject_start;
        acc_reject_time+=elapsed_secondsAll_acc_rejecty_time.count();
    }//end updating Px

    //update Py
    for (int i = 0; i < N0 * N1; i++)
    {int flattened_ind = unif_in_0_N0N1(e2);
        this->proposal_uni(Py_arma_vec_curr, Py_arma_vec_next, flattened_ind);
        t_compute_energy_start= std::chrono::steady_clock::now();
        this->HPy_update_colForm(flattened_ind, Py_arma_vec_curr, Py_arma_vec_next,
                         Px_arma_vec_curr, Qx_arma_vec_curr, Qy_arma_vec_curr, UCurr, UNext);
        t_compute_energy_end=std::chrono::steady_clock::now();
        elapsed_secondsAll_compute_energy_time=t_compute_energy_end-t_compute_energy_start;
        energy_time+=elapsed_secondsAll_compute_energy_time.count();

        double r = this->acceptanceRatio_uni(Py_arma_vec_curr, Py_arma_vec_next, flattened_ind, UCurr, UNext);
        double u = distUnif01(e2);
        if (u <= r)
        {
            Py_arma_vec_curr = Py_arma_vec_next;
            // UCurr = UNext;
            U_base_value+=UNext-UCurr;
        } //end of accept-reject
    }//end updating Py

    //update Qx
    for (int i = 0; i < N0 * N1; i++)
    {
        int flattened_ind = unif_in_0_N0N1(e2);
        this->proposal_uni(Qx_arma_vec_curr, Qx_arma_vec_next, flattened_ind);

        t_compute_energy_start= std::chrono::steady_clock::now();
        this->HQx_update_colForm(flattened_ind, Qx_arma_vec_curr, Qx_arma_vec_next,
                         Px_arma_vec_curr, Py_arma_vec_curr,
                         Qy_arma_vec_curr,
                         UCurr, UNext);

        t_compute_energy_end=std::chrono::steady_clock::now();
        elapsed_secondsAll_compute_energy_time=t_compute_energy_end-t_compute_energy_start;
        energy_time+=elapsed_secondsAll_compute_energy_time.count();

        double r = this->acceptanceRatio_uni(Qx_arma_vec_curr, Qx_arma_vec_next, flattened_ind, UCurr, UNext);
        double u = distUnif01(e2);
        if (u <= r)
        {
            Qx_arma_vec_curr = Qx_arma_vec_next;
            // UCurr = UNext;
            U_base_value+=UNext-UCurr;
        } //end of accept-reject
    }//end updating Qx

    //update Qy
    for (int i = 0; i < N0 * N1; i++)
    {
        int flattened_ind = unif_in_0_N0N1(e2);
        this->proposal_uni(Qy_arma_vec_curr, Qy_arma_vec_next, flattened_ind);

        t_compute_energy_start= std::chrono::steady_clock::now();
        this->HQy_update_colForm(flattened_ind, Qy_arma_vec_curr, Qy_arma_vec_next,
                         Px_arma_vec_curr,
                         Py_arma_vec_curr,
                         Qx_arma_vec_curr,
                         UCurr, UNext);
        t_compute_energy_end=std::chrono::steady_clock::now();
        elapsed_secondsAll_compute_energy_time=t_compute_energy_end-t_compute_energy_start;
        energy_time+=elapsed_secondsAll_compute_energy_time.count();
        double r=this->acceptanceRatio_uni(Qy_arma_vec_curr,Qy_arma_vec_next,flattened_ind,UCurr,
            UNext);
        double u = distUnif01(e2);

        if (u <= r)
        {
            Qy_arma_vec_curr=Qy_arma_vec_next;
            // UCurr = UNext;
            U_base_value+=UNext-UCurr;
        }//end of accept-reject


    }//end updating Qy

}

void mc_computation::execute_mc(const std::shared_ptr<double[]>& Px_vec,
                                const std::shared_ptr<double[]>& Py_vec,
                                const std::shared_ptr<double[]>& Qx_vec,
                                const std::shared_ptr<double[]>& Qy_vec,
                                const int& flushNum)
{
    arma::dvec Px_arma_vec_curr(Px_vec.get(), N0 * N1);
    arma::dvec Px_arma_vec_next(N0 * N1, arma::fill::zeros);

    arma::dvec Py_arma_vec_curr(Py_vec.get(), N0 * N1);
    arma::dvec Py_arma_vec_next(N0 * N1, arma::fill::zeros);

    arma::dvec Qx_arma_vec_curr(Qx_vec.get(), N0 * N1);
    arma::dvec Qx_arma_vec_next(N0 * N1, arma::fill::zeros);

    arma::dvec Qy_arma_vec_curr(Qy_vec.get(), N0 * N1);
    arma::dvec Qy_arma_vec_next(N0 * N1, arma::fill::zeros);

    // double UCurr=0;
    double U_base_value=-12345;

    int flushThisFileStart=this->flushLastFile+1;

    for (int fls = 0; fls < flushNum; fls++)
    {
        const auto tMCStart{std::chrono::steady_clock::now()};
        for (int swp = 0; swp < sweepToWrite*sweep_multiple; swp++)
        {
            this->execute_mc_one_sweep(Px_arma_vec_curr,
                Py_arma_vec_curr,
                Qx_arma_vec_curr,
                Qy_arma_vec_curr,
                U_base_value,
                Px_arma_vec_next,
                Py_arma_vec_next,
                Qx_arma_vec_next,
                Qy_arma_vec_next);

            if(swp%sweep_multiple==0)
            {
                int swp_out=swp/sweep_multiple;
                this->U_data_all_ptr[swp_out]=U_base_value;
                std::memcpy(Px_all_ptr.get()+swp_out*N0*N1,Px_arma_vec_curr.memptr(),N0*N1*sizeof(double));
                std::memcpy(Py_all_ptr.get()+swp_out*N0*N1,Py_arma_vec_curr.memptr(),N0*N1*sizeof(double));
                std::memcpy(Qx_all_ptr.get()+swp_out*N0*N1,Qx_arma_vec_curr.memptr(),N0*N1*sizeof(double));
                std::memcpy(Qy_all_ptr.get()+swp_out*N0*N1,Qy_arma_vec_curr.memptr(),N0*N1*sizeof(double));
            }//end save to array

        }//end sweep for

        int flushEnd=flushThisFileStart+fls;
        std::string fileNameMiddle =  "flushEnd" + std::to_string(flushEnd);

        std::string out_U_PickleFileName = out_U_path+"/" + fileNameMiddle + ".U.pkl";

        std::string out_Px_PickleFileName=out_Px_path+"/"+fileNameMiddle+".Px.pkl";

        std::string out_Py_PickleFileName=out_Py_path+"/"+fileNameMiddle+".Py.pkl";

        std::string out_Qx_PickleFileName=out_Qx_path+"/"+fileNameMiddle+".Qx.pkl";

        std::string out_Qy_PickleFileName=out_Qy_path+"/"+fileNameMiddle+".Qy.pkl";
        //save U
        this->save_array_to_pickle(U_data_all_ptr,sweepToWrite,out_U_PickleFileName);

        //save Px
        this->save_array_to_pickle(Px_all_ptr,sweepToWrite*N0*N1,out_Px_PickleFileName);
        //save Py
        this->save_array_to_pickle(Py_all_ptr,sweepToWrite*N0*N1,out_Py_PickleFileName);
        //save Qx
        this->save_array_to_pickle(Qx_all_ptr,sweepToWrite*N0*N1,out_Qx_PickleFileName);

        //save Qy
        this->save_array_to_pickle(Qy_all_ptr,sweepToWrite*N0*N1,out_Qy_PickleFileName);

        const auto tMCEnd{std::chrono::steady_clock::now()};
        const std::chrono::duration<double> elapsed_secondsAll{tMCEnd - tMCStart};
        std::cout << "flush " + std::to_string(flushEnd)  + ": "
                  << elapsed_secondsAll.count() / 3600.0 << " h" << std::endl;
    }//end flush for loop

    std::cout << "mc executed for " << flushNum << " flushes." << std::endl;


}

void mc_computation::init_and_run()
{
    this->compute_seed_vecs();
    this->compute_rotated_sets();

    // this->init_mats();
    this->init_mats_C3();
    this->init_Px_Py_Qx_Qy();
    this->execute_mc(Px_init,Py_init,Qx_init,Qy_init,newFlushNum);

    // int flattened_ind=47;
    // arma::dvec Px_arma_vec_curr = arma::randu<arma::dvec>(N0*N1);
    // arma::dvec Px_arma_vec_next = arma::randu<arma::dvec>(N0*N1);
    // arma::dvec Py_arma_vec_curr=arma::randu<arma::dvec>(N0*N1);
    // arma::dvec Qx_arma_vec_curr=arma::randu<arma::dvec>(N0*N1);
    // arma::dvec Qy_arma_vec_curr=arma::randu<arma::dvec>(N0*N1);
    // double UCurr,UNext;
    // this->HQy_update_colForm(flattened_ind,Px_arma_vec_curr,Px_arma_vec_next,
    //     Py_arma_vec_curr,Qx_arma_vec_curr,
    //     Qy_arma_vec_curr,UCurr,UNext);
}


///
/// @param Px_arma_vec
/// @param Py_arma_vec
/// @return total self energy H1
double mc_computation::sum_H1(const arma::dvec& Px_arma_vec, const arma::dvec& Py_arma_vec)
{

    double sumVal=0;
    for (int flat_ind=0;flat_ind<N0*N1;flat_ind++)
    {
        sumVal+=this->H1(flat_ind,Px_arma_vec,Py_arma_vec);
    }//end for

    return sumVal;

}


///
/// @param Qx_arma_vec
/// @param Qy_arma_vec
/// @return total self energy H2
double mc_computation::sum_H2(const arma::dvec& Qx_arma_vec, const arma::dvec& Qy_arma_vec)
{

    double sumVal=0;
    for (int flat_ind=0;flat_ind<N0*N1;flat_ind++)
    {
        sumVal+=this->H2(flat_ind,Qx_arma_vec,Qy_arma_vec);
    }//end for
    return sumVal;
}



///
/// @param Px_arma_vec
/// @param Py_arma_vec
/// @return 1/2 * total H3
double mc_computation::half_sum_H3(const arma::dvec& Px_arma_vec, const arma::dvec& Py_arma_vec)
{
double a_squared=std::pow(a,2.0);

double val0=J/(2.0*a_squared)
    *arma::as_scalar(Px_arma_vec.t()*this->A*Px_arma_vec);


    double val1=J/(2.0*a_squared)
    *arma::as_scalar(Py_arma_vec.t()*this->A*Py_arma_vec);


    double val2=-J/a_squared*arma::as_scalar(Px_arma_vec.t()*this->B*Px_arma_vec);

    double val3=-std::sqrt(3.0)*J/(2.0*a_squared)*arma::as_scalar(Px_arma_vec.t()*this->C*Py_arma_vec);

    double val4=-std::sqrt(3.0)*J/(2.0*a_squared)*arma::as_scalar(Py_arma_vec.t()*this->C*Px_arma_vec);

    double val5=-3.0/4.0*J/a_squared*arma::as_scalar(Py_arma_vec.t()*this->G*Py_arma_vec);


    return val0+val1+val2+val3+val4+val5;


}


///
/// @param Px_arma_vec
/// @param Py_arma_vec
/// @param Qx_arma_vec
/// @param Qy_arma_vec
/// @return total H4
double mc_computation::sum_H4(const arma::dvec& Px_arma_vec,const arma::dvec& Py_arma_vec,
    const arma::dvec& Qx_arma_vec,const arma::dvec& Qy_arma_vec)
{

    double a_squared=std::pow(a,2.0);

    double val0=J/a_squared
                *arma::as_scalar(Px_arma_vec.t()*this->R*Qx_arma_vec);

    double val1=J/a_squared
                *arma::as_scalar(Py_arma_vec.t()*this->R*Qy_arma_vec);

    double val2=-2.0*J/a_squared
                *arma::as_scalar(Px_arma_vec.t()*this->Gamma*Qx_arma_vec);

    double val3=-2.0*J/a_squared
                *arma::as_scalar(Px_arma_vec.t()*this->Theta*Qy_arma_vec);

    double val4=-2.0*J/a_squared
                *arma::as_scalar(Py_arma_vec.t()*this->Theta*Qx_arma_vec);

    double val5=-2.0*J/a_squared
                *arma::as_scalar(Py_arma_vec.t()*this->Lambda*Qy_arma_vec);


    return val0+val1+val2+val3+val4+val5;

}


///
/// @param Qx_arma_vec
/// @param Qy_arma_vec
/// @return 1/2* total H5
double mc_computation::half_sum_H5(const arma::dvec& Qx_arma_vec,const arma::dvec& Qy_arma_vec)
{
    double a_squared=std::pow(a,2.0);

    double val0=J/(2.0*a_squared)
                *arma::as_scalar(Qx_arma_vec.t()*this->A*Qx_arma_vec);

    double val1=J/(2.0*a_squared)
                *arma::as_scalar(Qy_arma_vec.t()*this->A*Qy_arma_vec);

    double val2=-J/a_squared
                *arma::as_scalar(Qx_arma_vec.t()*this->B*Qx_arma_vec);

    double val3=-std::sqrt(3.0)/(2.0*a_squared)
                *arma::as_scalar(Qx_arma_vec.t()*this->C*Qy_arma_vec);

    double val4=-std::sqrt(3.0)/(2.0*a_squared)
                *arma::as_scalar(Qy_arma_vec.t()*this->C*Qx_arma_vec);

    double val5=-3.0/4.0*J/a_squared
                *arma::as_scalar(Qy_arma_vec.t()*this->G*Qy_arma_vec);


    return val0+val1+val2+val3+val4+val5;


}

///
/// @param Px_arma_vec
/// @param Py_arma_vec
/// @param Qx_arma_vec
/// @param Qy_arma_vec
/// @return total energy
double mc_computation::H_total(const arma::dvec& Px_arma_vec,const arma::dvec& Py_arma_vec,
               const arma::dvec& Qx_arma_vec,const arma::dvec& Qy_arma_vec)
{

    double sum_H1_val=this->sum_H1(Px_arma_vec,Py_arma_vec);

    double sum_H2_val=this->sum_H2(Qx_arma_vec,Qy_arma_vec);

    double half_sum_H3_val=this->half_sum_H3(Px_arma_vec,Py_arma_vec);

    double sum_H4_val=this->sum_H4(Px_arma_vec,Py_arma_vec,
                                    Qx_arma_vec,Qy_arma_vec);

    double half_sum_H5_val=this->half_sum_H5(Qx_arma_vec,Qy_arma_vec);

    return sum_H1_val+sum_H2_val+half_sum_H3_val+sum_H4_val+half_sum_H5_val;


}