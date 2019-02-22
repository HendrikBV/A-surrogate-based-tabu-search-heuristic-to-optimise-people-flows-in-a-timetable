#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.setWindowTitle("Timetable Analyser and Optimiser");
    w.showMaximized();

    return a.exec();
}
