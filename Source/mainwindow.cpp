#include "mainwindow.h"
#include "ui_mainwindow.h"


// CONSTRUCTOR
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    set_initial_layout();
    initialize_solution_view();

    qRegisterMetaType<timetable::solution>();
    make_connections();
}




// CONNECTIONS
void MainWindow::make_connections()
{
    // Importing data
    connect(ui->actionImport_Timetable_Data, SIGNAL(triggered(bool)), this, SLOT(import_timetable_data()));
    connect(ui->actionClear_Timetable_Data, SIGNAL(triggered(bool)), this, SLOT(clear_timetable_data()));
    connect(ui->actionImport_Building_Data, SIGNAL(triggered(bool)), this, SLOT(import_building_data()));
    connect(ui->actionClear_Building_Data, SIGNAL(triggered(bool)), this, SLOT(clear_building_data()));
    connect(ui->actionImport_Solution, SIGNAL(triggered(bool)), this, SLOT(import_existing_solution()));
    connect(ui->actionClear_Solution, SIGNAL(triggered(bool)), this, SLOT(clear_existing_solution()));
    connect(ui->actionImport_Surrogate_Map, SIGNAL(triggered(bool)), this, SLOT(import_surrogate_map()));
    //connect(ui->actionImport_Decision_Function_Evacuations, SIGNAL(triggered(bool)), this, SLOT(import_decision_function_evac()));
    connect(ui->actionClear_Surrogate_Map, SIGNAL(triggered(bool)), this, SLOT(clear_surrogate_map()));
    //connect(ui->actionImport_Decision_Function_Traveltime, SIGNAL(triggered(bool)), this, SLOT(import_decision_function_tt()));
    //connect(ui->actionSave_Decision_Functions, SIGNAL(triggered(bool)), this, SLOT(save_decision_functions()));
    connect(ui->actionReset_Surrogates, SIGNAL(triggered(bool)), this, SLOT(clear_surrogates()));
    connect(ui->actionImport_Relevant_Timeslots_Traveltimes, SIGNAL(triggered(bool)), this, SLOT(import_relevant_timeslots_traveltimes()));
    connect(ui->actionImport_Scenario, SIGNAL(triggered(bool)), this, SLOT(import_scenario()));
    connect(ui->actionClear_Scenario, SIGNAL(triggered(bool)), this, SLOT(clear_scenario()));


    // Running algorithm
    //connect(ui->actionRun_Algorithm, SIGNAL(triggered(bool)), this, SLOT(do_calculations()));
    connect(ui->actionRun_Algorithm, SIGNAL(triggered(bool)), this, SLOT(run_algorithm()));

    connect(&timer_comptime, SIGNAL(timeout()), this, SLOT(update_time()));
    connect(&timetable_algorithm, SIGNAL(signal_algorithm_status(QString)), this, SLOT(output_algorithm_info(QString)));
    connect(&timetable_algorithm, SIGNAL(new_best_solution_found(timetable::solution)), this, SLOT(algorithm_update_solution(timetable::solution)));
    connect(&future_watcher, SIGNAL(finished()), this, SLOT(clear_time()));
    connect(&future_watcher, SIGNAL(finished()), this, SLOT(algorithm_finalize_solution_view()));
    connect(&future_watcher, SIGNAL(finished()), &timer_comptime, SLOT(stop()));
    connect(ui->actionSave_Solution, SIGNAL(triggered(bool)), this, SLOT(save_solution()));

    // Disable and enable GUI actions
    connect(&future_watcher, SIGNAL(started()), this, SLOT(disable_actions_GUI()));
    connect(&future_watcher, SIGNAL(finished()), this, SLOT(enable_actions_GUI()));
    connect(&future_watcher_ped, SIGNAL(started()), this, SLOT(disable_actions_GUI()));
    connect(&future_watcher_ped, SIGNAL(finished()), this, SLOT(enable_actions_GUI()));
    connect(&future_watcher_ml, SIGNAL(started()), this, SLOT(disable_actions_GUI()));
    connect(&future_watcher_ml, SIGNAL(finished()), this, SLOT(enable_actions_GUI()));

    connect(ui->actionCompare_Learning_Methods, SIGNAL(triggered(bool)), this, SLOT(compare_learning_methods_surrogates()));
    connect(&machine_learning_interface, SIGNAL(signal_status(QString)), this, SLOT(output_algorithm_info(QString)));
    connect(&future_watcher_ml, SIGNAL(finished()), this, SLOT(clear_time()));
    connect(&future_watcher_ml, SIGNAL(finished()), &timer_comptime, SLOT(stop()));
    connect(&ml_surrogate_paths, SIGNAL(signal_stat(QString)), this, SLOT(output_algorithm_info(QString)));
    connect(&mengeinterface, SIGNAL(signal_status(QString)), this, SLOT(output_algorithm_info(QString)));


    // Solution analysis
    connect(ui->actionAnalyze_Solution, SIGNAL(triggered(bool)), this, SLOT(analyze_solution_start()));
    connect(ui->actionAnalyze_Evacuation, SIGNAL(triggered(bool)), this, SLOT(analyze_evacuation()));
    connect(ui->actionAnalyze_Flow_Between_Consecutive_Events, SIGNAL(triggered(bool)), this, SLOT(analyze_flowsbetweenevents()));
    connect(&mengeinterface, SIGNAL(finished_one_calculation(int)), this, SLOT(analyze_solution_update(int)));
    connect(&mengeinterface, SIGNAL(finished()), this, SLOT(analyze_solution_finalize_view()));


    // Exhaustive search
    connect(ui->actionDo_Exhaustive_Search, SIGNAL(triggered(bool)), this, SLOT(do_exhaustive_search()));;


    // Generate instance
    connect(ui->actionGenerate_Timetable_Instance, SIGNAL(triggered(bool)), this, SLOT(generate_timetable_instance()));


    // Settings
    connect(ui->actionConfigure_Menge, SIGNAL(triggered(bool)), this, SLOT(settings_menge()));
    connect(ui->actionConfigure_Algorithm, SIGNAL(triggered(bool)), this, SLOT(settings_algorithm()));


    // Help
    connect(ui->actionHow_To, SIGNAL(triggered(bool)), this, SLOT(how_to()));
    connect(ui->actionAbout, SIGNAL(triggered(bool)), this, SLOT(about()));


    // Errors
    connect(&mengeinterface, SIGNAL(signal_error(QString)), this, SLOT(print_error(QString)));
    connect(&machine_learning_interface, SIGNAL(signal_error(QString)), this, SLOT(print_error(QString)));
    connect(&timetable_algorithm, SIGNAL(signal_error(QString)), this, SLOT(print_error(QString)));


    // Manually changing solution
    connect(table_timeslot_location, SIGNAL(currentCellChanged(int,int,int,int)), this, SLOT(manually_swap_cells(int,int,int,int)));
}




// ENABLE AND DISABLE ACTIONS IN GUI
void MainWindow::enable_actions_GUI()
{
    ui->actionAnalyze_Evacuation->setEnabled(true);
    ui->actionAnalyze_Flow_Between_Consecutive_Events->setEnabled(true);
    ui->actionAnalyze_Solution->setEnabled(true);
    ui->actionClear_Building_Data->setEnabled(true);
    ui->actionClear_Decision_Function_Evacuations->setEnabled(true);
    ui->actionClear_Decision_Function_Traveltime->setEnabled(true);
    ui->actionClear_Solution->setEnabled(true);
    ui->actionClear_Surrogate_Map->setEnabled(true);
    ui->actionClear_Timetable_Data->setEnabled(true);
    ui->actionCompare_Learning_Methods->setEnabled(true);
    ui->actionConfigure_Algorithm->setEnabled(true);
    ui->actionConfigure_Menge->setEnabled(true);
    ui->actionDo_Exhaustive_Search->setEnabled(true);
    ui->actionImport_Building_Data->setEnabled(true);
    ui->actionImport_Relevant_Timeslots_Traveltimes->setEnabled(true);
    ui->actionImport_Solution->setEnabled(true);
    ui->actionImport_Surrogate_Map->setEnabled(true);
    ui->actionImport_Timetable_Data->setEnabled(true);
    ui->actionReset_Surrogates->setEnabled(true);
    ui->actionRun_Algorithm->setEnabled(true);
    ui->actionSave_Solution->setEnabled(true);
}


void MainWindow::disable_actions_GUI()
{
    ui->actionAnalyze_Evacuation->setDisabled(true);
    ui->actionAnalyze_Flow_Between_Consecutive_Events->setDisabled(true);
    ui->actionAnalyze_Solution->setDisabled(true);
    ui->actionClear_Building_Data->setDisabled(true);
    ui->actionClear_Decision_Function_Evacuations->setDisabled(true);
    ui->actionClear_Decision_Function_Traveltime->setDisabled(true);
    ui->actionClear_Solution->setDisabled(true);
    ui->actionClear_Surrogate_Map->setDisabled(true);
    ui->actionClear_Timetable_Data->setDisabled(true);
    ui->actionCompare_Learning_Methods->setDisabled(true);
    ui->actionConfigure_Algorithm->setDisabled(true);
    ui->actionConfigure_Menge->setDisabled(true);
    ui->actionDo_Exhaustive_Search->setDisabled(true);
    ui->actionImport_Building_Data->setDisabled(true);
    ui->actionImport_Relevant_Timeslots_Traveltimes->setDisabled(true);
    ui->actionImport_Solution->setDisabled(true);
    ui->actionImport_Surrogate_Map->setDisabled(true);
    ui->actionImport_Timetable_Data->setDisabled(true);
    ui->actionReset_Surrogates->setDisabled(true);
    ui->actionRun_Algorithm->setDisabled(true);
    ui->actionSave_Solution->setDisabled(true);
}




// ERROR
void MainWindow::print_error(QString message)
{
    QMessageBox::critical(nullptr,
                          tr("Error occured!"),
                          message,
                          QMessageBox::Ok | QMessageBox::Default);

    ui->textedit_algorithm_output->clear();
}




// INITIAL LAYOUT
void MainWindow::set_initial_layout()
{
    // set respective sizes of widgets in the splitters
    QList<int> width_trees_solandoutput{ 120, 500 };
    QList<int> height_trees{ 100, 100 };
    QList<int> height_solandoutput{ 350, 100 };

    ui->splitter_overall->setSizes(width_trees_solandoutput);
    ui->splitter_trees->setSizes(height_trees);
    ui->splitter_solandoutput->setSizes(height_solandoutput);

    // clear treewidgets
    ui->tree_timetable_data->clear();
    ui->tree_timetable_data->setColumnCount(0);
    ui->tree_building_data->clear();
    ui->tree_building_data->setColumnCount(0);

    //
}


void MainWindow::initialize_solution_view()
{
    // Clear
    ui->tabwidget_timetable->clear();

    // Setup tabs
    table_timeslot_location = new QTableWidget;
    table_timeslot_eventgroup = new QTableWidget;
    chartview_evacuations = new QtCharts::QChartView;
    chartview_traveltimes = new QtCharts::QChartView;
    chart_evacuations = new QtCharts::QChart();
    chart_traveltimes = new QtCharts::QChart();

    ui->tabwidget_timetable->addTab(table_timeslot_location, tr("Solution: Timeslots-Locations"));
    ui->tabwidget_timetable->addTab(table_timeslot_eventgroup, tr("Solution: Timeslots-Eventgroups"));
    ui->tabwidget_timetable->addTab(chartview_evacuations, tr("Evacuation times"));
    ui->tabwidget_timetable->addTab(chartview_traveltimes, tr("Travel times"));
}





// IMPORT DATA / CLEAR DATA
void MainWindow::import_timetable_data()
{
    if(timetable::data_exist)
    {
        int result = QMessageBox::warning(nullptr,
                              tr("Warning!"),
                              tr("Importing new data will erase old data!\nDo you want to continue?"),
                              QMessageBox::Yes | QMessageBox::No);

        if(result == QMessageBox::No)
            return;
    }


    QString file_name = QFileDialog::getOpenFileName(nullptr,
                                                     tr("Input File"),
                                                     QDir::currentPath(),
                                                     tr("Text Files (*.txt)"));

    if(file_name.isEmpty())
        return;


    try
    {
        timetable::read_data(file_name);
        build_tree_timetable_data();
    }
    catch(const std::exception& ex)
    {
        QMessageBox::critical(nullptr,
                              tr("Error occured!"),
                              ex.what(),
                              QMessageBox::Ok | QMessageBox::Default);
    }
}


void MainWindow::clear_timetable_data()
{
    if(timetable::data_exist)
    {
        int result = QMessageBox::warning(nullptr,
                              tr("Warning!"),
                              tr("Are you sure you want to delete the timetable data?"),
                              QMessageBox::Yes | QMessageBox::No);

        if (result == QMessageBox::No)
            return;
    }

    timetable::clear_data();
    ui->tree_timetable_data->clear();
}


void MainWindow::import_building_data()
{
    if(building::data_exist)
    {
        int result = QMessageBox::warning(nullptr,
                              tr("Warning!"),
                              tr("Importing new data will erase old data!\nDo you want to continue?"),
                              QMessageBox::Yes | QMessageBox::No);

        if(result == QMessageBox::No)
            return;
    }

    QString file_name = QFileDialog::getOpenFileName(nullptr, tr("Input File"), QDir::currentPath(), tr("Text Files (*.txt)"));

    if(file_name.isEmpty())
        return;

    try
    {
        building::import_data(file_name);
        build_tree_building_data();
    }
    catch(const std::exception& ex)
    {
        QMessageBox::critical(nullptr,
                              tr("Error occured!"),
                              tr(ex.what()),
                              QMessageBox::Ok | QMessageBox::Default);
    }
}


void MainWindow::clear_building_data()
{
    if(building::data_exist)
    {
        int result = QMessageBox::warning(nullptr,
                              tr("Warning!"),
                              tr("Importing new data will erase old data!\nDo you want to continue?"),
                              QMessageBox::Yes | QMessageBox::No);

        if(result == QMessageBox::No)
            return;
    }


    building::clear_data();
    ui->tree_building_data->clear();
}


void MainWindow::import_existing_solution()
{
    if(!timetable::data_exist)
    {
        QMessageBox::warning(this, tr("No timetable data!"), tr("No timetable data exist! \nImport data first!"),QMessageBox::Ok);
        return;
    }
    if(!building::data_exist)
    {
        QMessageBox::warning(this, tr("No building data!"), tr("No building data exist! \nImport data first!"),QMessageBox::Ok);
        return;
    }

    if(!timetable_solution.is_empty())
    {
        int result = QMessageBox::warning(nullptr,
                              tr("Warning!"),
                              tr("Importing new solution will erase old solution!\nDo you want to continue?"),
                              QMessageBox::Yes | QMessageBox::No);

        if(result == QMessageBox::No)
            return;
    }


    QString file_name = QFileDialog::getOpenFileName(nullptr,
                                                     tr("Input File"),
                                                     QDir::currentPath(),
                                                     tr("Text Files (*.txt)"));

    if(file_name.isEmpty())
        return;

    try
    {
        timetable_solution.read_data(file_name);
        update_solution_view();
    }
    catch(const std::exception& ex)
    {
        QMessageBox::critical(nullptr,
                              tr("Error occured!"),
                              ex.what(),
                              QMessageBox::Ok | QMessageBox::Default);
    }
}


void MainWindow::clear_existing_solution()
{
    if(!timetable_solution.is_empty())
    {
        int result = QMessageBox::warning(nullptr,
                              tr("Warning!"),
                              tr("Are you sure you want to delete the timetable solution?"),
                              QMessageBox::Yes | QMessageBox::No);

        if (result == QMessageBox::No)
            return;
    }

    timetable_solution.clear();


    table_timeslot_location->clear();
    table_timeslot_eventgroup->clear();

    chart_evacuations->removeAllSeries();
    chart_traveltimes->removeAllSeries();
}


void MainWindow::import_scenario()
{
    if(scenario::data_exist)
    {
        int result = QMessageBox::warning(nullptr,
                              tr("Warning!"),
                              tr("Importing new data will erase old data!\nDo you want to continue?"),
                              QMessageBox::Yes | QMessageBox::No);

        if(result == QMessageBox::No)
            return;
    }


    QString file_name = QFileDialog::getOpenFileName(nullptr,
                                                     tr("Input File"),
                                                     QDir::currentPath(),
                                                     tr("Text Files (*.txt)"));

    if(file_name.isEmpty())
        return;


    try
    {
        scenario::import_data(file_name);
        QMessageBox::information(nullptr,
                              tr(""),
                              tr("Scenario successfully imported!"),
                              QMessageBox::Ok | QMessageBox::Default);
    }
    catch(const std::exception& ex)
    {
        QMessageBox::critical(nullptr,
                              tr("Error occured!"),
                              ex.what(),
                              QMessageBox::Ok | QMessageBox::Default);
    }

}


void MainWindow::clear_scenario()
{
    if(scenario::data_exist)
    {
        int result = QMessageBox::warning(nullptr,
                              tr("Warning!"),
                              tr("Are you sure you want to delete the scenario?"),
                              QMessageBox::Yes | QMessageBox::No);

        if (result == QMessageBox::No)
            return;
    }

    scenario::clear_data();
}


void MainWindow::import_surrogate_map()
{
    if(!timetable::data_exist)
    {
        QMessageBox::warning(this, tr("No timetable data!"), tr("No timetable data exist! \nImport data first!"),QMessageBox::Ok);
        return;
    }
    if(!building::data_exist)
    {
        QMessageBox::warning(this, tr("No building data!"), tr("No building data exist! \nImport data first!"),QMessageBox::Ok);
        return;
    }

    QString file_name = QFileDialog::getOpenFileName(nullptr,
                                                     tr("Input File"),
                                                     QDir::currentPath(),
                                                     tr("Text Files (*.txt)"));

    if(file_name.isEmpty())
        return;

    try
    {
        ml_surrogate_paths.read_data(file_name);
        QMessageBox::information(nullptr,
                              tr(""),
                              tr("Surrogate map successfully imported!"),
                              QMessageBox::Ok | QMessageBox::Default);
    }
    catch(const std::exception& ex)
    {
        QMessageBox::critical(nullptr,
                              tr("Error occured!"),
                              tr(ex.what()),
                              QMessageBox::Ok | QMessageBox::Default);
    }

}


void MainWindow::clear_surrogate_map()
{
    if(ml_surrogate_paths.data_exist())
    {
        int result = QMessageBox::warning(nullptr,
                              tr("Warning!"),
                              tr("Are you sure you want to delete the surrogate paths?"),
                              QMessageBox::Yes | QMessageBox::No);

        if (result == QMessageBox::No)
            return;
    }

    ml_surrogate_paths.clear_data();
}


void MainWindow::clear_surrogates()
{
    if(machine_learning_interface.is_trained())
    {
        int result = QMessageBox::warning(nullptr,
                              tr("Warning!"),
                              tr("Are you sure you want to reset the surrogates?"),
                              QMessageBox::Yes | QMessageBox::No);

        if(result == QMessageBox::No)
            return;
    }

    machine_learning_interface.clear();
}


void MainWindow::import_relevant_timeslots_traveltimes()
{
    if(!timetable::data_exist)
    {
        QMessageBox::warning(this, tr("No timetable data!"), tr("No timetable data exist! \nImport data first!"),QMessageBox::Ok);
        return;
    }


    if(!timetable_algorithm.timeslots_to_calculate_traveltimes_is_empty())
    {
        int result = QMessageBox::warning(nullptr,
                              tr("Warning!"),
                              tr("Importing new data will erase old data!\nDo you want to continue?"),
                              QMessageBox::Yes | QMessageBox::No);

        if(result == QMessageBox::No)
            return;
    }

    QString file_name = QFileDialog::getOpenFileName(nullptr,
                                                     tr("Input File"),
                                                     QDir::currentPath(),
                                                     tr("Text Files (*.txt)"));

    if(file_name.isEmpty())
        return;

    try
    {
        QFile file(file_name);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            throw std::runtime_error("Error in function MainWindow::import_relevant_timeslots_traveltimes. \nCouldn't open file.");
        }

        QTextStream stream(&file);
        QString input_token;
        bool input_ok;

        std::vector<int> timeslots;

        // number of data
        int nb_relevant_timeslots;
        stream >> input_token;
        nb_relevant_timeslots = input_token.toInt(&input_ok);
        if(!input_ok)
            throw std::runtime_error("Error in function MainWindow::import_relevant_timeslots_traveltimes. \nWrong input for \"nb_relevant_timeslots\". Should be an integer.");

        for(int i = 0; i < nb_relevant_timeslots; ++i)
        {
            int relevant_timeslot;
            stream >> input_token;
            relevant_timeslot = input_token.toInt(&input_ok);
            if(!input_ok)
                throw std::runtime_error("Error in function MainWindow::import_relevant_timeslots_traveltimes. \nWrong input for \"relevant_timeslot\". Should be an integer.");

            if(relevant_timeslot < 0)
                throw std::runtime_error("Error in function MainWindow::import_relevant_timeslots_traveltimes. \nWrong input for \"relevant_timeslot\". Cannot be smaller than 0.");
            if(relevant_timeslot >= timetable::nb_timeslots - 1)
                throw std::runtime_error("Error in function MainWindow::import_relevant_timeslots_traveltimes. \nWrong input for \"relevant_timeslot\". Cannot be larger than or equal to nb_timeslots-1 (numbering should start at 0).");


            timeslots.push_back(relevant_timeslot);
        }

        timetable_algorithm.set_timeslots_to_calculate_traveltimes(timeslots);

        QMessageBox::information(nullptr,
                              tr(""),
                              tr("Relevant timeslots to calculate traveltimes successfully imported!"),
                              QMessageBox::Ok | QMessageBox::Default);
    }
    catch(const std::exception& ex)
    {
        QMessageBox::critical(nullptr,
                              tr("Error occured!"),
                              tr(ex.what()),
                              QMessageBox::Ok | QMessageBox::Default);
    }

}




// GUI DISPLAY DATA
void MainWindow::build_tree_timetable_data()
{
    ui->tree_timetable_data->clear();
    ui->tree_timetable_data->setColumnCount(2);
    ui->tree_timetable_data->setHeaderLabels(QStringList() << tr("Element") << tr("Value"));


    // 0. INSTANCE NAME
    QTreeWidgetItem *instance;
    instance = new QTreeWidgetItem(ui->tree_timetable_data);
    instance->setText(0, tr("Instance name"));
    instance->setText(1, timetable::instance_name);

    // 1. BASIC DATA
    QTreeWidgetItem *basicdata;
    basicdata = new QTreeWidgetItem(ui->tree_timetable_data);
    basicdata->setText(0, tr("Basic data"));

    QTreeWidgetItem *item;
    item = new QTreeWidgetItem(basicdata);
    item->setText(0, tr("Number of events"));
    item->setText(1, QString::number(timetable::nb_events));

    item = new QTreeWidgetItem(basicdata);
    item->setText(0, tr("Number of eventgroups"));
    item->setText(1, QString::number(timetable::nb_eventgroups));

    item = new QTreeWidgetItem(basicdata);
    item->setText(0, tr("Number of timeslots"));
    item->setText(1, QString::number(timetable::nb_timeslots));

    item = new QTreeWidgetItem(basicdata);
    item->setText(0, tr("Number of locations"));
    item->setText(1, QString::number(timetable::nb_locations));



    // 2. EVENTS
    QTreeWidgetItem *events;
    events = new QTreeWidgetItem(ui->tree_timetable_data);
    events->setText(0, tr("Events"));

    for(int l = 0; l < timetable::nb_events; ++l)
    {
        // event name
        QTreeWidgetItem *event;
        event = new QTreeWidgetItem(events);
        event->setText(0, timetable::get_event_name(l));

        // number of people
        QTreeWidgetItem *nbpeople;
        nbpeople = new QTreeWidgetItem(event);
        nbpeople->setText(0, tr("Number of people"));
        nbpeople->setText(1, QString::number(timetable::get_event_nb_people(l)));

        // curricula
        QTreeWidgetItem *eventgroups;
        eventgroups = new QTreeWidgetItem(event);
        eventgroups->setText(0, tr("Eventgroups"));
        for(int c = 0; c < timetable::nb_eventgroups; ++c)
        {
            if(timetable::get_eventgroup_event(c,l))
            {
                QTreeWidgetItem *eventgroup;
                eventgroup = new QTreeWidgetItem(eventgroups);
                eventgroup->setText(0, timetable::get_eventgroup_name(c));
            }
        }

        // locations possible
        QTreeWidgetItem *locations;
        locations = new QTreeWidgetItem(event);
        locations->setText(0, tr("Locations"));
        for(int r = 0; r < timetable::nb_locations; ++r)
        {
            if(timetable::get_event_location_possible(l,r))
            {
                QTreeWidgetItem *location;
                location = new QTreeWidgetItem(locations);
                location->setText(0, timetable::get_location_name(r));
            }
        }
    }


    // 3. EVENTGROUPS
    QTreeWidgetItem *eventgroups;
    eventgroups = new QTreeWidgetItem(ui->tree_timetable_data);
    eventgroups->setText(0, tr("Eventgroups"));

    for(int c = 0; c < timetable::nb_eventgroups; ++c)
    {
        QTreeWidgetItem *eventgroup;
        eventgroup = new QTreeWidgetItem(eventgroups);
        eventgroup->setText(0, timetable::get_eventgroup_name(c));

        QTreeWidgetItem *nbp;
        nbp = new QTreeWidgetItem(eventgroup);
        nbp->setText(0, tr("Number of people"));
        nbp->setText(1, QString::number(timetable::get_eventgroup_nb_people(c)));

        for(int l = 0; l < timetable::nb_events; ++l)
        {
            if(timetable::get_eventgroup_event(c,l))
            {
                QTreeWidgetItem *lec;
                lec = new QTreeWidgetItem(eventgroup);
                lec->setText(0, timetable::get_event_name(l));
            }
        }
    }



    // 5. LOCATIONS
    QTreeWidgetItem *locations;
    locations = new QTreeWidgetItem(ui->tree_timetable_data);
    locations->setText(0, tr("Locations"));

    for(int r = 0; r < timetable::nb_locations; ++r)
    {
        QTreeWidgetItem *location;
        location = new QTreeWidgetItem(locations);
        location->setText(0, timetable::get_location_name(r));
    }


}


void MainWindow::build_tree_building_data()
{
    ui->tree_building_data->clear();
    ui->tree_building_data->setColumnCount(2);
    ui->tree_building_data->setHeaderLabels(QStringList() << tr("Element") << tr("Value"));


    // 0. INSTANCE NAME
    QTreeWidgetItem *instance;
    instance = new QTreeWidgetItem(ui->tree_building_data);
    instance->setText(0, tr("Instance name"));
    instance->setText(1, building::instance_name);


    // 1. OBSTACLES
    QTreeWidgetItem *obstacles;
    obstacles = new QTreeWidgetItem(ui->tree_building_data);
    obstacles->setText(0, tr("Obstacles"));

    for(int j = 0; j < building::obstacles.size(); ++j)
    {
        QTreeWidgetItem *obstacle;
        obstacle = new QTreeWidgetItem(obstacles);
        QString text = "Obstacle ";
        text.append(QString::number(j + 1));
        obstacle->setText(0, text);

        for(int k = 0; k < building::obstacles[j].nb_vertices; ++k)
        {
            QTreeWidgetItem *vertex = new QTreeWidgetItem(obstacle);
            text = "Vertex ";
            text.append(QString::number(k + 1));
            vertex->setText(0, text);

            QTreeWidgetItem *coor;
            coor = new QTreeWidgetItem(vertex);
            coor->setText(0, tr("x"));
            coor->setText(1, QString::number(building::obstacles[j].vertices_x[k]));

            coor = new QTreeWidgetItem(vertex);
            coor->setText(0, tr("y"));
            coor->setText(1, QString::number(building::obstacles[j].vertices_y[k]));
        }
    }


    // 2. STAIRS
    QTreeWidgetItem *stairs;
    stairs = new QTreeWidgetItem(ui->tree_building_data);
    stairs->setText(0, tr("Stairs"));

    for(int j = 0; j < building::stairs.size(); ++j)
    {
        QTreeWidgetItem *staircase;
        staircase = new QTreeWidgetItem(stairs);
        QString text = "Stairs Element ";
        text.append(QString::number(j + 1));
        staircase->setText(0, text);

        // stairwell
        QTreeWidgetItem *elem;
        elem = new QTreeWidgetItem(staircase);
        elem->setText(0, tr("Stairwell"));
        elem->setText(1, QString::number(building::stairs[j].stairwell));

        // floor
        elem = new QTreeWidgetItem(staircase);
        elem->setText(0, tr("Floor"));
        elem->setText(1, QString::number(building::stairs[j].floor));

        // up/down
        elem = new QTreeWidgetItem(staircase);
        elem->setText(0, tr("Up or down"));
        if(building::stairs[j].up)
            elem->setText(1, "up");
        else
            elem->setText(1, "down");

        // coordinates
        elem = new QTreeWidgetItem(staircase);
        elem->setText(0, tr("from_x_min"));
        elem->setText(1, QString::number(building::stairs[j].from_x_min));

        elem = new QTreeWidgetItem(staircase);
        elem->setText(0, tr("from_x_max"));
        elem->setText(1, QString::number(building::stairs[j].from_x_max));

        elem = new QTreeWidgetItem(staircase);
        elem->setText(0, tr("from_y_min"));
        elem->setText(1, QString::number(building::stairs[j].from_y_min));

        elem = new QTreeWidgetItem(staircase);
        elem->setText(0, tr("from_y_max"));
        elem->setText(1, QString::number(building::stairs[j].from_y_max));

        elem = new QTreeWidgetItem(staircase);
        elem->setText(0, tr("to_x_min"));
        elem->setText(1, QString::number(building::stairs[j].to_x_min));

        elem = new QTreeWidgetItem(staircase);
        elem->setText(0, tr("to_x_max"));
        elem->setText(1, QString::number(building::stairs[j].to_x_max));

        elem = new QTreeWidgetItem(staircase);
        elem->setText(0, tr("to_y_min"));
        elem->setText(1, QString::number(building::stairs[j].to_y_min));

        elem = new QTreeWidgetItem(staircase);
        elem->setText(0, tr("to_y_max"));
        elem->setText(1, QString::number(building::stairs[j].to_y_max));
    }


    // 3. TARGETS
    QTreeWidgetItem *targets;
    targets = new QTreeWidgetItem(ui->tree_building_data);
    targets->setText(0, tr("Room targets"));

    for(int j = 0; j < building::room_targets.size(); ++j)
    {
        QTreeWidgetItem *target;
        target = new QTreeWidgetItem(targets);
        QString text = "Target ";
        text.append(QString::number(j + 1));
        target->setText(0, text);

        // 2 coordinates
        QTreeWidgetItem *coor;
        coor = new QTreeWidgetItem(target);
        coor->setText(0, tr("x"));
        coor->setText(1, QString::number(building::room_targets[j].x));

        coor = new QTreeWidgetItem(target);
        coor->setText(0, tr("y"));
        coor->setText(1, QString::number(building::room_targets[j].y));

        coor = new QTreeWidgetItem(target);
        coor->setText(0, tr("tolerance"));
        coor->setText(1, QString::number(building::room_targets[j].dist_tolerance));
    }

    QTreeWidgetItem *exit_target;
    exit_target = new QTreeWidgetItem(ui->tree_building_data);
    exit_target->setText(0, tr("Exit targets"));
    for(int i = 0; i < building::exit_targets.size(); ++i)
    {
        QTreeWidgetItem *target;
        target = new QTreeWidgetItem(exit_target);
        QString text = "Target ";
        text.append(QString::number(i + 1));
        target->setText(0, text);

        // 2 coordinates
        QTreeWidgetItem *coor;
        coor = new QTreeWidgetItem(target);
        coor->setText(0, tr("x"));
        coor->setText(1, QString::number(building::exit_targets[i].x));

        coor = new QTreeWidgetItem(target);
        coor->setText(0, tr("y"));
        coor->setText(1, QString::number(building::exit_targets[i].y));

        coor = new QTreeWidgetItem(target);
        coor->setText(0, tr("tolerance"));
        coor->setText(1, QString::number(building::exit_targets[i].dist_tolerance));
    }


    // 4. TELEPORT TARGETS
    QTreeWidgetItem *teleporttargets;
    teleporttargets = new QTreeWidgetItem(ui->tree_building_data);
    teleporttargets->setText(0, tr("Teleport targets rooms"));

    for(int j = 0; j < building::teleport_locations_rooms.size(); ++j)
    {
        QTreeWidgetItem *target;
        target = new QTreeWidgetItem(teleporttargets);
        QString text = "Target ";
        text.append(QString::number(j + 1));
        target->setText(0, text);

        // 2 coordinates
        QTreeWidgetItem *coor;
        coor = new QTreeWidgetItem(target);
        coor->setText(0, tr("x"));
        coor->setText(1, QString::number(building::teleport_locations_rooms[j].x));

        coor = new QTreeWidgetItem(target);
        coor->setText(0, tr("y"));
        coor->setText(1, QString::number(building::teleport_locations_rooms[j].y));
    }

    QTreeWidgetItem *exit_target_teleport;
    exit_target_teleport = new QTreeWidgetItem(ui->tree_building_data);
    exit_target_teleport->setText(0, tr("Teleport target exit"));
    {
        // 2 coordinates
        QTreeWidgetItem *coor;
        coor = new QTreeWidgetItem(exit_target_teleport);
        coor->setText(0, tr("x"));
        coor->setText(1, QString::number(building::teleport_location_exit.x));

        coor = new QTreeWidgetItem(exit_target_teleport);
        coor->setText(0, tr("y"));
        coor->setText(1, QString::number(building::teleport_location_exit.y));
    }



    // 5. ROAD MAP
    QTreeWidgetItem *road_map;
    road_map = new QTreeWidgetItem(ui->tree_building_data);
    road_map->setText(0, tr("Road Map"));
    road_map->setText(1, building::road_map_file_name);
}





// GUI DISPLAY SOLUTION
void MainWindow::update_solution_view()
{
    // 1. TIMESLOT-ROOM
    table_timeslot_location->clear();
    table_timeslot_location->setSortingEnabled(false);
    table_timeslot_location->setColumnCount(timetable::nb_timeslots);
    table_timeslot_location->setRowCount(timetable::nb_locations);

    // Set headers
    for (int t = 0; t < timetable::nb_timeslots; ++t)
    {
        QTableWidgetItem* columnname = new QTableWidgetItem(tr("Timeslot %1").arg(t + 1));
        table_timeslot_location->setHorizontalHeaderItem(t, columnname);
    }
    for (int r = 0; r < timetable::nb_locations; ++r)
    {
        QTableWidgetItem* rowname = new QTableWidgetItem(timetable::get_location_name(r));
        table_timeslot_location->setVerticalHeaderItem(r, rowname);
    }

    // Fill in event
    for(int l = 0; l < timetable::nb_events; ++l)
    {
        int timeslot = timetable_solution.event_timeslot(l);
        int room = timetable_solution.event_location(l);

        QTableWidgetItem* lecture = new QTableWidgetItem;
        lecture->setText(timetable::get_event_name(l));
        lecture->setTextAlignment(Qt::AlignCenter);
        lecture->setBackground(QBrush(Qt::yellow, Qt::Dense3Pattern));
        table_timeslot_location->setItem(room, timeslot, lecture);
    }

    //table_timeslot_location->resizeColumnsToContents();


    // 2. TIMESLOT-CURRICULUM
    table_timeslot_eventgroup->clear();
    table_timeslot_eventgroup->setSortingEnabled(false);
    table_timeslot_eventgroup->setColumnCount(timetable::nb_timeslots);
    table_timeslot_eventgroup->setRowCount(timetable::nb_eventgroups);

    // Set headers
    for (int t = 0; t < timetable::nb_timeslots; ++t)
    {
        QTableWidgetItem* columnname = new QTableWidgetItem(tr("Timeslot %1").arg(t + 1));
        table_timeslot_eventgroup->setHorizontalHeaderItem(t, columnname);
    }
    for (int c = 0; c < timetable::nb_eventgroups; ++c)
    {
        QTableWidgetItem* rowname = new QTableWidgetItem(timetable::get_eventgroup_name(c));
        table_timeslot_eventgroup->setVerticalHeaderItem(c, rowname);
    }

    // Fill in lectures
    for(int c = 0; c < timetable::nb_eventgroups; ++c)
    {
       for(int l = 0; l < timetable::nb_events; ++l)
       {
           if(timetable::get_eventgroup_event(c,l))
           {
                int timeslot = timetable_solution.event_timeslot(l);
                int curriculum = c;

                QTableWidgetItem* lecture = new QTableWidgetItem;
                lecture->setText(timetable::get_event_name(l));
                lecture->setTextAlignment(Qt::AlignCenter);
                lecture->setBackground(QBrush(Qt::yellow, Qt::Dense3Pattern));
                table_timeslot_eventgroup->setItem(curriculum, timeslot, lecture);
           }
       }
    }
}


void MainWindow::manually_swap_cells(int current_row, int current_column, int previous_row, int previous_column)
{
    int& timeslot1 = previous_column;
    int& room1 = previous_row;
    int& timeslot2 = current_column;
    int& room2 = current_row;

    if(timeslot1 != timeslot2)
        return;

    if(timeslot1 == timeslot2 && room1 == room2)
        return;

    int event1 = timetable_solution.timeslot_location(timeslot1, room1);
    int event2 = timetable_solution.timeslot_location(timeslot2, room2);

    if(event1 >= 0 && event2 >= 0)
    {
        QString event1_name = timetable::get_event_name(event1);
        QString event2_name = timetable::get_event_name(event2);

        QString message = "Do you want to swap " + event1_name + " with " + event2_name + "?";
        int result = QMessageBox::information(this, "Warning!", message, QMessageBox::Yes, QMessageBox::No);
        if(result == QMessageBox::No)
            return;

        if(!timetable::get_event_location_possible(event1, room2))
        {
            QString room_name = timetable::get_location_name(room2);
            message = event1_name + " cannot be planned in room " + room_name
                    + "\nDo you want to continue anyway?";
            result = QMessageBox::information(this, "Warning!", message, QMessageBox::Yes, QMessageBox::No);
            if(result == QMessageBox::No)
                return;
        }
        if(!timetable::get_event_location_possible(event2, room1))
        {
            QString room_name = timetable::get_location_name(room1);
            message = event2_name + " cannot be planned in room " + room_name
                    + "\nDo you want to continue anyway?";
            result = QMessageBox::information(this, "Warning!", message, QMessageBox::Yes, QMessageBox::No);
            if(result == QMessageBox::No)
                return;
        }

        timetable_solution.set_event_location(event1, room2);
        timetable_solution.set_event_location(event2, room1);

        table_timeslot_location->removeCellWidget(room1, timeslot1);
        table_timeslot_location->removeCellWidget(room2, timeslot2);

        QTableWidgetItem* lecture = new QTableWidgetItem;
        lecture->setText(event2_name);
        lecture->setTextAlignment(Qt::AlignCenter);
        lecture->setBackground(QBrush(Qt::yellow, Qt::Dense3Pattern));
        table_timeslot_location->setItem(room1, timeslot1, lecture);

        lecture = new QTableWidgetItem;
        lecture->setText(event1_name);
        lecture->setTextAlignment(Qt::AlignCenter);
        lecture->setBackground(QBrush(Qt::yellow, Qt::Dense3Pattern));
        table_timeslot_location->setItem(room2, timeslot2, lecture);
    }
    else if (event1 >= 0)
    {
        QString event1_name = timetable::get_event_name(event1);

        QString message = "Do you want to move " + event1_name + " to the empty cell?";
        int result = QMessageBox::information(this, "Warning!", message, QMessageBox::Yes, QMessageBox::No);
        if(result == QMessageBox::No)
            return;

        if(!timetable::get_event_location_possible(event1, room2))
        {
            QString room_name = timetable::get_location_name(room2);
            message = event1_name + " cannot be planned in room " + room_name
                    + "\nDo you want to continue anyway?";
            result = QMessageBox::information(this, "Warning!", message, QMessageBox::Yes, QMessageBox::No);
            if(result == QMessageBox::No)
                return;
        }

        timetable_solution.set_event_location(event1, room2);

        table_timeslot_location->removeCellWidget(room1, timeslot1);
        table_timeslot_location->removeCellWidget(room2, timeslot2);

        QTableWidgetItem* lecture = nullptr;
        table_timeslot_location->setItem(room1, timeslot1, lecture);

        lecture = new QTableWidgetItem;
        lecture->setText(event1_name);
        lecture->setTextAlignment(Qt::AlignCenter);
        lecture->setBackground(QBrush(Qt::yellow, Qt::Dense3Pattern));
        table_timeslot_location->setItem(room2, timeslot2, lecture);
    }
    else if (event2 >= 0)
    {
        QString event2_name = timetable::get_event_name(event2);

        QString message = "Do you want to move " + event2_name + " to the empty cell?";
        int result = QMessageBox::information(this, "Warning!", message, QMessageBox::Yes, QMessageBox::No);
        if(result == QMessageBox::No)
            return;

        if(!timetable::get_event_location_possible(event2, room1))
        {
            QString room_name = timetable::get_location_name(room1);
            message = event2_name + " cannot be planned in room " + room_name
                    + "\nDo you want to continue anyway?";
            result = QMessageBox::information(this, "Warning!", message, QMessageBox::Yes, QMessageBox::No);
            if(result == QMessageBox::No)
                return;
        }

        timetable_solution.set_event_location(event2, room1);

        table_timeslot_location->removeCellWidget(room1, timeslot1);
        table_timeslot_location->removeCellWidget(room2, timeslot2);

        QTableWidgetItem* lecture = new QTableWidgetItem;
        lecture->setText(event2_name);
        lecture->setTextAlignment(Qt::AlignCenter);
        lecture->setBackground(QBrush(Qt::yellow, Qt::Dense3Pattern));
        table_timeslot_location->setItem(room1, timeslot1, lecture);

        lecture = nullptr;
        table_timeslot_location->setItem(room2, timeslot2, lecture);
    }
    else
    {
        // two empty cells
        return;
    }
}






// SETTINGS
void MainWindow::settings_menge()
{
    menge_dialog_parameter_settings dialog;
    dialog.setWindowTitle("Settings Menge");
    dialog.set_pedestrian_model(QString::fromStdString(mengeinterface.MODEL));
    dialog.set_substeps(mengeinterface.SUB_STEPS);
    dialog.set_time_step(mengeinterface.TIME_STEP);
    dialog.set_pref_speed(mengeinterface._Common_pref_speed);
    dialog.set_pref_speed_stddev(mengeinterface._Common_pref_speed_stddev);
    dialog.set_max_speed(mengeinterface._Common_max_speed);
    dialog.set_max_accel(mengeinterface._Common_max_accel);
    dialog.set_simulation_percentile(mengeinterface._percentile_simulation_stopping_criterion);


    if(dialog.exec() == QDialog::Accepted)
    {
        mengeinterface.MODEL = dialog.pedestrian_model().toStdString();
        mengeinterface.SUB_STEPS = dialog.substeps();
        mengeinterface.TIME_STEP = dialog.time_step();
        mengeinterface._Common_pref_speed = dialog.pref_speed();
        mengeinterface._Common_pref_speed_stddev = dialog.pref_speed_stddev();
        mengeinterface._Common_max_speed = dialog.max_speed();
        mengeinterface._Common_max_accel = dialog.max_accel();
        mengeinterface._percentile_simulation_stopping_criterion = dialog.simulation_percentile();
    }
}



void MainWindow::settings_algorithm()
{
    dialog_algorithm_settings dialog;
    dialog.setWindowTitle(tr("Settings algorithm"));

    // set current values
    // algorithm parameters
    dialog.set_tabulistlength(timetable_algorithm.get_tabu_list_length());
    dialog.set_nbreplicationmengeincremental(timetable_algorithm.get_nb_eval_menge_incremental());
    dialog.set_maxnbevallocalmin(timetable_algorithm.get_nb_eval_local_minimum());
    dialog.set_algorithm_analyze_performance(timetable_algorithm.get_analyze_performance());
    dialog.set_replicationbudget_IS(timetable_algorithm.get_replication_budget_identification_step());
    dialog.set_replicationbudget_TS(timetable_algorithm.get_replication_budget_tabu_search());

    // machine learning
    dialog.set_machine_learning_method(machine_learning_interface.get_learning_method());
    dialog.set_nb_training_data(machine_learning_interface.get_number_training_data());

    // logger
    dialog.set_logger_verbose(global::_logger.is_verbose());



    // update new values
    if(dialog.exec() == QDialog::Accepted)
    {
        // algorithm parameters
        timetable_algorithm.set_tabu_list_length(dialog.get_tabulistlength());
        timetable_algorithm.set_nb_eval_menge_incremental(dialog.get_nbreplicationsmengeincremental());
        timetable_algorithm.set_analyze_performance(dialog.get_algorithm_analyze_performance());
        timetable_algorithm.set_nb_eval_local_minimum(dialog.get_maxnbevallocalmin());
        timetable_algorithm.set_replication_budget_identification_step(dialog.get_replicationbudget_IS());
        timetable_algorithm.set_replication_budget_tabu_search(dialog.get_replicationbudget_TS());

        // machine learning
        machine_learning_interface.set_learning_method(dialog.get_machine_learning_method());
        machine_learning_interface.set_number_training_data(dialog.get_nb_training_data());

        // logger
        global::_logger.set_verbose(dialog.is_logger_verbose());
    }
}














// RUN ALGORITHM
void MainWindow::run_algorithm()
{
    if(!timetable::data_exist)
    {
        QMessageBox::warning(this, tr("No timetable data!"), tr("No timetable data exist! \nImport data first!"),QMessageBox::Ok);
        return;
    }
    if(!building::data_exist)
    {
        QMessageBox::warning(this, tr("No building data!"), tr("No building data exist! \nImport data first!"),QMessageBox::Ok);
        return;
    }
    if(timetable_solution.is_empty())
    {
        QMessageBox::warning(this, tr("No solution!"), tr("No solution exists! \nImport a solution first!"),QMessageBox::Ok);
        return;
    }
    if(!ml_surrogate_paths.data_exist())
    {
        QMessageBox::warning(this, tr("No surrogate paths!"), tr("No surrogate paths exists! \nImport a surrogate paths map first!"),QMessageBox::Ok);
        return;
    }
    if(timetable_algorithm.timeslots_to_calculate_traveltimes_is_empty())
    {
        // standard all timeslots
        std::vector<int> timeslots;
        timeslots.reserve(timetable::nb_timeslots);
        for(int t = 0; t < timetable::nb_timeslots - 2; ++t) // starting from 0 and not last (== nb_timeslots-1)
            timeslots.push_back(t);

        timetable_algorithm.set_timeslots_to_calculate_traveltimes(timeslots);
    }

    dialog_start_algorithm dialog;
    dialog.setWindowTitle(tr("Run algorithm"));

    if(dialog.exec() == QDialog::Accepted)
    {
        // instance names
        global::_logger << global::logger::log_type::INFORMATION
                        << "Instance name: " << timetable::instance_name
                        << "\nBuilding name: " << building::instance_name;

        // initialize algorithm
        timetable_algorithm.set_Menge(&mengeinterface);
        timetable_algorithm.set_machine_learning_interface(&machine_learning_interface);
        timetable_algorithm.set_start_solution(timetable_solution);
        machine_learning_interface.set_menge_interface(&mengeinterface);
        machine_learning_interface.set_surrogate_paths(&ml_surrogate_paths);

        // set parameters
        timetable_algorithm.set_alpha_objective(dialog.get_alpha());
        machine_learning_interface.set_alpha_objective(dialog.get_alpha());

        // start timer
        start_time = std::chrono::system_clock::now();
        timer_comptime.start(100);

        // start computation
        future_watcher.setFuture(QtConcurrent::run(&timetable_algorithm, &timetable::tabu_search::run));
    }
}


void MainWindow::update_time()
{
    std::chrono::nanoseconds nanoseconds = std::chrono::system_clock::now() - start_time;
    double elapsed_time = nanoseconds.count() / 1000000000.0;
    this->statusBar()->showMessage(QString::number(elapsed_time));
}


void MainWindow::clear_time()
{
    this->statusBar()->clearMessage();
}


void MainWindow::algorithm_update_solution(timetable::solution sol)
{
    table_timeslot_location->clearContents();

    for(int l = 0; l < timetable::nb_events; ++l)
    {
        int timeslot = sol.event_timeslot(l);
        int room_new = sol.event_location(l);
        int room_old = timetable_solution.event_location(l); // timetable_solution contains the old solution

        QTableWidgetItem* lecture = new QTableWidgetItem;
        lecture->setText(timetable::get_event_name(l));
        lecture->setTextAlignment(Qt::AlignCenter);
        if(room_new != room_old)
            lecture->setBackground(QBrush(Qt::green, Qt::Dense3Pattern));
        else
            lecture->setBackground(QBrush(Qt::yellow, Qt::Dense3Pattern));
        table_timeslot_location->setItem(room_new, timeslot, lecture);
    }

    // save the new solution
    timetable_solution = sol;
}


void MainWindow::algorithm_finalize_solution_view()
{
    // 1. TIMESLOT-ROOM
    timetable_solution = timetable_algorithm.get_best_solution();

    table_timeslot_location->clearContents();

    // Fill in event
    for(int l = 0; l < timetable::nb_events; ++l)
    {
        int timeslot = timetable_solution.event_timeslot(l);
        int room = timetable_solution.event_location(l);

        QTableWidgetItem* lecture = new QTableWidgetItem;
        lecture->setText(timetable::get_event_name(l));
        lecture->setTextAlignment(Qt::AlignCenter);
        lecture->setBackground(QBrush(Qt::yellow, Qt::Dense3Pattern));
        table_timeslot_location->setItem(room, timeslot, lecture);
    }

    // 2. CURRICULUM-TIMESLOT
    // nothing needs to be updated because only rooms change
}


void MainWindow::save_solution()
{
    if(!timetable::data_exist)
    {
        QMessageBox::warning(this, tr("No timetable data!"), tr("No timetable data exist! \nImport data first!"),QMessageBox::Ok);
        return;
    }
    if(!building::data_exist)
    {
        QMessageBox::warning(this, tr("No building data!"), tr("No building data exist! \nImport data first!"),QMessageBox::Ok);
        return;
    }
    if(timetable_solution.is_empty())
    {
        QMessageBox::warning(this, tr("No solution!"), tr("No solution exists! \nImport a solution first!"),QMessageBox::Ok);
        return;
    }

    QString file_name = QFileDialog::getSaveFileName(this, tr("Save File"), QDir::currentPath(), tr("Text Files (*.txt)"));

    if(file_name.isEmpty())
        return;

    QFile file(file_name);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream stream(&file);

        stream << "Event_name\tTimeslot\tRoom";
        for(int e = 0; e < timetable::nb_events; ++e)
        {
            stream << "\n";
            stream << timetable::get_event_name(e);
            stream << "\t";
            stream << timetable_solution.event_timeslot(e);
            stream << "\t";
            stream << timetable_solution.event_location(e);
        }
    }
}




void MainWindow::output_algorithm_info(QString information)
{
    ui->textedit_algorithm_output->appendPlainText(information);
}




// COMPARE LEARNING METHODS SURROGATES
void MainWindow::compare_learning_methods_surrogates()
{
    if(!timetable::data_exist)
    {
        QMessageBox::warning(this, tr("No timetable data!"), tr("No timetable data exist! \nImport data first!"),QMessageBox::Ok);
        return;
    }
    if(!building::data_exist)
    {
        QMessageBox::warning(this, tr("No building data!"), tr("No building data exist! \nImport data first!"),QMessageBox::Ok);
        return;
    }
    if(!ml_surrogate_paths.data_exist())
    {
        QMessageBox::warning(this, tr("No surrogate paths!"), tr("No surrogate paths exists! \nImport a surrogate paths map first!"),QMessageBox::Ok);
        return;
    }


    dialog_compare_learning_methods dialog;
    dialog.setWindowTitle("Compare learning methods surrogates");

    if(dialog.exec() == QDialog::Accepted)
    {
        // instance names
        global::_logger << global::logger::log_type::INFORMATION
                        << "Instance name: " << timetable::instance_name
                        << "\nBuilding name: " << building::instance_name;

        // set menge
        machine_learning_interface.set_menge_interface(&mengeinterface);
        machine_learning_interface.set_surrogate_paths(&ml_surrogate_paths);

        // set parameters
        machine_learning_interface.set_number_training_data(dialog.get_nb_training_data());
        machine_learning_interface.set_alpha_objective(dialog.get_alpha());

        // start timer
        start_time = std::chrono::system_clock::now();
        timer_comptime.start(100);

        // start computation
        future_watcher_ml.setFuture(QtConcurrent::run(&machine_learning_interface, &ml::machine_learning_interface::compare_learning_methods));
    }
}




// EXHAUSTIVE SEARCH
void MainWindow::do_exhaustive_search()
{
    if(!timetable::data_exist)
    {
        QMessageBox::warning(this, tr("No timetable data!"), tr("No timetable data exist! \nImport data first!"),QMessageBox::Ok);
        return;
    }
    if(!building::data_exist)
    {
        QMessageBox::warning(this, tr("No building data!"), tr("No building data exist! \nImport data first!"),QMessageBox::Ok);
        return;
    }
    if(timetable_solution.is_empty())
    {
        QMessageBox::warning(this, tr("No solution!"), tr("No solution exists! \nImport a solution first!"),QMessageBox::Ok);
        return;
    }
    if(!ml_surrogate_paths.data_exist())
    {
        QMessageBox::warning(this, tr("No surrogate paths!"), tr("No surrogate paths exists! \nImport a surrogate paths map first!"),QMessageBox::Ok);
        return;
    }
    if(timetable_algorithm.timeslots_to_calculate_traveltimes_is_empty())
    {
        // standard all timeslots
        std::vector<int> timeslots;
        timeslots.reserve(timetable::nb_timeslots);
        for(int t = 0; t < timetable::nb_timeslots - 2; ++t) // starting from 0 and not last (== nb_timeslots-1)
            timeslots.push_back(t);

        timetable_algorithm.set_timeslots_to_calculate_traveltimes(timeslots);
    }

    dialog_start_algorithm dialog;
    dialog.setWindowTitle(tr("Do exhaustive search"));

    if(dialog.exec() == QDialog::Accepted)
    {
        // instance names
        global::_logger << global::logger::log_type::INFORMATION
                        << "Instance name: " << timetable::instance_name
                        << "\nBuilding name: " << building::instance_name;

        // initialize algorithm
        timetable_algorithm.set_Menge(&mengeinterface);
        timetable_algorithm.set_start_solution(timetable_solution);

        // set parameters
        timetable_algorithm.set_alpha_objective(dialog.get_alpha());
        machine_learning_interface.set_alpha_objective(dialog.get_alpha());

        // start timer
        start_time = std::chrono::system_clock::now();
        timer_comptime.start(100);

        // start computation
        future_watcher.setFuture(QtConcurrent::run(&timetable_algorithm, &timetable::tabu_search::run_exhaustive_search));
    }
}






// GENERATE TIMETABLE INSTANCE
void MainWindow::generate_timetable_instance()
{
    dialog_instance_generator dialog;
    dialog.setWindowTitle(tr("Generate dataset"));


    if(dialog.exec() == QDialog::Accepted)
    {
        try
        {
            int instance_size = dialog.get_instance_size();
            if(instance_size == 1 || instance_size == 2 || instance_size == 3)
            {
                instance_generator.generate_instance(dialog.get_instance_name(),
                                                     instance_size,
                                                     dialog.get_nb_locations());
            }
            else
            {
                instance_generator.generate_instance_custom(dialog.get_instance_name(),
                                                            dialog.get_nb_timeslots(),
                                                            dialog.get_nb_events_per_timeslot(),
                                                            dialog.get_nb_locations(),
                                                            dialog.get_min_nb_people_per_event(),
                                                            dialog.get_max_nb_people_per_event());
            }

            QString text = "Dataset " + dialog.get_instance_name() + " successfully generated!";
            QMessageBox::information(nullptr,
                                  tr(""),
                                  text,
                                  QMessageBox::Ok | QMessageBox::Default);
        }
        catch(const std::exception& ex)
        {
            QMessageBox::critical(nullptr,
                                  tr("Error occured!"),
                                  ex.what(),
                                  QMessageBox::Ok | QMessageBox::Default);
        }
    }
}







// ANALYZE SOLUTION
// Analyze evacuation for existing solution
void MainWindow::analyze_evacuation()
{
    if(!timetable::data_exist)
    {
        QMessageBox::warning(this, tr("No timetable data!"), tr("No timetable data exist! \nImport data first!"),QMessageBox::Ok);
        return;
    }
    if(!building::data_exist)
    {
        QMessageBox::warning(this, tr("No building data!"), tr("No building data exist! \nImport data first!"),QMessageBox::Ok);
        return;
    }
    if(timetable_solution.is_empty())
    {
        QMessageBox::warning(this, tr("No solution!"), tr("No solution exists! \nImport a solution first!"),QMessageBox::Ok);
        return;
    }

    menge_dialog_start_simulation dialog;
    dialog.setWindowTitle(tr("Analyse evacuation"));
    dialog.set_max_timeslot(timetable::nb_timeslots);

    if(dialog.exec() == QDialog::Accepted)
    {
        int timeslot = dialog.timeslot() - 1;

        double time = mengeinterface.visualise_evacuation(timetable_solution, timeslot);
        QString text = "Evacuation time = ";
        text.append(QString::number(time));
        text.append(" seconds");

        QMessageBox::information(this, tr("Simulation has finished!"), text, QMessageBox::Ok);
    }
}


// Analyze flows between consecutive lectures for existing solution
void MainWindow::analyze_flowsbetweenevents()
{
    if(!timetable::data_exist)
    {
        QMessageBox::warning(this, tr("No timetable data!"), tr("No timetable data exist! \nImport data first!"),QMessageBox::Ok);
        return;
    }
    if(!building::data_exist)
    {
        QMessageBox::warning(this, tr("No building data!"), tr("No building data exist! \nImport data first!"),QMessageBox::Ok);
        return;
    }
    if(timetable_solution.is_empty())
    {
        QMessageBox::warning(this, tr("No solution!"), tr("No solution exists! \nImport a solution first!"),QMessageBox::Ok);
        return;
    }

    menge_dialog_start_simulation dialog;
    dialog.setWindowTitle(tr("Analyse flows between consecutive events"));
    dialog.set_max_timeslot(timetable::nb_timeslots - 1);

    if(dialog.exec() == QDialog::Accepted)
    {
        int timeslot = dialog.timeslot() - 1;

        double time = mengeinterface.visualise_flows(timetable_solution, timeslot);
        QString text = "Travel time = ";
        text.append(QString::number(time));
        text.append(" seconds");

        QMessageBox::information(this, tr("Simulation has finished!"), text, QMessageBox::Ok);
    }
}


// Analyze entire existing solution
void MainWindow::analyze_solution_start()
{
    if(!timetable::data_exist)
    {
        QMessageBox::warning(this, tr("No timetable data!"), tr("No timetable data exist! \nImport data first!"),QMessageBox::Ok);
        return;
    }
    if(!building::data_exist)
    {
        QMessageBox::warning(this, tr("No building data!"), tr("No building data exist! \nImport data first!"),QMessageBox::Ok);
        return;
    }
    if(timetable_solution.is_empty())
    {
        QMessageBox::warning(this, tr("No solution!"), tr("No solution exists! \nImport a solution first!"),QMessageBox::Ok);
        return;
    }

    menge_dialog_start_analysis dialog;
    dialog.setWindowTitle(tr("Analyse solution"));

    if(dialog.exec() == QDialog::Accepted)
    {
        // 1. set parameters
        int replications = dialog.replications();

        // 2. start calculation
        progressdialog = new QProgressDialog("Analyzing current solution ...", "Cancel", 0, replications*(2*timetable::nb_timeslots-1));
        progressdialog->setMinimumDuration(0);
        connect(progressdialog, SIGNAL(canceled()), &mengeinterface, SLOT(halt()));

        future_watcher_ped.setFuture(QtConcurrent::run(&mengeinterface,
                                   &ped::menge_interface::start_calculations,
                                   timetable_solution,
                                   replications));
    }
}


void MainWindow::analyze_solution_update(int value)
{
    progressdialog->setValue(value);
}


void MainWindow::analyze_solution_finalize_view()
{
    // delete old series
    chart_evacuations->removeAllSeries();
    chart_traveltimes->removeAllSeries();


    if(mengeinterface.replications() >= 3)
    {
        chartset_evactime = new QtCharts::QBarSet("Average");
        chartset_evactime_CImin = new QtCharts::QBarSet("CI min");
        chartset_evactime_CImax = new QtCharts::QBarSet("CI max");

        chartset_traveltime = new QtCharts::QBarSet("Average");
        chartset_traveltime_CImin = new QtCharts::QBarSet("CI min");
        chartset_traveltime_CImax = new QtCharts::QBarSet("CI max");


        for(int t = 0; t < timetable::nb_timeslots; ++t) {
            double evacuationtime = mengeinterface.evacuation_time_avg(t);
            chartset_evactime->append(evacuationtime);
            evacuationtime = mengeinterface.evacuation_time_CImin(t);
            chartset_evactime_CImin->append(evacuationtime);
            evacuationtime = mengeinterface.evacuation_time_CImax(t);
            chartset_evactime_CImax->append(evacuationtime);
        }
        for(int t = 0; t < timetable::nb_timeslots - 1; ++t) {
            double traveltime = mengeinterface.travel_time_avg(t);
            chartset_traveltime->append(traveltime);
            traveltime = mengeinterface.travel_time_CImin(t);
            chartset_traveltime_CImin->append(traveltime);
            traveltime = mengeinterface.travel_time_CImax(t);
            chartset_traveltime_CImax->append(traveltime);
        }


        chartseries_evacuations = new QtCharts::QBarSeries();
        chartseries_evacuations->append(chartset_evactime_CImin);
        chartseries_evacuations->append(chartset_evactime);
        chartseries_evacuations->append(chartset_evactime_CImax);

        chartseries_traveltimes = new QtCharts::QBarSeries();
        chartseries_traveltimes->append(chartset_traveltime_CImin);
        chartseries_traveltimes->append(chartset_traveltime);
        chartseries_traveltimes->append(chartset_traveltime_CImax);


        chart_evacuations->addSeries(chartseries_evacuations);
        chart_evacuations->setTitle("Evacuation times current solution");
        chart_evacuations->setAnimationOptions(QtCharts::QChart::SeriesAnimations);

        chart_traveltimes->addSeries(chartseries_traveltimes);
        chart_traveltimes->setTitle("Travel times current solution");
        chart_traveltimes->setAnimationOptions(QtCharts::QChart::SeriesAnimations);


        QStringList categories;
        for(int i = 0; i < timetable::nb_timeslots; ++i) {
            QString text = "Timeslot ";
            text.append(QString::number(i+1));
            categories.append(text);
        }
        chartaxis_evacuations = new QtCharts::QBarCategoryAxis();
        chartaxis_evacuations->append(categories);
        chart_evacuations->createDefaultAxes();
        chart_evacuations->setAxisX(chartaxis_evacuations, chartseries_evacuations);

        categories.clear();
        for(int i = 0; i < timetable::nb_timeslots - 1; ++i) {
            QString text = "From timeslot ";
            text.append(QString::number(i+1));
            text.append(" to timeslot ");
            text.append(QString::number(i+2));
            categories.append(text);
        }
        chartaxis_traveltimes = new QtCharts::QBarCategoryAxis();
        chartaxis_traveltimes->append(categories);
        chart_traveltimes->createDefaultAxes();
        chart_traveltimes->setAxisX(chartaxis_traveltimes, chartseries_traveltimes);


        chart_evacuations->legend()->setVisible(true);
        chart_evacuations->legend()->setAlignment(Qt::AlignBottom);

        chart_traveltimes->legend()->setVisible(true);
        chart_traveltimes->legend()->setAlignment(Qt::AlignBottom);


        //chartview = new QtCharts::QChartView(chart);
        chartview_evacuations->setChart(chart_evacuations);
        //chartview->setRenderHint(QPainter::Antialiasing)

        chartview_traveltimes->setChart(chart_traveltimes);

    }
    else
    {
        chartset_evactime = new QtCharts::QBarSet("Evacuation times");
        chartset_traveltime = new QtCharts::QBarSet("Travel times");

        for(int t = 0; t < timetable::nb_timeslots; ++t) {
            double evacuationtime = mengeinterface.evacuation_time_avg(t);
            chartset_evactime->append(evacuationtime);
        }
        for(int t = 0; t < timetable::nb_timeslots - 1; ++t) {
            double traveltime = mengeinterface.travel_time_avg(t);
            chartset_traveltime->append(traveltime);
        }


        chartseries_evacuations = new QtCharts::QBarSeries();
        chartseries_evacuations->append(chartset_evactime);

        chartseries_traveltimes = new QtCharts::QBarSeries();
        chartseries_traveltimes->append(chartset_traveltime);


        chart_evacuations->addSeries(chartseries_evacuations);
        chart_evacuations->setTitle("Evacuation times current solution");
        chart_evacuations->setAnimationOptions(QtCharts::QChart::SeriesAnimations);

        chart_traveltimes->addSeries(chartseries_traveltimes);
        chart_traveltimes->setTitle("Travel times current solution");
        chart_traveltimes->setAnimationOptions(QtCharts::QChart::SeriesAnimations);


        QStringList categories;
        for(int i = 0; i < timetable::nb_timeslots; ++i) {
            QString text = "Timeslot ";
            text.append(QString::number(i+1));
            categories.append(text);
        }
        chartaxis_evacuations = new QtCharts::QBarCategoryAxis();
        chartaxis_evacuations->append(categories);
        chart_evacuations->createDefaultAxes();
        chart_evacuations->setAxisX(chartaxis_evacuations, chartseries_evacuations);

        categories.clear();
        for(int i = 0; i < timetable::nb_timeslots - 1; ++i) {
            QString text = "From timeslot ";
            text.append(QString::number(i+1));
            text.append(" to timeslot ");
            text.append(QString::number(i+2));
            categories.append(text);
        }
        chartaxis_traveltimes = new QtCharts::QBarCategoryAxis();
        chartaxis_traveltimes->append(categories);
        chart_traveltimes->createDefaultAxes();
        chart_traveltimes->setAxisX(chartaxis_traveltimes, chartseries_traveltimes);


        chart_evacuations->legend()->setVisible(true);
        chart_evacuations->legend()->setAlignment(Qt::AlignBottom);

        chart_traveltimes->legend()->setVisible(true);
        chart_traveltimes->legend()->setAlignment(Qt::AlignBottom);


        //chartview = new QtCharts::QChartView(chart);
        chartview_evacuations->setChart(chart_evacuations);
        //chartview->setRenderHint(QPainter::Antialiasing)

        chartview_traveltimes->setChart(chart_traveltimes);
    }
}





// ABOUT
void MainWindow::about()
{
    QString info = QStringLiteral("Surrogate-based Tabu Search Heuristic"
            "\n\nDeveloped by Hendrik Vermuyten"
            "\n\nThis product was written in C++ and uses the "
            "\nDlib C++ machine learning library, "
            "\nsee http://dlib.net/."
            "\n\nThe Open Source Licence of the Qt framework"
            "\nversion 5.10 is used for the graphical user "
            "\ninterface, see https://www.qt.io/.");

    QMessageBox::about(this, tr("About"), info);
}





// HOW TO
void MainWindow::how_to()
{
    help_menu menu;
    menu.setWindowTitle(QStringLiteral("Help"));
    menu.exec();
}





// CLOSE WINDOW
void MainWindow::closeEvent(QCloseEvent *event)  // show prompt when user wants to close app
{
    event->ignore();
    if (QMessageBox::Yes == QMessageBox::question(this,
                                                  tr("Close Confirmation"),
                                                  tr("Are you sure you want to exit?"),
                                                  QMessageBox::Yes | QMessageBox::No))
    {
        global::_logger.close();
        event->accept();
    }
}





// DESTRUCTOR
MainWindow::~MainWindow()
{
    delete ui;
}
