/*
 *	Code for the surrogate-based tabu search algorithm
 *  to optimise people flows in a timetable.
 *
 *	Code author: Hendrik Vermuyten
 */

/*!
 *	@file		building_data.h
 *  @author     Hendrik Vermuyten
 *	@brief		A set of global variables related to the building information.
 */

#ifndef BUILDING_DATA_H
#define BUILDING_DATA_H

#include <vector>
#include <QString>

/*!
 *  @namespace building
 *  @brief	The namespace containing all elements related to the building elements.
 */
namespace building
{
    /*!
     *	@brief		An obstacle represents walls etc. in a building.
     */
    struct obstacle
    {
        size_t nb_vertices;                 ///< The number of vertices making up the obstacle
        std::vector<double> vertices_x;     ///< The x-coordinates of the vertices
        std::vector<double> vertices_y;     ///< The y-coordinates of the vertices
    };

    /*!
     *	@brief		An element that describes a stairwell in the building.
     */
    struct stairs_element
    {
        int stairwell;          ///< Number of the stairwell
        int floor;              ///< The floor on which this element of the stairwell is located
        bool up;                ///< Is this element of the stairwell going up or down?
        double from_x_min;      ///< First x-coordinate of the rectangle which delimits the 'entrance' of the stairwell
        double from_x_max;      ///< Second x-coordinate of the rectangle which delimits the 'entrance' of the stairwell
        double from_y_min;      ///< Third x-coordinate of the rectangle which delimits the 'entrance' of the stairwell
        double from_y_max;      ///< Fourth x-coordinate of the rectangle which delimits the 'entrance' of the stairwell
        double to_x_min;        ///< First x-coordinate of the rectangle which delimits the 'exit' of the stairwell
        double to_x_max;        ///< Second x-coordinate of the rectangle which delimits the 'exit' of the stairwell
        double to_y_min;        ///< Third x-coordinate of the rectangle which delimits the 'exit' of the stairwell
        double to_y_max;        ///< Fourth x-coordinate of the rectangle which delimits the 'exit' of the stairwell
    };

    /*!
     *	@brief		An element that describes a the goals or targets of agents.
     */
    struct target
    {
        double x;               ///< The x-coordinate of the target
        double y;               ///< The y-coordinate of the target
        double dist_tolerance;  ///< If an agent's distance to the target is less than this value, the agent is considered to have reached the target
    };

    /*!
     *	@brief		The location to which agents are teleported when they have reached their target, to eliminate them from the simulation.
     */
    struct teleport_location
    {
        double x;   ///< The x-coordinate of the teleport target
        double y;   ///< The y-coordinate of the teleport target
    };

    /*!
     *	@brief		Indicates whether building data have been read in.
     */
    extern bool data_exist;

    /*!
     *	@brief		The name of the building instance.
     */
    extern QString instance_name;

    /*!
     *	@brief		The name of the road map for the building.
     */
    extern QString road_map_file_name;

    /*!
     *	@brief		All obstacles that describe the building layout.
     */
    extern std::vector<obstacle> obstacles;

    /*!
     *	@brief		All stairwells in the building.
     */
    extern std::vector<stairs_element> stairs;

    /*!
     *	@brief		The targets for each room in the building.
     */
    extern std::vector<target> room_targets;

    /*!
     *	@brief		The targets for each exit in the building.
     */
    extern std::vector<target> exit_targets;

    /*!
     *	@brief		The teleport locations.
     */
    extern std::vector<teleport_location> teleport_locations_rooms;

    /*!
     *	@brief		The teleport location for the exits.
     */
    extern teleport_location teleport_location_exit;

    /*!
     *	@brief		Function to import the building data from a txt-file.
     *  @param      filename        The name of the txt-file from which the data are to be imported.
     */
    extern void import_data(const QString &filename);

    /*!
     *	@brief		Deletes the building data.
     */
    extern void clear_data();

} // namespace building

#endif // BUILDING_DATA_H
