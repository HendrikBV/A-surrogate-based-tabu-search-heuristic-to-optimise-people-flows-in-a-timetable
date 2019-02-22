/*
 *	Code for the surrogate-based tabu search algorithm
 *  to optimise people flows in a timetable.
 *
 *	Code author: Hendrik Vermuyten
 */

/*!
 *	@file		timetable_solution.h
 *  @author     Hendrik Vermuyten
 *	@brief		The definition of a timetable solution.
 */

#ifndef TIMETABLE_SOLUTION_H
#define TIMETABLE_SOLUTION_H

#include <QObject>
#include <vector>
#include <QString>
#include "timetable_global_data.h"

/*!
 *  @namespace timetable
 *  @brief	The namespace containing all elements related to the timetable and its optimisation.
 */
namespace timetable
{
    /*!
     *	@brief		The definition of a timetable solution.
     */
    class solution : public QObject
    {
        Q_OBJECT

    public:
        /*!
         *	@brief		Default constructor.
         */
        solution();

        /*!
         *	@brief		Copy constructor.
         *  @param      other   A timetable solution to copy in a new object.
         */
        solution(const solution &other);

        /*!
         *	@brief		Assignment operator.
         *  @param      other   A timetable solution to assign to another solution object.
         *  @returns    A reference to the changed solution object.
         */
        solution& operator= (const solution& other);

        /*!
         *	@brief		Check whether the solution is empty or not.
         *  @returns    True if the solution is empty, false otherwise.
         */
        bool is_empty() const { return m_is_empty; }

        /*!
         *	@brief		Import a solution from a txt-file.
         *  @param      filename        The name of the txt-file.
         */
        void read_data(const QString &filename);

        /*!
         *	@brief		Clear all solution data.
         */
        void clear();

        /*!
         *	@brief		Get the timeslot in which a given event is planned.
         *  @param      event       The event for which the timeslot is to be returned.
         *  @returns    The timeslot in which the given event is planned.
         */
        int event_timeslot(int event) const { return m_events_timeslot[event]; }

        /*!
         *	@brief		Get the location to which a given event is assigned.
         *  @param      event       The event for which the location is to be returned.
         *  @returns    The location to which a given event is assigned.
         */
        int event_location(int event) const { return m_events_location[event]; }

        /*!
         *	@brief		Check which event is planned in the given timeslot and location.
         *  @param      timeslot        The timeslot to check.
         *  @param      location        The location to check.
         *  @returns    The number of the event that is planned in the given timeslot and location, -1 if no event is planned in that timeslot in that location.
         */
        int timeslot_location(int timeslot, int location) const
        {
            for(int l = 0; l < nb_events; ++l) {
                if(m_events_timeslot[l] == timeslot && m_events_location[l] == location) {
                    return l;
                }
            }
            return -1; // no event planned in that timeslot & location
        }

        /*!
         *	@brief		Set the location of a given event.
         *  @param      event       The event for which the location is to be set.
         *  @param      location    The location in which the event will be planned.
         */
        void set_event_location(int event, int location) { m_events_location[event] = location;}

        /*!
         *	@brief		Reset the evacuation times for a given timeslot.
         *  @param      timeslot        The timeslot for which the evacuation times are reset.
         */
        void reset_objective_values_evac(int timeslot);

        /*!
         *	@brief		Reset the travel times for a given timeslot pair.
         *  @param      first_timeslot        The first timeslot of the timeslot pair for which the travel times are reset.
         */
        void reset_objective_values_travels(int first_timeslot);

        /*!
         *	@brief		Add the result of a single replication for the evacuation time in a given timeslot.
         *  @param      timeslot    The timeslot for which an evacuation time is added.
         *  @param      val         The evacuation time.
         */
        void add_objective_value_evac(int timeslot, double val);

        /*!
         *	@brief		Add the result of a single replication for the travel time in a given timeslot pair.
         *  @param      first_timeslot    The first timeslot of the timeslot pair for which a travel time is added.
         *  @param      val               The travel time.
         */
        void add_objective_value_travels(int first_timeslot, double val);

        /*!
         *	@brief		Calculate the mean and standard devation of the evacuation time in a given timeslot.
         *  @param      timeslot    The timeslot for which the mean and standard devation of the evacuation time are to be calculated.
         */
        void calculate_means_and_stddevs_evac(int timeslot);

        /*!
         *	@brief		Calculate the mean and standard devation of the evacuation time in a given timeslot.
         *  @param      timeslot    The timeslot for which the mean and standard devation of the evacuation time are to be calculated.
         */
        void calculate_means_and_stddevs_travels(int first_timeslot);

        /*!
         *	@brief		Get the mean evacuation time for a given timeslot.
         *  @param      timeslot        The timeslot for which the mean evacuation time is to be returned.
         *  @returns    The mean evacuation time for the given timeslot.
         */
        double mean_objective_value_evac(int timeslot) const;

        /*!
         *	@brief		Get the standard deviation of the evacuation time for a given timeslot.
         *  @param      timeslot        The timeslot for which the standard deviation of the evacuation time is to be returned.
         *  @returns    The standard deviation of the evacuation time for the given timeslot.
         */
        double stddev_estimator_obj_val_evac(int timeslot) const;

        /*!
         *	@brief		Get the mean travel time for a given timeslot pair.
         *  @param      first_timeslot        The first timeslot of the timeslot pair for which the mean evacuation time is to be returned.
         *  @returns    The mean travel time for the first timeslot of the given timeslot pair.
         */
        double mean_objective_value_travels(int first_timeslot) const;

        /*!
         *	@brief		Get the standard deviation of the mean travel time for a given timeslot pair.
         *  @param      first_timeslot        The first timeslot of the timeslot pair for which the standard deviation of the evacuation time is to be returned.
         *  @returns    The standard deviation of the travel time for the first timeslot of the given timeslot pair.
         */
        double stddev_estimator_obj_val_travels(int first_timeslot) const;

        /*!
         *	@brief		Get the total mean objective value for a given alpha.
         *  @param      alpha_objective        The relative weights of evacuations (alpha) and travels (1 - alpha).
         *  @returns    The total mean objective value for the given alpha.
         */
        double total_mean_objective_value(double alpha_objective) const;

        /*!
         *	@brief		Get the total standard deviation of the objective value for a given alpha.
         *  @param      alpha_objective        The relative weights of evacuations (alpha) and travels (1 - alpha).
         *  @returns    The total standard deviation of the objective value for the given alpha.
         */
        double total_stddev_objective_value(double alpha_objective) const;

        /*!
         *	@brief		Get the lower bound of the confidence interval for the total objective value for a given alpha.
         *  @param      alpha_objective        The relative weights of evacuations (alpha) and travels (1 - alpha).
         *  @returns    The the lower bound of the confidence interval for the total objective value for the given alpha.
         */
        double upper_95_CI_objective_value(double alpha_objective) const;

        /*!
         *	@brief		Get the upper bound of the confidence interval for the total objective value for a given alpha.
         *  @param      alpha_objective        The relative weights of evacuations (alpha) and travels (1 - alpha).
         *  @returns    The the upper bound of the confidence interval for the total objective value for the given alpha.
         */
        double lower_95_CI_objective_value(double alpha_objective) const;

        /*!
         *	@brief		Get a vector with the mean evacuation time for each timeslot.
         *  @returns    A vector with the mean evacuation time for each timeslot.
         */
        const std::vector<double>& means_objective_evac() const { return m_mean_objective_values_evac; }

        /*!
         *	@brief		Get a vector with the mean travel time for each timeslot.
         *  @returns    A vector with the mean travel time for each timeslot.
         */
        const std::vector<double>& means_objective_travels() const { return m_mean_objective_values_travels; }

        /*!
         *	@brief		Get a vector with the standard deviation of the evacuation time for each timeslot.
         *  @returns    A vector with the standard deviation of the evacuation time for each timeslot.
         */
        const std::vector<double>& stddevs_objective_evac() const { return m_stddev_objective_values_evac; }

        /*!
         *	@brief		Get a vector with the standard deviation of the travel time for each timeslot.
         *  @returns    A vector with the standard deviation of the travel time for each timeslot.
         */
        const std::vector<double>& stddevs_objective_travels() const { return m_stddev_objective_values_travels; }



    signals:
        /*!
         *	@brief		Indicates whether a change is made to the solution.
         */
        void changed();



    private:
        /*!
         *	@brief		Indicates whether the solution is empty or not.
         */
        bool m_is_empty = true;

        /*!
         *	@brief		A vector containing the timeslots assigned to each event.
         */
        std::vector<int> m_events_timeslot;

        /*!
         *	@brief		A vector containing the locations assigned to each event.
         */
        std::vector<int> m_events_location;

        /*!
         *	@brief		A vector containing the evacuation times for every replication and every timeslot.
         */
        std::vector<std::vector<double>> m_objective_values_evac;

        /*!
         *	@brief		A vector containing the travel times for every replication and every timeslot pair.
         */
        std::vector<std::vector<double>> m_objective_values_travels;

        /*!
         *	@brief		A vector containing the mean evacuation time for every timeslot.
         */
        std::vector<double> m_mean_objective_values_evac;

        /*!
         *	@brief		A vector containing the standard error of the evacuation time for every timeslot.
         *
         *  This value is the standard deviation of the predictor for the mean, i.e. s / root(n).
         */
        std::vector<double> m_stddev_objective_values_evac;

        /*!
         *	@brief		A vector containing the standard deviation of the evacuation time for every timeslot.
         */
        std::vector<double> m_mean_objective_values_travels;

        /*!
         *	@brief		A vector containing the standard error of the travel time for every timeslot.
         *
         *  This value is the standard deviation of the predictor for the mean, i.e. s / root(n).
         */
        std::vector<double> m_stddev_objective_values_travels;
    };

} // namespace timetable

Q_DECLARE_METATYPE(timetable::solution)



#endif // TIMETABLE_SOLUTION_H
