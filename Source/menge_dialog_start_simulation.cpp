#include "menge_dialog_start_simulation.h"
#include "ui_menge_dialog_start_simulation.h"

menge_dialog_start_simulation::menge_dialog_start_simulation(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::menge_dialog_start_simulation)
{
    ui->setupUi(this);
}

menge_dialog_start_simulation::~menge_dialog_start_simulation()
{
    delete ui;
}

int menge_dialog_start_simulation::timeslot() const
{
    return ui->spinbox_timeslot->value();
}

void menge_dialog_start_simulation::set_max_timeslot(int val)
{
    ui->spinbox_timeslot->setMaximum(val);
}
