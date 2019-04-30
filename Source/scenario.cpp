#include "scenario.h"

#include <QFile>
#include <QTextStream>
#include <stdexcept>

namespace scenario
{
    bool data_exist = false;
    QString instance_name;
    std::vector<group_firefighters> firefighter_groups;


    void import_data(const QString &filename)
    {
        data_exist = false;

        QFile file(filename);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            throw std::runtime_error("Error in function scenario::import_data. \nCouldn't open file.");
        }


        QTextStream stream(&file);
        QString input_token;
        bool input_ok;


        clear_data();


        // name instance
        stream >> instance_name;


        // firefighters groups
        int nb_groups_firefighters;
        stream >> input_token;
        nb_groups_firefighters = input_token.toInt(&input_ok);
        if(!input_ok)
            throw std::runtime_error("Error in function scenario::import_data. \nWrong input for \"nb_groups_firefighters\".");

        for(int i = 0; i < nb_groups_firefighters; ++i)
        {
            group_firefighters group;

            stream >> input_token;
            group.nb_firefighters = input_token.toInt(&input_ok);
            if(!input_ok)
                throw std::runtime_error("Error in function scenario::import_data. \nWrong input for \"nb_firefighters\".");

            stream >> input_token;
            group.destination_x = input_token.toDouble(&input_ok);
            if(!input_ok)
                throw std::runtime_error("Error in function scenario::import_data. \nWrong input for \"destination_x\".");

            stream >> input_token;
            group.destination_y = input_token.toDouble(&input_ok);
            if(!input_ok)
                throw std::runtime_error("Error in function scenario::import_data. \nWrong input for \"destination_y\".");

            firefighter_groups.push_back(group);
        }

        data_exist = true;
    }



    void clear_data()
    {
        data_exist = false;
        instance_name = "";
        firefighter_groups.clear();
    }


}
