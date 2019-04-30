/*
 *	Code for the surrogate-based tabu search algorithm
 *  to optimise people flows in a timetable.
 *
 *	Code author: Hendrik Vermuyten
 */

/*!
 *	@file		menge_dialog_parameter_settings.h
 *  @author     Hendrik Vermuyten
 *	@brief		Dialog to change the parameter settings of the Menge simulator.
 */

#ifndef MENGE_DIALOG_PARAMETER_SETTINGS_H
#define MENGE_DIALOG_PARAMETER_SETTINGS_H

#include <QDialog>

namespace Ui {
class menge_dialog_parameter_settings;
}

/*!
 *	@brief		Dialog to change the parameter settings of the Menge simulator.
 */
class menge_dialog_parameter_settings : public QDialog
{
    Q_OBJECT

public:
    /*!
     *	@brief		Constructor.
     */
    explicit menge_dialog_parameter_settings(QWidget *parent = 0);

    /*!
     *	@brief		Destructor.
     */
    ~menge_dialog_parameter_settings();

    /*!
     *	@brief		Return the name of the pedestrian model used in the simulation.
     *  @returns    The name of the model.
     */
    QString pedestrian_model() const;

    /*!
     *	@brief		Return the number of substeps used in the simulation.
     *  @returns    The number of substeps used in the simulation.
     */
    int substeps() const;

    /*!
     *	@brief		Return the time step used in the simulation.
     *  @returns    The time step used in the simulation.
     */
    double time_step() const;

    /*!
     *	@brief		Return the mean preferred speed of agents in the simulation.
     *  @returns    The mean preferred speed of agents in the simulation.
     */
    double pref_speed() const;

    /*!
     *	@brief		Return the standard deviation of the preferred speed of agents in the simulation.
     *  @returns    The standard deviation of the preferred speed of agents in the simulation.
     */
    double pref_speed_stddev() const;

    /*!
     *	@brief		Return the maximum speed of agents in the simulation.
     *  @returns    The maximum speed of agents in the simulation.
     */
    double max_speed() const;

    /*!
     *	@brief		Return the maximum acceleration of agents in the simulation.
     *  @returns    The maximum acceleration of agents in the simulation.
     */
    double max_accel() const;

    /*!
     *	@brief		Return the simulation percentile, i.e. when x% of people have reached their destination, the simulation ends.
     *  @returns    The simulation percentile.
     */
    double simulation_percentile() const;



    /*!
     *	@brief		Set the pedestrian model used in the simulation.
     *  @param      name        The name the pedestrian model used in the simulation.
     */
    void set_pedestrian_model(const QString &name);

    /*!
     *	@brief		Set the number of substeps used in the simulation.
     *  @param      val     The number of substeps used in the simulation.
     */
    void set_substeps(int val);

    /*!
     *	@brief		Set the time step used in the simulation.
     *  @param      val     The time step used in the simulation.
     */
    void set_time_step(double val);

    /*!
     *	@brief		Set the mean preferred speed of agents in the simulation.
     *  @param      val     The mean preferred speed of agents in the simulation.
     */
    void set_pref_speed(double val);

    /*!
     *	@brief		Set the standard deviation of the preferred speed of agents in the simulation.
     *  @param      val     The standard deviation of the preferred speed of agents in the simulation.
     */
    void set_pref_speed_stddev(double val);

    /*!
     *	@brief		Set the maximum speed of agents in the simulation.
     *  @param      val     The maximum speed of agents in the simulation.
     */
    void set_max_speed(double val);

    /*!
     *	@brief		Set the maximum acceleration of agents in the simulation.
     *  @param      val     The maximum acceleration of agents in the simulation.
     */
    void set_max_accel(double val);

    /*!
     *	@brief		Set the simulation percentile, i.e. when x% of people have reached their destination, the simulation ends.
     *  @returns    val     The simulation percentile.
     */
    void set_simulation_percentile(double val);



private slots:
    /*!
     *	@brief		Reset the parameters to their standard values.
     */
    void reset_values();



private:
    /*!
     *	@brief		The UI-elements.
     */
    Ui::menge_dialog_parameter_settings *ui;
};

#endif // MENGE_DIALOG_PARAMETER_SETTINGS_H
