/*
 *	Code for the surrogate-based tabu search algorithm
 *  to optimise people flows in a timetable.
 *
 *	Code author: Hendrik Vermuyten
 */

/*!
 *	@file		menge_dialog_start_simulation.h
 *  @author     Hendrik Vermuyten
 *	@brief		Dialog to start a visualised simulation with Menge.
 */

#ifndef MENGE_DIALOG_START_SIMULATION_H
#define MENGE_DIALOG_START_SIMULATION_H

#include <QDialog>

namespace Ui {
class menge_dialog_start_simulation;
}

/*!
 *	@brief		Dialog to start a visualised simulation with Menge.
 */
class menge_dialog_start_simulation : public QDialog
{
    Q_OBJECT

public:
    /*!
     *	@brief		Constructor.
     */
    explicit menge_dialog_start_simulation(QWidget *parent = 0);

    /*!
     *	@brief		Destructor.
     */
    ~menge_dialog_start_simulation();

    /*!
     *	@brief		Return the timeslot for which a simulation is to be executed.
     *  @returns    The timeslot for which a simulation is to be executed.
     */
    int timeslot() const;

    /*!
     *	@brief		Set the range of timeslots based on the largest timeslot in the problem instance data.
     *  @param      val     The largest timeslot in the problem instance data.
     */
    void set_max_timeslot(int val);

private:
    /*!
     *	@brief		The UI-elements.
     */
    Ui::menge_dialog_start_simulation *ui;
};

#endif // MENGE_DIALOG_START_SIMULATION_H
