#include "building_data.h"

#include <QFile>
#include <QTextStream>
#include <stdexcept>

namespace building
{
    bool data_exist = false;
    QString instance_name;
    QString road_map_file_name;
    std::vector<obstacle> obstacles;
    std::vector<stairs_element> stairs;
    std::vector<target> room_targets;
    std::vector<target> exit_targets;
    std::vector<teleport_location> teleport_locations_rooms;
    teleport_location teleport_location_exit;


    void import_data(const QString &filename)
    {
        data_exist = false;

        QFile file(filename);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            throw std::runtime_error("Error in function building::import_data. \nCouldn't open file.");
        }


        QTextStream stream(&file);
        QString input_token;
        bool input_ok;


        clear_data();


        // name instance
        stream >> instance_name;


        // obstacles
        int nb_obstacles;
        stream >> input_token;
        nb_obstacles = input_token.toInt(&input_ok);
        if(!input_ok)
            throw std::runtime_error("Error in function building::import_data. \nWrong input for \"nb_obstacles\".");

        for(int i = 0; i < nb_obstacles; ++i)
        {
            obstacles.push_back(obstacle());

            int st;
            stream >> input_token;
            st = input_token.toInt(&input_ok);
            if(!input_ok)
                throw std::runtime_error("Error in function building::import_data. \nWrong input for \"nb_vertices\".");
            obstacles.back().nb_vertices = st;

            for(int j = 0; j < obstacles.back().nb_vertices; ++j)
            {
                double k;

                stream >> input_token;
                k = input_token.toDouble(&input_ok);
                if(!input_ok)
                    throw std::runtime_error("Error in function building::import_data. \nWrong input for \"vertices_x\".");
                obstacles.back().vertices_x.push_back(k);

                stream >> input_token;
                k = input_token.toDouble(&input_ok);
                if(!input_ok)
                    throw std::runtime_error("Error in function building::import_data. \nWrong input for \"vertices_y\".");
                obstacles.back().vertices_y.push_back(k);
            }
        }

        // stairs
        int nb_stairs_elements;
        stream >> input_token;
        nb_stairs_elements = input_token.toInt(&input_ok);
        if(!input_ok)
            throw std::runtime_error("Error in function building::import_data. \nWrong input for \"nb_stairs_elements\".");

        for(int i = 0; i < nb_stairs_elements; ++i)
        {
            double k;
            int j;
            stairs.push_back(stairs_element());

            stream >> input_token;
            j = input_token.toInt(&input_ok);
            if(!input_ok)
                throw std::runtime_error("Error in function building::import_data. \nWrong input for \"stairwell\".");
            stairs.back().stairwell = j;

            stream >> input_token;
            j = input_token.toInt(&input_ok);
            if(!input_ok)
                throw std::runtime_error("Error in function building::import_data. \nWrong input for \"floor\".");
            stairs.back().floor = j;

            stream >> input_token;
            j = input_token.toInt(&input_ok);
            if(!input_ok)
                throw std::runtime_error("Error in function building::import_data. \nWrong input for \"up or down\".");
            stairs.back().up = j;

            stream >> input_token;
            k = input_token.toDouble(&input_ok);
            if(!input_ok)
                throw std::runtime_error("Error in function building::import_data. \nWrong input for \"from_x_min\".");
            stairs.back().from_x_min = k;

            stream >> input_token;
            k = input_token.toDouble(&input_ok);
            if(!input_ok)
                throw std::runtime_error("Error in function building::import_data. \nWrong input for \"from_x_max\".");
            stairs.back().from_x_max = k;

            if(stairs.back().from_x_min >= stairs.back().from_x_max)
                throw std::runtime_error("Error in function building::import_data. \nFor stairs, \"from_x_min\" should be smaller than \"from_x_max\".");


            stream >> input_token;
            k = input_token.toDouble(&input_ok);
            if(!input_ok)
                throw std::runtime_error("Error in function building::import_data. \nWrong input for \"from_y_min\".");
            stairs.back().from_y_min = k;

            stream >> input_token;
            k = input_token.toDouble(&input_ok);
            if(!input_ok)
                throw std::runtime_error("Error in function building::import_data. \nWrong input for \"from_y_max\".");
            stairs.back().from_y_max = k;

            if(stairs.back().from_y_min >= stairs.back().from_y_max)
                throw std::runtime_error("Error in function building::import_data. \nFor stairs, \"from_y_min\" should be smaller than \"from_y_max\".");


            stream >> input_token;
            k = input_token.toDouble(&input_ok);
            if(!input_ok)
                throw std::runtime_error("Error in function building::import_data. \nWrong input for \"to_x_min\".");
            stairs.back().to_x_min = k;

            stream >> input_token;
            k = input_token.toDouble(&input_ok);
            if(!input_ok)
                throw std::runtime_error("Error in function building::import_data. \nWrong input for \"to_x_max\".");
            stairs.back().to_x_max = k;

            if(stairs.back().to_x_min >= stairs.back().to_x_max)
                throw std::runtime_error("Error in function building::import_data. \nFor stairs, \"to_x_min\" should be smaller than \"to_x_max\".");


            stream >> input_token;
            k = input_token.toDouble(&input_ok);
            if(!input_ok)
                throw std::runtime_error("Error in function building::import_data. \nWrong input for \"to_y_min\".");
            stairs.back().to_y_min = k;

            stream >> input_token;
            k = input_token.toDouble(&input_ok);
            if(!input_ok)
                throw std::runtime_error("Error in function building::import_data. \nWrong input for \"to_y_max\".");
            stairs.back().to_y_max = k;

            if(stairs.back().to_y_min >= stairs.back().to_y_max)
                throw std::runtime_error("Error in function building::import_data. \nFor stairs, \"to_y_min\" should be smaller than \"to_y_max\".");
        }

        // room targets
        int nb_room_targets;
        stream >> input_token;
        nb_room_targets = input_token.toInt(&input_ok);
        if(!input_ok)
            throw std::runtime_error("Error in function building::import_data. \nWrong input for \"nb_room_targets\".");
        for(int i = 0; i < nb_room_targets; ++i)
        {
            double k;
            room_targets.push_back(target());

            stream >> input_token;
            k = input_token.toDouble(&input_ok);
            if(!input_ok)
                throw std::runtime_error("Error in function building::import_data. \nWrong input for \"room_targets_x\".");
            room_targets.back().x = k;

            stream >> input_token;
            k = input_token.toDouble(&input_ok);
            if(!input_ok)
                throw std::runtime_error("Error in function building::import_data. \nWrong input for \"room_targets_y\".");
            room_targets.back().y = k;

            stream >> input_token;
            k = input_token.toDouble(&input_ok);
            if(!input_ok)
                throw std::runtime_error("Error in function building::import_data. \nWrong input for \"room_targets_distance_tolerance\".");
            room_targets.back().dist_tolerance = k;
        }

        // exit targets
        int nb_exit_targets;
        stream >> input_token;
        nb_exit_targets = input_token.toInt(&input_ok);
        if(!input_ok)
            throw std::runtime_error("Error in function building::import_data. \nWrong input for \"nb_exit_targets\".");
        for(int i = 0; i < nb_exit_targets; ++i)
        {
            double k;
            exit_targets.push_back(target());

            stream >> input_token;
            k = input_token.toDouble(&input_ok);
            if(!input_ok)
                throw std::runtime_error("Error in function building::import_data. \nWrong input for \"exit_targets_x\".");
            exit_targets.back().x = k;

            stream >> input_token;
            k = input_token.toDouble(&input_ok);
            if(!input_ok)
                throw std::runtime_error("Error in function building::import_data. \nWrong input for \"exit_targets_y\".");
            exit_targets.back().y = k;

            stream >> input_token;
            k = input_token.toDouble(&input_ok);
            if(!input_ok)
                throw std::runtime_error("Error in function building::import_data. \nWrong input for \"exit_targets_distance_tolerance\".");
            exit_targets.back().dist_tolerance = k;
        }

        // rooms teleport targets
        for(int i = 0; i < nb_room_targets; ++i)
        {
            teleport_locations_rooms.push_back(teleport_location());
            double k;

            stream >> input_token;
            k = input_token.toDouble(&input_ok);
            if(!input_ok)
                throw std::runtime_error("Error in function building::import_data. \nWrong input for \"teleport_room_targets_x\".");
            teleport_locations_rooms.back().x = k;

            stream >> input_token;
            k = input_token.toDouble(&input_ok);
            if(!input_ok)
                throw std::runtime_error("Error in function building::import_data. \nWrong input for \"teleport_room_targets_y\".");
            teleport_locations_rooms.back().y = k;
        }

        // exit teleport target
        {
            double k;

            stream >> input_token;
            k = input_token.toDouble(&input_ok);
            if(!input_ok)
                throw std::runtime_error("Error in function building::import_data. \nWrong input for \"teleport_exit_target_x\".");
            teleport_location_exit.x = k;

            stream >> input_token;
            k = input_token.toDouble(&input_ok);
            if(!input_ok)
                throw std::runtime_error("Error in function building::import_data. \nWrong input for \"teleport_exit_target_y\".");
            teleport_location_exit.y = k;
        }

        // road map file name
        stream >> road_map_file_name;
        if(road_map_file_name.isNull())
            throw std::runtime_error("Error in function building::import_data. \nMissing input for \"road_map_file_name\".");

        data_exist = true;
    }



    void clear_data()
    {
        data_exist = false;

        instance_name = "";
        road_map_file_name = "";

        obstacles.clear();
        stairs.clear();
        room_targets.clear();
        exit_targets.clear();
    }
}
