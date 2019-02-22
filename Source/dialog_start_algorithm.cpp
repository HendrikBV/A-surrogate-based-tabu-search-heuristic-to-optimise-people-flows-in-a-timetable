#include "dialog_start_algorithm.h"
#include "ui_dialog_start_algorithm.h"

dialog_start_algorithm::dialog_start_algorithm(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::dialog_start_algorithm)
{
    ui->setupUi(this);

    connect(ui->doubleSpinBox_alpha, SIGNAL(valueChanged(double)), this, SLOT(update_spinbox_oneminusalpha(double)));
    connect(ui->doubleSpinBox_oneminusalpha, SIGNAL(valueChanged(double)), this, SLOT(update_spinbox_alpha(double)));
}





double dialog_start_algorithm::get_alpha()
{
    return ui->doubleSpinBox_alpha->value();
}



void dialog_start_algorithm::update_spinbox_alpha(double newvalue)
{
    ui->doubleSpinBox_alpha->setValue(1-newvalue);
}

void dialog_start_algorithm::update_spinbox_oneminusalpha(double newvalue)
{
    ui->doubleSpinBox_oneminusalpha->setValue(1-newvalue);
}




dialog_start_algorithm::~dialog_start_algorithm()
{
    delete ui;
}
