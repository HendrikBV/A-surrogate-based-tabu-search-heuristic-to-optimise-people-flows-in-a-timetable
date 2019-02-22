/*
 *	Code for the surrogate-based tabu search algorithm
 *  to optimise people flows in a timetable.
 *
 *	Code author: Hendrik Vermuyten
 */

/*!
 *	@file		machine_learning_interface.h
 *  @author     Hendrik Vermuyten
 *	@brief		The specification of a class that implements various machine learning tools
 */


#ifndef MACHINE_LEARNING_INTERFACE_H
#define MACHINE_LEARNING_INTERFACE_H

#include <QObject>

#include <vector>
#include <utility>
#include <QString>
#include <chrono>
#include <random>
#include <unordered_map>
#include <algorithm>

#include <dlib/svm.h>
#include <dlib/global_optimization.h>

#include "timetable_solution.h"
#include "menge_interface.h"

#include "logger.h"


/*!
 *  @namespace ml
 *  @brief	The namespace containing all machine learning elements.
 */
namespace ml
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    typedef dlib::matrix<double,0,1> sample_type;   ///< column vector; number of rows set at runtime

    typedef dlib::radial_basis_kernel<sample_type> radial_basis_kernel;
    typedef dlib::linear_kernel<sample_type> linear_kernel;
    typedef dlib::polynomial_kernel<sample_type> polynomial_kernel;
    typedef dlib::histogram_intersection_kernel<sample_type> histogram_intersection_kernel;

    typedef dlib::decision_function<radial_basis_kernel> decision_function_radial_basis_kernel;
    typedef dlib::decision_function<linear_kernel> decision_function_linear_kernel;
    typedef dlib::decision_function<polynomial_kernel> decision_function_polynomial_kernel;
    typedef dlib::decision_function<histogram_intersection_kernel> decision_function_histogram_intersection_kernel;



    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /*!
     *	@brief		Classifies available machine learning methods.
     */
    enum class learning_method
    {
        krr_trainer_radial_basis_kernel,                ///< Kernel Ridge Regression with Radial Basis Kernel
        krr_trainer_histogram_intersection_kernel,      ///< Kernel Ridge Regression with Histogram Intersection Kernel
        krr_trainer_polynomial_kernel_quadratic,        ///< Kernel Ridge Regression with Quadratic Polynomial Kernel
        krr_trainer_polynomial_kernel_cubic,            ///< Kernel Ridge Regression with Cubic Polynomial Kernel
        krr_trainer_linear_kernel,                      ///< Kernel Ridge Regression with Linear Kernel
        svr_trainer_radial_basis_kernel,                ///< Support Vector Regression with Radial Basis Kernel
        svr_trainer_histogram_intersection_kernel,      ///< Support Vector Regression with Histogram Intersection Kernel
        svr_trainer_polynomial_kernel_quadratic,        ///< Support Vector Regression with Quadratic Polynomial Kernel
        svr_trainer_polynomial_kernel_cubic,            ///< Support Vector Regression with Cubic Polynomial Kernel
        svr_trainer_linear_kernel,                      ///< Support Vector Regression with Linear Polynomial Kernel
        rvm_trainer_radial_basis_kernel,                ///< Relevance Vector Machine Regression with Radial Basis Kernel
        rvm_trainer_histogram_intersection_kernel,      ///< Relevance Vector Machine Regression with Histogram Intersection Kernel
        rvm_trainer_polynomial_kernel_quadratic,        ///< Relevance Vector Machine Regression with Quadratic Polynomial Kernel
        rvm_trainer_polynomial_kernel_cubic,            ///< Relevance Vector Machine Regression with Cubic Polynomial Kernel
        rvm_trainer_linear_kernel                       ///< Relevance Vector Machine Regression with Linear Kernel
    };



    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /*!
     *	@brief		Object to store all decision functions that are generated to find the best learning method.
     */
    struct decision_functions
    {
        decision_function_radial_basis_kernel dec_func_rbk;                 ///< Decision function
        decision_function_histogram_intersection_kernel dec_func_hik;
        decision_function_linear_kernel dec_func_lk;
        decision_function_polynomial_kernel dec_func_pk;
    };



    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /*!
     *	@brief		Object to store all decision functions that are generated to find the best learning method.
     */
    class surrogate_paths: public QObject
    {
        Q_OBJECT


    public:
        /*!
         *	@brief		Read in data containing the graph specification.
         *  @param      filename        The name of the .txt-file containing the specificiation.
         */
        void read_data(const QString& filename);

        /*!
         *	@brief		Deletes all data.
         */
        void clear_data();

        /*!
         *	@brief		Indicates whether the required data been specified.
         *  @returns    True if the required data have been specified, false otherwise.
         */
        bool data_exist() const { return _data_exist; }

        /*!
         *	@brief		Transforms a solution to a sample_type that can be used to predict the travel time using the surrogate model.
         *  @param      sol     A timetable::solution for which the travel time is to be predicted.
         *  @param      first_timeslot        The first timeslot for which the travel time is to be predicted.
         *  @returns    A sample_type that serves as input to the surrogate model.
         */
        sample_type transfrom_solution_to_ml_sample(const timetable::solution& sol, int first_timeslot) const;

        /*!
         *	@brief		Generate observations as input for the machine learning tool.
         *  @param      x_obs       Vector to store the x observations (independent variables).
         *  @param      y_obs       Vector to store the y observations (dependent variable).
         */
        void generate_observations(ped::menge_interface *menge, std::vector<sample_type>& x_obs, std::vector<double>& y_obs, int nb_observations);


    private:
        /*!
         *	@brief		Have the required data been specified.
         */
        bool _data_exist = false;

        /*!
         *	@brief		The number of nodes in the representation, including the exit node.
         */
        size_t _nb_nodes = 0;

        /*!
         *	@brief		The number of paths in the representation, including to the exit node.
         */
        size_t _nb_paths = 0;

        /*!
         *	@brief		A vector that stores the node corresponding to each room.
         */
        std::vector<int> _room_node;

        /*!
         *	@brief		A vector that stores the path corresponding to a pair of nodes.
         */
        std::vector<int> _node_node_path;

        /*!
         *	@brief		A vector that stores the rooms corresponding to a each path.
         */
        std::vector<std::vector<int>> _paths_rooms;


    signals:
        /*!
         *	@brief      Emits a string that contains information about the status of the machine learning algorithm.
         */
        void signal_stat(QString);

    };



    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /*!
     *	@brief		Class that implements the surrogate models.
     */
    class machine_learning_interface: public QObject
    {
        Q_OBJECT


    public:
        /*!
         *	@brief		Default constructor.
         */
        machine_learning_interface();

        /*!
         *	@brief		Destructor.
         */
        ~machine_learning_interface();

        /*!
         *	@brief		Predicts the evacuation time for a given solution and timeslot.
         *
         *  It is assumed that training of the surrogates has already occurred.
         *
         *  @param      sol         A constant reference to the solution for which the evacuation time is to be predicted.
         *  @param      timeslot    The timeslot for which the evacuation time is to be predicted.
         *  @returns    The predicted evacuation time.
         */
        double predict_evacuation_time(const timetable::solution& sol, int timeslot) const;

        /*!
         *	@brief		Predicts the travel time between consecutive events for a given solution and timeslot.
         *
         *  It is assumed that training of the surrogates has already occurred.
         *
         *  @param      sol         A constant reference to the solution for which the travel time is to be predicted.
         *  @param      timeslot    The (first) timeslot where the flows originate.
         *  @returns    The predicted travel time.
         */
        double predict_travel_time(const timetable::solution& sol, int first_timeslot) const;

        /*!
         *	@brief		Sets the menge interface.
         *
         *  @param      menge       A pointer to a menge interface object.
         */
        void set_menge_interface(ped::menge_interface *menge) { _menge = menge; }

        /*!
         *	@brief		Sets the surrogate_paths object.
         *
         *  @param      menge       A pointer to a surrogate_paths object.
         */
        void set_surrogate_paths(ml::surrogate_paths *sp) { _surrogate_paths = sp; }

        /*!
         *	@brief		Import a decision function to predict evacuations.
         *
         *  If during a previous run of the algorithm, a decision function has been derived and save to a file,
         *  this function can retrieve this function, so that it is not necessary to start the machine learning
         *  from scratch.
         *
         *  @param      file_name       The name of the file in which the decision function is saved.
         */
        void import_decision_function_evacuations(const QString& file_name);

        /*!
         *	@brief		Import a decision function to predict travel times between consecutive events.
         *
         *  If during a previous run of the algorithm, a decision function has been derived and save to a file,
         *  this function can retrieve this function, so that it is not necessary to start the machine learning
         *  from scratch.
         *
         *  @param      file_name       The name of the file in which the decision function is saved.
         */
        void import_decision_function_travels(const QString& filename);

        /*!
         *	@brief		Save a decision function to predict evacuations.
         *
         *  Save the decision function that has been learned, so that next time the algorithm is run,
         *  the learned decision function can be imported and used instead of starting the machine learning
         *  from scratch again.
         */
        void save_decision_function_evacuations();

        /*!
         *	@brief		Save a decision function to predict travel times between consecutive events.
         *
         *  Save the decision function that has been learned, so that next time the algorithm is run,
         *  the learned decision function can be imported and used instead of starting the machine learning
         *  from scratch again.
         */
        void save_decision_function_travels();

        /*!
         *	@brief		Trains the surrogates for the required objective functions given the choice of learning method.
         *
         *  If alpha < 0.01, only a surrogate model for evacuations is trained. If alpha > 0.99 only a surrogate model
         *  for travel between consecutive events is trained. Otherwise, both surrogate models are trained.
         *
         *  @param      alpha       The weight of both objective function elements,
         *                          viz. evacuations and travels between consecutive events.
         *  @param      lm          The learning method used in the surrogates.
         */
        void train();

        /*!
         *	@brief		Compare different learning methods and return the best one.
         *
         *  If alpha < 0.01, only a surrogate model for evacuations is trained. If alpha > 0.99 only a surrogate model
         *  for travel between consecutive events is trained. Otherwise, both surrogate models are trained.
         *
         *  @param      alpha       The weight of both objective function elements,
         *                          viz. evacuations and travels between consecutive events.
         *  @returns    The best learning method.
         */
        void compare_learning_methods();

        /*!
         *	@brief		Set the number of training data that is used to train the surrogates.
         *  @param      nb      The number of training data.
         */
        void set_number_training_data(int nb) { _nb_training_data = nb; }

        /*!
         *	@brief		Get the number of training data that is used to train the surrogates.
         *  @returns    The number of training data.
         */
        int get_number_training_data() const { return _nb_training_data; }

        /*!
         *	@brief      Set the learning method that is used to train the surrogates.
         *  @param      lm      The learning method.
         */
        void set_learning_method(learning_method lm) { _learning_method = lm; }

        /*!
         *	@brief      Get the learning method that is used to train the surrogates.
         *  @returns    The learning method.
         */
        learning_method get_learning_method() const { return _learning_method; }

        /*!
         *	@brief      Set the importance of both evacuations and travels.
         *  @param      alpha      The importance of both evacuations and travels.
         */
        void set_alpha_objective(double alpha) { _alpha_objective = alpha; }

        /*!
         *	@brief      Get the importance of both evacuations and travels.
         *  @returns    The importance of both evacuations and travels.
         */
        double get_alpha_objective() const { return _alpha_objective; }

        /*!
         *	@brief      To check whether surrogates have been trained before trying to use them.
         *  @returns    True if the correct models have been trained given the choice of alpha.
         */
        bool is_trained() const;

        /*!
         *	@brief      Clear all training data.
         */
        void clear();



    private:
        /*!
         *	@brief      Pointer to menge object to run the pedestrian simulations (no ownership).
         */
        ped::menge_interface *_menge;

        /*!
         *	@brief      Computation time in seconds that was required to generate training data for evacuations.
         */
        double _time_generating_data_evac;

        /*!
         *	@brief      Computation time in seconds that was required to generate training data for travel between consecutive events.
         */
        double _time_generating_data_travels;

        /*!
         *	@brief      Computation time in seconds that was required to train the surrogate for evacuations.
         */
        double _time_training_ml_evac;

        /*!
         *	@brief      Computation time in seconds that was required to train the surrogate for travel between consecutive events.
         */
        double _time_training_ml_travels;

        /*!
         *	@brief      The independent variables of the training data for evacuations.
         *
         *  A vector in which each element is an observation.
         *  Each observations is a column vector where element i indicates the number of people present in room i.
         */
        std::vector<sample_type> _training_data_evac_x;

        /*!
         *	@brief      The dependent variable of the training data for evacuations.
         *
         *  A vector in which each element is an observation, i.e. the observed evacuation time corresponding to that observation.
         */
        std::vector<double> _training_data_evac_y;

        /*!
         *	@brief      The independent variables of the training data for travels between consecutive events.
         *
         *  A vector in which each element is an observation.
         *  Each observations is a column vector with the number of people on all possible paths in the building.
         */
        std::vector<sample_type> _training_data_travels_x;

        /*!
         *	@brief      The dependent variable of the training data for evacuations.
         *
         *  A vector in which each element is an observation, i.e. the observed evacuation time corresponding to that observation.
         */
        std::vector<double> _training_data_travels_y;

        /*!
         *	@brief      How many observations will be used to train the surrogate models.
         */
        size_t _nb_training_data = 500;

        /*!
         *	@brief      Which machine learning method will be used to train the surrogate models.
         */
        learning_method _learning_method = learning_method::svr_trainer_radial_basis_kernel;

        /*!
         *	@brief      Are both objectives considered or not?
         *
         *  If alpha = 0, only evacuations; if alpha = 1, only travel between events in consecutive timeslots, otherwise, both.
         */
        double _alpha_objective = 0.0;

        /*!
         *	@brief      Has the surrogate model for evacuations been trained.
         */
        bool _trained_surrogate_evacuations = false;

        /*!
         *	@brief      Has the surrogate model for travels been trained.
         */
        bool _trained_surrogate_travels = false;

        /*!
         *	@brief      The decision functions that have been learned to predict evacuation times.
         */
        decision_functions _decision_functions_evacuations;

        /*!
         *	@brief      The decision functions that have been learned to predict travel times.
         */
        decision_functions _decision_functions_travels;



        /*!
         *	@brief      The minimum value for the gamma parameter for Radial Basis Kernels.
         *
         *  A Radial Basis Kernel works with 'exp(-gamma* ||a-b||^2)'.
         */
        static constexpr double _rbk_gamma_min = 1e-5;

        /*!
         *	@brief      The maximum value for the gamma parameter for Radial Basis Kernels.
         *
         *  A Radial Basis Kernel works with 'exp(-gamma* ||a-b||^2)'.
         */
        static constexpr double _rbk_gamma_max = 100.0;

        /*!
         *	@brief      The minimum value for the gamma parameter for Polynomial Kernels.
         *
         *  A Polynomial Kernel works with 'pow(gamma*trans(a)*b + coef, degree)'.
         */
        static constexpr double _pk_gamma_min = 1e-5;

        /*!
         *	@brief      The maximum value for the gamma parameter for Polynomial Kernels.
         *
         *  A Polynomial Kernel works with 'pow(gamma*trans(a)*b + coef, degree)'.
         */
        static constexpr double _pk_gamma_max = 100.0;

        /*!
         *	@brief      The minimum value for the gamma parameter for Polynomial Kernels.
         *
         *  A Polynomial Kernel works with 'pow(gamma*trans(a)*b + coef, degree)'.
         */
        static constexpr double _pk_coef_min = 0.0;

        /*!
         *	@brief      The maximum value for the gamma parameter for Polynomial Kernels.
         *
         *  A Polynomial Kernel works with 'pow(gamma*trans(a)*b + coef, degree)'.
         */
        static constexpr double _pk_coef_max = 1e6;

        ///< Linear Kernel has no parameters. It works with 'trans(a)*b'.

        ///< Histogram Intersection Kernel has no parameters. It works with 'sum over all i: std::min(a(i), b(i))'.

        /*!
         *	@brief      The minimum value for the lambda parameter (regularization) for Kernel Ridge Regression.
         *
         *  Smaller values encourage exact fitting while larger values of lambda may encourage better generalization.
         */
        static constexpr double _krr_lambda_min = 1e-5;

        /*!
         *	@brief      The maximum value for the lambda parameter (regularization) for Kernel Ridge Regression.
         *
         *  Smaller values encourage exact fitting while larger values of lambda may encourage better generalization.
         */
        static constexpr double _krr_lambda_max = 1e6;

        /*!
         *	@brief      The minimum value for the c parameter (regularization) for Support Vector Regression.
         *
         *  Larger values encourage exact fitting while smaller values of C may encourage better generalization.
         */
        static constexpr double _svr_c_min = 1e-5;

        /*!
         *	@brief      The maximum value for the c parameter (regularization) for Support Vector Regression.
         *
         *  Larger values encourage exact fitting while smaller values of C may encourage better generalization.
         */
        static constexpr double _svr_c_max = 1e6;




        /*!
         *	@brief      Uses Latin Hypercube Sampling to generate training data for evacuations.
         */
        void generate_training_data_evacuations();

        /*!
         *	@brief      Uses Latin Hypercube Sampling to generate training data for travel times.
         */
        void generate_training_data_travels();



        /*!
         *	@brief      Function to train a krr_trainer with radial_basis_kernel.
         */
        void train_krr_radial_basis_evacuations();

        /*!
         *	@brief      Function to train a krr_trainer with histogram_intersection_kernel.
         */
        void train_krr_histogram_intersection_evacuations();

        /*!
         *	@brief      Function to train a krr_trainer with linear_kernel.
         */
        void train_krr_linear_evacuations();

        /*!
         *	@brief      Function to train a krr_trainer with polynomial_kernel.
         */
        void train_krr_quadratic_evacuations();

        /*!
         *	@brief      Function to train a krr_trainer with polynomial_kernel.
         */
        void train_krr_cubic_evacuations();

        /*!
         *	@brief      Function to train a svr_trainer with radial_basis_kernel.
         */
        void train_svr_radial_basis_evacuations();

        /*!
         *	@brief      Function to train a svr_trainer with histogram_intersection_kernel.
         */
        void train_svr_histogram_intersection_evacuations();

        /*!
         *	@brief      Function to train a svr_trainer with linear_kernel.
         */
        void train_svr_linear_evacuations();

        /*!
         *	@brief      Function to train a svr_trainer with polynomial_kernel.
         */
        void train_svr_quadratic_evacuations();

        /*!
         *	@brief      Function to train a svr_trainer with polynomial_kernel.
         */
        void train_svr_cubic_evacuations();

        /*!
         *	@brief      Function to train a rvm_trainer with radial_basis_kernel.
         */
        void train_rvm_radial_basis_evacuations();

        /*!
         *	@brief      Function to train a rvm_trainer with histogram_intersection_kernel.
         */
        void train_rvm_histogram_intersection_evacuations();

        /*!
         *	@brief      Function to train a rvm_trainer with linear_kernel.
         */
        void train_rvm_linear_evacuations();

        /*!
         *	@brief      Function to train a rvm_trainer with polynomial_kernel.
         */
        void train_rvm_quadratic_evacuations();

        /*!
         *	@brief      Function to train a rvm_trainer with polynomial_kernel.
         */
        void train_rvm_cubic_evacuations();



        /*!
         *	@brief      Function to train a krr_trainer with radial_basis_kernel.
         */
        void train_krr_radial_basis_travels();

        /*!
         *	@brief      Function to train a krr_trainer with histogram_intersection_kernel.
         */
        void train_krr_histogram_intersection_travels();

        /*!
         *	@brief      Function to train a krr_trainer with linear_kernel.
         */
        void train_krr_linear_travels();

        /*!
         *	@brief      Function to train a krr_trainer with polynomial_kernel.
         */
        void train_krr_quadratic_travels();

        /*!
         *	@brief      Function to train a krr_trainer with polynomial_kernel.
         */
        void train_krr_cubic_travels();

        /*!
         *	@brief      Function to train a svr_trainer with radial_basis_kernel.
         */
        void train_svr_radial_basis_travels();

        /*!
         *	@brief      Function to train a svr_trainer with histogram_intersection_kernel.
         */
        void train_svr_histogram_intersection_travels();

        /*!
         *	@brief      Function to train a svr_trainer with linear_kernel.
         */
        void train_svr_linear_travels();

        /*!
         *	@brief      Function to train a svr_trainer with polynomial_kernel.
         */
        void train_svr_quadratic_travels();

        /*!
         *	@brief      Function to train a svr_trainer with polynomial_kernel.
         */
        void train_svr_cubic_travels();

        /*!
         *	@brief      Function to train a rvm_trainer with radial_basis_kernel.
         */
        void train_rvm_radial_basis_travels();

        /*!
         *	@brief      Function to train a rvm_trainer with histogram_intersection_kernel.
         */
        void train_rvm_histogram_intersection_travels();

        /*!
         *	@brief      Function to train a rvm_trainer with linear_kernel.
         */
        void train_rvm_linear_travels();

        /*!
         *	@brief      Function to train a rvm_trainer with polynomial_kernel.
         */
        void train_rvm_quadratic_travels();

        /*!
         *	@brief      Function to train a rvm_trainer with polynomial_kernel.
         */
        void train_rvm_cubic_travels();




        /*!
         *  @brief      Pointer to a surrogate_paths object.
         */
        surrogate_paths *_surrogate_paths;



    signals:
        /*!
         *	@brief		Passes a text to indicate an error occured.
         */
        void signal_error(QString);

        /*!
         *	@brief      Emits a string that contains information about the status of the machine learning algorithm.
         */
        void signal_status(QString);


    };

} // namespace ml

#endif // MACHINE_LEARNING_INTERFACE_H
