#include "dialog_algorithm_settings.h"
#include "ui_dialog_algorithm_settings.h"


dialog_algorithm_settings::dialog_algorithm_settings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::dialog_algorithm_settings)
{
    ui->setupUi(this);


    ui->comboBox_learningmethod->addItem(QStringLiteral("KRR Radial Basis Kernel"));
    ui->comboBox_learningmethod->addItem(QStringLiteral("KRR Histogram Intersection Kernel"));
    ui->comboBox_learningmethod->addItem(QStringLiteral("KRR Linear Kernel"));
    ui->comboBox_learningmethod->addItem(QStringLiteral("KRR Quadratic Kernel"));
    ui->comboBox_learningmethod->addItem(QStringLiteral("KRR Cubic Kernel"));

    ui->comboBox_learningmethod->addItem(QStringLiteral("SVR Radial Basis Kernel"));
    ui->comboBox_learningmethod->addItem(QStringLiteral("SVR Histogram Intersection Kernel"));
    ui->comboBox_learningmethod->addItem(QStringLiteral("SVR Linear Kernel"));
    ui->comboBox_learningmethod->addItem(QStringLiteral("SVR Quadratic Kernel"));
    ui->comboBox_learningmethod->addItem(QStringLiteral("SVR Cubic Kernel"));

    ui->comboBox_learningmethod->addItem(QStringLiteral("RVM Radial Basis Kernel"));
    ui->comboBox_learningmethod->addItem(QStringLiteral("RVM Histogram Intersection Kernel"));
    ui->comboBox_learningmethod->addItem(QStringLiteral("RVM Linear Kernel"));
    ui->comboBox_learningmethod->addItem(QStringLiteral("RVM Quadratic Kernel"));
    ui->comboBox_learningmethod->addItem(QStringLiteral("RVM Cubic Kernel"));

    ui->comboBox_learningmethod->setEditable(false);


    // connections
    connect(ui->pushButton_resetvaluesTS, SIGNAL(clicked(bool)), this, SLOT(reset_values_TS()));
}




// algorithm parameters
void dialog_algorithm_settings::set_maxnbevallocalmin(int nb)
{
    ui->spinBox_max_candidates_per_iteration->setValue(nb);
}


int dialog_algorithm_settings::get_maxnbevallocalmin() const
{
    return ui->spinBox_max_candidates_per_iteration->value();
}


void dialog_algorithm_settings::set_tabulistlength(int length)
{
    ui->spinBox_tabulistlength->setValue(length);
}


int dialog_algorithm_settings::get_tabulistlength() const
{
    return ui->spinBox_tabulistlength->value();
}


void dialog_algorithm_settings::set_nbreplicationmengeincremental(int nb)
{
    ui->spinBox_nbreplicationsmengeincremental->setValue(nb);
}


int dialog_algorithm_settings::get_nbreplicationsmengeincremental() const
{
    return ui->spinBox_nbreplicationsmengeincremental->value();
}


void dialog_algorithm_settings::set_replicationbudget_TS(int nb)
{
    ui->spinBox_budget_tabusearch->setValue(nb);
}


int dialog_algorithm_settings::get_replicationbudget_TS() const
{
    return ui->spinBox_budget_tabusearch->value();
}


void dialog_algorithm_settings::set_replicationbudget_IS(int nb)
{
    ui->spinBox_budget_identificationstep->setValue(nb);
}


int dialog_algorithm_settings::get_replicationbudget_IS() const
{
    return ui->spinBox_budget_identificationstep->value();
}



// Reset values TS
void dialog_algorithm_settings::reset_values_TS()
{
    ui->spinBox_budget_identificationstep->setValue(timetable::tabu_search::_replication_budget_identification_step_default);
    ui->spinBox_budget_tabusearch->setValue(timetable::tabu_search::_replication_budget_tabu_search_default);
    ui->spinBox_nbreplicationsmengeincremental->setValue(timetable::tabu_search::_nb_eval_menge_incremental_default);
    ui->spinBox_max_candidates_per_iteration->setValue(timetable::tabu_search::_nb_eval_local_minimum_default);
    ui->spinBox_tabulistlength->setValue(timetable::tabu_search::_tabu_list_length_default);
}




// machine learning
void dialog_algorithm_settings::set_machine_learning_method(ml::learning_method lm)
{
    QString lm_new;


    switch(lm)
    {
    case ml::learning_method::krr_trainer_radial_basis_kernel:
        lm_new = QStringLiteral("KRR Radial Basis Kernel");
        break;
    case ml::learning_method::krr_trainer_histogram_intersection_kernel:
        lm_new = QStringLiteral("KRR Histogram Intersection Kernel");
        break;
    case ml::learning_method::krr_trainer_linear_kernel:
        lm_new = QStringLiteral("KRR Linear Basis Kernel");
        break;
    case ml::learning_method::krr_trainer_polynomial_kernel_quadratic:
        lm_new = QStringLiteral("KRR Quadratic Kernel");
        break;
    case ml::learning_method::krr_trainer_polynomial_kernel_cubic:
        lm_new = QStringLiteral("KRR Cubic Kernel");
        break;
    case ml::learning_method::svr_trainer_radial_basis_kernel:
        lm_new = QStringLiteral("SVR Radial Basis Kernel");
        break;
    case ml::learning_method::svr_trainer_histogram_intersection_kernel:
        lm_new = QStringLiteral("SVR Histogram Intersection Kernel");
        break;
    case ml::learning_method::svr_trainer_linear_kernel:
        lm_new = QStringLiteral("SVR Linear Basis Kernel");
        break;
    case ml::learning_method::svr_trainer_polynomial_kernel_quadratic:
        lm_new = QStringLiteral("SVR Quadratic Kernel");
        break;
    case ml::learning_method::svr_trainer_polynomial_kernel_cubic:
        lm_new = QStringLiteral("SVR Cubic Kernel");
        break;
    case ml::learning_method::rvm_trainer_radial_basis_kernel:
        lm_new = QStringLiteral("RVM Radial Basis Kernel");
        break;
    case ml::learning_method::rvm_trainer_histogram_intersection_kernel:
        lm_new = QStringLiteral("RVM Histogram Intersection Kernel");
        break;
    case ml::learning_method::rvm_trainer_linear_kernel:
        lm_new = QStringLiteral("RVM Linear Basis Kernel");
        break;
    case ml::learning_method::rvm_trainer_polynomial_kernel_quadratic:
        lm_new = QStringLiteral("RVM Quadratic Kernel");
        break;
    case ml::learning_method::rvm_trainer_polynomial_kernel_cubic:
        lm_new = QStringLiteral("RVM Cubic Kernel");
        break;
    }


    for(int i = 0; i < ui->comboBox_learningmethod->count(); ++i)
    {
        QString lm_current = ui->comboBox_learningmethod->itemText(i);
        if(lm_current == lm_new)
        {
            ui->comboBox_learningmethod->setCurrentIndex(i);
            break;
        }
    }
}


ml::learning_method dialog_algorithm_settings::get_machine_learning_method() const
{
    QString ml_current = ui->comboBox_learningmethod->currentText();

    if(ml_current == QStringLiteral("KRR Radial Basis Kernel"))
        return ml::learning_method::krr_trainer_radial_basis_kernel;
    else if(ml_current == QStringLiteral("KRR Histogram Intersection Kernel"))
        return ml::learning_method::krr_trainer_histogram_intersection_kernel;
    else if(ml_current == QStringLiteral("KRR Linear Kernel"))
        return ml::learning_method::krr_trainer_linear_kernel;
    else if(ml_current == QStringLiteral("KRR Quadratic Kernel"))
        return ml::learning_method::krr_trainer_polynomial_kernel_quadratic;
    else if(ml_current == QStringLiteral("KRR Cubic Kernel"))
        return ml::learning_method::krr_trainer_polynomial_kernel_cubic;

    else if(ml_current == QStringLiteral("SVR Radial Basis Kernel"))
        return ml::learning_method::svr_trainer_radial_basis_kernel;
    else if(ml_current == QStringLiteral("SVR Histogram Intersection Kernel"))
        return ml::learning_method::svr_trainer_histogram_intersection_kernel;
    else if(ml_current == QStringLiteral("SVR Linear Kernel"))
        return ml::learning_method::svr_trainer_linear_kernel;
    else if(ml_current == QStringLiteral("SVR Quadratic Kernel"))
        return ml::learning_method::svr_trainer_polynomial_kernel_quadratic;
    else if(ml_current == QStringLiteral("SVR Cubic Kernel"))
        return ml::learning_method::svr_trainer_polynomial_kernel_cubic;

    else if(ml_current == QStringLiteral("RVM Radial Basis Kernel"))
        return ml::learning_method::rvm_trainer_radial_basis_kernel;
    else if(ml_current == QStringLiteral("RVM Histogram Intersection Kernel"))
        return ml::learning_method::rvm_trainer_histogram_intersection_kernel;
    else if(ml_current == QStringLiteral("RVM Linear Kernel"))
        return ml::learning_method::rvm_trainer_linear_kernel;
    else if(ml_current == QStringLiteral("RVM Quadratic Kernel"))
        return ml::learning_method::rvm_trainer_polynomial_kernel_quadratic;
    else if(ml_current == QStringLiteral("RVM Cubic Kernel"))
        return ml::learning_method::rvm_trainer_polynomial_kernel_cubic;

}


void dialog_algorithm_settings::set_nb_training_data(int value)
{
    ui->spinBox_nbtrainingdata->setValue(value);
}


int dialog_algorithm_settings::get_nb_training_data() const
{
    return ui->spinBox_nbtrainingdata->value();
}



// logger
void dialog_algorithm_settings::set_logger_verbose(bool verbose)
{
    if(verbose) {
        ui->radioButton_logger_detailed->setChecked(true);
    } else {
        ui->radioButton_logger_summary->setChecked(true);
    }
}


bool dialog_algorithm_settings::is_logger_verbose() const
{
    if(ui->radioButton_logger_detailed->isChecked())
        return true;
    else
        return false;
}


// algorithm run
void dialog_algorithm_settings::set_algorithm_analyze_performance(bool analyse_performance)
{
    if(analyse_performance)
        ui->radioButton_algorithmrun_analyseperformance->setChecked(true);
    else
        ui->radioButton_algorithmrun_standard->setChecked(true);
}


bool dialog_algorithm_settings::get_algorithm_analyze_performance() const
{
    if(ui->radioButton_algorithmrun_analyseperformance->isChecked())
        return true;
    else
        return false;
}



dialog_algorithm_settings::~dialog_algorithm_settings()
{
    delete ui;
}
