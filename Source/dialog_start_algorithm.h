/*
 *	Code for the surrogate-based tabu search algorithm
 *  to optimise people flows in a timetable.
 *
 *	Code author: Hendrik Vermuyten
 */

/*!
 *	@file		dialog_start_algorithm.h
 *  @author     Hendrik Vermuyten
 *	@brief		Dialog to start an algorithm run.
 */

#ifndef DIALOG_START_ALGORITHM_H
#define DIALOG_START_ALGORITHM_H

#include <QDialog>

namespace Ui {
class dialog_start_algorithm;
}

/*!
 *	@brief		Dialog to start an algorithm run.
 */
class dialog_start_algorithm : public QDialog
{
    Q_OBJECT

public:
    /*!
     *	@brief		Constructor.
     */
    explicit dialog_start_algorithm(QWidget *parent = 0);

    /*!
     *	@brief		Destructor.
     */
    ~dialog_start_algorithm();

    /*!
     *	@brief		Get the objective function coefficient alpha (1 = evacuations only; 0 = travels only).
     *  @returns    The objective function coefficient alpha.
     */
    double get_alpha();

private slots:
    /*!
     *	@brief		Updates the value of alpha if the value of (1-alpha) is changed.
     */
    void update_spinbox_alpha(double);

    /*!
     *	@brief		Updates the value of (1-alpha) if the value of alpha is changed.
     */
    void update_spinbox_oneminusalpha(double);

private:
    /*!
     *	@brief		The UI-elements.
     */
    Ui::dialog_start_algorithm *ui;
};

#endif // DIALOG_START_ALGORITHM_H
