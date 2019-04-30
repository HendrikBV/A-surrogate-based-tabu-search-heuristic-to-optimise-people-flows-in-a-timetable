/*
 *	Code for the surrogate-based tabu search algorithm
 *  to optimise people flows in a timetable.
 *
 *	Code author: Hendrik Vermuyten
 */

/*!
 *	@file		menge_interface.h
 *  @author     Hendrik Vermuyten
 *	@brief		The interface to use the Menge simulator.
 */

#ifndef MENGE_INTERFACE_H
#define MENGE_INTERFACE_H

#include <QObject>

#include "building_data.h"
#include "timetable_global_data.h"
#include "timetable_solution.h"
#include <string>
#include <chrono>

// forward declaration
namespace Menge
{
    class SimulatorDB;
    class SimulatorDBEntry;
} // namespace Menge

/*!
 *  @namespace ped
 *  @brief	The namespace containing all elements related to the simulation of pedestrians.
 */
namespace ped
{
    /*!
     *	@brief		The interface to use the Menge simulator.
     */
    class menge_interface: public QObject
    {
        Q_OBJECT

    public:
        /*!
         *	@brief		Constructor.
         */
        menge_interface();

        /*!
         *	@brief		Destructor.
         */
        ~menge_interface();

        /*!
         *	@brief		Visualise an evacuation for a given timeslot.
         *  @param      sol     The solution for which an evacuation will be simulated.
         *  @param      timeslot        The timeslot for which an evacuation will be simulated.
         */
        double visualise_evacuation(const timetable::solution& sol, int timeslot);

        /*!
         *	@brief		Visualise the people flows between events in consuctive timeslots for a given timeslot pair.
         *  @param      sol     The solution for which an evacuation will be simulated.
         *  @param      timeslot        The first timeslot of the timeslot pair for which an evacuation will be simulated.
         */
        double visualise_flows(const timetable::solution& sol, int first_timeslot);

        /*!
         *	@brief		Calculate the evacuation time without visualisation for a given timeslot.
         *  @param      sol     The solution for which an evacuation will be simulated.
         *  @param      timeslot        The timeslot for which an evacuation will be simulated.
         */
        double calculate_evacuation_time(const timetable::solution& sol, int timeslot);

        /*!
         *	@brief		Calculate the travel time without visualisation for a given timeslot pair.
         *  @param      sol     The solution for which an evacuation will be simulated.
         *  @param      timeslot        The first timeslot of the timeslot pair for which an evacuation will be simulated.
         */
        double calculate_flows_time(const timetable::solution& sol, int first_timeslot);

        /*!
         *	@brief		Calculate the travel time without visualisation for a given timeslot pair.
         *  @param      sol     The solution for which an evacuation will be simulated.
         *  @param      timeslot        The first timeslot of the timeslot pair for which an evacuation will be simulated.
         */
        void start_calculations(const timetable::solution& sol, int replications);

        /*!
         *	@brief		Get the mean evacuation time in the given timeslot.
         *  @param      timeslot     The timeslot for which the mean evacuation time is requested.
         *  @returns    The mean evacuation time in the given timeslot.
         */
        double evacuation_time_avg(int timeslot) { return evacuation_times_avg[timeslot]; }

        /*!
         *	@brief		Get the mean travel time in the given timeslot.
         *  @param      timeslot     The timeslot for which the mean travel time is requested.
         *  @returns    The mean travel time in the given timeslot.
         */
        double travel_time_avg(int timeslot) { return travel_times_avg[timeslot]; }

        /*!
         *	@brief		Get the standard deviation of the evacuation time in the given timeslot.
         *  @param      timeslot     The timeslot for which the standard deviation of the evacuation time is requested.
         *  @returns    The standard deviation of the evacuation time in the given timeslot.
         */
        double evacuation_time_stddev(int timeslot) { return evacuation_times_stddev[timeslot]; }

        /*!
         *	@brief		Get the standard deviation of the travel time in the given timeslot.
         *  @param      timeslot     The timeslot for which the standard deviation of the travel time is requested.
         *  @returns    The standard deviation of the travel time in the given timeslot.
         */
        double travel_time_stddev(int timeslot) { return travel_times_stddev[timeslot]; }

        /*!
         *	@brief		Get the lower bound for the confidence interval of the evacuation time in the given timeslot.
         *  @param      timeslot     The timeslot for which the lower bound for the confidence interval of the evacuation time is requested.
         *  @returns    The lower bound for the confidence interval of the evacuation time in the given timeslot.
         */
        double evacuation_time_CImin(int timeslot);

        /*!
         *	@brief		Get the lower bound for the confidence interval of the travel time in the given timeslot.
         *  @param      timeslot     The timeslot for which the lower bound for the confidence interval of the travel time is requested.
         *  @returns    The lower bound for the confidence interval of the travel time in the given timeslot.
         */
        double travel_time_CImin(int timeslot);

        /*!
         *	@brief		Get the upper bound for the confidence interval of the evacuation time in the given timeslot.
         *  @param      timeslot     The timeslot for which the upper bound for the confidence interval of the evacuation time is requested.
         *  @returns    The upper bound for the confidence interval of the evacuation time in the given timeslot.
         */
        double evacuation_time_CImax(int timeslot);

        /*!
         *	@brief		Get the upper bound for the confidence interval of the travel time in the given timeslot.
         *  @param      timeslot     The timeslot for which the upper bound for the confidence interval of the travel time is requested.
         *  @returns    The upper bound for the confidence interval of the travel time in the given timeslot.
         */
        double travel_time_CImax(int timeslot);

        /*!
         *	@brief		Get the number of replication used for the simulations.
         *  @returns    The number of replication used for the simulations.
         */
        int replications() { return m_replications; }

        /*!
         *	@brief		Calculate the evacuation time given a number of people in each room.
         *  @param      nb_people_per_room     The number of people in each room of the building.
         *  @returns    The mean evacuation time.
         */
        double calculate_custom_evacuation_time(const std::vector<int>& nb_people_per_room);

        /*!
         *	@brief		Calculate the travel time given a number of people in travelling between each pair of rooms.
         *  @param      nb_people_per_room     The number of people in travelling between each pair of rooms.
         *  @returns    The mean travel time.
         */
        double calculate_custom_travel_time(const std::vector<int>& nb_people_from_room_to_room);

        /*!
         *	@brief		Get the maximum duration for a single simulation.
         *  @returns    The maximum duration for a single simulation.
         */
        float max_sim_duration() { return SIM_DURATION; }

        /*!
         *	@brief		The percentile of people who have reached their destination,
         *              that is used to calculate the simulation (travel/evacuation)
         *              time.
         *
         *  1 = last person
         *  0.95 = 95% of people have reached destination
         *  etc.
         */
        double _percentile_simulation_stopping_criterion = 1;

        /*!
         *	@brief		The mean preferred free walking speed of agents.
         */
        double _Common_pref_speed = 1.34;

        /*!
         *	@brief		The standard deviation of the preferred free walking speed of agents.
         */
        double _Common_pref_speed_stddev = 0.26;

        /*!
         *	@brief		The maximum walking speed of agents.
         */
        double _Common_max_speed = 1.74;

        /*!
         *	@brief		The maximum acceleration of agents.
         */
        double _Common_max_accel = 50.0;

        /*!
         *	@brief		The time step used in the simulation in seconds.
         */
        float TIME_STEP = 0.10f;

        /*!
         *	@brief		The number of sub steps per time step used in the simulation.
         */
        size_t SUB_STEPS = 0;

        /*!
         *	@brief		The type of pedestrian model used.
         *
         *  Possible choices are
         *  - orca (Optimal Reciprocal Collision Avoidance)
         *  - helbing (Helbing 2000 social-force model)
         *  - pedvo (Pedestrian Velocity Obstalces model)
         *  - ...
         */
        std::string MODEL = "pedvo";



    signals:
        /*!
         *	@brief		Passes a text to indicate an error occured.
         */
        void signal_error(QString);

        /*!
         *	@brief		Indicates that the calculations have finished.
         */
        void finished();

        /*!
         *	@brief		Indicates that a single calculation has finished and indicates its number.
         */
        void finished_one_calculation(int);

        /*!
         *	@brief		Passes a text to report on the status of the calculations.
         */
        void signal_status(QString);



    public slots:
        /*!
         *	@brief		Stop the calculations.
         */
        void halt();



    private:
        // PARAMETERS
        // ORCA
        double _ORCA_tau = 3.0;
        double _ORCA_tauObst = 0.15;

        // PedVO
        double _PedVO_factor = 1.57;
        double _PedVO_buffer = 0.9;
        double _PedVO_tau = 3.0;
        double _PedVO_tauObst = 0.1;
        double _PedVO_turningBias = 1.0;

        // OpenSteer
        double _OpenSteer_max_force = 8.0;
        double _OpenSteer_leak_through = 0.1;
        double _OpenSteer_reaction_time = 0.5;
        double _OpenSteer_tau = 3.0;
        double _OpenSteer_tauObst = 6.0;

        // GCF
        double _GCF_reaction_time = 0.5;
        double _GCF_max_agent_dist = 2.0;
        double _GCF_max_agent_force = 3.0;
        double _GCF_agent_interp_width = 0.1;
        double _GCF_agent_force_strength = 0.35;
        double _GCF_stand_depth = 0.18;
        double _GCF_move_scale = 0.53;
        double _GCF_slow_width = 0.25;
        double _GCF_sway_change = 0.05;

        // Helbing
        double _Helbing_agent_scale = 2000.0;
        double _Helbing_obstacle_scale = 4000.0;
        double _Helbing_reaction_time = 0.5;
        double _Helbing_body_force = 1200.0;
        double _Helbing_friction = 2400.0;
        double _Helbing_force_distance = 0.015;
        double _Helbing_mass = 80.0;

        // Karamouzas
        double _Karamouzas_orient_weight = 0.8;
        double _Karamouzas_fov = 200.0;
        double _Karamouzas_reaction_time = 0.4;
        double _Karamouzas_wall_steepness = 2.0;
        double _Karamouzas_wall_distance = 2.0;
        double _Karamouzas_colliding_count = 5.0;
        double _Karamouzas_d_min = 1.0;
        double _Karamouzas_d_mid = 8.0;
        double _Karamouzas_d_max = 10.0;
        double _Karamouzas_agent_force = 4.0;
        double _Karamouzas_personal_space = 0.69;
        double _Karamouzas_anticipation = 8.0;

        // Zanlungo
        double _Zanlungo_agent_scale = 2000.0;
        double _Zanlungo_obstacle_scale = 4000.0;
        double _Zanlungo_reaction_time = 0.5;
        double _Zanlungo_force_distance = 0.005;
        double _Zanlungo_mass = 80.0;
        double _Zanlungo_orient_weight = 0.75;

        // Dummy
        double _Dummy_stddev = 1.0;

        // Common
        double _Common_time_step = 0.1;
        double _Common_max_angle_vel = 360;
        double _Common_max_neighbors = 10.0;
        double _Common_obstacleSet = 1.0;
        double _Common_neighbor_dist = 5.0;
        double _Common_r = 0.2;
        int _Common_class = 3;

        /*!
         *	@brief		The location of the executable - for basic executable resources.
         */
        std::string ROOT = "";

        /*!
         *	@brief		Controls whether the simulation is verbose or not.
         */
        bool VERBOSE = true;

        /*!
         *	@brief		The maximum duration of a simulation in seconds.
         */
        float SIM_DURATION = 1800.f;

        /*!
         *	@brief		Indicates whether the simulation should be visualised.
         */
        bool VISUALIZE;

        /*!
         *	@brief		Values of the 95% 2-tailed student's t distribution in function of the degrees of freedom.
         */
        const double student_t_values[11] = {0, 12.71, 4.303,  3.182, 2.776, 2.571, 2.447, 2.365, 2.306, 2.262, 2.228};

        /*!
         *	@brief		The evacuation time for every timeslot and replication.
         */
        std::vector<double> evacuation_times;

        /*!
         *	@brief		The travel time in for every timeslot and replication.
         */
        std::vector<double> travel_times;

        /*!
         *	@brief		The mean evacuation time in every timeslot.
         */
        std::vector<double> evacuation_times_avg;

        /*!
         *	@brief		The mean travel time in every timeslot.
         */
        std::vector<double> travel_times_avg;

        /*!
         *	@brief		The standard deviation of the evacuation time in every timeslot.
         */
        std::vector<double> evacuation_times_stddev;

        /*!
         *	@brief		The standard deviation of the travel time in every timeslot.
         */
        std::vector<double> travel_times_stddev;

        /*!
         *	@brief		The total number of calculations required (based on the number of timeslots and the number of replications).
         */
        int total_number_of_calculations;

        /*!
         *	@brief		Counts the number of calculations done so far.
         */
        int current_calculation_number = 0;

        /*!
         *	@brief		The number of replications used.
         */
        int m_replications = 1;

        /*!
         *	@brief		The elapsed time in seconds.
         */
        double elapsed_time;

        /*!
         *	@brief		Indicates whether the calculations need to be stopped.
         */
        bool stop = false;

        /*!
         *	@brief		The main simulation function.
         */
        double sim_main();

        /*!
         *	@brief		Function that calls the Menge simulator with the correct specifications.
         *  @param      dbEntry         The simulator database entry.
         *  @param      behaveFile      The file that contains the behaviour of the agents.
         *  @param      sceneFile       The file that contains the information on the simulation area (building etc.).
         *  @param      outFile         The full path to the output file to write the agent trajectories. If the empty string, no output file will be written.
         *  @param      scbVersion      The scb version to write.
         *  @param      visualize       Indicates whether the simulation should be visualised or not.
         *  @param      viewCfgFile     The configuration file for the visualisation.
         *  @param      dumpPath        The path to which output images should be written.
         *  @returns    The simulation time in seconds.
         */
        double simulate(Menge::SimulatorDBEntry * dbEntry, const std::string & behaveFile,
                      const std::string & sceneFile, const std::string & outFile,
                      const std::string & scbVersion, bool visualize, const std::string & viewCfgFile,
                      const std::string & dumpPath);

        /*!
         *	@brief		Write an xml file that contains the behaviour specification for the simulation.
         */
        void write_behavior_xml();

        /*!
         *	@brief		Write an xml file that contains the scene specification for the simulation.
         *  @param      sol     The solution for which an evacuation will be simulated.
         *  @param      timeslot        The timeslot in which an evacuation will be simulated.
         */
        void write_scene_xml_evacuation(const timetable::solution& sol, int timeslot);

        /*!
         *	@brief		Write an xml file that contains the scene specification for the simulation.
         *  @param      sol     The solution for which the people flows between events in consecutive timeslots will be simulated.
         *  @param      timeslot        The first timeslot in the timeslot pair for which the people flows between events in consecutive timeslots will be simulated.
         */
        void write_scene_xml_travel(const timetable::solution& sol, int first_timeslot);

        /*!
         *	@brief		Write an xml file that contains the visualisation specification for the simulation.
         */
        void write_view_xml();

        /*!
         *	@brief		Write an xml file that contains the scene specification for the simulation.
         *  @param      nb_people_per_room     The number of people in each room of the building.
         */
        void write_scene_xml_evacuation(const std::vector<int> &nb_people_per_room);

        /*!
         *	@brief		Write an xml file that contains the scene specification for the simulation.
         *  @param      nb_people_per_room     The number of people travelling between each pair of rooms in the building.
         */
        void write_scene_xml_travel(const std::vector<int>& nb_people_from_room_to_room);
    };

} // namespace ped

#endif // MENGE_INTERFACE_H
