/*
 *	Code for the surrogate-based tabu search algorithm
 *  to optimise people flows in a timetable.
 *
 *	Code author: Hendrik Vermuyten
 */

/*!
 *	@file		dialog_algorithm_settings.h
 *  @author     Hendrik Vermuyten
 *	@brief		Dialog to change the algorithm settings.
 */

#ifndef DIALOG_ALGORITHM_SETTINGS_H
#define DIALOG_ALGORITHM_SETTINGS_H

#include <QDialog>
#include "machine_learning_interface.h"
#include "timetable_tabu_search.h"
#include "logger.h"

namespace Ui {
class dialog_algorithm_settings;
}

/*!
 *	@brief		Dialog to change the algorithm settings.
 */
class dialog_algorithm_settings : public QDialog
{
    Q_OBJECT

public:
    /*!
     *	@brief		Constructor.
     */
    explicit dialog_algorithm_settings(QWidget *parent = 0);

    /*!
     *	@brief		Destructor.
     */
    ~dialog_algorithm_settings();



    /*!
     *	@brief		Set the length of the tabu list.
     *  @param      length      The length of the tabu list.
     */
    void set_tabulistlength(int length);

    /*!
     *	@brief		Get the length of the tabu list.
     *  @returns    The length of the tabu list.
     */
    int get_tabulistlength() const;

    /*!
     *	@brief		Set the number of replications of Menge per evaluation of a candidate solution.
     *  @param      nb      The number of replications.
     */
    void set_nbreplicationmengeincremental(int nb);

    /*!
     *	@brief		Get the number of replications of Menge per evaluation of a candidate solution.
     *  @returns    The number of replications.
     */
    int get_nbreplicationsmengeincremental() const;

    /*!
     *	@brief		Set the maximum number of evaluations before concluding we're in a local minimum.
     *  @param      nb      The number of evaluations before we conclude we're in a local minimum.
     */
    void set_maxnbevallocalmin(int nb);

    /*!
     *	@brief		Get the maximum number of evaluations before concluding we're in a local minimum.
     *  @returns    The number of evaluations before we conclude we're in a local minimum.
     */
    int get_maxnbevallocalmin() const;

    /*!
     *	@brief		Set the replication budget for the tabu search.
     *  @param      nb      The replication budget for the tabu search.
     */
    void set_replicationbudget_TS(int nb);

    /*!
     *	@brief		Get the replication budget for the tabu search.
     *  @returns    The replication budget for the tabu search.
     */
    int get_replicationbudget_TS() const;

    /*!
     *	@brief		Set the replication budget for the identification step.
     *  @param      nb      The replication budget for the identification step.
     */
    void set_replicationbudget_IS(int nb);

    /*!
     *	@brief		Get the replication budget for the identification step.
     *  @returns    The replication budget for the identification step.
     */
    int get_replicationbudget_IS() const;



    /*!
     *	@brief		Set the machine learning method.
     *  @param      lm      The machine learning method.
     */
    void set_machine_learning_method(ml::learning_method lm);

    /*!
     *	@brief		Get the machine learning method.
     *  @returns    The machine learning method.
     */
    ml::learning_method get_machine_learning_method() const;

    /*!
     *	@brief		Set the number of training data used to train the surrogate model.
     *  @param      value      The number of training data.
     */
    void set_nb_training_data(int value);

    /*!
     *	@brief		Get the number of training data used to train the surrogate model.
     *  @returns    The number of training data.
     */
    int get_nb_training_data() const;



    /*!
     *	@brief		Set the logger to verbose.
     *  @param      verbose     True if the logger is verbose.
     */
    void set_logger_verbose(bool verbose);

    /*!
     *	@brief		Return whether the logger is set to verbose.
     *  @returns    True if the logger is verbose.
     */
    bool is_logger_verbose() const;



    /*!
     *	@brief		Specify whether the performance of the surrogate models should be analysed during algorithm run.
     *  @param      analyse_performance      True if the performance of the surrogate models should be analysed during algorithm run.
     */
    void set_algorithm_analyze_performance(bool analyse_performance);

    /*!
     *	@brief		Return whether the performance of the surrogate models should be analysed during algorithm run.
     *  @returns    True if the performance of the surrogate models should be analysed during algorithm run.
     */
    bool get_algorithm_analyze_performance() const;


private slots:
    /*!
     *	@brief		Resets the parameters for the tabu search to their standard values.
     */
    void reset_values_TS();

private:
    /*!
     *	@brief		The UI-elements.
     */
    Ui::dialog_algorithm_settings *ui;
};

#endif // DIALOG_ALGORITHM_SETTINGS_H
