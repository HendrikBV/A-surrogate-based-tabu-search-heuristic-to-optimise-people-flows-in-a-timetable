#include "dialog_instance_generator.h"
#include "ui_dialog_instance_generator.h"

dialog_instance_generator::dialog_instance_generator(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::dialog_instance_generator)
{
    ui->setupUi(this);

    // disable all spinboxes for custom
    ui->spinBox_nbtimeslots->setDisabled(true);
    ui->spinBox_nbeventspertimeslot->setDisabled(true);
    ui->spinBox_minnbpeople->setDisabled(true);
    ui->spinBox_maxnbpeople->setDisabled(true);

    // establish connections between enables and toggled for custom
    connect(ui->radioButton_custom, SIGNAL(toggled(bool)), ui->spinBox_nbtimeslots, SLOT(setEnabled(bool)));
    connect(ui->radioButton_custom, SIGNAL(toggled(bool)), ui->spinBox_nbeventspertimeslot, SLOT(setEnabled(bool)));
    connect(ui->radioButton_custom, SIGNAL(toggled(bool)), ui->spinBox_minnbpeople, SLOT(setEnabled(bool)));
    connect(ui->radioButton_custom, SIGNAL(toggled(bool)), ui->spinBox_maxnbpeople, SLOT(setEnabled(bool)));
}

QString dialog_instance_generator::get_instance_name() const
{
    return ui->lineedit_instancename->text();
}

int dialog_instance_generator::get_instance_size() const
{
    if(ui->radioButton_small->isChecked())
        return 1;
    else if(ui->radioButton_medium->isChecked())
        return 2;
    else if(ui->radioButton_large->isChecked())
        return 3;
    else
        return 0;
}

int dialog_instance_generator::get_nb_locations() const
{
    return ui->spinBox_nblocations->value();
}

int dialog_instance_generator::get_nb_timeslots() const
{
    return ui->spinBox_nbtimeslots->value();
}

int dialog_instance_generator::get_nb_events_per_timeslot() const
{
    return ui->spinBox_nbeventspertimeslot->value();
}

int dialog_instance_generator::get_min_nb_people_per_event() const
{
    return ui->spinBox_minnbpeople->value();
}

int dialog_instance_generator::get_max_nb_people_per_event() const
{
    return ui->spinBox_maxnbpeople->value();
}

dialog_instance_generator::~dialog_instance_generator()
{
    delete ui;
}
