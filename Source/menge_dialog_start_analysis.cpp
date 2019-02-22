#include "menge_dialog_start_analysis.h"
#include "ui_menge_dialog_start_analysis.h"

menge_dialog_start_analysis::menge_dialog_start_analysis(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::menge_dialog_start_analysis)
{
    ui->setupUi(this);
}

menge_dialog_start_analysis::~menge_dialog_start_analysis()
{
    delete ui;
}

int menge_dialog_start_analysis::replications() const
{
    return ui->spinbox_replications->value();
}
