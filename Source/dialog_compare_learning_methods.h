/*
 *	Code for the surrogate-based tabu search algorithm
 *  to optimise people flows in a timetable.
 *
 *	Code author: Hendrik Vermuyten
 */

/*!
 *	@file		dialog_compare_learning_methods.h
 *  @author     Hendrik Vermuyten
 *	@brief		Dialog to change the settings for comparing machine learning methods.
 */

#ifndef DIALOG_COMPARE_LEARNING_METHODS_H
#define DIALOG_COMPARE_LEARNING_METHODS_H

#include <QDialog>

namespace Ui {
class dialog_compare_learning_methods;
}

/*!
 *	@brief		Dialog to change the settings for comparing machine learning methods.
 */
class dialog_compare_learning_methods : public QDialog
{
    Q_OBJECT

public:
    /*!
     *	@brief		Constructor.
     */
    explicit dialog_compare_learning_methods(QWidget *parent = 0);

    /*!
     *	@brief		Destructor.
     */
    ~dialog_compare_learning_methods();

    /*!
     *	@brief		Get the number of training data that is used to compare the machine learning methods.
     *  @returns    The number of training data used to compare the machine learning methods.
     */
    int get_nb_training_data() const;

    /*!
     *	@brief		Set the number of training data that is used to compare the machine learning methods.
     *  @param      value       The number of training data used to compare the machine learning methods.
     */
    void set_nb_training_data(int value);

    /*!
     *	@brief		Get the objective function coefficient alpha (1 = evacuations only; 0 = travels only).
     *  @returns    The objective function coefficient alpha.
     */
    double get_alpha() const;

    /*!
     *	@brief		Set the objective function coefficient alpha (1 = evacuations only; 0 = travels only).
     *  @returns    value       The value for the objective function coefficient alpha.
     */
    void set_alpha(double value);

private:
    /*!
     *	@brief		The UI-elements.
     */
    Ui::dialog_compare_learning_methods *ui;
};

#endif // DIALOG_COMPARE_LEARNING_METHODS_H
