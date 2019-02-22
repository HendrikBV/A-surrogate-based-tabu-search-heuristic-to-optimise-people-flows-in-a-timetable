/*
 *	Code for the surrogate-based tabu search algorithm
 *  to optimise people flows in a timetable.
 *
 *	Code author: Hendrik Vermuyten
 */

/*!
 *	@file		menge_dialog_start_analysis.h
 *  @author     Hendrik Vermuyten
 *	@brief		Dialog to start the analysis of the current solution with Menge.
 */

#ifndef MENGE_DIALOG_START_ANALYSIS_H
#define MENGE_DIALOG_START_ANALYSIS_H

#include <QDialog>

namespace Ui {
class menge_dialog_start_analysis;
}

/*!
 *	@brief		Dialog to start the analysis of the current solution with Menge.
 */
class menge_dialog_start_analysis : public QDialog
{
    Q_OBJECT

public:
    /*!
     *	@brief		Constructor.
     */
    explicit menge_dialog_start_analysis(QWidget *parent = 0);

    /*!
     *	@brief		Destructor.
     */
    ~menge_dialog_start_analysis();

    /*!
     *	@brief		Return the number of replications used in the analysis.
     *  @returns    The number of replications.
     */
    int replications() const;

private:
    /*!
     *	@brief		The UI-elements.
     */
    Ui::menge_dialog_start_analysis *ui;
};

#endif // MENGE_DIALOG_START_ANALYSIS_H
