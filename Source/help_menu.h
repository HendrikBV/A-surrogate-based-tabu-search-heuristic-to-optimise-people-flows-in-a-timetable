/*
 *	Code for the surrogate-based tabu search algorithm
 *  to optimise people flows in a timetable.
 *
 *	Code author: Hendrik Vermuyten
 */

/*!
 *	@file		help_menu.h
 *  @author     Hendrik Vermuyten
 *	@brief		Widget to display a help menu for using the GUI.
 */

#ifndef HELP_MENU_H
#define HELP_MENU_H

#include <QDialog>

namespace Ui {
class help_menu;
}

/*!
 *	@brief		Widget to display a help menu for using the GUI.
 */
class help_menu : public QDialog
{
    Q_OBJECT

public:
    /*!
     *	@brief		Constructor.
     */
    explicit help_menu(QWidget *parent = 0);

    /*!
     *	@brief		Destructor.
     */
    ~help_menu();

private:
    /*!
     *	@brief		The UI-elements.
     */
    Ui::help_menu *ui;
};

#endif // HELP_MENU_H
