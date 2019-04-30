/*
 *	Code for the surrogate-based tabu search algorithm
 *  to optimise people flows in a timetable.
 *
 *	Code author: Hendrik Vermuyten
 */

/*!
 *	@file		scenario.h
 *  @author     Hendrik Vermuyten
 *	@brief		A set of global variables related to a specific evacuation scenario.
 */

#ifndef SCENARIO_H
#define SCENARIO_H

#include <vector>
#include <QString>

/*!
 *  @namespace scenario
 *  @brief	The namespace containing all elements related to the elements defining a scenario.
 */
namespace scenario
{
    /*!
     *	@brief		An obstacle represents walls etc. in a building.
     */
    struct group_firefighters
    {
        int nb_firefighters;        ///< The number of firefighters in the group
        double destination_x;       ///< The x coordinate of their destination
        double destination_y;       ///< The y coordinate of their destination
    };

    /*!
     *	@brief		Indicates whether scenario data exist.
     */
    extern bool data_exist;

    /*!
     *	@brief		The name of the scenario instance.
     */
    extern QString instance_name;

    /*!
     *	@brief		The groups of firefighters.
     */
    extern std::vector<group_firefighters> firefighter_groups;


    /*!
     *	@brief		Function to import the scenario data from a txt-file.
     *  @param      filename        The name of the txt-file from which the data are to be imported.
     */
    extern void import_data(const QString &filename);

    /*!
     *	@brief		Deletes the scenario data.
     */
    extern void clear_data();
}

#endif // SCENARIO_H
