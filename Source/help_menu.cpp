#include "help_menu.h"
#include "ui_help_menu.h"

help_menu::help_menu(QWidget *parent) :  QDialog(parent), ui(new Ui::help_menu)
{
    ui->setupUi(this);

    connect(ui->pushButton, SIGNAL(clicked(bool)), this, SLOT(close()));

    QString document;
    document = QStringLiteral(
        "<a name = top><H1>How to use the Tabu Search Heuristic</H1></a>"
        );


    ui->textBrowser->setHtml(document);
}

help_menu::~help_menu()
{
    delete ui;
}
