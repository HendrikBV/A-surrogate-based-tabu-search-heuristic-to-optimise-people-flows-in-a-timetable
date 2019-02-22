#include "menge_dialog_parameter_settings.h"
#include "ui_menge_dialog_parameter_settings.h"

menge_dialog_parameter_settings::menge_dialog_parameter_settings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::menge_dialog_parameter_settings)
{
    ui->setupUi(this);

    ui->comboBox->addItem(QStringLiteral("ORCA"));
    ui->comboBox->addItem(QStringLiteral("PedVO"));
    ui->comboBox->addItem(QStringLiteral("GCF"));
    ui->comboBox->addItem(QStringLiteral("Helbing"));
    ui->comboBox->addItem(QStringLiteral("Karamouzas"));
    ui->comboBox->addItem(QStringLiteral("Zanlungo"));

    ui->comboBox->setEditable(false);

    connect(ui->pushButton_main_reset, SIGNAL(clicked(bool)), this, SLOT(reset_values()));
}

namespace
{
    constexpr double pref_speed_default = 1.34;
    constexpr double pref_speed_stddev_default = 0.26;
    constexpr double max_speed_default = 1.74;
    constexpr double max_accel_default = 50.0;
    constexpr float time_step_default = 0.02f;
    constexpr size_t sub_steps_default = 0;
}


void menge_dialog_parameter_settings::reset_values()
{
    ui->comboBox->setCurrentIndex(0);
    ui->spinBox_main_substeps->setValue(sub_steps_default);
    ui->doubleSpinBox_main_timestep->setValue(time_step_default);
    ui->doubleSpinBox_main_prefspeed->setValue(pref_speed_default);
    ui->doubleSpinBox_main_prefspeedstddev->setValue(pref_speed_stddev_default);
    ui->doubleSpinBox_main_maxspeed->setValue(max_speed_default);
    ui->doubleSpinBox_main_maxaccel->setValue(max_accel_default);
}



QString menge_dialog_parameter_settings::pedestrian_model() const
{
    QString model = ui->comboBox->currentText();
    model = model.toLower();
    return model;
}

int menge_dialog_parameter_settings::substeps() const
{
    return ui->spinBox_main_substeps->value();
}

double menge_dialog_parameter_settings::time_step() const
{
    return ui->doubleSpinBox_main_timestep->value();
}

double menge_dialog_parameter_settings::pref_speed() const
{
    return ui->doubleSpinBox_main_prefspeed->value();
}

double menge_dialog_parameter_settings::pref_speed_stddev() const
{
    return ui->doubleSpinBox_main_prefspeedstddev->value();
}

double menge_dialog_parameter_settings::max_speed() const
{
    return ui->doubleSpinBox_main_maxspeed->value();
}

double menge_dialog_parameter_settings::max_accel() const
{
    return ui->doubleSpinBox_main_maxaccel->value();
}



void menge_dialog_parameter_settings::set_pedestrian_model(const QString &name)
{
    for(int i = 0; i < ui->comboBox->count(); ++i)
    {
        QString model = ui->comboBox->itemText(i);
        model = model.toLower();
        if(model == name)
        {
            ui->comboBox->setCurrentIndex(i);
            break;
        }
    }
}

void menge_dialog_parameter_settings::set_substeps(int val)
{
    ui->spinBox_main_substeps->setValue(val);
}

void menge_dialog_parameter_settings::set_time_step(double val)
{
    ui->doubleSpinBox_main_timestep->setValue(val);
}

void menge_dialog_parameter_settings::set_pref_speed(double val)
{
    ui->doubleSpinBox_main_prefspeed->setValue(val);
}

void menge_dialog_parameter_settings::set_pref_speed_stddev(double val)
{
    ui->doubleSpinBox_main_prefspeedstddev->setValue(val);
}

void menge_dialog_parameter_settings::set_max_speed(double val)
{
    ui->doubleSpinBox_main_maxspeed->setValue(val);
}

void menge_dialog_parameter_settings::set_max_accel(double val)
{
    ui->doubleSpinBox_main_maxaccel->setValue(val);
}





menge_dialog_parameter_settings::~menge_dialog_parameter_settings()
{
    delete ui;
}
