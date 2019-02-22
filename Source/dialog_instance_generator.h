/*
 *	Code for the surrogate-based tabu search algorithm
 *  to optimise people flows in a timetable.
 *
 *	Code author: Hendrik Vermuyten
 */

/*!
 *	@file		dialog_instance_generator.h
 *  @author     Hendrik Vermuyten
 *	@brief		Dialog to set the parameters of the problem instance that is to be generated.
 */

#ifndef DIALOG_INSTANCE_GENERATOR_H
#define DIALOG_INSTANCE_GENERATOR_H

#include <QDialog>

namespace Ui {
class dialog_instance_generator;
}

/*!
 *	@brief		Dialog to set the parameters of the problem instance that is to be generated.
 */
class dialog_instance_generator : public QDialog
{
    Q_OBJECT

public:
    /*!
     *	@brief		Constructor.
     */
    explicit dialog_instance_generator(QWidget *parent = 0);

    /*!
     *	@brief		Destructor.
     */
    ~dialog_instance_generator();

    /*!
     *	@brief		Get the name of the instance.
     *  @returns    The name of the instance.
     */
    QString get_instance_name() const;

    /*!
     *	@brief		Get the size of the instance.
     *  @returns    The size of the instance.
     */
    int get_instance_size() const;

    /*!
     *	@brief		Get the number of locations.
     *  @returns    The number of locations.
     */
    int get_nb_locations() const;

    /*!
     *	@brief		Get the number of timeslots.
     *  @returns    The number of timeslots.
     */
    int get_nb_timeslots() const;

    /*!
     *	@brief		Get the number of events per timeslot.
     *  @returns    The number of events per timeslot.
     */
    int get_nb_events_per_timeslot() const;

    /*!
     *	@brief		Get the minimum number of people per event.
     *  @returns    The minimum number of people per event.
     */
    int get_min_nb_people_per_event() const;

    /*!
     *	@brief		Get the maximum number of people per event.
     *  @returns    The maximum number of people per event.
     */
    int get_max_nb_people_per_event() const;

private:
    /*!
     *	@brief		The UI-elements.
     */
    Ui::dialog_instance_generator *ui;
};

#endif // DIALOG_INSTANCE_GENERATOR_H
