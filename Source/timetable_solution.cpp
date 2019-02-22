#include "timetable_solution.h"
#include <QFile>
#include <QTextStream>
#include <stdexcept>

namespace
{
    const double student_t_values[11] = {0, 12.71, 4.303,  3.182, 2.776, 2.571, 2.447, 2.365, 2.306, 2.262, 2.228};
}




namespace timetable
{

    solution::solution()
    {

    }

    solution::solution(const solution &other)
    {
        m_events_location = other.m_events_location;
        m_events_timeslot = other.m_events_timeslot;

        m_objective_values_evac = other.m_objective_values_evac;
        m_objective_values_travels = other.m_objective_values_travels;
        m_mean_objective_values_evac = other.m_mean_objective_values_evac;
        m_mean_objective_values_travels = other.m_mean_objective_values_travels;
        m_stddev_objective_values_evac = other.m_stddev_objective_values_evac;
        m_stddev_objective_values_travels = other.m_stddev_objective_values_travels;
    }

    solution& solution::operator= (const solution& other)
    {       
        m_events_location = other.m_events_location;
        m_events_timeslot = other.m_events_timeslot;

        m_objective_values_evac = other.m_objective_values_evac;
        m_objective_values_travels = other.m_objective_values_travels;
        m_mean_objective_values_evac = other.m_mean_objective_values_evac;
        m_mean_objective_values_travels = other.m_mean_objective_values_travels;
        m_stddev_objective_values_evac = other.m_stddev_objective_values_evac;
        m_stddev_objective_values_travels = other.m_stddev_objective_values_travels;

        return *this;
    }




    void solution::read_data(const QString& filename)
    {
        QFile file(filename);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            throw std::runtime_error("Error in function timetable::solution::read_data. \nCouldn't open file.");
        }




        QTextStream stream(&file);
        QString input_token;
        bool input_ok;


        this->clear();



        m_events_timeslot.reserve(nb_events);
        m_events_location.reserve(nb_events);
        for(int l = 0; l < nb_events; ++l)
        {
            int time, location;

            stream >> input_token;
            time = input_token.toInt(&input_ok);
            if(!input_ok)
                throw std::runtime_error("Error in function timetable::solution::read_data. \nWrong input for \"event_timeslot\".");
            m_events_timeslot.push_back(time);

            stream >> input_token;
            location = input_token.toInt(&input_ok);
            if(!input_ok)
                throw std::runtime_error("Error in function timetable::solution::read_data. \nWrong input for \"event_location\".");
            m_events_location.push_back(location);
        }

        std::vector<double> _vec;
        for(int t = 0; t < nb_timeslots; ++t)
        {
            m_objective_values_evac.push_back(_vec);
            m_mean_objective_values_evac.push_back(0);
            m_stddev_objective_values_evac.push_back(0);
        }
        for(int t = 0; t < nb_timeslots - 1; ++t)
        {
            m_objective_values_travels.push_back(_vec);
            m_mean_objective_values_travels.push_back(0);
            m_stddev_objective_values_travels.push_back(0);
        }

        m_is_empty = false;
    }



    void solution::clear()
    {
        m_is_empty = true;
        m_events_timeslot.clear();
        m_events_location.clear();

        m_objective_values_evac.clear();
        m_objective_values_travels.clear();
        m_mean_objective_values_evac.clear();
        m_mean_objective_values_travels.clear();
        m_stddev_objective_values_evac.clear();
        m_stddev_objective_values_travels.clear();
    }



    void solution::reset_objective_values_evac(int timeslot)
    {
        m_objective_values_evac[timeslot].clear();
        m_mean_objective_values_evac[timeslot] = 0.0;
        m_stddev_objective_values_evac[timeslot] = 0.0;
    }

    void solution::reset_objective_values_travels(int first_timeslot)
    {
        m_objective_values_travels[first_timeslot].clear();
        m_mean_objective_values_travels[first_timeslot] = 0.0;
        m_stddev_objective_values_travels[first_timeslot] = 0.0;
    }

    void solution::add_objective_value_evac(int timeslot, double val)
    {
        m_objective_values_evac[timeslot].push_back(val);
    }

    void solution::add_objective_value_travels(int first_timeslot, double val)
    {
        m_objective_values_travels[first_timeslot].push_back(val);
    }

    void solution::calculate_means_and_stddevs_evac(int timeslot)
    {
        if(m_objective_values_evac[timeslot].size() > 0)
        {
            // mean
            double mean = 0.0;
            for(auto&& val: m_objective_values_evac[timeslot])
                mean += val;
            mean /= m_objective_values_evac[timeslot].size();

            m_mean_objective_values_evac[timeslot] = mean;


            // stddev
            double stddev = 0.0;
            for(auto&& val: m_objective_values_evac[timeslot])
                stddev += (m_mean_objective_values_evac[timeslot] - val)*(m_mean_objective_values_evac[timeslot] - val);
            stddev /= (m_objective_values_evac[timeslot].size()-1); // n-1
            stddev = std::sqrt(stddev);
            stddev /= std::sqrt((double)m_objective_values_evac[timeslot].size()); // root(n) => error of the predictor of the mean

            m_stddev_objective_values_evac[timeslot] = stddev;
        }
    }

    void solution::calculate_means_and_stddevs_travels(int first_timeslot)
    {
        if(m_objective_values_travels[first_timeslot].size() > 0)
        {
            // mean
            double mean = 0.0;
            for(auto&& val: m_objective_values_travels[first_timeslot])
                mean += val;
            mean /= m_objective_values_travels[first_timeslot].size();

            m_mean_objective_values_travels[first_timeslot] = mean;


            // stddev
            double stddev = 0.0;
            for(auto&& val: m_objective_values_travels[first_timeslot])
                stddev += (m_mean_objective_values_travels[first_timeslot] - val)*(m_mean_objective_values_travels[first_timeslot] - val);
            stddev /= (m_objective_values_travels[first_timeslot].size()-1); // n-1
            stddev = std::sqrt(stddev);
            stddev /= std::sqrt((double)m_objective_values_travels[first_timeslot].size()); // root(n) => error of the predictor of the mean

            m_stddev_objective_values_travels[first_timeslot] = stddev;
        }
    }

    double solution::mean_objective_value_evac(int timeslot) const
    {
        return m_mean_objective_values_evac[timeslot];
    }

    double solution::stddev_estimator_obj_val_evac(int timeslot) const
    {
        return m_stddev_objective_values_evac[timeslot];
    }

    double solution::mean_objective_value_travels(int first_timeslot) const
    {
        return m_mean_objective_values_travels[first_timeslot];
    }

    double solution::stddev_estimator_obj_val_travels(int first_timeslot) const
    {
        return m_stddev_objective_values_travels[first_timeslot];
    }


    double solution::total_mean_objective_value(double alpha_objective) const
    {
        double total_mean = 0.0;
        for(auto&& v: m_mean_objective_values_evac)
            total_mean += alpha_objective * v;
        for(auto&& v: m_mean_objective_values_travels)
            total_mean += (1-alpha_objective) * v;

        return total_mean;
    }

    double solution::total_stddev_objective_value(double alpha_objective) const
    {
        double total_stddev = 0.0;
        for(auto&& v: m_stddev_objective_values_evac)
            total_stddev += (alpha_objective*v) * (alpha_objective*v);
        for(auto&& v: m_stddev_objective_values_travels)
            total_stddev += ((1-alpha_objective)*v) * ((1-alpha_objective)*v);
        total_stddev = std::sqrt(total_stddev);

        return total_stddev;
    }

    double solution::upper_95_CI_objective_value(double alpha_objective) const
    {
        double total_mean = 0.0;
        for(auto&& v: m_mean_objective_values_evac)
            total_mean += alpha_objective * v;
        for(auto&& v: m_mean_objective_values_travels)
            total_mean += (1-alpha_objective) * v;

        double total_stddev = 0.0;
        for(auto&& v: m_stddev_objective_values_evac)
            total_stddev += (alpha_objective*v) * (alpha_objective*v);
        for(auto&& v: m_stddev_objective_values_travels)
            total_stddev += ((1-alpha_objective)*v) * ((1-alpha_objective)*v);
        total_stddev = std::sqrt(total_stddev);

        return (total_mean + 2.0 * total_stddev);
    }

    double solution::lower_95_CI_objective_value(double alpha_objective) const
    {
        double total_mean = 0.0;
        for(auto&& v: m_mean_objective_values_evac)
            total_mean += alpha_objective * v;
        for(auto&& v: m_mean_objective_values_travels)
            total_mean += (1-alpha_objective) * v;

        double total_stddev = 0.0;
        for(auto&& v: m_stddev_objective_values_evac)
            total_stddev += (alpha_objective*v) * (alpha_objective*v);
        for(auto&& v: m_stddev_objective_values_travels)
            total_stddev += ((1-alpha_objective)*v) * ((1-alpha_objective)*v);
        total_stddev = std::sqrt(total_stddev);

        return (total_mean - 2.0 * total_stddev);
    }
}







