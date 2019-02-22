#include "dialog_compare_learning_methods.h"
#include "ui_dialog_compare_learning_methods.h"

dialog_compare_learning_methods::dialog_compare_learning_methods(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::dialog_compare_learning_methods)
{
    ui->setupUi(this);
}

dialog_compare_learning_methods::~dialog_compare_learning_methods()
{
    delete ui;
}



int dialog_compare_learning_methods::get_nb_training_data() const
{
    return ui->spinBox_nbtrainingdata->value();
}


void dialog_compare_learning_methods::set_nb_training_data(int value)
{
    ui->spinBox_nbtrainingdata->setValue(value);
}


double dialog_compare_learning_methods::get_alpha() const
{
    if(ui->radioButton_evacuations->isChecked())
        return 1.0;
    else if(ui->radioButton_travels->isChecked())
        return 0.0;
    else //if (ui->radioButton_both->isChecked())
        return 0.5;
}


void dialog_compare_learning_methods::set_alpha(double value)
{
    if(value > 0.99)
        ui->radioButton_evacuations->setChecked(true);
    else if (value < 0.01)
        ui->radioButton_travels->setChecked(true);
    else
        ui->radioButton_both->setChecked(true);
}
