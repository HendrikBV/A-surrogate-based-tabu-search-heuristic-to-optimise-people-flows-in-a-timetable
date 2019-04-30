/*
 *	Code for the surrogate-based tabu search algorithm
 *  to optimise people flows in a timetable.
 *
 *	Code author: Hendrik Vermuyten
 */

/*!
 *	@file		mainwindow.h
 *  @author     Hendrik Vermuyten
 *	@brief		The main window of the GUI.
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtConcurrent/QtConcurrent>
#include <QFuture>
#include <QFutureWatcher>

#include <QString>
#include <QFile>
#include <QFileDialog>
#include <QProgressDialog>
#include <QDialog>
#include <QMessageBox>
#include <QInputDialog>
#include <QTextStream>
#include <QTextEdit>
#include <QPlainTextEdit>
#include <QTextDocument>
#include <QIcon>

#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QTabWidget>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QtCharts/QChartView>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QChart>
#include <QtCharts/QBarCategoryAxis>

#include <QTimer>
#include <QTimerEvent>

#include <QCloseEvent>
#include <QList>

#include <vector>
#include <chrono>

#include "building_data.h"
#include "scenario.h"
#include "menge_interface.h"
#include "timetable_global_data.h"
#include "timetable_solution.h"
#include "timetable_tabu_search.h"
#include "help_menu.h"
#include "dialog_algorithm_settings.h"
#include "dialog_start_algorithm.h"
#include "dialog_compare_learning_methods.h"
#include "menge_dialog_start_analysis.h"
#include "menge_dialog_start_simulation.h"
#include "menge_dialog_parameter_settings.h"
#include "machine_learning_interface.h"
#include "logger.h"
#include "dialog_instance_generator.h"
#include "timetable_instance_generator.h"


namespace Ui {
class MainWindow;
}

/*!
 *	@brief		The main window of the GUI.
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    /*!
     *	@brief		Constructor.
     */
    explicit MainWindow(QWidget *parent = 0);

    /*!
     *	@brief		Destructor.
     */
    ~MainWindow();



private:
    /*!
     *	@brief		Establish the Qt signal-slot connections for the GUI elements.
     */
    void make_connections();

    /*!
     *	@brief		Set the initial layout of the GUI main window.
     */
    void set_initial_layout();

    /*!
     *	@brief		Display the timetable data in a tree widget.
     */
    void build_tree_timetable_data();

    /*!
     *	@brief		Display the building data in a tree widget.
     */
    void build_tree_building_data();



private slots:
    /*!
     *	@brief		Swap two cells in the tablewidget of the solution tab.
     *  @param      previous_row        The row of the first cell in the tablewidget
     *  @param      previous_column     The column of the first cell in the tablewidget
     *  @param      current_row         The row of the second cell in the tablewidget
     *  @param      current_column      The column of the second cell in the tablewidget
     */
    void manually_swap_cells(int current_row, int current_column, int previous_row, int previous_column);

    /*!
     *	@brief		Enable actions in the main GUI.
     */
    void enable_actions_GUI();

    /*!
     *	@brief		Disable actions in the main GUI.
     */
    void disable_actions_GUI();

    /*!
     *	@brief		Prints the message when an error occurred in a subthread.
     */
    void print_error(QString message);

    /*!
     *	@brief		Set the initial layout for the solution pane.
     */
    void initialize_solution_view();

    /*!
     *	@brief		Display the initial solution after it has been imported from a file.
     */
    void update_solution_view();

    /*!
     *	@brief		Perform an exhaustive search.
     */
    void do_exhaustive_search();

    /*!
     *	@brief		Run the surrogate-based tabu search algorithm.
     */
    void run_algorithm();

    /*!
     *	@brief		Update the solution view with the new best solution found during algorithm run.
     *  @param      sol     The new best solution
     */
    void algorithm_update_solution(timetable::solution sol);

    /*!
     *	@brief		Display the best found solution after algorithm termination.
     */
    void algorithm_finalize_solution_view();

    /*!
     *	@brief		Show the current elapsed computation time in the status bar.
     */
    void update_time();

    /*!
     *	@brief		Delete the visualisation of the elapsed computation time.
     */
    void clear_time();

    /*!
     *	@brief		Save the best found solution to a txt-file.
     */
    void save_solution();

    /*!
     *	@brief		Compare the different learning methods to train the surrogate model.
     */
    void compare_learning_methods_surrogates();

    /*!
     *	@brief		Display the algorithm status in a text browser.
     *  @param      information     A text containing the algorithm status.
     */
    void output_algorithm_info(QString information);

    /*!
     *	@brief		Visualise the simulation of an evacuation.
     */
    void analyze_evacuation();

    /*!
     *	@brief		Visualise the simulation of the flows between events in two consecutive timeslots.
     */
    void analyze_flowsbetweenevents();

    /*!
     *	@brief		Start analysing the current solution with respect to evacuation and travel times.
     */
    void analyze_solution_start();

    /*!
     *	@brief		Update the progress bar during solution analysis.
     *  @param      value       The number of simulations already performed.
     */
    void analyze_solution_update(int value);

    /*!
     *	@brief		Show the results of the analysis (i.e. evacuation and travel times) in a bar chart.
     */
    void analyze_solution_finalize_view();

    /*!
     *	@brief		Generate a timetable instance to test the algorithm.
     */
    void generate_timetable_instance();

    /*!
     *	@brief		Import the timetable data from a txt-file.
     */
    void import_timetable_data();

    /*!
     *	@brief		Clear the timetable data.
     */
    void clear_timetable_data();

    /*!
     *	@brief		Import the building data from a txt-file.
     */
    void import_building_data();

    /*!
     *	@brief		Clear the building data.
     */
    void clear_building_data();

    /*!
     *	@brief		Import an initial solution from a txt-file.
     */
    void import_existing_solution();

    /*!
     *	@brief		Clear the current solution.
     */
    void clear_existing_solution();

    /*!
     *	@brief		Import the scenario data from a txt-file.
     */
    void import_scenario();

    /*!
     *	@brief		Clear the scenario data.
     */
    void clear_scenario();

    /*!
     *	@brief		Import a surrogate map from a txt-file.
     */
    void import_surrogate_map();

    /*!
     *	@brief		Clear the surrogate map.
     */
    void clear_surrogate_map();

    /*!
     *	@brief		Delete the trained surrogate model.
     */
    void clear_surrogates();

    /*!
     *	@brief		Import the relevant timeslots between which travel times need to be calculated from a txt-file.
     */
    void import_relevant_timeslots_traveltimes();

    /*!
     *	@brief		Change the settings of the Menge simulator.
     */
    void settings_menge();

    /*!
     *	@brief		Change the algorithm settings.
     */
    void settings_algorithm();

    /*!
     *	@brief		Display help menu.
     */
    void how_to();

    /*!
     *	@brief		Display information the GUI.
     */
    void about();

    /*!
     *	@brief		Reimplement the close event.
     */
    void closeEvent(QCloseEvent *event);



private:
    /*!
     *	@brief		The UI-elements.
     */
    Ui::MainWindow *ui;

    /*!
     *	@brief		The solution view where the rows refer to locations and the columns to timeslots.
     */
    QTableWidget* table_timeslot_location;

    /*!
     *	@brief		The solution view where the rows refer to eventgroups and the columns to timeslots.
     */
    QTableWidget* table_timeslot_eventgroup;

    /*!
     *	@brief		A chart view to visualise the evacuation times of the current solution.
     */
    QtCharts::QChartView* chartview_evacuations = nullptr;

    /*!
     *	@brief		A chart view to visualise the travel times of the current solution.
     */
    QtCharts::QChartView* chartview_traveltimes = nullptr;

    /*!
     *	@brief		A chart to visualise the evacuation times of the current solution.
     */
    QtCharts::QChart *chart_evacuations = nullptr;

    /*!
     *	@brief		A chart to visualise the travel times of the current solution.
     */
    QtCharts::QChart *chart_traveltimes = nullptr;

    /*!
     *	@brief		The bar to represent the mean evacuation times.
     */
    QtCharts::QBarSet *chartset_evactime;

    /*!
     *	@brief		The bar to represent the mean travel times.
     */
    QtCharts::QBarSet *chartset_traveltime;

    /*!
     *	@brief		The bar to represent the lower bound of the confidence interval for the evacuation times.
     */
    QtCharts::QBarSet *chartset_evactime_CImin;

    /*!
     *	@brief		The bar to represent the lower bound of the confidence interval for the travel times.
     */
    QtCharts::QBarSet *chartset_traveltime_CImin;

    /*!
     *	@brief		The bar to represent the upper bound of the confidence interval for the evacuation times.
     */
    QtCharts::QBarSet *chartset_evactime_CImax;

    /*!
     *	@brief		The bar to represent the upper bound of the confidence interval for the travel times.
     */
    QtCharts::QBarSet *chartset_traveltime_CImax;

    /*!
     *	@brief		A series to group the bars for each timeslot for the evacuation times.
     */
    QtCharts::QBarSeries *chartseries_evacuations;

    /*!
     *	@brief		A series to group the bars for each timeslot for the travel times.
     */
    QtCharts::QBarSeries *chartseries_traveltimes;

    /*!
     *	@brief		The horizontal axis for chart for the evacuation times.
     */
    QtCharts::QBarCategoryAxis *chartaxis_evacuations;

    /*!
     *	@brief		The horizontal axis for chart for the travel times.
     */
    QtCharts::QBarCategoryAxis *chartaxis_traveltimes;

    /*!
     *	@brief		A progress dialog to indicate the remaining time required for the solution analysis.
     */
    QProgressDialog *progressdialog;

    /*!
     *	@brief		The future watcher for the solution analysis by Menge.
     */
    QFutureWatcher<void> future_watcher_ped;

    /*!
     *	@brief		The future watcher for the algorithm.
     */
    QFutureWatcher<void> future_watcher;

    /*!
     *	@brief		The future watcher for the machine learning.
     */
    QFutureWatcher<void> future_watcher_ml;

    /*!
     *	@brief		A tabu search instance.
     */
    timetable::tabu_search timetable_algorithm;

    /*!
     *	@brief		The current solution or the best found solution.
     */
    timetable::solution timetable_solution;

    /*!
     *	@brief		The surrogate paths used in the machine learning for travel times.
     */
    ml::surrogate_paths ml_surrogate_paths;

    /*!
     *	@brief		A machine learning instance.
     */
    ml::machine_learning_interface machine_learning_interface;

    /*!
     *	@brief		A Menge interface instance.
     */
    ped::menge_interface mengeinterface;

    /*!
     *	@brief		An instance generator.
     */
    timetable::instance_generator instance_generator;

    /*!
     *	@brief		A timer to display the elapsed computation time in real time.
     */
    QTimer timer_comptime;

    /*!
     *	@brief		A time to keep track of the elapsed computation time.
     */
    std::chrono::system_clock::time_point start_time;
};

#endif // MAINWINDOW_H
