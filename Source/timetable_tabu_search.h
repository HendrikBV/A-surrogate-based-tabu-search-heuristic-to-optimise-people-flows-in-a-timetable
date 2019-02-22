/*
 *	Code for the surrogate-based tabu search algorithm
 *  to optimise people flows in a timetable.
 *
 *	Code author: Hendrik Vermuyten
 */

/*!
 *	@file		timetable_tabu_search.h
 *  @author     Hendrik Vermuyten
 *	@brief		The main surrogate-based tabu search algorithm
 */

#ifndef TIMETABLE_TABU_SEARCH_H
#define TIMETABLE_TABU_SEARCH_H

#include <QObject>
#include <vector>
#include <algorithm>
#include <random>
#include <cmath>
#include <chrono>

#include "menge_interface.h"
#include "machine_learning_interface.h"
#include "timetable_global_data.h"
#include "timetable_solution.h"

/*!
 *  @namespace timetable
 *  @brief	The namespace containing all elements related to the timetable and its optimisation.
 */
namespace timetable
{
    /*!
     *	@brief		The Surrogate-Based Tabu Search algorithm.
     */
    class tabu_search : public QObject
    {
        Q_OBJECT

    public:
        /*!
         *	@brief      Default constructor.
         */
        tabu_search();

        /*!
         *	@brief      Destructor.
         */
        ~tabu_search();

        /*!
         *	@brief      Set the menge interface.
         *  @param      menge       A pointer to a menge_interface instance.
         */
        void set_Menge(ped::menge_interface *menge) { _menge = menge; }

        /*!
         *	@brief      Set the machine learning interface.
         *  @param      ml_interface       A pointer to a machine_learning_interface instance.
         */
        void set_machine_learning_interface(ml::machine_learning_interface *ml_interface) { _surrogate = ml_interface; }

        /*!
         *	@brief      Set the initial solution to the algorithm.
         *  @param      startsol    An initial solution from which the Tabu Search algorithm starts.
         */
        void set_start_solution(const solution& startsol) { _current_solution = startsol; _best_solution = startsol; }

        /*!
         *	@brief      Get the best solution found by the Tabu Search algorithm.
         *  @returns    The best found solution.
         */
        const solution& get_best_solution() const { return _best_solution; }

        /*!
         *	@brief      Run the Tabu Search algorithm.
         */
        void run();

        /*!
         *	@brief      Run an exhaustive search.
         */
        void run_exhaustive_search();







        /*!
         *	@brief      Set the alpha that denotes the weight of both objective function terms (evacuations and travel times respectively).
         *  @param      alpha       The parameter that denotes the weight of both objective function terms.
         */
        void set_alpha_objective(double alpha) { _alpha_objective = alpha; }

        /*!
         *	@brief      Get the alpha that denotes the weight of both objective function terms (evacuations and travel times respectively).
         *  @returns    The alpha that denotes the weight of both objective function terms
         */
        double get_alpha_objective() const { return _alpha_objective; }

        /*!
         *	@brief      Set the replication budget for the tabu search.
         *  @param      budget       The replication budget for the tabu search.
         */
        void set_replication_budget_tabu_search(int budget) { _replication_budget_tabu_search = budget; }

        /*!
         *	@brief      Get the replication budget for the tabu search.
         *  @returns    The replication budget for the tabu search.
         */
        int get_replication_budget_tabu_search() const { return _replication_budget_tabu_search; }

        /*!
         *	@brief      Set the replication budget for the identification step.
         *  @param      budget       The replication budget for the identification step.
         */
        void set_replication_budget_identification_step(int budget) { _replication_budget_identification_step = budget; }

        /*!
         *	@brief      Get the replication budget for the identification step.
         *  @returns    The replication budget for the identification step.
         */
        int get_replication_budget_identification_step() const { return _replication_budget_identification_step; }

        /*!
         *	@brief      Set the tabu list length.
         *  @param      length       The length of the tabu list.
         */
        void set_tabu_list_length(int length) { _tabu_list_length = length; }

        /*!
         *	@brief      Get the tabu list length.
         *  @returns    The length of the tabu list.
         */
        int get_tabu_list_length() const { return _tabu_list_length; }

        /*!
         *	@brief      Set the (additional) number of replications of Menge that are used to (re-)evaluate a solution.
         *  @param      nb      The (additional) number of replications of Menge that are used to (re-)evaluate a solution.
         */
        void set_nb_eval_menge_incremental(int nb) { _nb_eval_menge_incremental = nb; }

        /*!
         *	@brief      Get the (additional) number of replications of Menge that are used to (re-)evaluate a solution.
         *  @returns    The (additional) number of replications of Menge that are used to (re-)evaluate a solution.
         */
        int get_nb_eval_menge_incremental() const { return _nb_eval_menge_incremental; }

        /*!
         *	@brief      Set the number of replications of Menge that are used to validate a solution.
         *  @param      nb       The number of replications of Menge that are used to validate a solution.
         */
        void set_nb_eval_menge_validate(int nb) { _nb_eval_menge_validate = nb; }

        /*!
         *	@brief      Get the number of replications of Menge that are used to validate a solution.
         *  @returns    The number of replications of Menge that are used to validate a solution.
         */
        int get_nb_eval_menge_validate() const { return _nb_eval_menge_validate; }

        /*!
         *	@brief      Set number of candidate solutions that are evaluated before concluding that a local minimum has been reached.
         *  @returns    nb      The number of candidate solutions that are evaluated before concluding that a local minimum has been reached.
         */
        void set_nb_eval_local_minimum(int nb) { _nb_eval_local_minimum = nb; }

        /*!
         *	@brief      Get number of candidate solutions that are evaluated before concluding that a local minimum has been reached.
         *  @returns    The number of candidate solutions that are evaluated before concluding that a local minimum has been reached.
         */
        int get_nb_eval_local_minimum() const { return _nb_eval_local_minimum; }

        /*!
         *	@brief      Set whether the performance of the Tabu Search and surrogates is evaluated during algorithm execution.
         *  @param      analyze_performance      True if the performance of the Tabu Search and surrogates is evaluated during algorithm execution, false otherwise.
         */
        void set_analyze_performance(bool analyze_performance) { _analyze_performance = analyze_performance; }

        /*!
         *	@brief      Get whether the performance of the Tabu Search and surrogates is evaluated during algorithm execution.
         *  @returns    True if the performance of the Tabu Search and surrogates is evaluated during algorithm execution.
         */
        bool get_analyze_performance() const { return _analyze_performance; }

        /*!
         *	@brief      Reset the values for the parameters of the Tabu Search.
         */
        void reset_TS_parameters();

        /*!
         *	@brief      Reset all values for the Tabu Search to start a new run.
         */
        void reset();

        /*!
         *	@brief      Set the timeslots for which the traveltimes need to be calculated.
         *  @param      timeslots       A vector containing the relevant timeslots.
         *                              The timeslot each time refers to the first timeslot of a timeslot pair in which the flows originate.
         */
        void set_timeslots_to_calculate_traveltimes(const std::vector<int>& timeslots) { _timeslots_to_calculate_traveltimes = timeslots; }

        /*!
         *	@brief      Check whether the vector _timeslots_to_calculate_traveltimes has not been set.
         *  @returns    true if the vector _timeslots_to_calculate_traveltimes has not been set.
         */
        bool timeslots_to_calculate_traveltimes_is_empty() { return _timeslots_to_calculate_traveltimes.empty(); }




        /*!
         *	@brief      Default value for the tabu list length.
         */
        static constexpr int _tabu_list_length_default = 40;

        /*!
         *	@brief      Default value for the simulation replication budget during tabu search.
         */
        static constexpr int _replication_budget_tabu_search_default = 10000;

        /*!
         *	@brief      Default value for the simulation replication budget during the identification step (OCBA).
         */
        static constexpr int _replication_budget_identification_step_default = 100;

        /*!
         *	@brief      Default value for the (additional) number of replications of Menge that are used to (re-)evaluate a solution.
         */
        static constexpr int _nb_eval_menge_incremental_default = 10;

        /*!
         *	@brief      Default value for the number of replications of Menge that are used to validate a solution.
         */
        static constexpr int _nb_eval_menge_validate_default = 100;

        /*!
         *	@brief      Default value for the maximum number of candidate solutions that are evaluated in a single iteration before deciding which to take.
         */
        static constexpr int _nb_eval_local_minimum_default = 10;







    signals:
        /*!
         *	@brief		Passes a text to indicate an error occured.
         */
        void signal_error(QString);

        /*!
         *	@brief      Pass a QString that signals the algorithm status.
         */
        void signal_algorithm_status(QString);

        /*!
         *	@brief      This signal is emitted when the Tabu Search has finished.
         */
        void finished();

        /*!
         *	@brief      This signal is emitted when a new best solution has been found.
         */
        void new_best_solution_found(timetable::solution);






    private:
        /*!
         *	@brief		Defines a swap (switching two assignments between cells) in the algorithm.
         */
        struct Swap
        {
            int timeslot = -1;                  ///< The timeslot in which the swap is executed.
            int location1 = -1;                 ///< The first location that is changed.
            int location2 = -1;                 ///< The second location that is changed.
            double obj_value_surrogate = 1e9;   ///< The objective value for the timeslot(s) of the swap of the new candidate solution evaluated by the surrogate.
            bool tabu = false;                  ///< Is the swap tabu?

            std::vector<double> observations_menge_evac;
            std::vector<double> observations_menge_tt1;
            std::vector<double> observations_menge_tt2;

            bool operator==(const Swap& other)
            {
                if(timeslot == other.timeslot
                        && ((location1 == other.location1 && location2 == other.location2)
                            || (location2 == other.location1 && location1 == other.location2)))
                    return true;
                else
                    return false;
            }

            Swap operator=(const Swap& other)
            {
                if(this!=&other)
                {
                    timeslot = other.timeslot;
                    location1 = other.location1;
                    location2 = other.location2;
                    obj_value_surrogate = other.obj_value_surrogate;
                    tabu = other.tabu;

                    observations_menge_evac = other.observations_menge_evac;
                    observations_menge_tt1 = other.observations_menge_tt1;
                    observations_menge_tt2 = other.observations_menge_tt2;
                }
                return *this;
            }
            bool operator<(const Swap& other)
            {
                return (this->obj_value_surrogate < other.obj_value_surrogate);
            }
        };

        /*!
         *	@brief      Function to sort the swaps based on their real objective values.
         *  @param s1   A possible swap
         *  @param s2   Another possible swap
         */
        bool sort_based_on_menge_values(const Swap& s1, const Swap& s2);





        /*!
         *	@brief      Pointer to menge object to run the pedestrian simulations (no ownership).
         */
        ped::menge_interface *_menge;

        /*!
         *	@brief      Pointer to surrogate object to run the pedestrian simulations (no ownership).
         */
        ml::machine_learning_interface *_surrogate;



        /*!
         *	@brief      Stores the current solution of the Tabu Search.
         *
         *  This can be worse than the best solution found so far.
         */
        solution _current_solution;

        /*!
         *	@brief      Stores the best solution found so far.
         */
        solution _best_solution;

        /*!
         *	@brief      Stores the best solutions found so far.
         */
        std::vector<solution> _best_solutions;

        /*!
         *	@brief      Stores the value of the upper 95 confidence interval for the best solution.
         */
        double _best_solution_upper_95_CI = 1e10;



        /*!
         *	@brief      Stores all swaps that are tabu.
         */
        std::vector<Swap> _tabu_list;

        /*!
         *	@brief      The length of the tabu list.
         */
        int _tabu_list_length = _tabu_list_length_default;

        /*!
         *	@brief      Indicates the importance of the evacuation time and travel time objectives respectively.
         *
         *  An alpha equal to 1 means that only evacuations are taken into account, an alpha equal to 0 means
         *  that only travel between events in consecutive timeslots is taken into account, and other values
         *  denote the weight of evacuations (alpha) and travels (1 - alpha) respectively.
         */
        double _alpha_objective = 1.0;

        /*!
         *	@brief      Replication budget for simulations during tabu search phase.
         */
        int _replication_budget_tabu_search = _replication_budget_tabu_search_default;

        /*!
         *	@brief      Replication budget for simulations during the identification step (OCBA).
         */
        int _replication_budget_identification_step = _replication_budget_identification_step_default;

        /*!
         *	@brief      The (additional) number of replications of Menge that are used to (re-)evaluate a solution.
         */
        int _nb_eval_menge_incremental = _nb_eval_menge_incremental_default;

        /*!
         *	@brief      The number of replications of Menge that are used to validate a solution.
         */
        int _nb_eval_menge_validate = _nb_eval_menge_validate_default;

        /*!
         *	@brief      The maximum number of candidate solutions that are evaluated in a single iteration before deciding which to take.
         */
        int _nb_eval_local_minimum = _nb_eval_local_minimum_default;


        /*!
         *	@brief      Time when the algorithm is started.
         */
        std::chrono::system_clock::time_point _start_time;

        /*!
         *	@brief      Indicates whether the performance of the Tabu Search and the surrogates is evaluated during algorithm run.
         */
        bool _analyze_performance = false;


        /*!
         *	@brief      Vector containing the timeslots in which the traveltimes are to be calculated.
         *
         *  Some timeslots are not relevant (e.g. between seperate days). Each timeslot refers to the first
         *  timeslot in the timeslot pair in which the flows originate.
         */
        std::vector<int> _timeslots_to_calculate_traveltimes;




        /*!
         *	@brief      Tabu Search where we take the best surrogate solution to re-evaluate.
         */
        void tabu_search_method_A();

        /*!
         *	@brief      Exploration loop for the Tabu Search.
         */
        void tabu_search_method_A_exploration_loop();




        /*!
         *	@brief      Recursive function that generates all possible solutions (for travels).
         *  @param      current_event   The current event that needs to be scheduled (i.e., the current iteration).
         */
        void generate_all_possible_solutions(int current_event);

        /*!
         *	@brief      Recursive function that generates all possible solutions for each timeslot independently (for evacuations).
         *  @param      timeslot    The timeslot for which we generate all possible solutions
         *  @param      current_event   The current event that needs to be scheduled (i.e., the current iteration).
         */
        void generate_all_possible_solutions_independently(int timeslot, int current_event);

        /*!
         *	@brief      The number of the current solution evaluated by exhaustive search.
         */
        int exhaustive_search_solutions_number = 0;

    };

} // namespace timetable

#endif // TIMETABLE_TABU_SEARCH_H
