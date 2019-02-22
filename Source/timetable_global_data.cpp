#include "timetable_global_data.h"
#include <QTextStream>
#include <QFileDialog>
#include <QFile>
#include <QMessageBox>
#include <QDebug>
#include <stdexcept>

namespace timetable
{
    bool data_exist = false;

    QString instance_name;

    int nb_events = 0;
    int nb_timeslots = 0;
    int nb_locations = 0;
    int nb_eventgroups = 0;

    std::vector<QString> event_names;
    std::vector<QString> eventgroup_names;
    std::vector<QString> location_names;

    std::vector<bool> eventgroup_event;
    std::vector<bool> event_location_possible;
    std::vector<int> event_block;
    std::vector<int> event_nb_people;
    std::vector<int> eventgroup_nb_people;

    // access functions
    const QString& get_event_name(int event) { return event_names[event]; }
    const QString& get_eventgroup_name(int eventgroup) { return eventgroup_names[eventgroup]; }
    const QString& get_location_name(int location) { return location_names[location]; }
    bool get_eventgroup_event(int eventgroup, int event) { return eventgroup_event[eventgroup * nb_events + event]; }
    bool get_event_location_possible(int event, int location) { if(event < 0) return true; else return event_location_possible[event * nb_locations + location]; }
    int get_event_nb_people(int event) { return event_nb_people[event]; }
    int get_eventgroup_nb_people(int eventgroup) { return eventgroup_nb_people[eventgroup]; }




    // input
    void read_data(const QString &filename)
    {
        data_exist = false;

        QFile file(filename);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            throw std::runtime_error("Error in function timetable::read_data. \nCouldn't open file.");
        }

        QTextStream stream(&file);
        QString input_token;
        bool input_ok;


        clear_data();



        // instance name
        stream >> instance_name;


        // first read in basic data
        stream >> input_token;
        nb_events = input_token.toInt(&input_ok);
        if(!input_ok)
            throw std::runtime_error("Error in function timetable::read_data. \nWrong input for \"nb_events\".");

        stream >> input_token;
        nb_eventgroups = input_token.toInt(&input_ok);
        if(!input_ok)
            throw std::runtime_error("Error in function timetable::read_data. \nWrong input for \"nb_eventgroups\".");

        stream >> input_token;
        nb_timeslots = input_token.toInt(&input_ok);
        if(!input_ok)
            throw std::runtime_error("Error in function timetable::read_data. \nWrong input for \"nb_timeslots\".");

        stream >> input_token;
        nb_locations = input_token.toInt(&input_ok);
        if(!input_ok)
            throw std::runtime_error("Error in function timetable::read_data. \nWrong input for \"nb_locations\".");



        // names events
        event_names.reserve(nb_events);
        for(int l = 0; l < nb_events; ++l)
        {
            QString name;
            stream >> name;
            if(name.isNull())
                throw std::runtime_error("Error in function timetable::read_data. \nEmpty input for \"event_name\".");
            event_names.push_back(name);
        }

        // names eventgroups
        eventgroup_names.reserve(nb_eventgroups);
        for(int c = 0; c < nb_eventgroups; ++c)
        {
            QString name;
            stream >> name;
            if(name.isNull())
                throw std::runtime_error("Error in function timetable::read_data. \nEmpty input for \"eventgroup_name\".");
            eventgroup_names.push_back(name);
        }

        // names locations
        location_names.reserve(nb_locations);
        for(int c = 0; c < nb_locations; ++c)
        {
            QString name;
            stream >> name;
            if(name.isNull())
                throw std::runtime_error("Error in function timetable::read_data. \nEmpty input for \"location_name\".");
            location_names.push_back(name);
        }

        // eventgroup event
        eventgroup_event.reserve(nb_eventgroups*nb_events);
        for(int c = 0; c < nb_eventgroups; ++c)
        {
            for(int l = 0; l < nb_events; ++l)
            {
                int k = -1;
                stream >> input_token;
                k = input_token.toInt(&input_ok);
                if(!input_ok || k < 0)
                    throw std::runtime_error("Error in function timetable::read_data. \nWrong input for \"eventgroup_event\".");
                eventgroup_event.push_back(k);
            }
        }

        // event location possible
        event_location_possible.reserve(nb_events*nb_locations);
        for(int l = 0; l < nb_events; ++l)
        {
            for(int r = 0; r < nb_locations; ++r)
            {
                int k = -1;
                stream >> input_token;
                k = input_token.toInt(&input_ok);
                if(!input_ok || k < 0)
                    throw std::runtime_error("Error in function timetable::read_data. \nWrong input for \"event_location_possible\".");
                event_location_possible.push_back(k);
            }
        }

        // event nb_people
        event_nb_people.reserve(nb_events);
        for(int l = 0; l < nb_events; ++l)
        {
            int k = -1;
            stream >> input_token;
            k = input_token.toInt(&input_ok);
            if(!input_ok || k < 0)
                throw std::runtime_error("Error in function timetable::read_data. \nWrong input for \"event_nb_people\".");
            event_nb_people.push_back(k);
        }

        // eventgroup nb_people
        eventgroup_nb_people.reserve(nb_eventgroups);
        for(int c = 0; c < nb_eventgroups; ++c)
        {
            int k = -1;
            stream >> input_token;
            k = input_token.toInt(&input_ok);
            if(!input_ok || k < 0)
                throw std::runtime_error("Error in function timetable::read_data. \nWrong input for \"eventgroup_nb_people\".");
            eventgroup_nb_people.push_back(k);
        }

        // end file input
        data_exist = true;
    }



    void clear_data()
    {
        data_exist = false;

        instance_name = "";

        nb_events = 0;
        nb_timeslots = 0;
        nb_locations = 0;
        nb_eventgroups = 0;

        event_names.clear();
        eventgroup_names.clear();
        location_names.clear();

        eventgroup_event.clear();
        event_location_possible.clear();
        event_nb_people.clear();
        eventgroup_nb_people.clear();
    }
}
