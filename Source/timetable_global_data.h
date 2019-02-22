/*
 *	Code for the surrogate-based tabu search algorithm
 *  to optimise people flows in a timetable.
 *
 *	Code author: Hendrik Vermuyten
 */

/*!
 *	@file		timetable_global_data.h
 *  @author     Hendrik Vermuyten
 *	@brief		A set of global variables related to the timetable information.
 */

#ifndef TIMETABLE_GLOBAL_DATA_H
#define TIMETABLE_GLOBAL_DATA_H

#include <vector>
#include <QString>

/*!
 *  @namespace timetable
 *  @brief	The namespace containing all elements related to the timetable and its optimisation.
 */
namespace timetable
{
    /*!
     *	@brief		Indicates whether data have been imported.
     */
    extern bool data_exist;

    /*!
     *	@brief		The name of the timetable instance.
     */
    extern QString instance_name;

    /*!
     *	@brief		The number of events in the instance.
     */
    extern int nb_events;

    /*!
     *	@brief		The number of timeslots in the instance.
     */
    extern int nb_timeslots;

    /*!
     *	@brief		The number of locations in the instance.
     */
    extern int nb_locations;

    /*!
     *	@brief		The number of eventgroups in the instance.
     */
    extern int nb_eventgroups;

    /*!
     *	@brief		The names of the events.
     */
    extern std::vector<QString> event_names;

    /*!
     *	@brief		The names of the eventgroups.
     */
    extern std::vector<QString> eventgroup_names;

    /*!
     *	@brief		The names of the locations (rooms).
     */
    extern std::vector<QString> location_names;

    /*!
     *	@brief		A vector of nb_eventgroups x nb_events elements where a 1 indicates that an event belongs to a given eventgroup.
     */
    extern std::vector<bool> eventgroup_event;

    /*!
     *	@brief		A vector of nb_events x nb_locations elements where a 1 indicates that an event can be assigned to a given location.
     */
    extern std::vector<bool> event_location_possible;

    /*!
     *	@brief		The number of people that attend each event.
     */
    extern std::vector<int> event_nb_people;

    /*!
     *	@brief		The number of people in each eventgroup.
     */
    extern std::vector<int> eventgroup_nb_people;

    /*!
     *	@brief		Return the name of a given event.
     *  @param      event       The event for which the name is to be returned.
     *  @returns    The name of the given event.
     */
    extern const QString& get_event_name(int event);

    /*!
     *	@brief		Return the name of a given eventgroup.
     *  @param      eventgroup       The eventgroup for which the name is to be returned.
     *  @returns    The name of the given eventgroup.
     */
    extern const QString& get_eventgroup_name(int eventgroup);

    /*!
     *	@brief		Return the name of a given location.
     *  @param      location       The location for which the name is to be returned.
     *  @returns    The name of the given location.
     */
    extern const QString& get_location_name(int location);

    /*!
     *	@brief		Check whether an event belongs to a certain eventgroup.
     *  @param      eventgroup  The given eventgroup.
     *  @param      event       The given event.
     *  @returns    True if the event belongs to the given eventgroup, false otherwise.
     */
    extern bool get_eventgroup_event(int eventgroup, int event);

    /*!
     *	@brief		Check whether an event can be assigned to a certain location.
     *  @param      event       The given event.
     *  @param      location    The given location.
     *  @returns    True if the event can be assigned to the given location, false otherwise.
     */
    extern bool get_event_location_possible(int event, int location);

    /*!
     *	@brief		Get the number of people that attend a given event.
     *  @param      event       The given event.
     *  @returns    The number of people that attend a given event.
     */
    extern int get_event_nb_people(int event);

    /*!
     *	@brief		Get the number of people that belong to a given eventgroup.
     *  @param      eventgroup       The given eventgroup.
     *  @returns    The number of people that belong to a given eventgroup.
     */
    extern int get_eventgroup_nb_people(int eventgroup);

    /*!
     *	@brief		Import the timetable data from a txt-file.
     *  @param      filename        The name of the txt-file.
     */
    extern void read_data(const QString& filename);

    /*!
     *	@brief		Clear all timetable data.
     */
    extern void clear_data();

} // namespace timetable

#endif // TIMETABLE_GLOBAL_DATA_H
