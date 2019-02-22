#include "timetable_tabu_search.h"
#include <chrono>
#include <random>
#include <stdexcept>
#include <QDebug>


namespace
{
    std::random_device randdev;
    std::seed_seq seedseq{ randdev(), randdev(), randdev(), randdev(), randdev(), randdev(), randdev(), randdev() };
    std::mt19937_64 generator;

    constexpr double NANO = 1000000000.0;

    double student_t_values(int degrees_of_freedom)
    {
        switch(degrees_of_freedom)
        {
        case 1:
            return 12.710;
        case 2:
            return 4.303;
        case 3:
            return 3.182;
        case 4:
            return 2.776;
        case 5:
            return 2.571;
        case 6:
            return 2.447;
        case 7:
            return 2.365;
        case 8:
            return 2.306;
        case 9:
            return 2.262;
        case 10:
            return 2.228;
        case 11:
            return 2.201;
        case 12:
            return 2.179;
        case 13:
            return 2.160;
        case 14:
            return 2.145;
        case 15:
            return 2.131;
        case 16:
            return 2.120;
        case 17:
            return 2.110;
        case 18:
            return 2.101;
        case 19:
            return 2.093;
        case 20:
            return 2.086;
        default:
            return 2.000;
        }
    }

    // Mean Squared Error
    double calculate_MSE(const std::vector<double>& real, const std::vector<double>& predicted)
    {
        if(real.size() != predicted.size())
            throw std::logic_error("Error in \"calculate_MSE\": vectors of real and predicted values do not have same size");

        double MSE = 0.0;
        for(size_t i = 0; i < real.size(); ++i)
            MSE += (real[i] - predicted[i])*(real[i] - predicted[i]);
        MSE /= (double)real.size();

        return MSE;
    }

    // Mean Average Error
    double calculate_MAE(const std::vector<double>& real, const std::vector<double>& predicted)
    {
        if(real.size() != predicted.size())
            throw std::logic_error("Error in \"calculate_MAE\": vectors of real and predicted values do not have same size");

        double MAE = 0.0;
        for(size_t i = 0; i < real.size(); ++i)
            MAE += std::abs(real[i] - predicted[i]);
        MAE /= (double)real.size();

        return MAE;
    }

    // Pearson correlation coefficient between predicted and real
    double calculate_PCC(const std::vector<double>& real, const std::vector<double>& predicted)
    {
        if(real.size() != predicted.size())
            throw std::logic_error("Error in \"calculate_PCC\": vectors of real and predicted values do not have same size");

        double sum_R = 0.0, sum_P = 0.0, sum_RP = 0.0;
        double squaresum_R = 0.0, squaresum_P = 0.0;
        for(size_t i = 0; i < real.size(); ++i)
        {
            sum_R += real[i];
            sum_P += predicted[i];
            sum_RP += (real[i]*predicted[i]);
            squaresum_R += (real[i]*real[i]);
            squaresum_P += (predicted[i]*predicted[i]);
        }

        double PCC = (real.size()*sum_RP - sum_R*sum_P) / std::sqrt((real.size()*squaresum_R - sum_R*sum_R) * (real.size()*squaresum_P - sum_P*sum_P));

        return PCC;
    }

    // Calculate mean
    double calculate_mean(const std::vector<double>& values)
    {
        double mean = 0.0;
        for(auto&& v: values)
            mean += v;
        mean /= std::max(values.size(),(size_t)1); // do not divide by zero

        return mean;
    }

    // Calculate mean of evacuation and travel means
    double calculate_mean(const std::vector<double>& values_evac, const std::vector<double>& values_travels, double alpha_objective)
    {
        double mean = 0.0;
        for(auto&& v: values_evac)
            mean += alpha_objective*v;

        for(auto&& v: values_travels)
            mean += (1-alpha_objective)*v;

        return mean;
    }

    // Calculate standard deviation from standard deviations
    double calculate_stddev_sum_of_stddevs(const std::vector<double>& values_evac, const std::vector<double>& values_travels, double alpha_objective)
    {
        double stddev = 0.0;
        for(auto&& v: values_evac)
            stddev += (alpha_objective*v) * (alpha_objective*v);
        for(auto&& v: values_travels)
            stddev += ((1-alpha_objective)*v) * ((1-alpha_objective)*v);
        stddev = std::sqrt(stddev);
        return stddev;
    }

    // Calculate standard deviation from observations
    double calculate_stddev_estimator_observations(const std::vector<double>& values, double mean)
    {
        double stddev = 0.0;
        for(auto&& v: values)
            stddev += (v-mean)*(v-mean);
        stddev /= std::max(values.size()-1, (size_t)1);
        stddev = std::sqrt(stddev);
        stddev /= std::sqrt(std::max((double)values.size(), 1.0)); // divide by root(n) for stddev of mean
        return stddev;
    }

} // anonymous namespace





namespace timetable
{
    // SORTING FUNCTION
    bool tabu_search::sort_based_on_menge_values(const Swap& s1, const Swap& s2)
    {
        double mean_s1 = 0.0;
        {
            std::vector<double> travels_s1 = s1.observations_menge_tt1;
            travels_s1.insert(travels_s1.end(), s1.observations_menge_tt2.begin(), s1.observations_menge_tt2.end());
            mean_s1 = calculate_mean(s1.observations_menge_evac, travels_s1, _alpha_objective);
        }

        double mean_s2 = 0.0;
        {
            std::vector<double> travels_s2 = s2.observations_menge_tt1;
            travels_s2.insert(travels_s2.end(), s2.observations_menge_tt2.begin(), s2.observations_menge_tt2.end());
            mean_s2 = calculate_mean(s2.observations_menge_evac, travels_s2, _alpha_objective);
        }

        return (mean_s1 < mean_s2);
    }








    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // CONSTRUCTOR
    tabu_search::tabu_search()
    {

    }


    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // DESTRUCTOR
    tabu_search::~tabu_search()
    {

    }


    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // PARAMETERS
    void tabu_search::reset_TS_parameters()
    {
        _replication_budget_identification_step = _replication_budget_identification_step_default;
        _replication_budget_tabu_search = _replication_budget_tabu_search_default;
        _tabu_list_length = _tabu_list_length_default;
        _nb_eval_menge_incremental = _nb_eval_menge_incremental_default;
        _nb_eval_menge_validate = _nb_eval_menge_validate_default;
        _nb_eval_local_minimum = _nb_eval_local_minimum_default;
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // RUN ALGORITHM
    void tabu_search::run()
    {
        try
        {
            // do check on number of moves vs. tabu list length
            {
                size_t nb_possible_moves = 0;
                for(int t = 0; t < nb_timeslots; ++t)
                {
                    for(int r1 = 0; r1 < nb_locations; ++r1)
                    {
                        for(int r2 = r1 + 1; r2 < nb_locations; ++r2)
                        {
                            // swap room r1 & r2
                            // get event1 and event2
                            int event1 = _current_solution.timeslot_location(t, r1);
                            int event2 = _current_solution.timeslot_location(t, r2);

                            // no empty swap && feasible
                            if((event1 >= 0 || event2 >= 0)
                                    && get_event_location_possible(event1, r2)  // is swap feasible?
                                    && get_event_location_possible(event2, r1))
                                ++nb_possible_moves;
                        }
                    }
                }
                if(_tabu_list_length >= nb_possible_moves)
                    throw std::runtime_error("Error in tabu_search. The length of the tabu list ("
                                             + std::to_string(_tabu_list_length) + ") cannot be larger than the number of possible moves("
                                             + std::to_string(nb_possible_moves) + ")");
            }

            // 1. Train the surrogates
            if(!_surrogate->is_trained())
                _surrogate->train();

            // 2. Start the tabu search
            tabu_search_method_A();

            emit(finished());
        }
        catch(const std::exception& ex)
        {
            QString message(ex.what());
            emit(signal_error(message));
        }
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // RESET
    void tabu_search::reset()
    {
        _current_solution.clear();
        _best_solution.clear();
        _best_solutions.clear();
        _best_solution_upper_95_CI = 1e10;

        _tabu_list.clear();

        _timeslots_to_calculate_traveltimes.clear();

        reset_TS_parameters();
    }







    // TABU SEARCH IMPLEMENTATIONS

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void tabu_search::tabu_search_method_A()
    {
        QString output_text;
        QString logger_text;



        output_text = "\n\nStarting Tabu Search ...";
        emit(signal_algorithm_status(output_text));
        logger_text = "Starting Tabu Search ...";
        global::_logger << global::logger::log_type::INFORMATION << logger_text;


        // start timer
        _start_time = std::chrono::system_clock::now();

        output_text = "Calculating objective value initial solution with Menge ...";
        emit(signal_algorithm_status(output_text));

        // 1. analyze start solution
        if(_alpha_objective > 0.01) // evacuations
        {
            for(int t = 0; t < nb_timeslots; ++t)
            {
                for(int replication = 0; replication < _nb_eval_menge_incremental; ++replication)
                {
                    double evactt = _menge->max_sim_duration();
                    int again = 0;
                    do { // if simulation gets stuck, recalculate
                        evactt = _menge->calculate_evacuation_time(_current_solution, t);
                        ++again;
                    } while (evactt > _menge->max_sim_duration() - 1.f && again < 3); // recalculate max 3 times
                    _current_solution.add_objective_value_evac(t,evactt);
                }
                _current_solution.calculate_means_and_stddevs_evac(t);
            }
        }
        if(_alpha_objective < 0.99) // travel times
        {
            for(int t : _timeslots_to_calculate_traveltimes)
            {
                for(int replication = 0; replication < _nb_eval_menge_incremental; ++replication)
                {
                    double traveltt = _menge->max_sim_duration();
                    int again = 0;
                    do { // if simulation gets stuck, recalculate
                        traveltt = _menge->calculate_flows_time(_current_solution, t);
                        ++again;
                    } while (traveltt > _menge->max_sim_duration() - 1.f && again < 3); // recalculate max 3 times
                    _current_solution.add_objective_value_travels(t,traveltt);
                }
                _current_solution.calculate_means_and_stddevs_travels(t);
            }
        }

        _best_solutions.push_back(_current_solution);
        _best_solution_upper_95_CI = _current_solution.upper_95_CI_objective_value(_alpha_objective);

        // output to logger / screen
        {
            output_text = "Mean objective value initial solution: ";
            output_text.append(QString::number(_current_solution.total_mean_objective_value(_alpha_objective)));
            output_text.append("\nConfidence interval objective value initial solution: [");
            output_text.append(QString::number(_current_solution.lower_95_CI_objective_value(_alpha_objective)));
            output_text.append(", ");
            output_text.append(QString::number(_current_solution.upper_95_CI_objective_value(_alpha_objective)));
            output_text.append("]");
            emit(signal_algorithm_status(output_text));
            logger_text = output_text;
            global::_logger << global::logger::log_type::INFORMATION << logger_text;
        }




        // Tabu Search (exploration step)
        tabu_search_method_A_exploration_loop();




        // Identification Step
        /*{
            output_text = "\n\nStarting Replication Step ...";
            emit(signal_algorithm_status(output_text));
            _start_time = std::chrono::system_clock::now();

            std::vector<double> N_current;          N_current.reserve(_best_solutions.size());
            std::vector<double> N_new;              N_new.reserve(_best_solutions.size());
            std::vector<double> means_OCBA;         means_OCBA.reserve(_best_solutions.size());
            std::vector<double> stddevs_OCBA;       stddevs_OCBA.reserve(_best_solutions.size());
            std::vector<double> ratios_OCBA;        ratios_OCBA.reserve(_best_solutions.size());

            int _simulations_done = 0;

            // Step 0. Perform n0 simulation replications for all designs (already done)
            int delta = std::max(_replication_budget_identification_step / 10, 5);
            int current_budget = _nb_eval_menge_incremental * _best_solutions.size() + delta;
            int total_budget = _nb_eval_menge_incremental * _best_solutions.size() + _replication_budget_identification_step;
            for(int i = 0; i < _best_solutions.size(); ++i)
            {
                N_current.push_back(_nb_eval_menge_incremental);
                N_new.push_back(0);
                means_OCBA.push_back(0);
                stddevs_OCBA.push_back(0);
                ratios_OCBA.push_back(1);
            }


            while(true)
            {
                // Step 1. Check if budget exhausted
                {
                    int sum_N = 0;
                    for(auto&& v: N_current)
                        sum_N += v;
                    if(sum_N >= current_budget)
                        break;

                    output_text = "Current budget = ";
                    output_text.append(QString::number(current_budget));
                    output_text.append("\nUsed budget = ");
                    output_text.append(QString::number(sum_N));
                    emit(signal_algorithm_status(output_text));
                }

                // TEST
                //for(int i = 0; i < N_new.size(); ++i)
                //{
                //    output_text = "Current total number of evaluations of solution ";
                 //   output_text.append(QString::number(i+1));
                 //   output_text.append(" = ");
                //    output_text.append(QString::number(N_current[i]));
                //    emit(signal_algorithm_status(output_text));
                //}


                // Step 2. Calculate new budget
                {
                    // calculate means and stddevs
                    for(int i = 0; i < _best_solutions.size(); ++i)
                    {
                        for(int t = 0; t < nb_timeslots; ++t)
                        {
                            if(_alpha_objective > 0.01)
                                _best_solutions[i].calculate_means_and_stddevs_evac(t);
                            if(_alpha_objective < 0.99 && t < nb_timeslots - 1)
                                _best_solutions[i].calculate_means_and_stddevs_travels(t);
                        }
                        double val = _best_solutions[i].total_mean_objective_value(_alpha_objective);
                        means_OCBA[i] = val;
                        val = _best_solutions[i].total_stddev_objective_value(_alpha_objective);
                        stddevs_OCBA[i] = val;
                    }


                    // find solution with lowest mean
                    int index_sol_lowest_mean;
                    double objval_sol_lowest_mean = 1e10;
                    for(int i = 0; i < _best_solutions.size(); ++i)
                    {
                        if(means_OCBA[i] <= objval_sol_lowest_mean)
                        {
                            index_sol_lowest_mean = i;
                            objval_sol_lowest_mean = means_OCBA[i];
                        }
                    }


                    // put this solution in front of vector
                    if(index_sol_lowest_mean != 0)
                    {
                        std::swap(_best_solutions[0], _best_solutions[index_sol_lowest_mean]);
                        std::swap(means_OCBA[0], means_OCBA[index_sol_lowest_mean]);
                        std::swap(stddevs_OCBA[0], stddevs_OCBA[index_sol_lowest_mean]);
                    }


                    // calculate ratios
                    // calculate ratios
                    for (int i = 1; i < _best_solutions.size(); ++i)
                    {
                        if (i == 1)
                        {
                            ratios_OCBA[i] = 1;
                        }
                        else
                        {
                            double numerator = (stddevs_OCBA[1] / std::max(means_OCBA[1] - means_OCBA[index_sol_lowest_mean], 0.001));		// avoid disivion by 0
                            double denominator = (stddevs_OCBA[i] / std::max(means_OCBA[i] - means_OCBA[index_sol_lowest_mean], 0.001));	// avoid disivion by 0
                            if (std::fabs(numerator - denominator) < 0.0001)
                            {
                                ratios_OCBA[i] = 1;
                            }
                            else
                            {
                                double val = numerator / denominator;
                                ratios_OCBA[i] = val * val;
                            }
                        }
                    }


                    // solve budget equation
                    auto equation_mismatch = [&](const double value_N1) -> double
                    {
                        double lefthandside = 0.0;

                        // write everything as function of N1
                        for(int i = 1; i < _best_solutions.size(); ++i)
                            lefthandside += (1.0/(ratios_OCBA[i]+0.000001)) * value_N1;

                        double val_in_root = 0.0;
                        for(int i = 1; i < _best_solutions.size(); ++i)
                        {
                            val_in_root += (value_N1 / stddevs_OCBA[i]) * (value_N1 / stddevs_OCBA[i]);
                        }
                        lefthandside += stddevs_OCBA[0] * std::sqrt(val_in_root);

                        return std::abs(lefthandside - current_budget);
                    };
                    auto new_value_N1 = dlib::find_min_global(equation_mismatch,
                                                                 {0},          // lower bound constraint on value_N1
                                                                 {100000},     // upper bound constraint on value_N1
                                                                 dlib::max_function_calls(50));


                    // calculate values for every solution from N_1
                    N_new[1] = new_value_N1.x(0);
                    for(int i = 2; i < _best_solutions.size(); ++i)
                        N_new[i] = new_value_N1.x(0) * ratios_OCBA[i];
                    double val_in_root = 0.0;
                    for(int i = 1; i < _best_solutions.size(); ++i)
                        val_in_root += (N_new[i] / stddevs_OCBA[i]) * (N_new[i] / stddevs_OCBA[i]);
                    N_new[0] = stddevs_OCBA[0] * std::sqrt(val_in_root);
                    if(N_new[0] < 1)
                        N_new[0] = 1;
                }


                // TEST
                for(int i = 0; i < N_new.size(); ++i)
               // {
                 //   output_text = "New number of evaluations of solution ";
                 //   output_text.append(QString::number(i+1));
                //    output_text.append(" = ");
                //    output_text.append(QString::number(N_new[i]));
                //    emit(signal_algorithm_status(output_text));
                //}


                // Step 3. Perform the additional simulations
                for(int i = 0; i < _best_solutions.size(); ++i)
                {
                    if(_alpha_objective > 0.01) // evacuations
                    {
                        for(int t = 0; t < nb_timeslots; ++t)
                        {
                            for(int replication = 0; replication < std::max(0, int(N_new[i] + 0.5)); ++replication)
                            {
                                ++_simulations_done;
                                double evactt = _menge->max_sim_duration();
                                int again = 0;
                                do { // if simulation gets stuck, recalculate
                                    evactt = _menge->calculate_evacuation_time(_best_solutions[i], t);
                                    ++again;
                                } while (evactt > _menge->max_sim_duration() - 1.f && again < 3); // recalculate max 3 times
                                _best_solutions[i].add_objective_value_evac(t, evactt);
                            }
                            _best_solutions[i].calculate_means_and_stddevs_evac(t);
                        }
                    }
                    if(_alpha_objective < 0.99) // travel times
                    {
                        for(int t : _timeslots_to_calculate_traveltimes)
                        {
                            for(int replication = 0; replication < std::max(0, int(N_new[i] + 0.5)); ++replication)
                            {
                                ++_simulations_done;
                                double traveltt = _menge->max_sim_duration();
                                int again = 0;
                                do { // if simulation gets stuck, recalculate
                                    traveltt = _menge->calculate_flows_time(_best_solutions[i], t);
                                    ++again;
                                } while (traveltt > _menge->max_sim_duration() - 1.f && again < 3); // recalculate max 3 times
                                _best_solutions[i].add_objective_value_travels(t, traveltt);
                            }
                            _best_solutions[i].calculate_means_and_stddevs_travels(t);
                        }
                    }
                }



                // New iteration
                for(int i = 0; i < N_current.size(); ++i)
                    N_current[i] += N_new[i];
                if(current_budget < total_budget)
                    current_budget += delta;
                //if(current_budget >= total_budget)
                //    break;

            }

            std::chrono::nanoseconds elapsed_time = std::chrono::system_clock::now() - _start_time;
            output_text = "\n\nIdentification step completed.\nElapsed time (seconds): ";
            output_text.append(QString::number(elapsed_time.count() / NANO));
            emit(signal_algorithm_status(output_text));
            logger_text = "Identification step completed.\nElapsed time (seconds): ";
            logger_text.append(QString::number(elapsed_time.count() / NANO));
            global::_logger << global::logger::log_type::INFORMATION << logger_text;
        }*/


        // return best found solution qua average
        // calculate means and stddevs
        int index_best_solution;
        double best_mean_obj_val = 1e10;
        double best_upper_95_CI = 1e10;
        double best_lower_95_CI = 1e10;
        global::_logger << global::logger::log_type::INFORMATION;
        for(int i = 0; i < _best_solutions.size(); ++i)
        {
            double mean = _best_solutions[i].total_mean_objective_value(_alpha_objective);
            double CI_plus = _best_solutions[i].upper_95_CI_objective_value(_alpha_objective);
            double CI_min = _best_solutions[i].lower_95_CI_objective_value(_alpha_objective);

            logger_text = "\nPossible best solution,";
            logger_text.append(QString::number(i+1));
            logger_text.append(",has a mean objective value of,");
            logger_text.append(QString::number(mean));
            logger_text.append(",and 95 percent confidence interval = [,");
            logger_text.append(QString::number(CI_min));
            logger_text.append(",");
            logger_text.append(QString::number(CI_plus));
            logger_text.append(",],");

            global::_logger << logger_text;

            if(mean < best_mean_obj_val)
            {
                index_best_solution = i;
                best_mean_obj_val = mean;
                best_upper_95_CI = CI_plus;
                best_lower_95_CI = CI_min;
            }
        }
        _best_solution = _best_solutions[index_best_solution];
        output_text = "\n\nBest found solution has a mean objective value of ";
        output_text.append(QString::number(best_mean_obj_val));
        output_text.append("\nand 95 percent confidence interval = [");
        output_text.append(QString::number(best_lower_95_CI));
        output_text.append(", ");
        output_text.append(QString::number(best_upper_95_CI));
        output_text.append("]");
        emit(signal_algorithm_status(output_text));
        emit(new_best_solution_found(_best_solution));
        logger_text = "\n";
        logger_text += output_text;

        global::_logger << global::logger::log_type::INFORMATION << logger_text;
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void tabu_search::tabu_search_method_A_exploration_loop()
    {
        constexpr int iterations_analyze_performance[3] = { 1, 10, 100 };
        QString output_text;
        QString logger_text;

        std::vector<Swap> candidate_moves;
        candidate_moves.reserve(nb_locations*(nb_locations-1)/2);

        _tabu_list.clear();
        _tabu_list.reserve(_tabu_list_length);
        for(int i = 0; i < _tabu_list_length; ++i)
            _tabu_list.push_back(Swap());

        int iteration = 0;
        int remaining_budget_tabu_search = _replication_budget_tabu_search;


        global::_logger << global::logger::log_type::INFORMATION;
        while(true)
        {
            ++iteration;
            output_text = "\nIteration ";
            output_text.append(QString::number(iteration));
            emit(signal_algorithm_status(output_text));
            logger_text = "\nIteration,";
            logger_text.append(QString::number(iteration));


            // select a timeslot and generate all possible moves for that timeslot
            candidate_moves.clear();
            std::uniform_int_distribution<int> dist_timeslots(0,nb_timeslots-1);
            int timeslot = dist_timeslots(generator);
            for(int r1 = 0; r1 < nb_locations; ++r1)
            {
                for(int r2 = r1 + 1; r2 < nb_locations; ++r2)
                {
                    // swap room r1 & r2
                    // get event1 and event2
                    int event1 = _current_solution.timeslot_location(timeslot, r1);
                    int event2 = _current_solution.timeslot_location(timeslot, r2);

                    // no empty swap && feasible
                    if((event1 >= 0 || event2 >= 0)
                            && get_event_location_possible(event1, r2)  // is swap feasible?
                            && get_event_location_possible(event2, r1))
                    {
                        Swap swap;
                        swap.timeslot = timeslot;
                        swap.location1 = r1;
                        swap.location2 = r2;
                        swap.obj_value_surrogate = 0.0;

                        // do the swap
                        if(event1 >= 0)
                            _current_solution.set_event_location(event1, r2);
                        if(event2 >= 0)
                            _current_solution.set_event_location(event2, r1);

                        // calculate the objective value
                        {
                            // evacuations
                            if(_alpha_objective > 0.01)
                            {
                                swap.obj_value_surrogate += _alpha_objective * _surrogate->predict_evacuation_time(_current_solution, timeslot);
                            }
                            // travels
                            if(_alpha_objective < 0.99)
                            {
                                // from t-1 to t
                                if(timeslot > 0)
                                {
                                    swap.obj_value_surrogate += (1-_alpha_objective) * _surrogate->predict_travel_time(_current_solution, timeslot - 1);
                                }

                                // from t to t+1
                                if(timeslot < nb_timeslots - 1)
                                {
                                    swap.obj_value_surrogate += (1-_alpha_objective) * _surrogate->predict_travel_time(_current_solution, timeslot);
                                }
                            }

                        }


                        // reverse the swap
                        if(event1 >= 0)
                            _current_solution.set_event_location(event1, r1);
                        if(event2 >= 0)
                            _current_solution.set_event_location(event2, r2);


                        // put candidate swap in vector
                        candidate_moves.push_back(swap);
                    }


                }
            }


            // sort all moves
            std::sort(candidate_moves.begin(), candidate_moves.end());
            for(int i = 0; i < std::min(_nb_eval_local_minimum, (int)candidate_moves.size()); ++i) {
                output_text = "Objective value surrogate best move ";
                output_text.append(QString::number(i+1));
                output_text.append(": ");
                output_text.append(QString::number(candidate_moves[i].obj_value_surrogate));
                emit(signal_algorithm_status(output_text));
            }


            // analyse performance of surrogates
            if(_analyze_performance)
            {
                /*bool local_minimum = true;
                for(int i = 0; i < candidate_moves.size(); ++i) {
                    if(candidate_moves[i].obj_value_surrogate < current_solution.mean_objective_value(timeslot)) {
                        local_minimum = false;
                        break;
                    }
                }*/

                // analyse performance of surrogates
                if(iteration == iterations_analyze_performance[0]
                        || iteration == iterations_analyze_performance[1]
                        || iteration == iterations_analyze_performance[2]
                        /*|| local_minimum*/)
                {
                    output_text = "Testing Quality of Surrogates ...";
                    emit(signal_algorithm_status(output_text));

                    std::vector<double> surrogate_values;
                    std::vector<double> real_values;
                    surrogate_values.reserve(candidate_moves.size());
                    real_values.reserve(candidate_moves.size());

                    // reevaluate all candidate moves
                    for(int i = 0; i < candidate_moves.size(); ++i)
                    {
                        int r1 = candidate_moves[i].location1;
                        int r2 = candidate_moves[i].location2;

                        int timeslot = candidate_moves[i].timeslot;

                        int event1 = _current_solution.timeslot_location(timeslot, r1);
                        int event2 = _current_solution.timeslot_location(timeslot, r2);


                        // store values calculated by surrogates
                        surrogate_values.push_back(candidate_moves[i].obj_value_surrogate);


                        // do the swap
                        if(event1 >= 0)
                            _current_solution.set_event_location(event1, r2);
                        if(event2 >= 0)
                            _current_solution.set_event_location(event2, r1);

                        std::vector<double> observations_evac, observations_tt1, observations_tt2;
                        observations_evac.reserve(_nb_eval_menge_validate);
                        observations_tt1.reserve(_nb_eval_menge_validate);
                        observations_tt2.reserve(_nb_eval_menge_validate);
                        for(int j = 0; j < _nb_eval_menge_validate; ++j)
                        {
                            // evacuations
                            if(_alpha_objective > 0.01)
                            {
                                double evactt = _menge->max_sim_duration();
                                int again = 0;
                                do { // if simulation gets stuck, recalculate
                                   evactt = _menge->calculate_evacuation_time(_current_solution, timeslot);
                                   ++again;
                                } while(evactt > _menge->max_sim_duration() - 1.f && again < 3); // recalculate max 3 times
                                observations_evac.push_back(evactt);
                            }

                            // travel times
                            if(_alpha_objective < 0.99)
                            {
                                // both previous and current timeslot change
                                if(timeslot > 0) // only previous if not first ts
                                {
                                    double traveltt = _menge->max_sim_duration();
                                    int again = 0;
                                    do { // if simulation gets stuck, recalculate
                                        traveltt = _menge->calculate_flows_time(_current_solution, timeslot-1);
                                    } while(traveltt > _menge->max_sim_duration() - 1.f && again < 3); // recalculate max 3 times
                                    observations_tt1.push_back(traveltt);
                                }
                                if(timeslot < nb_timeslots - 1) // only current if not last ts
                                {
                                    double traveltt = _menge->max_sim_duration();
                                    int again = 0;
                                    do { // if simulation gets stuck, recalculate
                                        traveltt = _menge->calculate_flows_time(_current_solution, timeslot);
                                    } while(traveltt > _menge->max_sim_duration() - 1.f && again < 3); // recalculate max 3 times
                                    observations_tt2.push_back(traveltt);
                                }
                            }
                        }


                        // reverse the swap
                        if(event1 >= 0)
                            _current_solution.set_event_location(event1, r1);
                        if(event2 >= 0)
                            _current_solution.set_event_location(event2, r2);


                        // objective value
                        double real_obj_val = _alpha_objective*calculate_mean(observations_evac) + (1-_alpha_objective)*(calculate_mean(observations_tt1) + calculate_mean(observations_tt2));
                        real_values.push_back(real_obj_val);
                    }

                    // calculate MSE, MAE, and PCC and output results
                    //if(local_minimum)
                    //    output_text = "Quality of surrogates - local minimum:";
                    //else {
                        output_text = "Quality of surrogates - iteration ";
                        output_text += QString::number(iteration);
                    //}


                    double MSE = calculate_MSE(real_values, surrogate_values);
                    double PCC = calculate_PCC(real_values, surrogate_values);
                    double MAE = calculate_MAE(real_values, surrogate_values);
                    output_text += "\n    MSE = ";
                    output_text += QString::number(MSE);
                    output_text += "\n    PCC = ";
                    output_text += QString::number(PCC);
                    output_text += "\n    MAE = ";
                    output_text += QString::number(MAE);
                    emit(signal_algorithm_status(output_text));
                    global::_logger << global::logger::log_type::INFORMATION << output_text;

                    // resort the moves
                    //std::sort(candidate_moves.begin(), candidate_moves.end(), compare_candidates_based_on_menge_mean);
                }
            }


            // calculate objective value of current_solution
            double obj_current_solution = 1e10;
            {
                std::vector<double> means_evac, means_travels;
                if(_alpha_objective > 0.01)
                {
                    means_evac.reserve(nb_timeslots);
                    for(int t = 0; t < nb_timeslots; ++t) {
                        _current_solution.calculate_means_and_stddevs_evac(t);
                        means_evac.push_back(_current_solution.mean_objective_value_evac(t));
                    }
                }
                if(_alpha_objective < 0.99)
                {
                    means_travels.reserve(nb_timeslots-1);
                    for(int t : _timeslots_to_calculate_traveltimes) {
                        _current_solution.calculate_means_and_stddevs_travels(t);
                        means_travels.push_back(_current_solution.mean_objective_value_travels(t));
                    }
                }
                obj_current_solution = calculate_mean(means_evac, means_travels, _alpha_objective);
            }
            // evaluate the best move with Menge
            // if it is better, accept it; unless it is tabu, then only accept if better than best solution
            bool move_found = false;
            for(int i = 0; i < std::min((int)candidate_moves.size(), _nb_eval_local_minimum); ++i) // start with best candidate, then second, etc. MAX nb_eval_local_minimum
            {
                // iterate over tabu list to check whether move is tabu
                for(int j = 0; j < _tabu_list.size(); ++j)
                {
                    if(_tabu_list[j] == candidate_moves[i]) // if tabu
                    {
                        candidate_moves[i].tabu = true;

                        // stop searching tabu list
                        break;
                    }
                }


                // no aspiration criterion
                // so only evaluate if *not* tabu
                if(!candidate_moves[i].tabu)
                {
                    // evaluate move 'i' with Menge
                    {
                        int r1 = candidate_moves[i].location1;
                        int r2 = candidate_moves[i].location2;

                        int timeslot = candidate_moves[i].timeslot;

                        int event1 = _current_solution.timeslot_location(timeslot, r1);
                        int event2 = _current_solution.timeslot_location(timeslot, r2);

                        // do the swap
                        if(event1 >= 0)
                            _current_solution.set_event_location(event1, r2);
                        if(event2 >= 0)
                            _current_solution.set_event_location(event2, r1);


                        for(int j = 0; j < _nb_eval_menge_incremental; ++j)
                        {
                            // evacuations
                            if(_alpha_objective > 0.01)
                            {
                                double evactt = _menge->max_sim_duration();
                                int again = 0;
                                do { // if simulation gets stuck, recalculate
                                   evactt = _menge->calculate_evacuation_time(_current_solution, timeslot);
                                   ++again;
                                } while(evactt > _menge->max_sim_duration() - 1.f && again < 3); // recalculate max 3 times
                                candidate_moves[i].observations_menge_evac.push_back(evactt);
                            }


                            // travel times
                            if(_alpha_objective < 0.99)
                            {
                                // both previous and current timeslot change
                                for(int ts: _timeslots_to_calculate_traveltimes)
                                {
                                    if(timeslot - 1 == ts) // only previous if previous timeslot in vector
                                    {
                                        double traveltt = _menge->max_sim_duration();
                                        int again = 0;
                                        do { // if simulation gets stuck, recalculate
                                            traveltt = _menge->calculate_flows_time(_current_solution, timeslot-1);
                                            ++again;
                                        } while(traveltt > _menge->max_sim_duration() - 1.f && again < 3); // recalculate max 3 times
                                        candidate_moves[i].observations_menge_tt1.push_back(traveltt);
                                    }
                                    if(timeslot == ts) // only current if current timeslot in vector
                                    {
                                        double traveltt = _menge->max_sim_duration();
                                        int again = 0;
                                        do { // if simulation gets stuck, recalculate
                                            traveltt = _menge->calculate_flows_time(_current_solution, timeslot);
                                            ++again;
                                        } while(traveltt > _menge->max_sim_duration() - 1.f && again < 3); // recalculate max 3 times
                                        candidate_moves[i].observations_menge_tt2.push_back(traveltt);
                                    }
                                }
                            }


                            --remaining_budget_tabu_search;
                            if(remaining_budget_tabu_search <= 0)
                            {
                                std::chrono::nanoseconds elapsed_time = std::chrono::system_clock::now() - _start_time;
                                output_text = "\n\n\n\nStopping criterion tabu search reached.\nElapsed time (seconds): ";
                                output_text.append(QString::number(elapsed_time.count() / NANO));
                                emit(signal_algorithm_status(output_text));
                                logger_text = "Stopping criterion tabu search reached.\nElapsed time (seconds): ";
                                logger_text.append(QString::number(elapsed_time.count() / NANO));
                                global::_logger << global::logger::log_type::INFORMATION << logger_text;

                                return;
                            }
                        }



                        // reverse the swap
                        if(event1 >= 0)
                            _current_solution.set_event_location(event1, r1);
                        if(event2 >= 0)
                            _current_solution.set_event_location(event2, r2);

                    }

                    // calculate lower 95 CI of candidate solution
                    // take all means from current solution except for changed timeslot
                    double candidate_solution_lower_95_CI = 0.0;
                    double candidate_solution_upper_95_CI = 0.0;
                    double candidate_solution_mean_obj_val = 0.0;
                    {
                        std::vector<double> means_evac, stddevs_evac;
                        std::vector<double> means_travels, stddevs_travels;
                        if(_alpha_objective > 0.01)
                        {
                            means_evac.reserve(nb_timeslots);
                            stddevs_evac.reserve(nb_timeslots);
                            for(int t = 0; t < nb_timeslots; ++t) {
                                if(t != timeslot) {
                                    means_evac.push_back(_current_solution.mean_objective_value_evac(t));
                                    stddevs_evac.push_back(_current_solution.stddev_estimator_obj_val_evac(t));
                                }
                                else {
                                    means_evac.push_back(calculate_mean(candidate_moves[i].observations_menge_evac));
                                    stddevs_evac.push_back(calculate_stddev_estimator_observations(candidate_moves[i].observations_menge_evac,means_evac.back()));
                                }
                            }
                        }
                        if(_alpha_objective < 0.99)
                        {
                            means_travels.reserve(nb_timeslots-1);
                            stddevs_travels.reserve(nb_timeslots-1);
                            for(int t : _timeslots_to_calculate_traveltimes) {
                                if(t != timeslot && t != timeslot-1) {
                                    means_travels.push_back(_current_solution.mean_objective_value_travels(t));
                                    stddevs_travels.push_back(_current_solution.stddev_estimator_obj_val_travels(t));
                                } else if(t==timeslot-1) {
                                    means_travels.push_back(calculate_mean(candidate_moves[i].observations_menge_tt1));
                                    stddevs_travels.push_back(calculate_stddev_estimator_observations(candidate_moves[i].observations_menge_tt1,means_travels.back()));
                                } else if(t==timeslot) {
                                    means_travels.push_back(calculate_mean(candidate_moves[i].observations_menge_tt2));
                                    stddevs_travels.push_back(calculate_stddev_estimator_observations(candidate_moves[i].observations_menge_tt2,means_travels.back()));
                                }
                            }
                        }

                        candidate_solution_mean_obj_val = calculate_mean(means_evac, means_travels, _alpha_objective);
                        double stddev_candidate = calculate_stddev_sum_of_stddevs(stddevs_evac, stddevs_travels, _alpha_objective);
                        candidate_solution_lower_95_CI = candidate_solution_mean_obj_val - 2.0 * stddev_candidate;
                        candidate_solution_upper_95_CI = candidate_solution_mean_obj_val + 2.0 * stddev_candidate;
                    }


                    // if move is not tabu
                    // calculate objective value current solution
                    if(candidate_solution_mean_obj_val < obj_current_solution)
                    {
                        // do swap
                        int r1 = candidate_moves[i].location1;
                        int r2 = candidate_moves[i].location2;
                        int timeslot = candidate_moves[i].timeslot;

                        int event1 = _current_solution.timeslot_location(timeslot, r1);
                        int event2 = _current_solution.timeslot_location(timeslot, r2);

                        if(event1 >= 0)
                            _current_solution.set_event_location(event1, r2);
                        if(event2 >= 0)
                            _current_solution.set_event_location(event2, r1);


                        // put objective of candidate in current_solution
                        if(_alpha_objective > 0.01) {
                            _current_solution.reset_objective_values_evac(timeslot);
                            for(auto&& v: candidate_moves[i].observations_menge_evac)
                                _current_solution.add_objective_value_evac(timeslot,v);
                            _current_solution.calculate_means_and_stddevs_evac(timeslot);
                        }
                        if(_alpha_objective < 0.99) {
                            if(timeslot > 0) {
                                _current_solution.reset_objective_values_travels(timeslot-1);
                                for(auto&& v: candidate_moves[i].observations_menge_tt1)
                                    _current_solution.add_objective_value_travels(timeslot-1,v);
                                _current_solution.calculate_means_and_stddevs_travels(timeslot-1);
                            }
                            if(timeslot < nb_timeslots-1) {
                                _current_solution.reset_objective_values_travels(timeslot);
                                for(auto&& v: candidate_moves[i].observations_menge_tt2)
                                    _current_solution.add_objective_value_travels(timeslot,v);
                                _current_solution.calculate_means_and_stddevs_travels(timeslot);
                            }
                        }


                        // if better than best solution, put in list best_solutions
                        if(candidate_solution_lower_95_CI < _best_solution_upper_95_CI)
                        {
                            _best_solutions.push_back(_current_solution);

                            // update best upper CI of all best solutions
                            if(candidate_solution_upper_95_CI < _best_solution_upper_95_CI)
                                _best_solution_upper_95_CI = candidate_solution_upper_95_CI;


                            output_text = "New possible best solution found.\nMean objective value: ";
                            output_text.append(QString::number(candidate_solution_mean_obj_val));
                            emit(signal_algorithm_status(output_text));
                            emit(new_best_solution_found(_current_solution));
                            logger_text += ",New possible best solution found,Mean objective value,";
                            logger_text += QString::number(candidate_solution_mean_obj_val);
                        }
                        else
                        {
                            output_text = "New current solution accepted.\nMean objective value: ";
                            obj_current_solution = _current_solution.total_mean_objective_value(_alpha_objective);
                            output_text.append(QString::number(obj_current_solution));
                            emit(signal_algorithm_status(output_text));
                            logger_text += ",New current solution accepted,Mean objective value,";
                            logger_text += QString::number(obj_current_solution);
                        }


                        // put move in tabu list
                        _tabu_list[iteration % _tabu_list_length] = candidate_moves[i];


                        move_found = true;
                        break;

                    }


                    // if a move has been executed stop
                    if(move_found)
                        break;
                }
            }

            // if we are in a local minimum, and thus no improving move can be found, we execute the best non-tabu move
            if(!move_found)
            {
                output_text = "No improving move found. ";
                emit(signal_algorithm_status(output_text));

                // resort the first moves based on their real values
                {
                    int pos = std::min(_nb_eval_local_minimum, (int)candidate_moves.size());
                    std::sort(candidate_moves.begin(), candidate_moves.begin() + pos);
                }

                for(int i = 0; i < candidate_moves.size(); ++i)
                {
                    if(!candidate_moves[i].tabu)
                    {
                        // do swap
                        int r1 = candidate_moves[i].location1;
                        int r2 = candidate_moves[i].location2;
                        int timeslot = candidate_moves[i].timeslot;

                        int event1 = _current_solution.timeslot_location(timeslot, r1);
                        int event2 = _current_solution.timeslot_location(timeslot, r2);

                        if(event1 >= 0)
                            _current_solution.set_event_location(event1, r2);
                        if(event2 >= 0)
                            _current_solution.set_event_location(event2, r1);


                        // if not yet simulated with Menge (but instead only surrogate), use Menge for real evaluation
                        if(candidate_moves[i].observations_menge_evac.size() <= 0
                                && candidate_moves[i].observations_menge_tt1.size() <= 0
                                && candidate_moves[i].observations_menge_tt2.size() <= 0)
                        {
                            for(int j = 0; j < _nb_eval_menge_incremental; ++j)
                            {
                                // evacuations
                                if(_alpha_objective > 0.01)
                                {
                                    double evactt = _menge->max_sim_duration();
                                    int again = 0;
                                    do { // if simulation gets stuck, recalculate
                                       evactt = _menge->calculate_evacuation_time(_current_solution, timeslot);
                                       ++again;
                                    } while(evactt > _menge->max_sim_duration() - 1.f && again < 3); // recalculate max 3 times
                                    candidate_moves[i].observations_menge_evac.push_back(evactt);
                                }


                                // travel times
                                if(_alpha_objective < 0.99)
                                {
                                    // both previous and current timeslot change
                                    if(timeslot > 0) // only previous if not first ts
                                    {
                                        double traveltt = _menge->max_sim_duration();
                                        int again = 0;
                                        do { // if simulation gets stuck, recalculate
                                            try {
                                                traveltt = _menge->calculate_flows_time(_current_solution, timeslot-1);
                                            } catch(const std::exception& ex) {

                                            }
                                            ++again;
                                        } while(traveltt > _menge->max_sim_duration() - 1.f && again < 3); // recalculate max 3 times
                                        candidate_moves[i].observations_menge_tt1.push_back(traveltt);
                                    }
                                    if(timeslot < nb_timeslots - 1) // only current if not last ts
                                    {
                                        double traveltt = _menge->max_sim_duration();
                                        int again = 0;
                                        do { // if simulation gets stuck, recalculate
                                            try {
                                                traveltt = _menge->calculate_flows_time(_current_solution, timeslot);
                                            } catch(const std::exception& ex) {

                                            }
                                            ++again;
                                        } while(traveltt > _menge->max_sim_duration() - 1.f && again < 3); // recalculate max 3 times
                                        candidate_moves[i].observations_menge_tt2.push_back(traveltt);
                                    }
                                }


                                --remaining_budget_tabu_search;
                                if(remaining_budget_tabu_search <= 0)
                                {
                                    std::chrono::nanoseconds elapsed_time = std::chrono::system_clock::now() - _start_time;
                                    output_text = "\n\n\n\nStopping criterion tabu search reached.\nElapsed time (seconds): ";
                                    output_text.append(QString::number(elapsed_time.count() / NANO));
                                    emit(signal_algorithm_status(output_text));
                                    logger_text = "Stopping criterion tabu search reached.\nElapsed time (seconds): ";
                                    logger_text.append(QString::number(elapsed_time.count() / NANO));
                                    global::_logger << global::logger::log_type::INFORMATION << logger_text;

                                    return;
                                }
                            }
                        }




                        // put objective of candidate in current_solution
                        if(_alpha_objective > 0.01) {
                            _current_solution.reset_objective_values_evac(timeslot);
                            for(auto&& v: candidate_moves[i].observations_menge_evac)
                                _current_solution.add_objective_value_evac(timeslot,v);
                            _current_solution.calculate_means_and_stddevs_evac(timeslot);
                        }
                        if(_alpha_objective < 0.99) {
                            if(timeslot > 0) {
                                _current_solution.reset_objective_values_travels(timeslot-1);
                                for(auto&& v: candidate_moves[i].observations_menge_tt1)
                                    _current_solution.add_objective_value_travels(timeslot-1,v);
                                _current_solution.calculate_means_and_stddevs_travels(timeslot-1);
                            }
                            if(timeslot < nb_timeslots-1) {
                                _current_solution.reset_objective_values_travels(timeslot);
                                for(auto&& v: candidate_moves[i].observations_menge_tt2)
                                    _current_solution.add_objective_value_travels(timeslot,v);
                                _current_solution.calculate_means_and_stddevs_travels(timeslot);
                            }
                        }



                        // if better than best solution, put in list best_solutions
                        obj_current_solution = _current_solution.total_mean_objective_value(_alpha_objective);
                        if(obj_current_solution < _best_solution_upper_95_CI)
                        {
                            _best_solutions.push_back(_current_solution);

                            // update best upper CI of all best solutions
                            if(obj_current_solution < _best_solution_upper_95_CI)
                                _best_solution_upper_95_CI = obj_current_solution;


                            output_text = "New possible best solution found.\nMean objective value: ";
                            output_text.append(QString::number(obj_current_solution));
                            emit(signal_algorithm_status(output_text));
                            emit(new_best_solution_found(_current_solution));

                            logger_text += ",New possible best solution found,Mean objective value,";
                            logger_text += QString::number(obj_current_solution);
                        }
                        else
                        {
                            output_text = "New current solution accepted.\nMean objective value: ";
                            output_text.append(QString::number(obj_current_solution));
                            emit(signal_algorithm_status(output_text));
                            logger_text += ",New current solution accepted,Mean objective value,";
                            logger_text += QString::number(obj_current_solution);
                        }


                        // put move in tabu list
                        _tabu_list[iteration % _tabu_list_length] = candidate_moves[i];

                        move_found = true;
                        break;
                    }
                }
            }


            global::_logger << logger_text;
        }


    }






    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // EXHAUSTIVE SEARCH
    void tabu_search::run_exhaustive_search()
    {
        QString output_text;
        QString logger_text;

        output_text = "\n\nStarting Exhaustive Search ...";
        emit(signal_algorithm_status(output_text));
        logger_text = "Starting Exhaustive Search ...";
        global::_logger << global::logger::log_type::INFORMATION << logger_text;
        global::_logger << global::logger::log_type::INFORMATION;


        try
        {
            // reset counter
            exhaustive_search_solutions_number = 0;

            // clear room assignments
            for(int e = 0; e < nb_events; ++e)
                _current_solution.set_event_location(e, -1);

            // if evacuations only
            if(_alpha_objective > 0.99)
            {
                // every timeslot is independent of all others
                for(int t = 0; t < nb_timeslots; ++t)
                {
                    generate_all_possible_solutions_independently(t, 0);
                }
            }
            // travels between consecutive timeslots only
            else
            {
                generate_all_possible_solutions(0);
            }

            // If done
            emit(finished());
        }
        catch(const std::exception& ex)
        {
            QString message(ex.what());
            emit(signal_error(message));
        }
    }



    // RECURSIVE FUNCTION TO GENERATE ALL SOLUTIONS (for travels)
    void tabu_search::generate_all_possible_solutions(int current_event)
    {
        // solution has been generated entirely
        if(current_event >= nb_events)
        {
            // evaluate the objective value for every timeslot
            for(int t = 0; t < nb_timeslots; ++t)
            {
                // reset values
                _current_solution.reset_objective_values_evac(t);
                for(int replication = 0; replication < _nb_eval_menge_incremental; ++replication)
                {
                    double traveltt = _menge->max_sim_duration();
                    int again = 0;
                    do { // if simulation gets stuck, recalculate
                        traveltt = _menge->calculate_flows_time(_current_solution, t);
                        ++again;
                    } while (traveltt > _menge->max_sim_duration() - 1.f && again < 3); // recalculate max 3 times
                    _current_solution.add_objective_value_travels(t, traveltt);
                }
                _current_solution.calculate_means_and_stddevs_travels(t);
            }

            // print the objective value
            ++exhaustive_search_solutions_number;
            QString output_text;
            output_text = "Solution \t" + QString::number(exhaustive_search_solutions_number)
                    + "\tMean objective value \t" + QString::number(_current_solution.total_mean_objective_value(_alpha_objective))
                    + "\tStddev objective value \t" + QString::number(_current_solution.total_stddev_objective_value(_alpha_objective));
            emit(signal_algorithm_status(output_text));
            output_text = "\nSolutiont" + QString::number(exhaustive_search_solutions_number) + ","
                    + "Mean objective value," + QString::number(_current_solution.total_mean_objective_value(_alpha_objective)) + ","
                    + "Stddev objective value," + QString::number(_current_solution.total_stddev_objective_value(_alpha_objective));
            global::_logger << output_text;
        }
        else
        {
            int current_timeslot = _current_solution.event_timeslot(current_event);
            for(int l = 0; l < nb_locations; ++l)
            {
                // check if location available and feasible
                if(_current_solution.timeslot_location(current_timeslot, l) == -1
                        && get_event_location_possible(current_event, l))
                {
                    // set the event in that location
                    _current_solution.set_event_location(current_event, l);

                    // go to the next event
                    generate_all_possible_solutions(current_event + 1);

                    // when returning, reset the last assignment
                    _current_solution.set_event_location(current_event, -1);
                }
            }
        }
    }



    // RECURSIVE FUNCTION TO GENERATE ALL SOLUTIONS FOR EACH TIMESLOT INDEPENDENTLY (for evacuations)
    void tabu_search::generate_all_possible_solutions_independently(int timeslot, int current_event)
    {
        // solution has been generated entirely
        if(current_event >= nb_events)
        {
            // evaluate the objective value for this timeslot
            _current_solution.reset_objective_values_evac(timeslot); // first reset values
            for(int replication = 0; replication < _nb_eval_menge_incremental; ++replication)
            {
                double evactt = _menge->max_sim_duration();
                int again = 0;
                do { // if simulation gets stuck, recalculate
                    evactt = _menge->calculate_evacuation_time(_current_solution, timeslot);
                    ++again;
                } while (evactt > _menge->max_sim_duration() - 1.f && again < 3); // recalculate max 3 times
                _current_solution.add_objective_value_evac(timeslot, evactt);
            }
            _current_solution.calculate_means_and_stddevs_evac(timeslot);

            // print the objective value
            ++exhaustive_search_solutions_number;
            QString sol;
            for(int e = 0; e < nb_events; ++e)
            {
                sol.append(QString::number(_current_solution.event_location(e)));
                sol.append("|");
            }
            QString output_text;
            output_text = "Solution \t" + QString::number(exhaustive_search_solutions_number)
                    + "\ttimeslot \t" + QString::number(timeslot + 1)
                    + "\t" + sol
                    + "\tMean objective value \t" + QString::number(_current_solution.mean_objective_value_evac(timeslot))
                    + "\tStddev objective value \t" + QString::number(_current_solution.stddev_estimator_obj_val_evac(timeslot));
            emit(signal_algorithm_status(output_text));
            output_text = "\nSolution," + QString::number(exhaustive_search_solutions_number) + ","
                    + "timeslot," + QString::number(timeslot + 1) + ","
                    + sol + ","
                    + "Mean objective value," + QString::number(_current_solution.mean_objective_value_evac(timeslot)) + ","
                    + "Stddev objective value," + QString::number(_current_solution.stddev_estimator_obj_val_evac(timeslot));
            global::_logger << output_text;
        }
        else
        {
            // only if the event is planned in this timeslot, generate all possible rooms
            if(_current_solution.event_timeslot(current_event) == timeslot)
            {
                for(int l = 0; l < nb_locations; ++l)
                {
                    // check if location available and feasible
                    if(_current_solution.timeslot_location(timeslot, l) == -1
                            && get_event_location_possible(current_event, l))
                    {
                        // set the event in that location
                        _current_solution.set_event_location(current_event, l);

                        // go to the next event
                        generate_all_possible_solutions_independently(timeslot, current_event + 1);

                        // when returning, reset the last assignment
                        _current_solution.set_event_location(current_event, -1);
                    }
                }
            }

        }
    }

} // namespace timetable
