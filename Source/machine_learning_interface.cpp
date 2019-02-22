#include "machine_learning_interface.h"

#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <stdexcept>



namespace
{
    std::random_device randdev;
    std::seed_seq seedseq{ randdev(), randdev(), randdev(), randdev(), randdev(), randdev(), randdev(), randdev() };
    std::mt19937 generator(seedseq);

    constexpr double NANO = 1000000000.0;
    constexpr std::chrono::nanoseconds max_runtime_findminglobal(1000*(long long)NANO); // maximum runtime of find_min_global set to 1000 seconds
}


namespace ml
{   
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void surrogate_paths::read_data(const QString& filename)
    {
        clear_data();
        _room_node.reserve(timetable::nb_locations);



        QFile file(filename);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            throw std::runtime_error("Error in function ml::surrogate_paths::read_data. \nCouldn't open file.");
        }


        QTextStream stream(&file);
        QString input_token;
        bool input_ok;


        // room-node
        _nb_nodes = 0;
        for(size_t i = 0; i < timetable::nb_locations; ++i)
        {
            int node;
            stream >> input_token;
            node = input_token.toInt(&input_ok);
            if(!input_ok)
                throw std::runtime_error("Error in function ml::surrogate_paths::read_data. \nWrong input for \"node\".");
            _room_node.push_back(node);

            if(node >= _nb_nodes)
                ++_nb_nodes;
        }
        _room_node.push_back(_nb_nodes); // node == _nb_nodes - 1 (last one); position == timetable::nb_locations
        ++_nb_nodes; // exit node

        // node-node-path
        _nb_paths = 0;
        _node_node_path.reserve(_nb_nodes*_nb_nodes);
        for(size_t i = 0; i < _nb_nodes*_nb_nodes; ++i)
            _node_node_path.push_back(-1);
        for(size_t i = 0; i < _nb_nodes; ++i)
        {
            for(size_t j = i + 1; j < _nb_nodes; ++j)
            {
                _node_node_path.at(i*_nb_nodes + j) = _nb_paths;
                _node_node_path.at(j*_nb_nodes + i) = _nb_paths;   // symmetric 'matrix'
                ++_nb_paths;
            }
        }

        // path-rooms
        std::vector<int> vec_;
        for(size_t i = 0; i < _nb_paths; ++i)
            _paths_rooms.push_back(vec_);
        for(size_t r1 = 0; r1 < _room_node.size(); ++r1) // including exit
        {
            for(size_t r2 = r1 + 1; r2 < _room_node.size(); ++r2)
            {
                int node1 = _room_node.at(r1);
                int node2 = _room_node.at(r2);

                int path = _node_node_path.at(node1 * _nb_nodes + node2);

                if(path != -1)
                {
                    _paths_rooms.at(path).push_back(r1);
                    _paths_rooms.at(path).push_back(r2);
                }
            }
        }


        _data_exist = true;
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void surrogate_paths::clear_data()
    {
        _data_exist = false;
        _nb_paths = 0;
        _nb_nodes = 0;
        _room_node.clear();
        _node_node_path.clear();
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    sample_type surrogate_paths::transfrom_solution_to_ml_sample(const timetable::solution& sol, int first_timeslot) const
    {
        sample_type sample;
        sample.set_size(_nb_paths);
        for(size_t i = 0; i < sample.size(); ++i)
            sample(i) = 0;

        for(int c = 0; c < timetable::nb_eventgroups; ++c)
        {
            // check the situation for eventgroup c:
            // a. t and t+1
            // b. t, not t+1
            // c. not t, t+1
            // d. not t, not t+1

            int nb_people_in_group = timetable::get_eventgroup_nb_people(c);

            int event_first_timeslot = -1;
            int room_first_timeslot = -1;
            int event_second_timeslot = -1;
            int room_second_timeslot = -1;

            for(int l = 0; l < timetable::nb_events; ++l)
            {
                if(timetable::get_eventgroup_event(c,l))
                {
                    // first timeslot
                    if(sol.event_timeslot(l) == first_timeslot)
                    {
                        event_first_timeslot = l;

                        for(int r = 0; r < timetable::nb_locations; ++r)
                        {
                            if(sol.event_location(l) == r)
                            {
                                room_first_timeslot = r;
                                break;
                            }
                        }
                    }

                    // second timeslot
                    if(sol.event_timeslot(l) == first_timeslot + 1)
                    {
                        event_second_timeslot = l;

                        for(int r = 0; r < timetable::nb_locations; ++r)
                        {
                            if(sol.event_location(l) == r)
                            {
                                room_second_timeslot = r;
                                break;
                            }
                        }
                    }
                }
            }

            // A. class time t, class time t+1
            if(event_first_timeslot >= 0 && event_second_timeslot >= 0)
            {
                int node1 = _room_node[room_first_timeslot];
                int node2 = _room_node[room_second_timeslot];

                if(node1 != node2) // same node => no paths (path == -1 => access error)
                {
                    int path = _node_node_path[node1 * _nb_nodes + node2];

                    sample(path) += nb_people_in_group;
                }
            }

            // B. class time t, no class time t+1
            else if(event_first_timeslot >= 0)
            {
                int node1 = _room_node[room_first_timeslot];
                int node2 = _room_node[timetable::nb_locations];

                int path = _node_node_path[node1 * _nb_nodes + node2];

                sample(path) += nb_people_in_group;
            }

            // C. no class time t, class time t+1
            else if(event_second_timeslot >= 0)
            {
                int node1 = _room_node[timetable::nb_locations];
                int node2 = _room_node[room_second_timeslot];

                int path = _node_node_path[node1 * _nb_nodes + node2];

                sample(path) += nb_people_in_group;
            }

            // D. no classes at time t or time t+1
            // Do nothing

        }


        //qDebug() << "Solution to sample";
        //for(int p = 0; p < _nb_paths; ++p)
        //    qDebug() << "Path " << p+1 << ", nb_people = " << sample(p);

        return sample;
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void surrogate_paths::generate_observations(ped::menge_interface *menge, std::vector<sample_type>& x_obs, std::vector<double>& y_obs, int nb_observations)
    {
        // DESIGN OF EXPERIMENTS
        QString output_text;
        std::chrono::system_clock::time_point start_time = std::chrono::system_clock::now();



        /*QString filename = QStringLiteral("training_data_travels.txt");
        QFile file(filename);

        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) // read data from file
        {
            output_text = "\nImporting training data travels from file";
            emit(signal_stat(output_text));

            QTextStream stream(&file);
            stream >> nb_observations;

            for(size_t i = 0; i < nb_observations; ++i)
            {
                sample_type sample;
                sample.set_size(_nb_paths);
                for(size_t j = 0; j < _nb_paths; ++j)
                {
                    int people;
                    stream >> people;
                    sample(j) = people;
                }
                x_obs.push_back(sample);

                double traveltime;
                stream >> traveltime;
                y_obs.push_back(traveltime);
            }
        }

        else*/

        {
            output_text = "\nGenerating training data travels ...";
            emit(signal_stat(output_text));

            // 1. initialize vector of training data
            x_obs.clear();
            x_obs.reserve(nb_observations);
            y_obs.clear();
            y_obs.reserve(nb_observations);


            // 2. maximum number of people on one path (approximate)
            int compression_factor = std::ceil((double)building::room_targets.size() / _nb_nodes) + 0.1;
            std::vector<int> sorted_nb_people_per_event = timetable::eventgroup_nb_people;
            std::sort(sorted_nb_people_per_event.begin(), sorted_nb_people_per_event.end());
            int max_nb_people = 0;
            int pos = sorted_nb_people_per_event.size() - 1;
            for(int p = 0; p < compression_factor; ++p)
            {
                max_nb_people += sorted_nb_people_per_event[pos];
                --pos;
            }


            // 3. latin hypercube sampling
            std::vector<std::vector<int>> paths_remaining_strata;
            std::vector<int> vec_;
            vec_.reserve(nb_observations);
            for(int i = 0; i < nb_observations; ++i)
                vec_.push_back(i);

            for(int p = 0; p < _nb_paths; ++p)   // initialize strata
                paths_remaining_strata.push_back(vec_);

            // generate N observations and use menge to analyze them
            for(int obs = 0; obs < nb_observations - 1; ++obs)
            {
                std::vector<int> paths_nb_people;
                paths_nb_people.reserve(timetable::nb_locations);

                //qDebug() << "Observation " << obs+1;

                for(int p = 0; p < _nb_paths; ++p) // generate the number of every people on each path
                {
                    const double probability_path_used = 0.5;
                    std::bernoulli_distribution dist_path_used(probability_path_used);
                    if(dist_path_used(generator)) // if path used
                    {
                        std::uniform_int_distribution<int> dist_index(0, paths_remaining_strata[p].size() - 1);
                        int index = dist_index(generator);
                        int stratum = paths_remaining_strata[p][index];
                        int nbp = stratum * max_nb_people / (nb_observations - 1) +0.1;     // borders of the stratum (one group less so that both endpoints are sampled)
                        //int nbp = (int)((double)(stratum + 0.5) * room_max_nb_people[r] / nb_training_data + 0.5); // middle of the stratum

                        if(obs == 0)
                            paths_nb_people.push_back(max_nb_people);
                        else
                            paths_nb_people.push_back(nbp);

                        // delete this stratum from the vector
                        if(obs > 0)
                            paths_remaining_strata[p].erase(paths_remaining_strata[p].begin() + index);
                    }
                    else // path not used
                        paths_nb_people.push_back(0);

                    //qDebug() << "Path " << p+1 << ", nb_people = " << paths_nb_people.back();
                }

                // divide people on path evenly(?) (randomly) over rooms
                std::vector<int> room_room_nb_people;
                room_room_nb_people.reserve((timetable::nb_locations+1)*(timetable::nb_locations+1));
                for(int i = 0; i < _room_node.size()*_room_node.size(); ++i)
                    room_room_nb_people.push_back(0);
                for(int p = 0; p < _nb_paths; ++p)
                {
                    const int nb_combos = _paths_rooms[p].size() * (_paths_rooms[p].size()-1);
                    int nbp = std::round((double)paths_nb_people[p] / nb_combos);
                    for(auto&& r1: _paths_rooms[p])
                    {
                        for(auto&& r2: _paths_rooms[p])
                        {
                            if(r1!=r2)
                                room_room_nb_people[r1*_room_node.size() + r2] += nbp;
                        }
                    }
                }

                // use menge to estimate travel time
                double traveltime = menge->calculate_custom_travel_time(room_room_nb_people);
                if(traveltime < 0)
                    traveltime = 0;

                // put observation in matrix
                sample_type mat;
                mat.set_size(paths_nb_people.size());
                for(int p = 0; p < _nb_paths; ++p)
                    mat(p) = paths_nb_people[p];
                x_obs.push_back(mat);
                y_obs.push_back(traveltime);

                output_text = "    Observation ";
                output_text.append(QString::number(obs+1));
                output_text.append("; Travel time = ");
                output_text.append(QString::number(traveltime));
                output_text.append(" seconds");
                emit(signal_stat(output_text));
            }

            // add row with all zeros
            sample_type mat;
            mat.set_size(_nb_paths);
            for(int p = 0; p < _nb_paths; ++p)
                mat(p) = 0;
            x_obs.push_back(mat);
            y_obs.push_back(0.0);
            output_text = "    Observation ";
            output_text.append(QString::number(nb_observations));
            output_text.append("; Travel time = 0 seconds");
            emit(signal_stat(output_text));



            std::chrono::nanoseconds elapsed_time = std::chrono::system_clock::now() - start_time;
            double _time_generating_data_travels = elapsed_time.count() / NANO;
            output_text = "Finished generating training data. Elapsed time: ";
            output_text.append(QString::number(_time_generating_data_travels));
            output_text.append(" seconds");
            emit(signal_stat(output_text));
            global::_logger << global::logger::log_type::INFORMATION << "Generated " << nb_observations << " training data travels.\nElapsed time = " << _time_generating_data_travels << " seconds.";



            /*QFile savefile(filename);
            if(savefile.open(QIODevice::WriteOnly | QIODevice::Text))
            {
                QTextStream stream(&savefile);
                stream << nb_observations << "\n";

                for(size_t i = 0; i < nb_observations; ++i)
                {
                    stream << "\n";
                    for(size_t j = 0; j < timetable::nb_locations; ++j)
                        stream << x_obs[i](j) << "\t";
                    stream << y_obs[i];
                }
            }*/
        }
    }













    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///                                                                                                              ///
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


    machine_learning_interface::machine_learning_interface()
    {

    }


    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    machine_learning_interface::~machine_learning_interface()
    {

    }





    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void machine_learning_interface::clear()
    {
        _nb_training_data = 0;
        _training_data_evac_x.clear();
        _training_data_evac_y.clear();
        _training_data_travels_x.clear();
        _training_data_travels_y.clear();
        _trained_surrogate_evacuations = false;
        _trained_surrogate_travels = false;
    }





    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    double machine_learning_interface::predict_evacuation_time(const timetable::solution& sol, int timeslot) const
    {
        // 1. transform data into sample_type
        sample_type input_data;
        input_data.set_size(timetable::nb_locations);
        for(int r = 0; r < timetable::nb_locations; ++r) {
            int event = sol.timeslot_location(timeslot, r);
            if(event >= 0) {
                int nb_people = timetable::get_event_nb_people(event);
                input_data(r) = nb_people;
            }
            else {
                input_data(r) = 0;
            }
        }

        // 2. use correct decision function
        if(_learning_method == learning_method::krr_trainer_radial_basis_kernel
                || _learning_method == learning_method::svr_trainer_radial_basis_kernel
                || _learning_method == learning_method::rvm_trainer_radial_basis_kernel)
        {
            return _decision_functions_evacuations.dec_func_rbk(input_data);
        }
        else if(_learning_method == learning_method::krr_trainer_histogram_intersection_kernel
                || _learning_method == learning_method::svr_trainer_histogram_intersection_kernel
                || _learning_method == learning_method::rvm_trainer_histogram_intersection_kernel)
        {
            return _decision_functions_evacuations.dec_func_hik(input_data);
        }
        else if(_learning_method == learning_method::krr_trainer_polynomial_kernel_quadratic
                || _learning_method == learning_method::krr_trainer_polynomial_kernel_cubic
                || _learning_method == learning_method::svr_trainer_polynomial_kernel_quadratic
                || _learning_method == learning_method::svr_trainer_polynomial_kernel_cubic
                || _learning_method == learning_method::rvm_trainer_polynomial_kernel_quadratic
                || _learning_method == learning_method::rvm_trainer_polynomial_kernel_cubic)
        {
            return _decision_functions_evacuations.dec_func_pk(input_data);
        }
        else if(_learning_method == learning_method::krr_trainer_linear_kernel
                || _learning_method == learning_method::svr_trainer_linear_kernel
                || _learning_method == learning_method::rvm_trainer_linear_kernel)
        {
            return _decision_functions_evacuations.dec_func_lk(input_data);
        }


        return -1;
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    double machine_learning_interface::predict_travel_time(const timetable::solution& sol, int first_timeslot) const
    {
        // 1. transform data into sample_type
        sample_type input_data = _surrogate_paths->transfrom_solution_to_ml_sample(sol, first_timeslot);


        // 2. use correct decision function
        if(_learning_method == learning_method::krr_trainer_radial_basis_kernel
                || _learning_method == learning_method::svr_trainer_radial_basis_kernel
                || _learning_method == learning_method::rvm_trainer_radial_basis_kernel)
        {
            return _decision_functions_travels.dec_func_rbk(input_data);
        }
        else if(_learning_method == learning_method::krr_trainer_histogram_intersection_kernel
                || _learning_method == learning_method::svr_trainer_histogram_intersection_kernel
                || _learning_method == learning_method::rvm_trainer_histogram_intersection_kernel)
        {
            return _decision_functions_travels.dec_func_hik(input_data);
        }
        else if(_learning_method == learning_method::krr_trainer_polynomial_kernel_quadratic
                || _learning_method == learning_method::krr_trainer_polynomial_kernel_cubic
                || _learning_method == learning_method::svr_trainer_polynomial_kernel_quadratic
                || _learning_method == learning_method::svr_trainer_polynomial_kernel_cubic
                || _learning_method == learning_method::rvm_trainer_polynomial_kernel_quadratic
                || _learning_method == learning_method::rvm_trainer_polynomial_kernel_cubic)
        {
            return _decision_functions_travels.dec_func_pk(input_data);
        }
        else if(_learning_method == learning_method::krr_trainer_linear_kernel
                || _learning_method == learning_method::svr_trainer_linear_kernel
                || _learning_method == learning_method::rvm_trainer_linear_kernel)
        {
            return _decision_functions_travels.dec_func_lk(input_data);
        }


        return -1;
    }





    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void machine_learning_interface::train()
    {      
        if(_alpha_objective > 0.01 && _alpha_objective < 0.99)
            emit(signal_status("\n\nTraining surrogate models"));
        else
            emit(signal_status("\n\nTraining surrogate model"));

        std::chrono::system_clock::time_point start_time = std::chrono::system_clock::now();


        // evacuations
        if(_alpha_objective > 0.01)
        {
            int attempts = 0;
            while(true)
            {
                ++attempts;
                if(attempts > 3)
                {
                    if(attempts > 9)
                        throw std::runtime_error("Fatal error occurred in ml::machine_learning_interface::train.");

                    _training_data_evac_x.clear();
                    _training_data_evac_y.clear();

                    QString text = "Failure to train surrogate model evacuations persists. \nGenerating new data ... ";
                    emit(signal_status(text));
                }


                if(_training_data_evac_y.size() < _nb_training_data)
                    generate_training_data_evacuations();

                try
                {
                    switch(_learning_method)
                    {
                    case learning_method::krr_trainer_radial_basis_kernel:
                        train_krr_radial_basis_evacuations();
                        break;
                    case learning_method::krr_trainer_histogram_intersection_kernel:
                        train_krr_histogram_intersection_evacuations();
                        break;
                    case learning_method::krr_trainer_linear_kernel:
                        train_krr_linear_evacuations();
                        break;
                    case learning_method::krr_trainer_polynomial_kernel_quadratic:
                        train_krr_quadratic_evacuations();
                        break;
                    case learning_method::krr_trainer_polynomial_kernel_cubic:
                        train_krr_cubic_evacuations();
                        break;
                    case learning_method::svr_trainer_radial_basis_kernel:
                        train_svr_radial_basis_evacuations();
                        break;
                    case learning_method::svr_trainer_histogram_intersection_kernel:
                        train_svr_histogram_intersection_evacuations();
                        break;
                    case learning_method::svr_trainer_linear_kernel:
                        train_svr_linear_evacuations();
                        break;
                    case learning_method::svr_trainer_polynomial_kernel_quadratic:
                        train_svr_quadratic_evacuations();
                        break;
                    case learning_method::svr_trainer_polynomial_kernel_cubic:
                        train_svr_cubic_evacuations();
                        break;
                    case learning_method::rvm_trainer_radial_basis_kernel:
                        train_rvm_radial_basis_evacuations();
                        break;
                    case learning_method::rvm_trainer_histogram_intersection_kernel:
                        train_rvm_histogram_intersection_evacuations();
                        break;
                    case learning_method::rvm_trainer_linear_kernel:
                        train_rvm_linear_evacuations();
                        break;
                    case learning_method::rvm_trainer_polynomial_kernel_quadratic:
                        train_rvm_quadratic_evacuations();
                        break;
                    case learning_method::rvm_trainer_polynomial_kernel_cubic:
                        train_rvm_cubic_evacuations();
                        break;
                    }


                    // if successful stop loop and continue
                    _trained_surrogate_evacuations = true;
                    break;
                }
                catch(std::exception& e)
                {
                    QString text = "Training surrogate model evacuations failed. Reason: ";
                    text.append(e.what());
                    text.append("\nTrying again ... \n");
                    emit(signal_status(text));
                }
            }
        }


        // travels
        if(_alpha_objective < 0.99)
        {
            int attempts = 0;
            while(true)
            {
                ++attempts;
                if(attempts > 3)
                {
                    if(attempts > 9)
                        throw std::runtime_error("Fatal error occurred in ml::machine_learning_interface::train.");

                    _training_data_travels_x.clear();
                    _training_data_travels_y.clear();

                    QString text = "Failure to train surrogate model evacuations persists. \nGenerating new data ... ";
                    emit(signal_status(text));
                }


                if(_training_data_travels_y.size() < _nb_training_data)
                    generate_training_data_travels();

                try
                {
                    switch(_learning_method)
                    {
                    case learning_method::krr_trainer_radial_basis_kernel:
                        train_krr_radial_basis_travels();
                        break;
                    case learning_method::krr_trainer_histogram_intersection_kernel:
                        train_krr_histogram_intersection_travels();
                        break;
                    case learning_method::krr_trainer_linear_kernel:
                        train_krr_linear_travels();
                        break;
                    case learning_method::krr_trainer_polynomial_kernel_quadratic:
                        train_krr_quadratic_travels();
                        break;
                    case learning_method::krr_trainer_polynomial_kernel_cubic:
                        train_krr_cubic_travels();
                        break;
                    case learning_method::svr_trainer_radial_basis_kernel:
                        train_svr_radial_basis_travels();
                        break;
                    case learning_method::svr_trainer_histogram_intersection_kernel:
                        train_svr_histogram_intersection_travels();
                        break;
                    case learning_method::svr_trainer_linear_kernel:
                        train_svr_linear_travels();
                        break;
                    case learning_method::svr_trainer_polynomial_kernel_quadratic:
                        train_svr_quadratic_travels();
                        break;
                    case learning_method::svr_trainer_polynomial_kernel_cubic:
                        train_svr_cubic_travels();
                        break;
                    case learning_method::rvm_trainer_radial_basis_kernel:
                        train_rvm_radial_basis_travels();
                        break;
                    case learning_method::rvm_trainer_histogram_intersection_kernel:
                        train_rvm_histogram_intersection_travels();
                        break;
                    case learning_method::rvm_trainer_linear_kernel:
                        train_rvm_linear_travels();
                        break;
                    case learning_method::rvm_trainer_polynomial_kernel_quadratic:
                        train_rvm_quadratic_travels();
                        break;
                    case learning_method::rvm_trainer_polynomial_kernel_cubic:
                        train_rvm_cubic_travels();
                        break;
                    }


                    // if successful, stop loop and continue
                    _trained_surrogate_travels = true;
                    break;
                }
                catch(std::exception& e)
                {
                    QString text = "Training surrogate model travels failed. Reason: ";
                    text.append(e.what());
                    text.append("\nTrying again ... \n");
                    emit(signal_status(text));
                }
            }
        }



        std::chrono::nanoseconds comptime = std::chrono::system_clock::now() - start_time;

        if(_alpha_objective > 0.01 && _alpha_objective < 0.99) {
            QString info = "\nFinished training surrogate models.\nElapsed time = ";
            info += QString::number(comptime.count() / NANO);
            info += " seconds";
            emit(signal_status(info));
        }
        else {
            QString info = "\nFinished training surrogate model.\nElapsed time = ";
            info += QString::number(comptime.count() / NANO);
            info += " seconds";
            emit(signal_status(info));
        }
    }





    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    void machine_learning_interface::compare_learning_methods()
    {
        emit(signal_status("\n\nComparing learning methods"));
        std::chrono::system_clock::time_point start_time = std::chrono::system_clock::now();

        // evacuations
        if(_alpha_objective > 0.01)
        {
            if(_training_data_evac_y.size() < _nb_training_data)
                generate_training_data_evacuations();


            // KRR Trainers
            try {
                train_krr_radial_basis_evacuations();
            } catch(std::exception& e) {
                QString text = "Training krr_radial_basis_evacuations failed. Reason: ";
                text.append(e.what());
                emit(signal_status(text));
            }

            try {
                train_krr_histogram_intersection_evacuations();
            } catch(std::exception& e) {
                QString text = "Training krr_histogram_intersection_evacuations failed. Reason: ";
                text.append(e.what());
                emit(signal_status(text));
            }

            try {
                train_krr_linear_evacuations();
            } catch(std::exception& e) {
                QString text = "Training krr_linear_evacuations failed. Reason: ";
                text.append(e.what());
                emit(signal_status(text));
            }

            try {
                train_krr_quadratic_evacuations();
            } catch(std::exception& e) {
                QString text = "Training krr_quadratic_evacuations failed. Reason: ";
                text.append(e.what());
                emit(signal_status(text));
            }

            try {
                //train_krr_cubic_evacuations();
            } catch(std::exception& e) {
                QString text = "Training krr_cubic_evacuations failed. Reason: ";
                text.append(e.what());
                emit(signal_status(text));
            }


            // SVR Trainers
            try {
                train_svr_radial_basis_evacuations();
            } catch(std::exception& e) {
                QString text = "Training svr_radial_basis_evacuations failed. Reason: ";
                text.append(e.what());
                emit(signal_status(text));
            }

            try {
                //train_svr_histogram_intersection_evacuations();
            } catch(std::exception& e) {
                QString text = "Training svr_histogram_intersection_evacuations failed. Reason: ";
                text.append(e.what());
                emit(signal_status(text));
            }

            try {
                //train_svr_linear_evacuations();
            } catch(std::exception& e) {
                QString text = "Training svr_linear_evacuations failed. Reason: ";
                text.append(e.what());
                emit(signal_status(text));
            }

            try {
                //train_svr_quadratic_evacuations();
            } catch(std::exception& e) {
                QString text = "Training svr_quadratic_evacuations failed. Reason: ";
                text.append(e.what());
                emit(signal_status(text));
            }

            try {
                //train_svr_cubic_evacuations();
            } catch(std::exception& e) {
                QString text = "Training svr_cubic_evacuations failed. Reason: ";
                text.append(e.what());
                emit(signal_status(text));
            }


            // RVM Trainers
            try {
                train_rvm_radial_basis_evacuations();
            } catch(std::exception& e) {
                QString text = "Training rvm_radial_basis_evacuations failed. Reason: ";
                text.append(e.what());
                emit(signal_status(text));
            }

            try {
                train_rvm_histogram_intersection_evacuations();
            } catch(std::exception& e) {
                QString text = "Training rvm_histogram_intersection_evacuations failed. Reason: ";
                text.append(e.what());
                emit(signal_status(text));
            }

            try {
                train_rvm_linear_evacuations();
            } catch(std::exception& e) {
                QString text = "Training rvm_linear_evacuations failed. Reason: ";
                text.append(e.what());
                emit(signal_status(text));
            }

            try {
                train_rvm_quadratic_evacuations();
            } catch(std::exception& e) {
                QString text = "Training rvm_quadratic_evacuations failed. Reason: ";
                text.append(e.what());
                emit(signal_status(text));
            }

            try {
                //train_rvm_cubic_evacuations();
            } catch(std::exception& e) {
                QString text = "Training rvm_cubic_evacuations failed. Reason: ";
                text.append(e.what());
                emit(signal_status(text));
            }
        }

        // travels
        else if(_alpha_objective < 0.99)
        {
            if(_training_data_travels_y.size() < _nb_training_data)
                generate_training_data_travels();


            // KRR Trainers
            try {
                train_krr_radial_basis_travels();
            } catch(std::exception& e) {
                QString text = "Training krr_radial_basis_travels failed. Reason: ";
                text.append(e.what());
                emit(signal_status(text));
            }

            try {
                train_krr_histogram_intersection_travels();
            } catch(std::exception& e) {
                QString text = "Training krr_histogram_intersection_travels failed. Reason: ";
                text.append(e.what());
                emit(signal_status(text));
            }

            try {
                train_krr_linear_travels();
            } catch(std::exception& e) {
                QString text = "Training krr_linear_travels failed. Reason: ";
                text.append(e.what());
                emit(signal_status(text));
            }

            try {
                train_krr_quadratic_travels();
            } catch(std::exception& e) {
                QString text = "Training krr_quadratic_travels failed. Reason: ";
                text.append(e.what());
                emit(signal_status(text));
            }

            try {
                //train_krr_cubic_travels();
            } catch(std::exception& e) {
                QString text = "Training krr_cubic_travels failed. Reason: ";
                text.append(e.what());
                emit(signal_status(text));
            }


            // SVR Trainers
            try {
                train_svr_radial_basis_travels();
            } catch(std::exception& e) {
                QString text = "Training svr_radial_basis_travels failed. Reason: ";
                text.append(e.what());
                emit(signal_status(text));
            }

            try {
                train_svr_histogram_intersection_travels();
            } catch(std::exception& e) {
                QString text = "Training svr_histogram_intersection_travels failed. Reason: ";
                text.append(e.what());
                emit(signal_status(text));
            }

            try {
                train_svr_linear_travels();
            } catch(std::exception& e) {
                QString text = "Training svr_linear_travels failed. Reason: ";
                text.append(e.what());
                emit(signal_status(text));
            }

            try {
                train_svr_quadratic_travels();
            } catch(std::exception& e) {
                QString text = "Training svr_quadratic_travels failed. Reason: ";
                text.append(e.what());
                emit(signal_status(text));
            }

            try {
                //train_svr_cubic_travels();
            } catch(std::exception& e) {
                QString text = "Training svr_cubic_travels failed. Reason: ";
                text.append(e.what());
                emit(signal_status(text));
            }


            // RVM Trainers
            try {
                train_rvm_radial_basis_travels();
            } catch(std::exception& e) {
                QString text = "Training rvm_radial_basis_travels failed. Reason: ";
                text.append(e.what());
                emit(signal_status(text));
            }

            try {
                train_rvm_histogram_intersection_travels();
            } catch(std::exception& e) {
                QString text = "Training rvm_histogram_intersection_travels failed. Reason: ";
                text.append(e.what());
                emit(signal_status(text));
            }

            try {
                train_rvm_linear_travels();
            } catch(std::exception& e) {
                QString text = "Training rvm_linear_travels failed. Reason: ";
                text.append(e.what());
                emit(signal_status(text));
            }

            try {
                train_rvm_quadratic_travels();
            } catch(std::exception& e) {
                QString text = "Training rvm_quadratic_travels failed. Reason: ";
                text.append(e.what());
                emit(signal_status(text));
            }

            try {
                //train_rvm_cubic_travels();
            } catch(std::exception& e) {
                QString text = "Training rvm_cubic_travels failed. Reason: ";
                text.append(e.what());
                emit(signal_status(text));
            }
        }

        std::chrono::nanoseconds comptime = std::chrono::system_clock::now() - start_time;
        QString info = "\nFinished comparing learning methods.\nElapsed time: ";
        info += QString::number(comptime.count() / NANO);
        info += " seconds";
        emit(signal_status(info));
    }








    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void machine_learning_interface::train_krr_radial_basis_evacuations()
    {
        // computation time and status information
        std::chrono::system_clock::time_point start_time = std::chrono::system_clock::now();
        QString info = "\nStart training KRR Trainer with Radial Basis Kernel for evacuations ...";
        emit(signal_status(info));


        // Randomize the samples
        dlib::randomize_samples(_training_data_evac_x, _training_data_evac_y);

        // Here we define a function, cross_validation_score(),
        // that will do the cross-validation we mentioned and
        // return a number indicating how good a particular setting
        // of gamma (kernel) and lambda (regularization) is.
        auto cross_validation_score = [&](const double gamma, const double lambda) -> double
        {
            // define krls object with corresponding parameters
            dlib::krr_trainer<radial_basis_kernel> krr_trainer;

            // set the kernel and its parameter gamma
            krr_trainer.set_kernel(radial_basis_kernel(gamma));
            krr_trainer.set_lambda(lambda);

            // perform 10-fold cross validation and return the results.
            dlib::matrix<double> result = dlib::cross_validate_regression_trainer(krr_trainer, _training_data_evac_x, _training_data_evac_y, 10);

            // We return a number indicating how good the parameters are.  Bigger is
            // better in this example.  We're returning the harmonic mean between the
            // accuracies of each class.
            //return 2*dlib::prod(result)/dlib::sum(result);

            return result(0);   // result(0) = MSE (mean squared error)
                                // result(1) = correlation between y' and y
                                // result(2) = MAE (mean absolute error)
                                // result(3) = the standard deviation of the absolute error
        };

        // We call this global optimizer that will search for the best parameters.
        // It will call cross_validation_score() 50 times with different settings and return
        // the best parameter setting it finds.
        auto best_parameters = dlib::find_min_global(cross_validation_score,
                                                     {_rbk_gamma_min, _krr_lambda_min},    // lower bound constraint on gamma and lambda
                                                     {_rbk_gamma_max, _krr_lambda_max},    // upper bound constraint on gamma and lambda
                                                     dlib::max_function_calls(50),
                                                     max_runtime_findminglobal);

        // Define krr object with best found parameters
        double rbk_gamma = best_parameters.x(0);
        double krr_lambda = best_parameters.x(1);      
        dlib::krr_trainer<radial_basis_kernel> krr_trainer;
        krr_trainer.set_kernel(radial_basis_kernel(rbk_gamma));
        krr_trainer.set_lambda(krr_lambda);

        // Now we train on the full set of data and get the resulting decision function
        _decision_functions_evacuations.dec_func_rbk = krr_trainer.train(_training_data_evac_x, _training_data_evac_y);

        // do 10-fold cross-validation
        dlib::matrix<double> result = dlib::cross_validate_regression_trainer(krr_trainer, _training_data_evac_x, _training_data_evac_y, 10);



        // signal output
        std::chrono::nanoseconds comptime = std::chrono::system_clock::now() - start_time;

        info = "Finished training KRR Trainer with Radial Basis Kernel for evacuations.";
        info += "\n    Best value for gamma (kernel) = ";
        info += QString::number(rbk_gamma);
        info += "\n    Best value for lambda (regularization) = ";
        info += QString::number(krr_lambda);
        info += "\n    Mean squared error = ";
        info += QString::number(result(0));
        info += "\n    Pearson correlation = ";
        info += QString::number(result(1));
        info += "\n    Mean absolute error = ";
        info += QString::number(result(2));
        info += "\n    Standard deviation absolute error = ";
        info += QString::number(result(3));
        info += "\nElapsed time: ";
        info += QString::number(comptime.count() / NANO);
        info += " seconds";

        global::_logger << global::logger::log_type::INFORMATION << info;
        emit(signal_status(info));
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void machine_learning_interface::train_krr_histogram_intersection_evacuations()
    {
        // computation time and status information
        std::chrono::system_clock::time_point start_time = std::chrono::system_clock::now();
        QString info = "\nStart training KRR Trainer with Histogram Intersection Kernel for evacuations ...";
        emit(signal_status(info));


        // Randomize the samples
        dlib::randomize_samples(_training_data_evac_x, _training_data_evac_y);

        // Here we define a function, cross_validation_score(),
        // that will do the cross-validation we mentioned and
        // return a number indicating how good a particular setting
        // of lambda (regularization) is.
        auto cross_validation_score = [&](const double lambda) -> double
        {
            // define krls object with corresponding parameters
            dlib::krr_trainer<histogram_intersection_kernel> krr_trainer;

            // set the kernel and its parameter c
            krr_trainer.set_kernel(histogram_intersection_kernel());
            krr_trainer.set_lambda(lambda);  // regularization parameter

            // perform 10-fold cross validation and return the results.
            dlib::matrix<double> result = dlib::cross_validate_regression_trainer(krr_trainer, _training_data_evac_x, _training_data_evac_y, 10);

            return result(0);   // result(0) = MSE (mean squared error)
                                // result(1) = correlation between y' and y
                                // result(2) = MAE (mean absolute error)
                                // result(3) = the standard deviation of the absolute error
        };

        // We call this global optimizer that will search for the best parameters.
        // It will call cross_validation_score() 50 times with different settings and return
        // the best parameter setting it finds.
        auto best_parameters = dlib::find_min_global(cross_validation_score,
                                            {_krr_lambda_min},       // lower bound constraint on lambda
                                            {_krr_lambda_max},       // upper bound constraint on lambda
                                            dlib::max_function_calls(50),
                                            max_runtime_findminglobal);

        // Define svr object with best found parameters
        double krr_lambda = best_parameters.x(0);
        dlib::krr_trainer<histogram_intersection_kernel> krr_trainer;
        krr_trainer.set_kernel(histogram_intersection_kernel());
        krr_trainer.set_lambda(krr_lambda);

        // Now we train on the full set of data and get the resulting decision function
        _decision_functions_evacuations.dec_func_hik = krr_trainer.train(_training_data_evac_x, _training_data_evac_y);

        // do 10-fold cross-validation
        dlib::matrix<double> result = dlib::cross_validate_regression_trainer(krr_trainer, _training_data_evac_x, _training_data_evac_y, 10);



        // signal output
        std::chrono::nanoseconds comptime = std::chrono::system_clock::now() - start_time;

        info = "Finished training KRR Trainer with Histogram Intersection Kernel for evacuations.";
        info += "\n    Best value for lambda (regularization) = ";
        info += QString::number(krr_lambda);
        info += "\n    Mean squared error = ";
        info += QString::number(result(0));
        info += "\n    Pearson correlation = ";
        info += QString::number(result(1));
        info += "\n    Mean absolute error = ";
        info += QString::number(result(2));
        info += "\n    Standard deviation absolute error = ";
        info += QString::number(result(3));
        info += "\nElapsed time: ";
        info += QString::number(comptime.count() / NANO);
        info += " seconds";

        global::_logger << global::logger::log_type::INFORMATION << info;
        emit(signal_status(info));
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void machine_learning_interface::train_krr_quadratic_evacuations()
    {
        // parameters kernel
        constexpr int degree = 2;

        // computation time and status information
        std::chrono::system_clock::time_point start_time = std::chrono::system_clock::now();
        QString info = "\nStart training KRR Trainer with Polynomial Kernel (Quadratic) for evacuations ...";
        emit(signal_status(info));

        // Randomize the samples
        dlib::randomize_samples(_training_data_evac_x, _training_data_evac_y);

        // Here we define a function, cross_validation_score(),
        // that will do the cross-validation we mentioned and
        // return a number indicating how good a particular setting
        // of gamma (kernel), coef (kernel), and lambda (regularization) is.
        auto cross_validation_score = [&](const double gamma, const double coef, const double lambda) -> double
        {
            // define krls object with corresponding parameters
            dlib::krr_trainer<polynomial_kernel> krr_trainer;

            // set the kernel and its parameters gamma and coef and the regularization parameter lambda
            krr_trainer.set_kernel(polynomial_kernel(gamma, coef, degree));      // degree == 2
            krr_trainer.set_lambda(lambda);

            // perform 10-fold cross validation and return the results.
            dlib::matrix<double> result = dlib::cross_validate_regression_trainer(krr_trainer, _training_data_evac_x, _training_data_evac_y, 10);

            return result(0);   // result(0) = MSE (mean squared error)
                                // result(1) = correlation between y' and y
                                // result(2) = MAE (mean absolute error)
                                // result(3) = the standard deviation of the absolute error
        };

        // We call this global optimizer that will search for the best parameters.
        // It will call cross_validation_score() 50 times with different settings and return
        // the best parameter setting it finds.
        auto best_parameters = dlib::find_min_global(cross_validation_score,
                                            {_pk_gamma_min, _pk_coef_min, _krr_lambda_min},        // lower bound constraint on gamma, coef, and lambda
                                            {_pk_gamma_max, _pk_coef_max, _krr_lambda_max},        // upper bound constraint on gamma, coef, and lambda
                                            dlib::max_function_calls(50),
                                            max_runtime_findminglobal);

        // Define krr object with best found parameters
        double pk_gamma = best_parameters.x(0);
        double pk_coef = best_parameters.x(1);
        double krr_lambda = best_parameters.x(2);
        dlib::krr_trainer<polynomial_kernel> krr_trainer;
        krr_trainer.set_kernel(polynomial_kernel(pk_gamma, pk_coef, degree));
        krr_trainer.set_lambda(krr_lambda);

        // Now we train on the full set of data and get the resulting decision function
        _decision_functions_evacuations.dec_func_pk = krr_trainer.train(_training_data_evac_x, _training_data_evac_y);

        // do 10-fold cross-validation
        dlib::matrix<double> result = dlib::cross_validate_regression_trainer(krr_trainer, _training_data_evac_x, _training_data_evac_y, 10);



        // signal output
        std::chrono::nanoseconds comptime = std::chrono::system_clock::now() - start_time;

        info = "Finished training KRR Trainer with Polynomial Kernel (Quadratic) for evacuations.";
        info += "\n    Best value for gamma (kernel) = ";
        info += QString::number(pk_gamma);
        info += "\n    Best value for coef (kernel) = ";
        info += QString::number(pk_coef);
        info += "\n    Best value for lambda (regularization) = ";
        info += QString::number(krr_lambda);
        info += "\n    Mean squared error = ";
        info += QString::number(result(0));
        info += "\n    Pearson correlation = ";
        info += QString::number(result(1));
        info += "\n    Mean absolute error = ";
        info += QString::number(result(2));
        info += "\n    Standard deviation absolute error = ";
        info += QString::number(result(3));
        info += "\nElapsed time: ";
        info += QString::number(comptime.count() / NANO);
        info += " seconds";

        global::_logger << global::logger::log_type::INFORMATION << info;
        emit(signal_status(info));
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void machine_learning_interface::train_krr_cubic_evacuations()
    {
        // parameters kernel
        constexpr int degree = 3;

        // computation time and status information
        std::chrono::system_clock::time_point start_time = std::chrono::system_clock::now();
        QString info = "\nStart training KRR Trainer with Polynomial Kernel (Cubic) for evacuations ...";
        emit(signal_status(info));

        // Randomize the samples
        dlib::randomize_samples(_training_data_evac_x, _training_data_evac_y);

        // Here we define a function, cross_validation_score(),
        // that will do the cross-validation we mentioned and
        // return a number indicating how good a particular setting
        // of gamma (kernel), coef (kernel), and lambda (regularization) is.
        auto cross_validation_score = [&](const double gamma, const double coef, const double lambda) -> double
        {
            // define krls object with corresponding parameters
            dlib::krr_trainer<polynomial_kernel> krr_trainer;

            // set the kernel and its parameters gamma and coef and the regularization parameter lambda
            krr_trainer.set_kernel(polynomial_kernel(gamma, coef, degree));      // degree == 3
            krr_trainer.set_lambda(lambda);

            // perform 10-fold cross validation and return the results.
            dlib::matrix<double> result = dlib::cross_validate_regression_trainer(krr_trainer, _training_data_evac_x, _training_data_evac_y, 10);

            return result(0);   // result(0) = MSE (mean squared error)
                                // result(1) = correlation between y' and y
                                // result(2) = MAE (mean absolute error)
                                // result(3) = the standard deviation of the absolute error
        };

        // We call this global optimizer that will search for the best parameters.
        // It will call cross_validation_score() 50 times with different settings and return
        // the best parameter setting it finds.
        auto best_parameters = dlib::find_min_global(cross_validation_score,
                                            {_pk_gamma_min, _pk_coef_min, _krr_lambda_min},        // lower bound constraint on gamma, coef, and lambda
                                            {_pk_gamma_max, _pk_coef_max, _krr_lambda_max},        // upper bound constraint on gamma, coef, and lambda
                                            dlib::max_function_calls(50),
                                            max_runtime_findminglobal);

        // Define krr object with best found parameters
        double pk_gamma = best_parameters.x(0);
        double pk_coef = best_parameters.x(1);
        double krr_lambda = best_parameters.x(2);
        dlib::krr_trainer<polynomial_kernel> krr_trainer;
        krr_trainer.set_kernel(polynomial_kernel(pk_gamma, pk_coef, degree));
        krr_trainer.set_lambda(krr_lambda);

        // Now we train on the full set of data and get the resulting decision function
        _decision_functions_evacuations.dec_func_pk = krr_trainer.train(_training_data_evac_x, _training_data_evac_y);

        // do 10-fold cross-validation
        dlib::matrix<double> result = dlib::cross_validate_regression_trainer(krr_trainer, _training_data_evac_x, _training_data_evac_y, 10);



        // signal output
        std::chrono::nanoseconds comptime = std::chrono::system_clock::now() - start_time;

        info = "Finished training KRR Trainer with Polynomial Kernel (Cubic) for evacuations.";
        info += "\n    Best value for gamma (kernel) = ";
        info += QString::number(pk_gamma);
        info += "\n    Best value for coef (kernel) = ";
        info += QString::number(pk_coef);
        info += "\n    Best value for lambda (regularization) = ";
        info += QString::number(krr_lambda);
        info += "\n    Mean squared error = ";
        info += QString::number(result(0));
        info += "\n    Pearson correlation = ";
        info += QString::number(result(1));
        info += "\n    Mean absolute error = ";
        info += QString::number(result(2));
        info += "\n    Standard deviation absolute error = ";
        info += QString::number(result(3));
        info += "\nElapsed time: ";
        info += QString::number(comptime.count() / NANO);
        info += " seconds";

        global::_logger << global::logger::log_type::INFORMATION << info;
        emit(signal_status(info));
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void machine_learning_interface::train_krr_linear_evacuations()
    {
        std::chrono::system_clock::time_point start_time = std::chrono::system_clock::now();
        QString info = "\nStart training KRR Trainer with Linear Kernel for evacuations ...";
        emit(signal_status(info));


        // Randomize the samples
        dlib::randomize_samples(_training_data_evac_x, _training_data_evac_y);

        // Here we define a function, cross_validation_score(),
        // that will do the cross-validation we mentioned and
        // return a number indicating how good a particular setting
        // of lambda (regularization) is.
        auto cross_validation_score = [&](const double lambda) -> double
        {
            // define krls object with corresponding parameters
            dlib::krr_trainer<linear_kernel> krr_trainer;

            // set the kernel and its parameter gamma
            krr_trainer.set_kernel(linear_kernel());
            krr_trainer.set_lambda(lambda);

            // perform 10-fold cross validation and return the results.
            dlib::matrix<double> result = dlib::cross_validate_regression_trainer(krr_trainer, _training_data_evac_x, _training_data_evac_y, 10);

            return result(0);   // result(0) = MSE (mean squared error)
                                // result(1) = correlation between y' and y
                                // result(2) = MAE (mean absolute error)
                                // result(3) = the standard deviation of the absolute error
        };

        // We call this global optimizer that will search for the best parameters.
        // It will call cross_validation_score() 50 times with different settings and return
        // the best parameter setting it finds.
        auto best_parameters = dlib::find_min_global(cross_validation_score,
                                            {_krr_lambda_min},    // lower bound constraint on lambda
                                            {_krr_lambda_max},    // upper bound constraint on lambda
                                            dlib::max_function_calls(50),
                                            max_runtime_findminglobal);

        // Define krr object with best found parameters
        double krr_lambda = best_parameters.x(0);
        dlib::krr_trainer<linear_kernel> krr_trainer;
        krr_trainer.set_kernel(linear_kernel());
        krr_trainer.set_lambda(krr_lambda);

        // Now we train on the full set of data and get the resulting decision function
        _decision_functions_evacuations.dec_func_lk = krr_trainer.train(_training_data_evac_x, _training_data_evac_y);

        // do 10-fold cross-validation
        dlib::matrix<double> result = dlib::cross_validate_regression_trainer(krr_trainer, _training_data_evac_x, _training_data_evac_y, 10);



        // signal output
        std::chrono::nanoseconds comptime = std::chrono::system_clock::now() - start_time;

        info = "Finished training KRR Trainer with Linear Kernel for evacuations.";
        info += "\n    Best value for lambda (regularization) = ";
        info += QString::number(krr_lambda);
        info += "\n    Mean squared error = ";
        info += QString::number(result(0));
        info += "\n    Pearson correlation = ";
        info += QString::number(result(1));
        info += "\n    Mean absolute error = ";
        info += QString::number(result(2));
        info += "\n    Standard deviation absolute error = ";
        info += QString::number(result(3));
        info += "\nElapsed time: ";
        info += QString::number(comptime.count() / NANO);
        info += " seconds";

        global::_logger << global::logger::log_type::INFORMATION << info;
        emit(signal_status(info));
    }




    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void machine_learning_interface::train_svr_radial_basis_evacuations()
    {
        std::chrono::system_clock::time_point start_time = std::chrono::system_clock::now();
        QString info = "\nStart training SVR Trainer with Radial Basis Kernel for evacuations ...";
        emit(signal_status(info));


        // Randomize the samples
        dlib::randomize_samples(_training_data_evac_x, _training_data_evac_y);

        // Here we define a function, cross_validation_score(),
        // that will do the cross-validation we mentioned and
        // return a number indicating how good a particular setting
        // of gamma and c (regularization) is.
        auto cross_validation_score = [&](const double gamma, const double c) -> double
        {
            // define krls object with corresponding parameters
            dlib::svr_trainer<radial_basis_kernel> svr_trainer;

            // set the kernel and its parameter c
            svr_trainer.set_kernel(radial_basis_kernel(gamma));
            svr_trainer.set_c(c);  // regularization parameter: higher values = exact fitting; smaller values = higher generalization
            svr_trainer.set_epsilon_insensitivity(0.1);    // stop fitting data point once it is "close enough"

            // perform 10-fold cross validation and return the results.
            dlib::matrix<double> result = dlib::cross_validate_regression_trainer(svr_trainer, _training_data_evac_x, _training_data_evac_y, 10);

            return result(0);   // result(0) = MSE (mean squared error)
                                // result(1) = correlation between y' and y
                                // result(2) = MAE (mean absolute error)
                                // result(3) = the standard deviation of the absolute error
        };

        // We call this global optimizer that will search for the best parameters.
        // It will call cross_validation_score() 50 times with different settings and return
        // the best parameter setting it finds.
        auto best_parameters = dlib::find_min_global(cross_validation_score,
                                            {_rbk_gamma_min, _svr_c_min},      // lower bound constraint on gamma and c
                                            {_rbk_gamma_max, _svr_c_max},      // upper bound constraint on gamma and c
                                            dlib::max_function_calls(50),
                                            max_runtime_findminglobal);

        // Define svr object with best found parameters
        double rbk_gamma = best_parameters.x(0);
        double svr_c = best_parameters.x(1);
        dlib::svr_trainer<radial_basis_kernel> svr_trainer;
        svr_trainer.set_kernel(radial_basis_kernel(rbk_gamma));
        svr_trainer.set_c(svr_c);

        // Now we train on the full set of data and get the resulting decision function
        _decision_functions_evacuations.dec_func_rbk = svr_trainer.train(_training_data_evac_x, _training_data_evac_y);

        // do 10-fold cross-validation
        dlib::matrix<double> result = dlib::cross_validate_regression_trainer(svr_trainer, _training_data_evac_x, _training_data_evac_y, 10);



        // signal output
        std::chrono::nanoseconds comptime = std::chrono::system_clock::now() - start_time;

        info = "Finished training SVR Trainer with Radial Basis Kernel for evacuations.";
        info += "\n    Best value for gamma (kernel) = ";
        info += QString::number(rbk_gamma);
        info += "\n    Best value for c (regularization) = ";
        info += QString::number(svr_c);
        info += "\n    Mean squared error = ";
        info += QString::number(result(0));
        info += "\n    Pearson correlation = ";
        info += QString::number(result(1));
        info += "\n    Mean absolute error = ";
        info += QString::number(result(2));
        info += "\n    Standard deviation absolute error = ";
        info += QString::number(result(3));
        info += "\nElapsed time: ";
        info += QString::number(comptime.count() / NANO);
        info += " seconds";

        global::_logger << global::logger::log_type::INFORMATION << info;
        emit(signal_status(info));
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void machine_learning_interface::train_svr_histogram_intersection_evacuations()
    {
        std::chrono::system_clock::time_point start_time = std::chrono::system_clock::now();
        QString info = "\nStart training SVR Trainer with Histogram Intersection Kernel for evacuations ...";
        emit(signal_status(info));


        // Randomize the samples
        dlib::randomize_samples(_training_data_evac_x, _training_data_evac_y);

        // Here we define a function, cross_validation_score(),
        // that will do the cross-validation we mentioned and
        // return a number indicating how good a particular setting
        // of c (regularization) is.
        auto cross_validation_score = [&](const double c) -> double
        {
            // define krls object with corresponding parameters
            dlib::svr_trainer<histogram_intersection_kernel> svr_trainer;

            // set the kernel and its parameter c
            svr_trainer.set_kernel(histogram_intersection_kernel());
            svr_trainer.set_c(c);  // regularization parameter: higher values = exact fitting; smaller values = higher generalization
            svr_trainer.set_epsilon_insensitivity(0.1);    // stop fitting data point once it is "close enough"

            // perform 10-fold cross validation and return the results.
            dlib::matrix<double> result = dlib::cross_validate_regression_trainer(svr_trainer, _training_data_evac_x, _training_data_evac_y, 10);

            return result(0);   // result(0) = MSE (mean squared error)
                                // result(1) = correlation between y' and y
                                // result(2) = MAE (mean absolute error)
                                // result(3) = the standard deviation of the absolute error
        };

        // We call this global optimizer that will search for the best parameters.
        // It will call cross_validation_score() 50 times with different settings and return
        // the best parameter setting it finds.
        auto best_parameters = dlib::find_min_global(cross_validation_score,
                                            {_svr_c_min},      // lower bound constraint on c
                                            {_svr_c_max},      // upper bound constraint on c
                                            dlib::max_function_calls(50),
                                            max_runtime_findminglobal);

        // Define svr object with best found parameters
        double svr_c = best_parameters.x(0);
        dlib::svr_trainer<histogram_intersection_kernel> svr_trainer;
        svr_trainer.set_kernel(histogram_intersection_kernel());
        svr_trainer.set_c(svr_c);

        // Now we train on the full set of data and get the resulting decision function
        _decision_functions_evacuations.dec_func_hik = svr_trainer.train(_training_data_evac_x, _training_data_evac_y);

        // do 10-fold cross-validation
        dlib::matrix<double> result = dlib::cross_validate_regression_trainer(svr_trainer, _training_data_evac_x, _training_data_evac_y, 10);



        // signal output
        std::chrono::nanoseconds comptime = std::chrono::system_clock::now() - start_time;

        info = "Finished training SVR Trainer with Histogram Intersection Kernel for evacuations.";
        info += "\n    Best value for c (regularization) = ";
        info += QString::number(svr_c);
        info += "\n    Mean squared error = ";
        info += QString::number(result(0));
        info += "\n    Pearson correlation = ";
        info += QString::number(result(1));
        info += "\n    Mean absolute error = ";
        info += QString::number(result(2));
        info += "\n    Standard deviation absolute error = ";
        info += QString::number(result(3));
        info += "\nElapsed time: ";
        info += QString::number(comptime.count() / NANO);
        info += " seconds";

        global::_logger << global::logger::log_type::INFORMATION << info;
        emit(signal_status(info));
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void machine_learning_interface::train_svr_quadratic_evacuations()
    {
        // parameters kernel
        constexpr int degree = 2;

        // computation time and status information
        std::chrono::system_clock::time_point start_time = std::chrono::system_clock::now();
        QString info = "\nStart training SVR Trainer with Polynomial Kernel (Quadratic) for evacuations ...";
        emit(signal_status(info));

        // Randomize the samples
        dlib::randomize_samples(_training_data_evac_x, _training_data_evac_y);

        // Here we define a function, cross_validation_score(),
        // that will do the cross-validation we mentioned and
        // return a number indicating how good a particular setting
        // of gamma (kernel), coef (kernel), and c (regularization) is.
        auto cross_validation_score = [&](const double gamma, const double coef, const double c) -> double
        {
            // define krls object with corresponding parameters
            dlib::svr_trainer<polynomial_kernel> svr_trainer;

            // set the kernel and its parameters gamma and coef and the regularization parameter lambda
            svr_trainer.set_kernel(polynomial_kernel(gamma, coef, degree));      // degree == 2
            svr_trainer.set_c(c);

            // perform 10-fold cross validation and return the results.
            dlib::matrix<double> result = dlib::cross_validate_regression_trainer(svr_trainer, _training_data_evac_x, _training_data_evac_y, 10);

            return result(0);   // result(0) = MSE (mean squared error)
                                // result(1) = correlation between y' and y
                                // result(2) = MAE (mean absolute error)
                                // result(3) = the standard deviation of the absolute error
        };

        // We call this global optimizer that will search for the best parameters.
        // It will call cross_validation_score() 50 times with different settings and return
        // the best parameter setting it finds.
        auto best_parameters = dlib::find_min_global(cross_validation_score,
                                            {_pk_gamma_min, _pk_coef_min, _svr_c_min},        // lower bound constraint on gamma, coef, and lambda
                                            {_pk_gamma_max, _pk_coef_max, _svr_c_max},        // upper bound constraint on gamma, coef, and lambda
                                            dlib::max_function_calls(50),
                                            max_runtime_findminglobal);

        // Define krr object with best found parameters
        double pk_gamma = best_parameters.x(0);
        double pk_coef = best_parameters.x(1);
        double svr_c = best_parameters.x(2);
        dlib::svr_trainer<polynomial_kernel> svr_trainer;
        svr_trainer.set_kernel(polynomial_kernel(pk_gamma, pk_coef, degree));
        svr_trainer.set_c(svr_c);

        // Now we train on the full set of data and get the resulting decision function
        _decision_functions_evacuations.dec_func_pk = svr_trainer.train(_training_data_evac_x, _training_data_evac_y);

        // do 10-fold cross-validation
        dlib::matrix<double> result = dlib::cross_validate_regression_trainer(svr_trainer, _training_data_evac_x, _training_data_evac_y, 10);



        // signal output
        std::chrono::nanoseconds comptime = std::chrono::system_clock::now() - start_time;

        info = "Finished training SVR Trainer with Polynomial Kernel (Quadratic) for evacuations.";
        info += "\n    Best value for gamma (kernel) = ";
        info += QString::number(pk_gamma);
        info += "\n    Best value for coef (kernel) = ";
        info += QString::number(pk_coef);
        info += "\n    Best value for c (regularization) = ";
        info += QString::number(svr_c);
        info += "\n    Mean squared error = ";
        info += QString::number(result(0));
        info += "\n    Pearson correlation = ";
        info += QString::number(result(1));
        info += "\n    Mean absolute error = ";
        info += QString::number(result(2));
        info += "\n    Standard deviation absolute error = ";
        info += QString::number(result(3));
        info += "\nElapsed time: ";
        info += QString::number(comptime.count() / NANO);
        info += " seconds";

        global::_logger << global::logger::log_type::INFORMATION << info;
        emit(signal_status(info));
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void machine_learning_interface::train_svr_cubic_evacuations()
    {
        // parameters kernel
        constexpr int degree = 3;

        // computation time and status information
        std::chrono::system_clock::time_point start_time = std::chrono::system_clock::now();
        QString info = "\nStart training SVR Trainer with Polynomial Kernel (Quadratic) for evacuations ...";
        emit(signal_status(info));

        // Randomize the samples
        dlib::randomize_samples(_training_data_evac_x, _training_data_evac_y);

        // Here we define a function, cross_validation_score(),
        // that will do the cross-validation we mentioned and
        // return a number indicating how good a particular setting
        // of gamma (kernel), coef (kernel), and c (regularization) is.
        auto cross_validation_score = [&](const double gamma, const double coef, const double c) -> double
        {
            // define krls object with corresponding parameters
            dlib::svr_trainer<polynomial_kernel> svr_trainer;

            // set the kernel and its parameters gamma and coef and the regularization parameter lambda
            svr_trainer.set_kernel(polynomial_kernel(gamma, coef, degree));      // degree == 2
            svr_trainer.set_c(c);

            // perform 10-fold cross validation and return the results.
            dlib::matrix<double> result = dlib::cross_validate_regression_trainer(svr_trainer, _training_data_evac_x, _training_data_evac_y, 10);

            return result(0);   // result(0) = MSE (mean squared error)
                                // result(1) = correlation between y' and y
                                // result(2) = MAE (mean absolute error)
                                // result(3) = the standard deviation of the absolute error
        };

        // We call this global optimizer that will search for the best parameters.
        // It will call cross_validation_score() 50 times with different settings and return
        // the best parameter setting it finds.
        auto best_parameters = dlib::find_min_global(cross_validation_score,
                                            {_pk_gamma_min, _pk_coef_min, _svr_c_min},        // lower bound constraint on gamma, coef, and lambda
                                            {_pk_gamma_max, _pk_coef_max, _svr_c_max},        // upper bound constraint on gamma, coef, and lambda
                                            dlib::max_function_calls(50),
                                            max_runtime_findminglobal);

        // Define krr object with best found parameters
        double pk_gamma = best_parameters.x(0);
        double pk_coef = best_parameters.x(1);
        double svr_c = best_parameters.x(2);
        dlib::svr_trainer<polynomial_kernel> svr_trainer;
        svr_trainer.set_kernel(polynomial_kernel(pk_gamma, pk_coef, degree));
        svr_trainer.set_c(svr_c);

        // Now we train on the full set of data and get the resulting decision function
        _decision_functions_evacuations.dec_func_pk = svr_trainer.train(_training_data_evac_x, _training_data_evac_y);

        // do 10-fold cross-validation
        dlib::matrix<double> result = dlib::cross_validate_regression_trainer(svr_trainer, _training_data_evac_x, _training_data_evac_y, 10);



        // signal output
        std::chrono::nanoseconds comptime = std::chrono::system_clock::now() - start_time;

        info = "Finished training SVR Trainer with Polynomial Kernel (Quadratic) for evacuations.";
        info += "\n    Best value for gamma (kernel) = ";
        info += QString::number(pk_gamma);
        info += "\n    Best value for coef (kernel) = ";
        info += QString::number(pk_coef);
        info += "\n    Best value for c (regularization) = ";
        info += QString::number(svr_c);
        info += "\n    Mean squared error = ";
        info += QString::number(result(0));
        info += "\n    Pearson correlation = ";
        info += QString::number(result(1));
        info += "\n    Mean absolute error = ";
        info += QString::number(result(2));
        info += "\n    Standard deviation absolute error = ";
        info += QString::number(result(3));
        info += "\nElapsed time: ";
        info += QString::number(comptime.count() / NANO);
        info += " seconds";

        global::_logger << global::logger::log_type::INFORMATION << info;
        emit(signal_status(info));
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void machine_learning_interface::train_svr_linear_evacuations()
    {
        std::chrono::system_clock::time_point start_time = std::chrono::system_clock::now();
        QString info = "\nStart training SVR Trainer with Linear Kernel for evacuations ...";
        emit(signal_status(info));


        // Randomize the samples
        dlib::randomize_samples(_training_data_evac_x, _training_data_evac_y);

        // Here we define a function, cross_validation_score(),
        // that will do the cross-validation we mentioned and
        // return a number indicating how good a particular setting
        // of c (regularization) is.
        auto cross_validation_score = [&](const double c) -> double
        {
            // define krls object with corresponding parameters
            dlib::svr_trainer<linear_kernel> svr_linear_trainer;

            // set its parameter c
            svr_linear_trainer.set_kernel(linear_kernel());
            svr_linear_trainer.set_c(c);  // regularization parameter: higher values = exact fitting; smaller values = higher generalization
            svr_linear_trainer.set_epsilon_insensitivity(0.1);    // stop fitting data point once it is "close enough"

            // perform 10-fold cross validation and return the results.
            dlib::matrix<double> result = dlib::cross_validate_regression_trainer(svr_linear_trainer, _training_data_evac_x, _training_data_evac_y, 10);

            return result(0);   // result(0) = MSE (mean squared error)
                                // result(1) = correlation between y' and y
                                // result(2) = MAE (mean absolute error)
                                // result(3) = the standard deviation of the absolute error
        };

        // We call this global optimizer that will search for the best parameters.
        // It will call cross_validation_score() 50 times with different settings and return
        // the best parameter setting it finds.
        auto best_parameters = dlib::find_min_global(cross_validation_score,
                                            {_svr_c_min},      // lower bound constraint on c
                                            {_svr_c_max},      // upper bound constraint on c
                                            dlib::max_function_calls(50),
                                            max_runtime_findminglobal);

        // Define svr object with best found parameters
        double svr_c = best_parameters.x(0);
        dlib::svr_trainer<linear_kernel> svr_linear_trainer;
        svr_linear_trainer.set_kernel(linear_kernel());
        svr_linear_trainer.set_c(svr_c);

        // Now we train on the full set of data and get the resulting decision function
        _decision_functions_evacuations.dec_func_lk = svr_linear_trainer.train(_training_data_evac_x, _training_data_evac_y);

        // do 10-fold cross-validation
        dlib::matrix<double> result = dlib::cross_validate_regression_trainer(svr_linear_trainer, _training_data_evac_x, _training_data_evac_y, 10);



        // signal output
        std::chrono::nanoseconds comptime = std::chrono::system_clock::now() - start_time;

        info = "Finished training SVR Trainer with Linear Kernel for evacuations.";
        info += "\n    Best value for c (regularization) = ";
        info += QString::number(svr_c);
        info += "\n    Mean squared error = ";
        info += QString::number(result(0));
        info += "\n    Pearson correlation = ";
        info += QString::number(result(1));
        info += "\n    Mean absolute error = ";
        info += QString::number(result(2));
        info += "\n    Standard deviation absolute error = ";
        info += QString::number(result(3));
        info += "\nElapsed time: ";
        info += QString::number(comptime.count() / NANO);
        info += " seconds";

        global::_logger << global::logger::log_type::INFORMATION << info;
        emit(signal_status(info));
    }




    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void machine_learning_interface::train_rvm_radial_basis_evacuations()
    {
        // computation time and status information
        std::chrono::system_clock::time_point start_time = std::chrono::system_clock::now();
        QString info = "\nStart training RVM Trainer with Radial Basis Kernel for evacuations ...";
        emit(signal_status(info));


        // Randomize the samples
        dlib::randomize_samples(_training_data_evac_x, _training_data_evac_y);

        // Here we define a function, cross_validation_score(),
        // that will do the cross-validation we mentioned and
        // return a number indicating how good a particular setting
        // of gamma (kernel) is.
        auto cross_validation_score = [&](const double gamma) -> double
        {
            // define krls object with corresponding parameters
            dlib::rvm_trainer<radial_basis_kernel> rvm_trainer;

            // set the kernel and its parameter gamma
            rvm_trainer.set_kernel(radial_basis_kernel(gamma));

            // perform 10-fold cross validation and return the results.
            dlib::matrix<double> result = dlib::cross_validate_regression_trainer(rvm_trainer, _training_data_evac_x, _training_data_evac_y, 10);

            // We return a number indicating how good the parameters are.  Bigger is
            // better in this example.  We're returning the harmonic mean between the
            // accuracies of each class.
            //return 2*dlib::prod(result)/dlib::sum(result);

            return result(0);   // result(0) = MSE (mean squared error)
                                // result(1) = correlation between y' and y
                                // result(2) = MAE (mean absolute error)
                                // result(3) = the standard deviation of the absolute error
        };

        // We call this global optimizer that will search for the best parameters.
        // It will call cross_validation_score() 50 times with different settings and return
        // the best parameter setting it finds.
        auto best_parameters = dlib::find_min_global(cross_validation_score,
                                                     {_rbk_gamma_min},    // lower bound constraint on gamma
                                                     {_rbk_gamma_max},    // upper bound constraint on gamma
                                                     dlib::max_function_calls(50),
                                                     max_runtime_findminglobal);

        // Define krr object with best found parameters
        double rbk_gamma = best_parameters.x(0);
        dlib::rvm_trainer<radial_basis_kernel> rvm_trainer;
        rvm_trainer.set_kernel(radial_basis_kernel(rbk_gamma));

        // Now we train on the full set of data and get the resulting decision function
        _decision_functions_evacuations.dec_func_rbk = rvm_trainer.train(_training_data_evac_x, _training_data_evac_y);

        // do 10-fold cross-validation
        dlib::matrix<double> result = dlib::cross_validate_regression_trainer(rvm_trainer, _training_data_evac_x, _training_data_evac_y, 10);



        // signal output
        std::chrono::nanoseconds comptime = std::chrono::system_clock::now() - start_time;

        info = "Finished training RVM Trainer with Radial Basis Kernel for evacuations.";
        info += "\n    Best value for gamma (kernel) = ";
        info += QString::number(rbk_gamma);
        info += "\n    Mean squared error = ";
        info += QString::number(result(0));
        info += "\n    Pearson correlation = ";
        info += QString::number(result(1));
        info += "\n    Mean absolute error = ";
        info += QString::number(result(2));
        info += "\n    Standard deviation absolute error = ";
        info += QString::number(result(3));
        info += "\nElapsed time: ";
        info += QString::number(comptime.count() / NANO);
        info += " seconds";

        global::_logger << global::logger::log_type::INFORMATION << info;
        emit(signal_status(info));
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void machine_learning_interface::train_rvm_histogram_intersection_evacuations()
    {
        // computation time and status information
        std::chrono::system_clock::time_point start_time = std::chrono::system_clock::now();
        QString info = "\nStart training RVM Trainer with Histogram Intersection Kernel for evacuations ...";
        emit(signal_status(info));


        // Randomize the samples
        dlib::randomize_samples(_training_data_evac_x, _training_data_evac_y);

        // Define svr object (no parameters)
        dlib::rvm_trainer<histogram_intersection_kernel> rvm_trainer;
        rvm_trainer.set_kernel(histogram_intersection_kernel());

        // Now we train on the full set of data and get the resulting decision function
        _decision_functions_evacuations.dec_func_hik = rvm_trainer.train(_training_data_evac_x, _training_data_evac_y);

        // do 10-fold cross-validation
        dlib::matrix<double> result = dlib::cross_validate_regression_trainer(rvm_trainer, _training_data_evac_x, _training_data_evac_y, 10);



        // signal output
        std::chrono::nanoseconds comptime = std::chrono::system_clock::now() - start_time;

        info = "Finished training RVM Trainer with Histogram Intersection Kernel for evacuations.";
        info += "\n    Mean squared error = ";
        info += QString::number(result(0));
        info += "\n    Pearson correlation = ";
        info += QString::number(result(1));
        info += "\n    Mean absolute error = ";
        info += QString::number(result(2));
        info += "\n    Standard deviation absolute error = ";
        info += QString::number(result(3));
        info += "\nElapsed time: ";
        info += QString::number(comptime.count() / NANO);
        info += " seconds";

        global::_logger << global::logger::log_type::INFORMATION << info;
        emit(signal_status(info));
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void machine_learning_interface::train_rvm_linear_evacuations()
    {
        std::chrono::system_clock::time_point start_time = std::chrono::system_clock::now();
        QString info = "\nStart training RVM Trainer with Linear Kernel for evacuations ...";
        emit(signal_status(info));


        // Randomize the samples
        dlib::randomize_samples(_training_data_evac_x, _training_data_evac_y);

        // Define rvm object (no parameters)
        dlib::rvm_trainer<linear_kernel> rvm_trainer;
        rvm_trainer.set_kernel(linear_kernel());

        // Now we train on the full set of data and get the resulting decision function
        _decision_functions_evacuations.dec_func_lk = rvm_trainer.train(_training_data_evac_x, _training_data_evac_y);

        // do 10-fold cross-validation
        dlib::matrix<double> result = dlib::cross_validate_regression_trainer(rvm_trainer, _training_data_evac_x, _training_data_evac_y, 10);


        // signal output
        std::chrono::nanoseconds comptime = std::chrono::system_clock::now() - start_time;

        info = "Finished training RVM Trainer with Linear Kernel for evacuations.";
        info += "\n    Mean squared error = ";
        info += QString::number(result(0));
        info += "\n    Pearson correlation = ";
        info += QString::number(result(1));
        info += "\n    Mean absolute error = ";
        info += QString::number(result(2));
        info += "\n    Standard deviation absolute error = ";
        info += QString::number(result(3));
        info += "\nElapsed time: ";
        info += QString::number(comptime.count() / NANO);
        info += " seconds";

        global::_logger << global::logger::log_type::INFORMATION << info;
        emit(signal_status(info));
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void machine_learning_interface::train_rvm_quadratic_evacuations()
    {
        // parameters kernel
        constexpr int degree = 2;

        // computation time and status information
        std::chrono::system_clock::time_point start_time = std::chrono::system_clock::now();
        QString info = "\nStart training RVM Trainer with Polynomial Kernel (Quadratic) for evacuations ...";
        emit(signal_status(info));

        // Randomize the samples
        dlib::randomize_samples(_training_data_evac_x, _training_data_evac_y);

        // Here we define a function, cross_validation_score(),
        // that will do the cross-validation we mentioned and
        // return a number indicating how good a particular setting
        // of gamma (kernel) and coef (kernel) is.
        auto cross_validation_score = [&](const double gamma, const double coef) -> double
        {
            // define krls object with corresponding parameters
            dlib::rvm_trainer<polynomial_kernel> rvm_trainer;

            // set the kernel and its parameters gamma and coef
            rvm_trainer.set_kernel(polynomial_kernel(gamma, coef, degree));      // degree == 2

            // perform 10-fold cross validation and return the results.
            dlib::matrix<double> result = dlib::cross_validate_regression_trainer(rvm_trainer, _training_data_evac_x, _training_data_evac_y, 10);

            return result(0);   // result(0) = MSE (mean squared error)
                                // result(1) = correlation between y' and y
                                // result(2) = MAE (mean absolute error)
                                // result(3) = the standard deviation of the absolute error
        };

        // We call this global optimizer that will search for the best parameters.
        // It will call cross_validation_score() 50 times with different settings and return
        // the best parameter setting it finds.
        auto best_parameters = dlib::find_min_global(cross_validation_score,
                                            {_pk_gamma_min, _pk_coef_min},        // lower bound constraint on gamma and coef
                                            {_pk_gamma_max, _pk_coef_max},        // upper bound constraint on gamma and coef
                                            dlib::max_function_calls(50),
                                            max_runtime_findminglobal);

        // Define krr object with best found parameters
        double pk_gamma = best_parameters.x(0);
        double pk_coef = best_parameters.x(1);
        dlib::rvm_trainer<polynomial_kernel> rvm_trainer;
        rvm_trainer.set_kernel(polynomial_kernel(pk_gamma, pk_coef, degree));

        // Now we train on the full set of data and get the resulting decision function
        _decision_functions_evacuations.dec_func_pk = rvm_trainer.train(_training_data_evac_x, _training_data_evac_y);

        // do 10-fold cross-validation
        dlib::matrix<double> result = dlib::cross_validate_regression_trainer(rvm_trainer, _training_data_evac_x, _training_data_evac_y, 10);



        // signal output
        std::chrono::nanoseconds comptime = std::chrono::system_clock::now() - start_time;

        info = "Finished training RVM Trainer with Polynomial Kernel (Quadratic) for evacuations.";
        info += "\n    Best value for gamma (kernel) = ";
        info += QString::number(pk_gamma);
        info += "\n    Best value for coef (kernel) = ";
        info += QString::number(pk_coef);
        info += "\n    Mean squared error = ";
        info += QString::number(result(0));
        info += "\n    Pearson correlation = ";
        info += QString::number(result(1));
        info += "\n    Mean absolute error = ";
        info += QString::number(result(2));
        info += "\n    Standard deviation absolute error = ";
        info += QString::number(result(3));
        info += "\nElapsed time: ";
        info += QString::number(comptime.count() / NANO);
        info += " seconds";

        global::_logger << global::logger::log_type::INFORMATION << info;
        emit(signal_status(info));
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void machine_learning_interface::train_rvm_cubic_evacuations()
    {
        // parameters kernel
        constexpr int degree = 3;

        // computation time and status information
        std::chrono::system_clock::time_point start_time = std::chrono::system_clock::now();
        QString info = "\nStart training RVM Trainer with Polynomial Kernel (Cubic) for evacuations ...";
        emit(signal_status(info));

        // Randomize the samples
        dlib::randomize_samples(_training_data_evac_x, _training_data_evac_y);

        // Here we define a function, cross_validation_score(),
        // that will do the cross-validation we mentioned and
        // return a number indicating how good a particular setting
        // of gamma (kernel) and coef (kernel), is.
        auto cross_validation_score = [&](const double gamma, const double coef) -> double
        {
            // define krls object with corresponding parameters
            dlib::rvm_trainer<polynomial_kernel> rvm_trainer;

            // set the kernel and its parameters gamma and coef
            rvm_trainer.set_kernel(polynomial_kernel(gamma, coef, degree));      // degree == 3

            // perform 10-fold cross validation and return the results.
            dlib::matrix<double> result = dlib::cross_validate_regression_trainer(rvm_trainer, _training_data_evac_x, _training_data_evac_y, 10);

            return result(0);   // result(0) = MSE (mean squared error)
                                // result(1) = correlation between y' and y
                                // result(2) = MAE (mean absolute error)
                                // result(3) = the standard deviation of the absolute error
        };

        // We call this global optimizer that will search for the best parameters.
        // It will call cross_validation_score() 50 times with different settings and return
        // the best parameter setting it finds.
        auto best_parameters = dlib::find_min_global(cross_validation_score,
                                            {_pk_gamma_min, _pk_coef_min},        // lower bound constraint on gamma and coef
                                            {_pk_gamma_max, _pk_coef_max},        // upper bound constraint on gamma and coef
                                            dlib::max_function_calls(50),
                                            max_runtime_findminglobal);

        // Define krr object with best found parameters
        double pk_gamma = best_parameters.x(0);
        double pk_coef = best_parameters.x(1);
        dlib::rvm_trainer<polynomial_kernel> rvm_trainer;
        rvm_trainer.set_kernel(polynomial_kernel(pk_gamma, pk_coef, degree));

        // Now we train on the full set of data and get the resulting decision function
        _decision_functions_evacuations.dec_func_pk = rvm_trainer.train(_training_data_evac_x, _training_data_evac_y);

        // do 10-fold cross-validation
        dlib::matrix<double> result = dlib::cross_validate_regression_trainer(rvm_trainer, _training_data_evac_x, _training_data_evac_y, 10);



        // signal output
        std::chrono::nanoseconds comptime = std::chrono::system_clock::now() - start_time;

        info = "Finished training RVM Trainer with Polynomial Kernel (Cubic) for evacuations.";
        info += "\n    Best value for gamma (kernel) = ";
        info += QString::number(pk_gamma);
        info += "\n    Best value for coef (kernel) = ";
        info += QString::number(pk_coef);
        info += "\n    Mean squared error = ";
        info += QString::number(result(0));
        info += "\n    Pearson correlation = ";
        info += QString::number(result(1));
        info += "\n    Mean absolute error = ";
        info += QString::number(result(2));
        info += "\n    Standard deviation absolute error = ";
        info += QString::number(result(3));
        info += "\nElapsed time: ";
        info += QString::number(comptime.count() / NANO);
        info += " seconds";

        global::_logger << global::logger::log_type::INFORMATION << info;
        emit(signal_status(info));
    }




    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void machine_learning_interface::train_krr_radial_basis_travels()
    {
        // computation time and status information
        std::chrono::system_clock::time_point start_time = std::chrono::system_clock::now();
        QString info = "\nStart training KRR Trainer with Radial Basis Kernel for travels ...";
        emit(signal_status(info));


        // Randomize the samples
        dlib::randomize_samples(_training_data_travels_x, _training_data_travels_y);

        // Here we define a function, cross_validation_score(),
        // that will do the cross-validation we mentioned and
        // return a number indicating how good a particular setting
        // of gamma (kernel) and lambda (regularization) is.
        auto cross_validation_score = [&](const double gamma, const double lambda) -> double
        {
            // define krls object with corresponding parameters
            dlib::krr_trainer<radial_basis_kernel> krr_trainer;

            // set the kernel and its parameter gamma
            krr_trainer.set_kernel(radial_basis_kernel(gamma));
            krr_trainer.set_lambda(lambda);

            // perform 10-fold cross validation and return the results.
            dlib::matrix<double> result = dlib::cross_validate_regression_trainer(krr_trainer, _training_data_travels_x, _training_data_travels_y, 10);

            // We return a number indicating how good the parameters are.  Bigger is
            // better in this example.  We're returning the harmonic mean between the
            // accuracies of each class.
            //return 2*dlib::prod(result)/dlib::sum(result);

            return result(0);   // result(0) = MSE (mean squared error)
                                // result(1) = correlation between y' and y
                                // result(2) = MAE (mean absolute error)
                                // result(3) = the standard deviation of the absolute error
        };

        // We call this global optimizer that will search for the best parameters.
        // It will call cross_validation_score() 50 times with different settings and return
        // the best parameter setting it finds.
        auto best_parameters = dlib::find_min_global(cross_validation_score,
                                                     {_rbk_gamma_min, _krr_lambda_min},    // lower bound constraint on gamma and lambda
                                                     {_rbk_gamma_max, _krr_lambda_max},    // upper bound constraint on gamma and lambda
                                                     dlib::max_function_calls(50),
                                                     max_runtime_findminglobal);

        //qDebug() << "best param found";


        // Define krr object with best found parameters
        double rbk_gamma = best_parameters.x(0);
        double krr_lambda = best_parameters.x(1);
        dlib::krr_trainer<radial_basis_kernel> krr_trainer;
        krr_trainer.set_kernel(radial_basis_kernel(rbk_gamma));
        krr_trainer.set_lambda(krr_lambda);

        // Now we train on the full set of data and get the resulting decision function
        _decision_functions_travels.dec_func_rbk = krr_trainer.train(_training_data_travels_x, _training_data_travels_y);

        // do 10-fold cross-validation
        dlib::matrix<double> result = dlib::cross_validate_regression_trainer(krr_trainer, _training_data_travels_x, _training_data_travels_y, 10);



        // signal output
        std::chrono::nanoseconds comptime = std::chrono::system_clock::now() - start_time;

        info = "Finished training KRR Trainer with Radial Basis Kernel for travels.";
        info += "\n    Best value for gamma (kernel) = ";
        info += QString::number(rbk_gamma);
        info += "\n    Best value for lambda (regularization) = ";
        info += QString::number(krr_lambda);
        info += "\n    Mean squared error = ";
        info += QString::number(result(0));
        info += "\n    Pearson correlation = ";
        info += QString::number(result(1));
        info += "\n    Mean absolute error = ";
        info += QString::number(result(2));
        info += "\n    Standard deviation absolute error = ";
        info += QString::number(result(3));
        info += "\nElapsed time: ";
        info += QString::number(comptime.count() / NANO);
        info += " seconds";

        global::_logger << global::logger::log_type::INFORMATION << info;
        emit(signal_status(info));
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void machine_learning_interface::train_krr_histogram_intersection_travels()
    {
        // computation time and status information
        std::chrono::system_clock::time_point start_time = std::chrono::system_clock::now();
        QString info = "\nStart training KRR Trainer with Histogram Intersection Kernel for travels ...";
        emit(signal_status(info));


        // Randomize the samples
        dlib::randomize_samples(_training_data_travels_x, _training_data_travels_y);

        // Here we define a function, cross_validation_score(),
        // that will do the cross-validation we mentioned and
        // return a number indicating how good a particular setting
        // of lambda (regularization) is.
        auto cross_validation_score = [&](const double lambda) -> double
        {
            // define krls object with corresponding parameters
            dlib::krr_trainer<histogram_intersection_kernel> krr_trainer;

            // set the kernel and its parameter c
            krr_trainer.set_kernel(histogram_intersection_kernel());
            krr_trainer.set_lambda(lambda);  // regularization parameter

            // perform 10-fold cross validation and return the results.
            dlib::matrix<double> result = dlib::cross_validate_regression_trainer(krr_trainer, _training_data_travels_x, _training_data_travels_y, 10);

            return result(0);   // result(0) = MSE (mean squared error)
                                // result(1) = correlation between y' and y
                                // result(2) = MAE (mean absolute error)
                                // result(3) = the standard deviation of the absolute error
        };

        // We call this global optimizer that will search for the best parameters.
        // It will call cross_validation_score() 50 times with different settings and return
        // the best parameter setting it finds.
        auto best_parameters = dlib::find_min_global(cross_validation_score,
                                            {_krr_lambda_min},       // lower bound constraint on lambda
                                            {_krr_lambda_max},       // upper bound constraint on lambda
                                            dlib::max_function_calls(50),
                                            max_runtime_findminglobal);

        // Define svr object with best found parameters
        double krr_lambda = best_parameters.x(0);
        dlib::krr_trainer<histogram_intersection_kernel> krr_trainer;
        krr_trainer.set_kernel(histogram_intersection_kernel());
        krr_trainer.set_lambda(krr_lambda);

        // Now we train on the full set of data and get the resulting decision function
        _decision_functions_travels.dec_func_hik = krr_trainer.train(_training_data_travels_x, _training_data_travels_y);

        // do 10-fold cross-validation
        dlib::matrix<double> result = dlib::cross_validate_regression_trainer(krr_trainer, _training_data_travels_x, _training_data_travels_y, 10);



        // signal output
        std::chrono::nanoseconds comptime = std::chrono::system_clock::now() - start_time;

        info = "Finished training KRR Trainer with Histogram Intersection Kernel for travels.";
        info += "\n    Best value for lambda (regularization) = ";
        info += QString::number(krr_lambda);
        info += "\n    Mean squared error = ";
        info += QString::number(result(0));
        info += "\n    Pearson correlation = ";
        info += QString::number(result(1));
        info += "\n    Mean absolute error = ";
        info += QString::number(result(2));
        info += "\n    Standard deviation absolute error = ";
        info += QString::number(result(3));
        info += "\nElapsed time: ";
        info += QString::number(comptime.count() / NANO);
        info += " seconds";

        global::_logger << global::logger::log_type::INFORMATION << info;
        emit(signal_status(info));
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void machine_learning_interface::train_krr_quadratic_travels()
    {
        // parameters kernel
        constexpr int degree = 2;

        // computation time and status information
        std::chrono::system_clock::time_point start_time = std::chrono::system_clock::now();
        QString info = "\nStart training KRR Trainer with Polynomial Kernel (Quadratic) for travels ...";
        emit(signal_status(info));

        // Randomize the samples
        dlib::randomize_samples(_training_data_travels_x, _training_data_travels_y);

        // Here we define a function, cross_validation_score(),
        // that will do the cross-validation we mentioned and
        // return a number indicating how good a particular setting
        // of gamma (kernel), coef (kernel), and lambda (regularization) is.
        auto cross_validation_score = [&](const double gamma, const double coef, const double lambda) -> double
        {
            // define krls object with corresponding parameters
            dlib::krr_trainer<polynomial_kernel> krr_trainer;

            // set the kernel and its parameters gamma and coef and the regularization parameter lambda
            krr_trainer.set_kernel(polynomial_kernel(gamma, coef, degree));      // degree == 2
            krr_trainer.set_lambda(lambda);

            // perform 10-fold cross validation and return the results.
            dlib::matrix<double> result = dlib::cross_validate_regression_trainer(krr_trainer, _training_data_travels_x, _training_data_travels_y, 10);

            return result(0);   // result(0) = MSE (mean squared error)
                                // result(1) = correlation between y' and y
                                // result(2) = MAE (mean absolute error)
                                // result(3) = the standard deviation of the absolute error
        };

        // We call this global optimizer that will search for the best parameters.
        // It will call cross_validation_score() 50 times with different settings and return
        // the best parameter setting it finds.
        auto best_parameters = dlib::find_min_global(cross_validation_score,
                                            {_pk_gamma_min, _pk_coef_min, _krr_lambda_min},        // lower bound constraint on gamma, coef, and lambda
                                            {_pk_gamma_max, _pk_coef_max, _krr_lambda_max},        // upper bound constraint on gamma, coef, and lambda
                                            dlib::max_function_calls(50),
                                            max_runtime_findminglobal);

        // Define krr object with best found parameters
        double pk_gamma = best_parameters.x(0);
        double pk_coef = best_parameters.x(1);
        double krr_lambda = best_parameters.x(2);
        dlib::krr_trainer<polynomial_kernel> krr_trainer;
        krr_trainer.set_kernel(polynomial_kernel(pk_gamma, pk_coef, degree));
        krr_trainer.set_lambda(krr_lambda);

        // Now we train on the full set of data and get the resulting decision function
        _decision_functions_travels.dec_func_pk = krr_trainer.train(_training_data_travels_x, _training_data_travels_y);

        // do 10-fold cross-validation
        dlib::matrix<double> result = dlib::cross_validate_regression_trainer(krr_trainer, _training_data_travels_x, _training_data_travels_y, 10);



        // signal output
        std::chrono::nanoseconds comptime = std::chrono::system_clock::now() - start_time;

        info = "Finished training KRR Trainer with Polynomial Kernel (Quadratic) for travels.";
        info += "\n    Best value for gamma (kernel) = ";
        info += QString::number(pk_gamma);
        info += "\n    Best value for coef (kernel) = ";
        info += QString::number(pk_coef);
        info += "\n    Best value for lambda (regularization) = ";
        info += QString::number(krr_lambda);
        info += "\n    Mean squared error = ";
        info += QString::number(result(0));
        info += "\n    Pearson correlation = ";
        info += QString::number(result(1));
        info += "\n    Mean absolute error = ";
        info += QString::number(result(2));
        info += "\n    Standard deviation absolute error = ";
        info += QString::number(result(3));
        info += "\nElapsed time: ";
        info += QString::number(comptime.count() / NANO);
        info += " seconds";

        global::_logger << global::logger::log_type::INFORMATION << info;
        emit(signal_status(info));
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void machine_learning_interface::train_krr_cubic_travels()
    {
        // parameters kernel
        constexpr int degree = 3;

        // computation time and status information
        std::chrono::system_clock::time_point start_time = std::chrono::system_clock::now();
        QString info = "\nStart training KRR Trainer with Polynomial Kernel (Cubic) for travels ...";
        emit(signal_status(info));

        // Randomize the samples
        dlib::randomize_samples(_training_data_travels_x, _training_data_travels_y);

        // Here we define a function, cross_validation_score(),
        // that will do the cross-validation we mentioned and
        // return a number indicating how good a particular setting
        // of gamma (kernel), coef (kernel), and lambda (regularization) is.
        auto cross_validation_score = [&](const double gamma, const double coef, const double lambda) -> double
        {
            // define krls object with corresponding parameters
            dlib::krr_trainer<polynomial_kernel> krr_trainer;

            // set the kernel and its parameters gamma and coef and the regularization parameter lambda
            krr_trainer.set_kernel(polynomial_kernel(gamma, coef, degree));      // degree == 3
            krr_trainer.set_lambda(lambda);

            // perform 10-fold cross validation and return the results.
            dlib::matrix<double> result = dlib::cross_validate_regression_trainer(krr_trainer, _training_data_travels_x, _training_data_travels_y, 10);

            return result(0);   // result(0) = MSE (mean squared error)
                                // result(1) = correlation between y' and y
                                // result(2) = MAE (mean absolute error)
                                // result(3) = the standard deviation of the absolute error
        };

        // We call this global optimizer that will search for the best parameters.
        // It will call cross_validation_score() 50 times with different settings and return
        // the best parameter setting it finds.
        auto best_parameters = dlib::find_min_global(cross_validation_score,
                                            {_pk_gamma_min, _pk_coef_min, _krr_lambda_min},        // lower bound constraint on gamma, coef, and lambda
                                            {_pk_gamma_max, _pk_coef_max, _krr_lambda_max},        // upper bound constraint on gamma, coef, and lambda
                                            dlib::max_function_calls(50),
                                            max_runtime_findminglobal);

        // Define krr object with best found parameters
        double pk_gamma = best_parameters.x(0);
        double pk_coef = best_parameters.x(1);
        double krr_lambda = best_parameters.x(2);
        dlib::krr_trainer<polynomial_kernel> krr_trainer;
        krr_trainer.set_kernel(polynomial_kernel(pk_gamma, pk_coef, degree));
        krr_trainer.set_lambda(krr_lambda);

        // Now we train on the full set of data and get the resulting decision function
        _decision_functions_travels.dec_func_pk = krr_trainer.train(_training_data_travels_x, _training_data_travels_y);

        // do 10-fold cross-validation
        dlib::matrix<double> result = dlib::cross_validate_regression_trainer(krr_trainer, _training_data_travels_x, _training_data_travels_y, 10);



        // signal output
        std::chrono::nanoseconds comptime = std::chrono::system_clock::now() - start_time;

        info = "Finished training KRR Trainer with Polynomial Kernel (Cubic) for travels.";
        info += "\n    Best value for gamma (kernel) = ";
        info += QString::number(pk_gamma);
        info += "\n    Best value for coef (kernel) = ";
        info += QString::number(pk_coef);
        info += "\n    Best value for lambda (regularization) = ";
        info += QString::number(krr_lambda);
        info += "\n    Mean squared error = ";
        info += QString::number(result(0));
        info += "\n    Pearson correlation = ";
        info += QString::number(result(1));
        info += "\n    Mean absolute error = ";
        info += QString::number(result(2));
        info += "\n    Standard deviation absolute error = ";
        info += QString::number(result(3));
        info += "\nElapsed time: ";
        info += QString::number(comptime.count() / NANO);
        info += " seconds";

        global::_logger << global::logger::log_type::INFORMATION << info;
        emit(signal_status(info));
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void machine_learning_interface::train_krr_linear_travels()
    {
        std::chrono::system_clock::time_point start_time = std::chrono::system_clock::now();
        QString info = "\nStart training KRR Trainer with Linear Kernel for travels ...";
        emit(signal_status(info));


        // Randomize the samples
        dlib::randomize_samples(_training_data_travels_x, _training_data_travels_y);

        // Here we define a function, cross_validation_score(),
        // that will do the cross-validation we mentioned and
        // return a number indicating how good a particular setting
        // of lambda (regularization) is.
        auto cross_validation_score = [&](const double lambda) -> double
        {
            // define krls object with corresponding parameters
            dlib::krr_trainer<linear_kernel> krr_trainer;

            // set the kernel and its parameter gamma
            krr_trainer.set_kernel(linear_kernel());
            krr_trainer.set_lambda(lambda);

            // perform 10-fold cross validation and return the results.
            dlib::matrix<double> result = dlib::cross_validate_regression_trainer(krr_trainer, _training_data_travels_x, _training_data_travels_y, 10);

            return result(0);   // result(0) = MSE (mean squared error)
                                // result(1) = correlation between y' and y
                                // result(2) = MAE (mean absolute error)
                                // result(3) = the standard deviation of the absolute error
        };

        // We call this global optimizer that will search for the best parameters.
        // It will call cross_validation_score() 50 times with different settings and return
        // the best parameter setting it finds.
        auto best_parameters = dlib::find_min_global(cross_validation_score,
                                            {_krr_lambda_min},    // lower bound constraint on lambda
                                            {_krr_lambda_max},    // upper bound constraint on lambda
                                            dlib::max_function_calls(50),
                                            max_runtime_findminglobal);

        // Define krr object with best found parameters
        double krr_lambda = best_parameters.x(0);
        dlib::krr_trainer<linear_kernel> krr_trainer;
        krr_trainer.set_kernel(linear_kernel());
        krr_trainer.set_lambda(krr_lambda);

        // Now we train on the full set of data and get the resulting decision function
        _decision_functions_travels.dec_func_lk = krr_trainer.train(_training_data_travels_x, _training_data_travels_y);

        // do 10-fold cross-validation
        dlib::matrix<double> result = dlib::cross_validate_regression_trainer(krr_trainer, _training_data_travels_x, _training_data_travels_y, 10);



        // signal output
        std::chrono::nanoseconds comptime = std::chrono::system_clock::now() - start_time;

        info = "Finished training KRR Trainer with Linear Kernel for travels.";
        info += "\n    Best value for lambda (regularization) = ";
        info += QString::number(krr_lambda);
        info += "\n    Mean squared error = ";
        info += QString::number(result(0));
        info += "\n    Pearson correlation = ";
        info += QString::number(result(1));
        info += "\n    Mean absolute error = ";
        info += QString::number(result(2));
        info += "\n    Standard deviation absolute error = ";
        info += QString::number(result(3));
        info += "\nElapsed time: ";
        info += QString::number(comptime.count() / NANO);
        info += " seconds";

        global::_logger << global::logger::log_type::INFORMATION << info;
        emit(signal_status(info));
    }




    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void machine_learning_interface::train_svr_radial_basis_travels()
    {
        std::chrono::system_clock::time_point start_time = std::chrono::system_clock::now();
        QString info = "\nStart training SVR Trainer with Radial Basis Kernel for travels ...";
        emit(signal_status(info));


        // Randomize the samples
        dlib::randomize_samples(_training_data_travels_x, _training_data_travels_y);

        // Here we define a function, cross_validation_score(),
        // that will do the cross-validation we mentioned and
        // return a number indicating how good a particular setting
        // of gamma and c (regularization) is.
        auto cross_validation_score = [&](const double gamma, const double c) -> double
        {
            // define krls object with corresponding parameters
            dlib::svr_trainer<radial_basis_kernel> svr_trainer;

            // set the kernel and its parameter c
            svr_trainer.set_kernel(radial_basis_kernel(gamma));
            svr_trainer.set_c(c);  // regularization parameter: higher values = exact fitting; smaller values = higher generalization
            svr_trainer.set_epsilon_insensitivity(0.1);    // stop fitting data point once it is "close enough"

            // perform 10-fold cross validation and return the results.
            dlib::matrix<double> result = dlib::cross_validate_regression_trainer(svr_trainer, _training_data_travels_x, _training_data_travels_y, 10);

            return result(0);   // result(0) = MSE (mean squared error)
                                // result(1) = correlation between y' and y
                                // result(2) = MAE (mean absolute error)
                                // result(3) = the standard deviation of the absolute error
        };

        // We call this global optimizer that will search for the best parameters.
        // It will call cross_validation_score() 50 times with different settings and return
        // the best parameter setting it finds.
        auto best_parameters = dlib::find_min_global(cross_validation_score,
                                            {_rbk_gamma_min, _svr_c_min},      // lower bound constraint on gamma and c
                                            {_rbk_gamma_max, _svr_c_max},      // upper bound constraint on gamma and c
                                            dlib::max_function_calls(50),
                                            max_runtime_findminglobal);

        // Define svr object with best found parameters
        double rbk_gamma = best_parameters.x(0);
        double svr_c = best_parameters.x(1);
        dlib::svr_trainer<radial_basis_kernel> svr_trainer;
        svr_trainer.set_kernel(radial_basis_kernel(rbk_gamma));
        svr_trainer.set_c(svr_c);

        // Now we train on the full set of data and get the resulting decision function
        _decision_functions_travels.dec_func_rbk = svr_trainer.train(_training_data_travels_x, _training_data_travels_y);

        // do 10-fold cross-validation
        dlib::matrix<double> result = dlib::cross_validate_regression_trainer(svr_trainer, _training_data_travels_x, _training_data_travels_y, 10);



        // signal output
        std::chrono::nanoseconds comptime = std::chrono::system_clock::now() - start_time;

        info = "Finished training SVR Trainer with Radial Basis Kernel for travels.";
        info += "\n    Best value for gamma (kernel) = ";
        info += QString::number(rbk_gamma);
        info += "\n    Best value for c (regularization) = ";
        info += QString::number(svr_c);
        info += "\n    Mean squared error = ";
        info += QString::number(result(0));
        info += "\n    Pearson correlation = ";
        info += QString::number(result(1));
        info += "\n    Mean absolute error = ";
        info += QString::number(result(2));
        info += "\n    Standard deviation absolute error = ";
        info += QString::number(result(3));
        info += "\nElapsed time: ";
        info += QString::number(comptime.count() / NANO);
        info += " seconds";

        global::_logger << global::logger::log_type::INFORMATION << info;
        emit(signal_status(info));
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void machine_learning_interface::train_svr_histogram_intersection_travels()
    {
        std::chrono::system_clock::time_point start_time = std::chrono::system_clock::now();
        QString info = "\nStart training SVR Trainer with Histogram Intersection Kernel for travels ...";
        emit(signal_status(info));


        // Randomize the samples
        dlib::randomize_samples(_training_data_travels_x, _training_data_travels_y);

        // Here we define a function, cross_validation_score(),
        // that will do the cross-validation we mentioned and
        // return a number indicating how good a particular setting
        // of c (regularization) is.
        auto cross_validation_score = [&](const double c) -> double
        {
            // define krls object with corresponding parameters
            dlib::svr_trainer<histogram_intersection_kernel> svr_trainer;

            // set the kernel and its parameter c
            svr_trainer.set_kernel(histogram_intersection_kernel());
            svr_trainer.set_c(c);  // regularization parameter: higher values = exact fitting; smaller values = higher generalization
            svr_trainer.set_epsilon_insensitivity(0.1);    // stop fitting data point once it is "close enough"

            // perform 10-fold cross validation and return the results.
            dlib::matrix<double> result = dlib::cross_validate_regression_trainer(svr_trainer, _training_data_travels_x, _training_data_travels_y, 10);

            return result(0);   // result(0) = MSE (mean squared error)
                                // result(1) = correlation between y' and y
                                // result(2) = MAE (mean absolute error)
                                // result(3) = the standard deviation of the absolute error
        };

        // We call this global optimizer that will search for the best parameters.
        // It will call cross_validation_score() 50 times with different settings and return
        // the best parameter setting it finds.
        auto best_parameters = dlib::find_min_global(cross_validation_score,
                                            {_svr_c_min},      // lower bound constraint on c
                                            {_svr_c_max},      // upper bound constraint on c
                                            dlib::max_function_calls(50),
                                            max_runtime_findminglobal);

        // Define svr object with best found parameters
        double svr_c = best_parameters.x(0);
        dlib::svr_trainer<histogram_intersection_kernel> svr_trainer;
        svr_trainer.set_kernel(histogram_intersection_kernel());
        svr_trainer.set_c(svr_c);

        // Now we train on the full set of data and get the resulting decision function
        _decision_functions_travels.dec_func_hik = svr_trainer.train(_training_data_travels_x, _training_data_travels_y);

        // do 10-fold cross-validation
        dlib::matrix<double> result = dlib::cross_validate_regression_trainer(svr_trainer, _training_data_travels_x, _training_data_travels_y, 10);



        // signal output
        std::chrono::nanoseconds comptime = std::chrono::system_clock::now() - start_time;

        info = "Finished training SVR Trainer with Histogram Intersection Kernel for travels.";
        info += "\n    Best value for c (regularization) = ";
        info += QString::number(svr_c);
        info += "\n    Mean squared error = ";
        info += QString::number(result(0));
        info += "\n    Pearson correlation = ";
        info += QString::number(result(1));
        info += "\n    Mean absolute error = ";
        info += QString::number(result(2));
        info += "\n    Standard deviation absolute error = ";
        info += QString::number(result(3));
        info += "\nElapsed time: ";
        info += QString::number(comptime.count() / NANO);
        info += " seconds";

        global::_logger << global::logger::log_type::INFORMATION << info;
        emit(signal_status(info));
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void machine_learning_interface::train_svr_quadratic_travels()
    {
        // parameters kernel
        constexpr int degree = 2;

        // computation time and status information
        std::chrono::system_clock::time_point start_time = std::chrono::system_clock::now();
        QString info = "\nStart training SVR Trainer with Polynomial Kernel (Quadratic) for travels ...";
        emit(signal_status(info));

        // Randomize the samples
        dlib::randomize_samples(_training_data_travels_x, _training_data_travels_y);

        // Here we define a function, cross_validation_score(),
        // that will do the cross-validation we mentioned and
        // return a number indicating how good a particular setting
        // of gamma (kernel), coef (kernel), and c (regularization) is.
        auto cross_validation_score = [&](const double gamma, const double coef, const double c) -> double
        {
            // define krls object with corresponding parameters
            dlib::svr_trainer<polynomial_kernel> svr_trainer;

            // set the kernel and its parameters gamma and coef and the regularization parameter lambda
            svr_trainer.set_kernel(polynomial_kernel(gamma, coef, degree));      // degree == 2
            svr_trainer.set_c(c);

            // perform 10-fold cross validation and return the results.
            dlib::matrix<double> result = dlib::cross_validate_regression_trainer(svr_trainer, _training_data_travels_x, _training_data_travels_y, 10);

            return result(0);   // result(0) = MSE (mean squared error)
                                // result(1) = correlation between y' and y
                                // result(2) = MAE (mean absolute error)
                                // result(3) = the standard deviation of the absolute error
        };

        // We call this global optimizer that will search for the best parameters.
        // It will call cross_validation_score() 50 times with different settings and return
        // the best parameter setting it finds.
        auto best_parameters = dlib::find_min_global(cross_validation_score,
                                            {_pk_gamma_min, _pk_coef_min, _svr_c_min},        // lower bound constraint on gamma, coef, and lambda
                                            {_pk_gamma_max, _pk_coef_max, _svr_c_max},        // upper bound constraint on gamma, coef, and lambda
                                            dlib::max_function_calls(50),
                                            max_runtime_findminglobal);

        // Define krr object with best found parameters
        double pk_gamma = best_parameters.x(0);
        double pk_coef = best_parameters.x(1);
        double svr_c = best_parameters.x(2);
        dlib::svr_trainer<polynomial_kernel> svr_trainer;
        svr_trainer.set_kernel(polynomial_kernel(pk_gamma, pk_coef, degree));
        svr_trainer.set_c(svr_c);

        // Now we train on the full set of data and get the resulting decision function
        _decision_functions_travels.dec_func_pk = svr_trainer.train(_training_data_travels_x, _training_data_travels_y);

        // do 10-fold cross-validation
        dlib::matrix<double> result = dlib::cross_validate_regression_trainer(svr_trainer, _training_data_travels_x, _training_data_travels_y, 10);



        // signal output
        std::chrono::nanoseconds comptime = std::chrono::system_clock::now() - start_time;

        info = "Finished training SVR Trainer with Polynomial Kernel (Quadratic) for travels.";
        info += "\n    Best value for gamma (kernel) = ";
        info += QString::number(pk_gamma);
        info += "\n    Best value for coef (kernel) = ";
        info += QString::number(pk_coef);
        info += "\n    Best value for c (regularization) = ";
        info += QString::number(svr_c);
        info += "\n    Mean squared error = ";
        info += QString::number(result(0));
        info += "\n    Pearson correlation = ";
        info += QString::number(result(1));
        info += "\n    Mean absolute error = ";
        info += QString::number(result(2));
        info += "\n    Standard deviation absolute error = ";
        info += QString::number(result(3));
        info += "\nElapsed time: ";
        info += QString::number(comptime.count() / NANO);
        info += " seconds";

        global::_logger << global::logger::log_type::INFORMATION << info;
        emit(signal_status(info));
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void machine_learning_interface::train_svr_cubic_travels()
    {
        // parameters kernel
        constexpr int degree = 3;

        // computation time and status information
        std::chrono::system_clock::time_point start_time = std::chrono::system_clock::now();
        QString info = "\nStart training SVR Trainer with Polynomial Kernel (Quadratic) for travels ...";
        emit(signal_status(info));

        // Randomize the samples
        dlib::randomize_samples(_training_data_travels_x, _training_data_travels_y);

        // Here we define a function, cross_validation_score(),
        // that will do the cross-validation we mentioned and
        // return a number indicating how good a particular setting
        // of gamma (kernel), coef (kernel), and c (regularization) is.
        auto cross_validation_score = [&](const double gamma, const double coef, const double c) -> double
        {
            // define krls object with corresponding parameters
            dlib::svr_trainer<polynomial_kernel> svr_trainer;

            // set the kernel and its parameters gamma and coef and the regularization parameter lambda
            svr_trainer.set_kernel(polynomial_kernel(gamma, coef, degree));      // degree == 2
            svr_trainer.set_c(c);

            // perform 10-fold cross validation and return the results.
            dlib::matrix<double> result = dlib::cross_validate_regression_trainer(svr_trainer, _training_data_travels_x, _training_data_travels_y, 10);

            return result(0);   // result(0) = MSE (mean squared error)
                                // result(1) = correlation between y' and y
                                // result(2) = MAE (mean absolute error)
                                // result(3) = the standard deviation of the absolute error
        };

        // We call this global optimizer that will search for the best parameters.
        // It will call cross_validation_score() 50 times with different settings and return
        // the best parameter setting it finds.
        auto best_parameters = dlib::find_min_global(cross_validation_score,
                                            {_pk_gamma_min, _pk_coef_min, _svr_c_min},        // lower bound constraint on gamma, coef, and lambda
                                            {_pk_gamma_max, _pk_coef_max, _svr_c_max},        // upper bound constraint on gamma, coef, and lambda
                                            dlib::max_function_calls(50),
                                            max_runtime_findminglobal);

        // Define krr object with best found parameters
        double pk_gamma = best_parameters.x(0);
        double pk_coef = best_parameters.x(1);
        double svr_c = best_parameters.x(2);
        dlib::svr_trainer<polynomial_kernel> svr_trainer;
        svr_trainer.set_kernel(polynomial_kernel(pk_gamma, pk_coef, degree));
        svr_trainer.set_c(svr_c);

        // Now we train on the full set of data and get the resulting decision function
        _decision_functions_travels.dec_func_pk = svr_trainer.train(_training_data_travels_x, _training_data_travels_y);

        // do 10-fold cross-validation
        dlib::matrix<double> result = dlib::cross_validate_regression_trainer(svr_trainer, _training_data_travels_x, _training_data_travels_y, 10);



        // signal output
        std::chrono::nanoseconds comptime = std::chrono::system_clock::now() - start_time;

        info = "Finished training SVR Trainer with Polynomial Kernel (Quadratic) for travels.";
        info += "\n    Best value for gamma (kernel) = ";
        info += QString::number(pk_gamma);
        info += "\n    Best value for coef (kernel) = ";
        info += QString::number(pk_coef);
        info += "\n    Best value for c (regularization) = ";
        info += QString::number(svr_c);
        info += "\n    Mean squared error = ";
        info += QString::number(result(0));
        info += "\n    Pearson correlation = ";
        info += QString::number(result(1));
        info += "\n    Mean absolute error = ";
        info += QString::number(result(2));
        info += "\n    Standard deviation absolute error = ";
        info += QString::number(result(3));
        info += "\nElapsed time: ";
        info += QString::number(comptime.count() / NANO);
        info += " seconds";

        global::_logger << global::logger::log_type::INFORMATION << info;
        emit(signal_status(info));
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void machine_learning_interface::train_svr_linear_travels()
    {
        std::chrono::system_clock::time_point start_time = std::chrono::system_clock::now();
        QString info = "\nStart training SVR Trainer with Linear Kernel for travels ...";
        emit(signal_status(info));


        // Randomize the samples
        dlib::randomize_samples(_training_data_travels_x, _training_data_travels_y);

        // Here we define a function, cross_validation_score(),
        // that will do the cross-validation we mentioned and
        // return a number indicating how good a particular setting
        // of c (regularization) is.
        auto cross_validation_score = [&](const double c) -> double
        {
            // define krls object with corresponding parameters
            dlib::svr_trainer<linear_kernel> svr_linear_trainer;

            // set its parameter c
            svr_linear_trainer.set_kernel(linear_kernel());
            svr_linear_trainer.set_c(c);  // regularization parameter: higher values = exact fitting; smaller values = higher generalization
            svr_linear_trainer.set_epsilon_insensitivity(0.1);    // stop fitting data point once it is "close enough"

            // perform 10-fold cross validation and return the results.
            dlib::matrix<double> result = dlib::cross_validate_regression_trainer(svr_linear_trainer, _training_data_travels_x, _training_data_travels_y, 10);

            return result(0);   // result(0) = MSE (mean squared error)
                                // result(1) = correlation between y' and y
                                // result(2) = MAE (mean absolute error)
                                // result(3) = the standard deviation of the absolute error
        };

        // We call this global optimizer that will search for the best parameters.
        // It will call cross_validation_score() 50 times with different settings and return
        // the best parameter setting it finds.
        auto best_parameters = dlib::find_min_global(cross_validation_score,
                                            {_svr_c_min},      // lower bound constraint on c
                                            {_svr_c_max},      // upper bound constraint on c
                                            dlib::max_function_calls(50),
                                            max_runtime_findminglobal);

        // Define svr object with best found parameters
        double svr_c = best_parameters.x(0);
        dlib::svr_trainer<linear_kernel> svr_linear_trainer;
        svr_linear_trainer.set_kernel(linear_kernel());
        svr_linear_trainer.set_c(svr_c);

        // Now we train on the full set of data and get the resulting decision function
        _decision_functions_travels.dec_func_lk = svr_linear_trainer.train(_training_data_travels_x, _training_data_travels_y);

        // do 10-fold cross-validation
        dlib::matrix<double> result = dlib::cross_validate_regression_trainer(svr_linear_trainer, _training_data_travels_x, _training_data_travels_y, 10);



        // signal output
        std::chrono::nanoseconds comptime = std::chrono::system_clock::now() - start_time;

        info = "Finished training SVR Trainer with Linear Kernel for travels.";
        info += "\n    Best value for c (regularization) = ";
        info += QString::number(svr_c);
        info += "\n    Mean squared error = ";
        info += QString::number(result(0));
        info += "\n    Pearson correlation = ";
        info += QString::number(result(1));
        info += "\n    Mean absolute error = ";
        info += QString::number(result(2));
        info += "\n    Standard deviation absolute error = ";
        info += QString::number(result(3));
        info += "\nElapsed time: ";
        info += QString::number(comptime.count() / NANO);
        info += " seconds";

        global::_logger << global::logger::log_type::INFORMATION << info;
        emit(signal_status(info));
    }




    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void machine_learning_interface::train_rvm_radial_basis_travels()
    {
        // computation time and status information
        std::chrono::system_clock::time_point start_time = std::chrono::system_clock::now();
        QString info = "\nStart training RVM Trainer with Radial Basis Kernel for travels ...";
        emit(signal_status(info));


        // Randomize the samples
        dlib::randomize_samples(_training_data_travels_x, _training_data_travels_y);

        // Here we define a function, cross_validation_score(),
        // that will do the cross-validation we mentioned and
        // return a number indicating how good a particular setting
        // of gamma (kernel) is.
        auto cross_validation_score = [&](const double gamma) -> double
        {
            // define krls object with corresponding parameters
            dlib::rvm_trainer<radial_basis_kernel> rvm_trainer;

            // set the kernel and its parameter gamma
            rvm_trainer.set_kernel(radial_basis_kernel(gamma));

            // perform 10-fold cross validation and return the results.
            dlib::matrix<double> result = dlib::cross_validate_regression_trainer(rvm_trainer, _training_data_travels_x, _training_data_travels_y, 10);

            // We return a number indicating how good the parameters are.  Bigger is
            // better in this example.  We're returning the harmonic mean between the
            // accuracies of each class.
            //return 2*dlib::prod(result)/dlib::sum(result);

            return result(0);   // result(0) = MSE (mean squared error)
                                // result(1) = correlation between y' and y
                                // result(2) = MAE (mean absolute error)
                                // result(3) = the standard deviation of the absolute error
        };

        // We call this global optimizer that will search for the best parameters.
        // It will call cross_validation_score() 50 times with different settings and return
        // the best parameter setting it finds.
        auto best_parameters = dlib::find_min_global(cross_validation_score,
                                                     {_rbk_gamma_min},    // lower bound constraint on gamma
                                                     {_rbk_gamma_max},    // upper bound constraint on gamma
                                                     dlib::max_function_calls(50),
                                                     max_runtime_findminglobal);

        // Define krr object with best found parameters
        double rbk_gamma = best_parameters.x(0);
        dlib::rvm_trainer<radial_basis_kernel> rvm_trainer;
        rvm_trainer.set_kernel(radial_basis_kernel(rbk_gamma));

        // Now we train on the full set of data and get the resulting decision function
        _decision_functions_travels.dec_func_rbk = rvm_trainer.train(_training_data_travels_x, _training_data_travels_y);

        // do 10-fold cross-validation
        dlib::matrix<double> result = dlib::cross_validate_regression_trainer(rvm_trainer, _training_data_travels_x, _training_data_travels_y, 10);



        // signal output
        std::chrono::nanoseconds comptime = std::chrono::system_clock::now() - start_time;

        info = "Finished training RVM Trainer with Radial Basis Kernel for travels.";
        info += "\n    Best value for gamma (kernel) = ";
        info += QString::number(rbk_gamma);
        info += "\n    Mean squared error = ";
        info += QString::number(result(0));
        info += "\n    Pearson correlation = ";
        info += QString::number(result(1));
        info += "\n    Mean absolute error = ";
        info += QString::number(result(2));
        info += "\n    Standard deviation absolute error = ";
        info += QString::number(result(3));
        info += "\nElapsed time: ";
        info += QString::number(comptime.count() / NANO);
        info += " seconds";

        global::_logger << global::logger::log_type::INFORMATION << info;
        emit(signal_status(info));
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void machine_learning_interface::train_rvm_histogram_intersection_travels()
    {
        // computation time and status information
        std::chrono::system_clock::time_point start_time = std::chrono::system_clock::now();
        QString info = "\nStart training RVM Trainer with Histogram Intersection Kernel for travels ...";
        emit(signal_status(info));


        // Randomize the samples
        dlib::randomize_samples(_training_data_travels_x, _training_data_travels_y);

        // Define svr object (no parameters)
        dlib::rvm_trainer<histogram_intersection_kernel> rvm_trainer;
        rvm_trainer.set_kernel(histogram_intersection_kernel());

        // Now we train on the full set of data and get the resulting decision function
        _decision_functions_travels.dec_func_hik = rvm_trainer.train(_training_data_travels_x, _training_data_travels_y);

        // do 10-fold cross-validation
        dlib::matrix<double> result = dlib::cross_validate_regression_trainer(rvm_trainer, _training_data_travels_x, _training_data_travels_y, 10);



        // signal output
        std::chrono::nanoseconds comptime = std::chrono::system_clock::now() - start_time;

        info = "Finished training RVM Trainer with Histogram Intersection Kernel for travels.";
        info += "\n    Mean squared error = ";
        info += QString::number(result(0));
        info += "\n    Pearson correlation = ";
        info += QString::number(result(1));
        info += "\n    Mean absolute error = ";
        info += QString::number(result(2));
        info += "\n    Standard deviation absolute error = ";
        info += QString::number(result(3));
        info += "\nElapsed time: ";
        info += QString::number(comptime.count() / NANO);
        info += " seconds";

        global::_logger << global::logger::log_type::INFORMATION << info;
        emit(signal_status(info));
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void machine_learning_interface::train_rvm_linear_travels()
    {
        std::chrono::system_clock::time_point start_time = std::chrono::system_clock::now();
        QString info = "\nStart training RVM Trainer with Linear Kernel for travels ...";
        emit(signal_status(info));


        // Randomize the samples
        dlib::randomize_samples(_training_data_travels_x, _training_data_travels_y);

        // Define rvm object (no parameters)
        dlib::rvm_trainer<linear_kernel> rvm_trainer;
        rvm_trainer.set_kernel(linear_kernel());

        // Now we train on the full set of data and get the resulting decision function
        _decision_functions_travels.dec_func_lk = rvm_trainer.train(_training_data_travels_x, _training_data_travels_y);

        // do 10-fold cross-validation
        dlib::matrix<double> result = dlib::cross_validate_regression_trainer(rvm_trainer, _training_data_travels_x, _training_data_travels_y, 10);


        // signal output
        std::chrono::nanoseconds comptime = std::chrono::system_clock::now() - start_time;

        info = "Finished training RVM Trainer with Linear Kernel for travels.";
        info += "\n    Mean squared error = ";
        info += QString::number(result(0));
        info += "\n    Pearson correlation = ";
        info += QString::number(result(1));
        info += "\n    Mean absolute error = ";
        info += QString::number(result(2));
        info += "\n    Standard deviation absolute error = ";
        info += QString::number(result(3));
        info += "\nElapsed time: ";
        info += QString::number(comptime.count() / NANO);
        info += " seconds";

        global::_logger << global::logger::log_type::INFORMATION << info;
        emit(signal_status(info));
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void machine_learning_interface::train_rvm_quadratic_travels()
    {
        // parameters kernel
        constexpr int degree = 2;

        // computation time and status information
        std::chrono::system_clock::time_point start_time = std::chrono::system_clock::now();
        QString info = "\nStart training RVM Trainer with Polynomial Kernel (Quadratic) for travels ...";
        emit(signal_status(info));

        // Randomize the samples
        dlib::randomize_samples(_training_data_travels_x, _training_data_travels_y);

        // Here we define a function, cross_validation_score(),
        // that will do the cross-validation we mentioned and
        // return a number indicating how good a particular setting
        // of gamma (kernel) and coef (kernel) is.
        auto cross_validation_score = [&](const double gamma, const double coef) -> double
        {
            // define krls object with corresponding parameters
            dlib::rvm_trainer<polynomial_kernel> rvm_trainer;

            // set the kernel and its parameters gamma and coef
            rvm_trainer.set_kernel(polynomial_kernel(gamma, coef, degree));      // degree == 2

            // perform 10-fold cross validation and return the results.
            dlib::matrix<double> result = dlib::cross_validate_regression_trainer(rvm_trainer, _training_data_travels_x, _training_data_travels_y, 10);

            return result(0);   // result(0) = MSE (mean squared error)
                                // result(1) = correlation between y' and y
                                // result(2) = MAE (mean absolute error)
                                // result(3) = the standard deviation of the absolute error
        };

        // We call this global optimizer that will search for the best parameters.
        // It will call cross_validation_score() 50 times with different settings and return
        // the best parameter setting it finds.
        auto best_parameters = dlib::find_min_global(cross_validation_score,
                                            {_pk_gamma_min, _pk_coef_min},        // lower bound constraint on gamma and coef
                                            {_pk_gamma_max, _pk_coef_max},        // upper bound constraint on gamma and coef
                                            dlib::max_function_calls(50),
                                            max_runtime_findminglobal);

        // Define krr object with best found parameters
        double pk_gamma = best_parameters.x(0);
        double pk_coef = best_parameters.x(1);
        dlib::rvm_trainer<polynomial_kernel> rvm_trainer;
        rvm_trainer.set_kernel(polynomial_kernel(pk_gamma, pk_coef, degree));

        // Now we train on the full set of data and get the resulting decision function
        _decision_functions_travels.dec_func_pk = rvm_trainer.train(_training_data_travels_x, _training_data_travels_y);

        // do 10-fold cross-validation
        dlib::matrix<double> result = dlib::cross_validate_regression_trainer(rvm_trainer, _training_data_travels_x, _training_data_travels_y, 10);



        // signal output
        std::chrono::nanoseconds comptime = std::chrono::system_clock::now() - start_time;

        info = "Finished training RVM Trainer with Polynomial Kernel (Quadratic) for travels.";
        info += "\n    Best value for gamma (kernel) = ";
        info += QString::number(pk_gamma);
        info += "\n    Best value for coef (kernel) = ";
        info += QString::number(pk_coef);
        info += "\n    Mean squared error = ";
        info += QString::number(result(0));
        info += "\n    Pearson correlation = ";
        info += QString::number(result(1));
        info += "\n    Mean absolute error = ";
        info += QString::number(result(2));
        info += "\n    Standard deviation absolute error = ";
        info += QString::number(result(3));
        info += "\nElapsed time: ";
        info += QString::number(comptime.count() / NANO);
        info += " seconds";

        global::_logger << global::logger::log_type::INFORMATION << info;
        emit(signal_status(info));
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void machine_learning_interface::train_rvm_cubic_travels()
    {
        // parameters kernel
        constexpr int degree = 3;

        // computation time and status information
        std::chrono::system_clock::time_point start_time = std::chrono::system_clock::now();
        QString info = "\nStart training RVM Trainer with Polynomial Kernel (Cubic) for travels ...";
        emit(signal_status(info));

        // Randomize the samples
        dlib::randomize_samples(_training_data_travels_x, _training_data_travels_y);

        // Here we define a function, cross_validation_score(),
        // that will do the cross-validation we mentioned and
        // return a number indicating how good a particular setting
        // of gamma (kernel) and coef (kernel), is.
        auto cross_validation_score = [&](const double gamma, const double coef) -> double
        {
            // define krls object with corresponding parameters
            dlib::rvm_trainer<polynomial_kernel> rvm_trainer;

            // set the kernel and its parameters gamma and coef
            rvm_trainer.set_kernel(polynomial_kernel(gamma, coef, degree));      // degree == 3

            // perform 10-fold cross validation and return the results.
            dlib::matrix<double> result = dlib::cross_validate_regression_trainer(rvm_trainer, _training_data_travels_x, _training_data_travels_y, 10);

            return result(0);   // result(0) = MSE (mean squared error)
                                // result(1) = correlation between y' and y
                                // result(2) = MAE (mean absolute error)
                                // result(3) = the standard deviation of the absolute error
        };

        // We call this global optimizer that will search for the best parameters.
        // It will call cross_validation_score() 50 times with different settings and return
        // the best parameter setting it finds.
        auto best_parameters = dlib::find_min_global(cross_validation_score,
                                            {_pk_gamma_min, _pk_coef_min},        // lower bound constraint on gamma and coef
                                            {_pk_gamma_max, _pk_coef_max},        // upper bound constraint on gamma and coef
                                            dlib::max_function_calls(50),
                                            max_runtime_findminglobal);

        // Define krr object with best found parameters
        double pk_gamma = best_parameters.x(0);
        double pk_coef = best_parameters.x(1);
        dlib::rvm_trainer<polynomial_kernel> rvm_trainer;
        rvm_trainer.set_kernel(polynomial_kernel(pk_gamma, pk_coef, degree));

        // Now we train on the full set of data and get the resulting decision function
        _decision_functions_travels.dec_func_pk = rvm_trainer.train(_training_data_travels_x, _training_data_travels_y);

        // do 10-fold cross-validation
        dlib::matrix<double> result = dlib::cross_validate_regression_trainer(rvm_trainer, _training_data_travels_x, _training_data_travels_y, 10);



        // signal output
        std::chrono::nanoseconds comptime = std::chrono::system_clock::now() - start_time;

        info = "Finished training RVM Trainer with Polynomial Kernel (Cubic) for travels.";
        info += "\n    Best value for gamma (kernel) = ";
        info += QString::number(pk_gamma);
        info += "\n    Best value for coef (kernel) = ";
        info += QString::number(pk_coef);
        info += "\n    Mean squared error = ";
        info += QString::number(result(0));
        info += "\n    Pearson correlation = ";
        info += QString::number(result(1));
        info += "\n    Mean absolute error = ";
        info += QString::number(result(2));
        info += "\n    Standard deviation absolute error = ";
        info += QString::number(result(3));
        info += "\nElapsed time: ";
        info += QString::number(comptime.count() / NANO);
        info += " seconds";

        global::_logger << global::logger::log_type::INFORMATION << info;
        emit(signal_status(info));
    }










    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void machine_learning_interface::generate_training_data_evacuations()
    {
        // DESIGN OF EXPERIMENTS
        QString output_text;
        std::chrono::system_clock::time_point start_time = std::chrono::system_clock::now();





        /*QString filename = QStringLiteral("training_data_evac.txt");
        QFile file(filename);

        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) // read data from file
        {
            output_text = "\nImporting training data evacuations from file";
            emit(signal_status(output_text));

            QTextStream stream(&file);
            stream >> _nb_training_data;

            for(size_t i = 0; i < _nb_training_data; ++i)
            {
                sample_type sample;
                sample.set_size(timetable::nb_locations);
                for(size_t j = 0; j < timetable::nb_locations; ++j)
                {
                    int people;
                    stream >> people;
                    sample(j) = people;
                }
                _training_data_evac_x.push_back(sample);

                double evactime;
                stream >> evactime;
                _training_data_evac_y.push_back(evactime);
            }
        }

        else*/ // generate data
        {

            output_text = "\nGenerating training data evacuations ...";
            emit(signal_status(output_text));

            // 1. initialize vector of training data
            _training_data_evac_x.clear();
            _training_data_evac_x.reserve(_nb_training_data);
            _training_data_evac_y.clear();
            _training_data_evac_y.reserve(_nb_training_data);


            // 2. for every room find maximally allowed number of people
            std::vector<int> room_max_nb_people;
            room_max_nb_people.reserve(timetable::nb_locations);
            for(int r = 0; r < timetable::nb_locations; ++r)
            {
                room_max_nb_people.push_back(0);
                for(int e = 0; e < timetable::nb_events; ++e)
                {
                    if(timetable::get_event_location_possible(e,r))
                    {
                        if(timetable::get_event_nb_people(e) > room_max_nb_people[r])
                            room_max_nb_people[r] = timetable::get_event_nb_people(e);
                    }
                }
            }



            // 3. latin hypercube sampling
            std::vector<std::vector<int>> rooms_remaining_strata;
            std::vector<int> vec_;
            vec_.reserve(_nb_training_data);
            for(int i = 0; i < _nb_training_data; ++i)
                vec_.push_back(i);

            for(int r = 0; r < timetable::nb_locations; ++r)   // initialize strata
                rooms_remaining_strata.push_back(vec_);

            // generate N observations and use menge to analyze them
            for(int obs = 0; obs < _nb_training_data - 1; ++obs)
            {
                std::vector<int> room_nb_people;
                room_nb_people.reserve(timetable::nb_locations);

                for(int r = 0; r < timetable::nb_locations; ++r) // generate the number of every people in each room
                {
                    std::uniform_int_distribution<int> dist_index(0, rooms_remaining_strata[r].size() - 1);
                    int index = dist_index(generator);
                    int stratum = rooms_remaining_strata[r][index];
                    int nbp = stratum * room_max_nb_people[r] / (_nb_training_data - 1) +0.1;     // borders of the stratum (one group less so that both endpoints are sampled)
                    //int nbp = (int)((double)(stratum + 0.5) * room_max_nb_people[r] / nb_training_data + 0.5); // middle of the stratum

                    if(obs == 0)
                        room_nb_people.push_back(room_max_nb_people[r]);
                    else
                        room_nb_people.push_back(nbp);

                    // delete this stratum from the vector
                    if(obs > 0)
                        rooms_remaining_strata[r].erase(rooms_remaining_strata[r].begin() + index);
                }

                // use menge to estimate evacuation time
                double evactime = _menge->calculate_custom_evacuation_time(room_nb_people);
                if(evactime < 0)
                    evactime = 0;


                // put observation in matrix
                sample_type mat;
                mat.set_size(timetable::nb_locations);
                for(int r = 0; r < timetable::nb_locations; ++r)
                    mat(r) = room_nb_people[r];
                _training_data_evac_x.push_back(mat);
                _training_data_evac_y.push_back(evactime);

                output_text = "    Observation ";
                output_text.append(QString::number(obs+1));
                output_text.append("; Evacuation time = ");
                output_text.append(QString::number(evactime));
                output_text.append(" seconds");
                emit(signal_status(output_text));
            }

            // add row with all zeros
            sample_type mat;
            mat.set_size(timetable::nb_locations);
            for(int r = 0; r < timetable::nb_locations; ++r)
                mat(r) = 0;
            _training_data_evac_x.push_back(mat);
            _training_data_evac_y.push_back(0.0);
            output_text = "    Observation ";
            output_text.append(QString::number(_nb_training_data));
            output_text.append("; Evacuation time = 0 seconds");
            emit(signal_status(output_text));



            std::chrono::nanoseconds elapsed_time = std::chrono::system_clock::now() - start_time;
            _time_generating_data_evac = elapsed_time.count() / NANO;
            output_text = "Finished generating training data. Elapsed time: ";
            output_text.append(QString::number(_time_generating_data_evac));
            output_text.append(" seconds");
            emit(signal_status(output_text));
            global::_logger << global::logger::log_type::INFORMATION << "Generated " << _nb_training_data << " training data evacuations.\nElapsed time = " << _time_generating_data_evac << " seconds.";



            /*QFile savefile(filename);
            if(savefile.open(QIODevice::WriteOnly | QIODevice::Text))
            {
                QTextStream stream(&savefile);
                stream << _nb_training_data << "\n";

                for(size_t i = 0; i < _nb_training_data; ++i)
                {
                    stream << "\n";
                    for(size_t j = 0; j < timetable::nb_locations; ++j)
                        stream << _training_data_evac_x[i](j) << "\t";
                    stream << _training_data_evac_y[i];
                }
            }*/
        }
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void machine_learning_interface::generate_training_data_travels()
    {
        if(_surrogate_paths != nullptr)
            _surrogate_paths->generate_observations(_menge, _training_data_travels_x, _training_data_travels_y, _nb_training_data);
        else
            emit(signal_status("Couldn't generate training data travels: no paths for the surrogate specified"));
    }








    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void machine_learning_interface::import_decision_function_evacuations(const QString& filename)
    {
        QString output_text = "\nImporting decision function for evacuations from file ...";
        emit(signal_status(output_text));

        std::string stdfilename = filename.toStdString();

        // which kernel type?
        try
        {
            if(filename.contains("RBK", Qt::CaseInsensitive))
            {
                dlib::deserialize(stdfilename) >> _decision_functions_evacuations.dec_func_rbk;
                _learning_method = learning_method::krr_trainer_radial_basis_kernel;    // only decision function type matters (RBK, HIK, PK, or LK)
            }
            else if(filename.contains("HIK", Qt::CaseInsensitive))
            {
                dlib::deserialize(stdfilename) >> _decision_functions_evacuations.dec_func_hik;
                _learning_method = learning_method::krr_trainer_histogram_intersection_kernel;  // only decision function type matters (RBK, HIK, PK, or LK)
            }
            else if(filename.contains("PK", Qt::CaseInsensitive))
            {
                dlib::deserialize(stdfilename) >> _decision_functions_evacuations.dec_func_pk;
                _learning_method = learning_method::krr_trainer_polynomial_kernel_quadratic;    // only decision function type matters (RBK, HIK, PK, or LK)
            }
            else if(filename.contains("LK", Qt::CaseInsensitive))
            {
                dlib::deserialize(stdfilename) >> _decision_functions_evacuations.dec_func_lk; // only decision function type matters (RBK, HIK, PK, or LK)
                _learning_method = learning_method::krr_trainer_linear_kernel;
            }
            else
            {
                throw std::exception("File name does not specify kernel type of decision function! Either RBK, HIK, PK, or LK");
            }


            output_text = "\nFinished importing decision function.";
            emit(signal_status(output_text));
            _trained_surrogate_evacuations = true;

        } catch(std::exception& ex)
        {
            output_text = "\n\n\nCouldn't import decision function!";
            output_text += "\nReason: ";
            output_text += ex.what();
            emit(signal_status(output_text));
            _trained_surrogate_evacuations = false;
        }
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void machine_learning_interface::import_decision_function_travels(const QString& filename)
    {
        QString output_text = "\nImporting decision function for travels from file ...";
        emit(signal_status(output_text));

        std::string stdfilename = filename.toStdString();

        // which kernel type?
        try
        {
            if(filename.contains("RBK", Qt::CaseInsensitive))
            {
                dlib::deserialize(stdfilename) >> _decision_functions_travels.dec_func_rbk;
                _learning_method = learning_method::krr_trainer_radial_basis_kernel;    // only decision function type matters (RBK, HIK, PK, or LK)
            }
            else if(filename.contains("HIK", Qt::CaseInsensitive))
            {
                dlib::deserialize(stdfilename) >> _decision_functions_travels.dec_func_hik;
                _learning_method = learning_method::krr_trainer_histogram_intersection_kernel;  // only decision function type matters (RBK, HIK, PK, or LK)
            }
            else if(filename.contains("PK", Qt::CaseInsensitive))
            {
                dlib::deserialize(stdfilename) >> _decision_functions_travels.dec_func_pk;
                _learning_method = learning_method::krr_trainer_polynomial_kernel_quadratic;    // only decision function type matters (RBK, HIK, PK, or LK)
            }
            else if(filename.contains("LK", Qt::CaseInsensitive))
            {
                dlib::deserialize(stdfilename) >> _decision_functions_travels.dec_func_lk; // only decision function type matters (RBK, HIK, PK, or LK)
                _learning_method = learning_method::krr_trainer_linear_kernel;
            }
            else
            {
                throw std::exception("File name does not specify kernel type of decision function! Either RBK, HIK, PK, or LK");
            }


            output_text = "\nFinished importing decision function.";
            emit(signal_status(output_text));
            _trained_surrogate_travels = true;

        } catch(std::exception& ex)
        {
            output_text = "\n\n\nCouldn't import decision function!";
            output_text += "\nReason: ";
            output_text += ex.what();
            emit(signal_status(output_text));
            _trained_surrogate_evacuations = false;
        }
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void machine_learning_interface::save_decision_function_evacuations()
    {
        if(_trained_surrogate_evacuations)
        {
            std::string file_name = "Decision_Function_Evacuations_";

            // include current day in the name
            {
                std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
                std::time_t thetime = std::chrono::system_clock::to_time_t(now);
                std::tm *today = std::localtime(&thetime);

                int year = 1900 + today->tm_year;
                int month = 1 + today->tm_mon;
                int day = today->tm_mday;
                int hours = today->tm_hour;
                int min = today->tm_min;

                file_name += std::to_string(year);
                file_name += "-";
                file_name += std::to_string(month);
                file_name += "-";
                file_name += std::to_string(day);
                file_name += "-";
                file_name += std::to_string(hours);
                file_name += "-";
                file_name += std::to_string(min);
                file_name += "_";
            }

            if(_learning_method == learning_method::krr_trainer_radial_basis_kernel
                    || _learning_method == learning_method::svr_trainer_radial_basis_kernel
                    || _learning_method == learning_method::rvm_trainer_radial_basis_kernel)
            {
                file_name += "RBK.dat";
                dlib::serialize(file_name) << _decision_functions_evacuations.dec_func_rbk;
            }
            else if(_learning_method == learning_method::krr_trainer_histogram_intersection_kernel
                    || _learning_method == learning_method::svr_trainer_histogram_intersection_kernel
                    || _learning_method == learning_method::rvm_trainer_histogram_intersection_kernel)
            {
                file_name += "HIK.dat";
                dlib::serialize(file_name) << _decision_functions_evacuations.dec_func_hik;
            }
            else if(_learning_method == learning_method::krr_trainer_polynomial_kernel_quadratic
                    || _learning_method == learning_method::krr_trainer_polynomial_kernel_cubic
                    || _learning_method == learning_method::svr_trainer_polynomial_kernel_quadratic
                    || _learning_method == learning_method::svr_trainer_polynomial_kernel_cubic
                    || _learning_method == learning_method::rvm_trainer_polynomial_kernel_quadratic
                    || _learning_method == learning_method::rvm_trainer_polynomial_kernel_cubic)
            {
                file_name += "PK.dat";
                dlib::serialize(file_name) << _decision_functions_evacuations.dec_func_pk;
            }
            else if(_learning_method == learning_method::krr_trainer_linear_kernel
                    || _learning_method == learning_method::svr_trainer_linear_kernel
                    || _learning_method == learning_method::rvm_trainer_linear_kernel)
            {
                file_name += "LK.dat";
                dlib::serialize(file_name) << _decision_functions_evacuations.dec_func_lk;
            }

            emit(signal_status("Decision function evacuations saved"));
        }
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void machine_learning_interface::save_decision_function_travels()
    {
        if(_trained_surrogate_travels)
        {
            std::string file_name = "Decision_Function_Travels_";

            // include current day in the name
            {
                std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
                std::time_t thetime = std::chrono::system_clock::to_time_t(now);
                std::tm *today = std::localtime(&thetime);

                int year = 1900 + today->tm_year;
                int month = 1 + today->tm_mon;
                int day = today->tm_mday;
                int hours = today->tm_hour;
                int min = today->tm_min;

                file_name += std::to_string(year);
                file_name += "-";
                file_name += std::to_string(month);
                file_name += "-";
                file_name += std::to_string(day);
                file_name += "-";
                file_name += std::to_string(hours);
                file_name += "-";
                file_name += std::to_string(min);
                file_name += "_";
            }

            if(_learning_method == learning_method::krr_trainer_radial_basis_kernel
                    || _learning_method == learning_method::svr_trainer_radial_basis_kernel
                    || _learning_method == learning_method::rvm_trainer_radial_basis_kernel)
            {
                file_name += "RBK.dat";
                dlib::serialize(file_name) << _decision_functions_travels.dec_func_rbk;
            }
            else if(_learning_method == learning_method::krr_trainer_histogram_intersection_kernel
                    || _learning_method == learning_method::svr_trainer_histogram_intersection_kernel
                    || _learning_method == learning_method::rvm_trainer_histogram_intersection_kernel)
            {
                file_name += "HIK.dat";
                dlib::serialize(file_name) << _decision_functions_travels.dec_func_hik;
            }
            else if(_learning_method == learning_method::krr_trainer_polynomial_kernel_quadratic
                    || _learning_method == learning_method::krr_trainer_polynomial_kernel_cubic
                    || _learning_method == learning_method::svr_trainer_polynomial_kernel_quadratic
                    || _learning_method == learning_method::svr_trainer_polynomial_kernel_cubic
                    || _learning_method == learning_method::rvm_trainer_polynomial_kernel_quadratic
                    || _learning_method == learning_method::rvm_trainer_polynomial_kernel_cubic)
            {
                file_name += "PK.dat";
                dlib::serialize(file_name) << _decision_functions_travels.dec_func_pk;
            }
            else if(_learning_method == learning_method::krr_trainer_linear_kernel
                    || _learning_method == learning_method::svr_trainer_linear_kernel
                    || _learning_method == learning_method::rvm_trainer_linear_kernel)
            {
                file_name += "LK.dat";
                dlib::serialize(file_name) << _decision_functions_travels.dec_func_lk;
            }

            emit(signal_status("Decision function travels saved"));
        }
    }















    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    bool machine_learning_interface::is_trained() const
    {
        if(_alpha_objective < 0.01) {
            if(_trained_surrogate_evacuations)
                return true;
            else
                return false;
        }
        else if (_alpha_objective > 0.99) {
            if(_trained_surrogate_travels)
                return true;
            else
                return false;
        }
        else {
            if(_trained_surrogate_evacuations && _trained_surrogate_travels)
                return true;
            else
                return false;
        }
    }



}

