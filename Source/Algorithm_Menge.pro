#-------------------------------------------------
#
# Project created by QtCreator 2018-03-28T11:06:12
#
#-------------------------------------------------

QT       += core gui

CONFIG += c++14

QT += charts

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Algorithm_Menge
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += ../../../../../../Downloads/Menge-master/Menge-master/src/thirdParty
INCLUDEPATH += ../../../../../../Downloads/Menge-master/Menge-master/src/thirdParty/SDL
INCLUDEPATH += ../../../../../../Downloads/Menge-master/Menge-master/src/thirdParty/tclap
INCLUDEPATH += ../../../../../../Downloads/Menge-master/Menge-master/src/include
INCLUDEPATH += ../../../../../../Downloads/Menge-master/Menge-master/src
INCLUDEPATH += ../../../../../../Downloads/Menge-master/Menge-master/src/Menge

INCLUDEPATH += ../../../../../../Downloads/dlib-19.10/dlib-19.10/



DEPENDPATH += ../../../../../../Downloads/Menge-master/Menge-master/src/thirdParty
DEPENDPATH += ../../../../../../Downloads/Menge-master/Menge-master/src/thirdParty/SDL
DEPENDPATH += ../../../../../../Downloads/Menge-master/Menge-master/src/thirdParty/tclap
DEPENDPATH += ../../../../../../Downloads/Menge-master/Menge-master/src/include
DEPENDPATH += ../../../../../../Downloads/Menge-master/Menge-master/src
DEPENDPATH += ../../../../../../Downloads/Menge-master/Menge-master/src/Menge




LIBS += ../../../../../../QtProjects/Algorithm_Menge/AgtDummy.lib
LIBS += ../../../../../../QtProjects/Algorithm_Menge/AgtGCF.lib
LIBS += ../../../../../../QtProjects/Algorithm_Menge/AgtHelbing.lib
LIBS += ../../../../../../QtProjects/Algorithm_Menge/AgtJohansson.lib
LIBS += ../../../../../../QtProjects/Algorithm_Menge/AgtKaramouzas.lib
LIBS += ../../../../../../QtProjects/Algorithm_Menge/AgtZanlungo.lib
LIBS += ../../../../../../QtProjects/Algorithm_Menge/MengeCore.lib
LIBS += ../../../../../../QtProjects/Algorithm_Menge/MengeVis.lib
LIBS += ../../../../../../QtProjects/Algorithm_Menge/tinyxml.lib


SOURCES += \
        main.cpp \
        mainwindow.cpp \
    menge_interface.cpp \
    help_menu.cpp \
    timetable_solution.cpp \
    timetable_global_data.cpp \
    building_data.cpp \
    menge_dialog_start_simulation.cpp \
    menge_dialog_start_analysis.cpp \
    menge_dialog_parameter_settings.cpp \
    machine_learning_interface.cpp \
    ../../../../../../Downloads/dlib-19.10/dlib-19.10/dlib/all/source.cpp \
    logger.cpp \
    dialog_algorithm_settings.cpp \
    dialog_start_algorithm.cpp \
    dialog_compare_learning_methods.cpp \
    timetable_tabu_search.cpp \
    timetable_instance_generator.cpp \
    dialog_instance_generator.cpp

HEADERS += \
        mainwindow.h \
    menge_interface.h \
    help_menu.h \
    timetable_solution.h \
    timetable_global_data.h \
    building_data.h \
    menge_dialog_start_simulation.h \
    menge_dialog_start_analysis.h \
    menge_dialog_parameter_settings.h \
    machine_learning_interface.h \
    logger.h \
    dialog_algorithm_settings.h \
    dialog_start_algorithm.h \
    dialog_compare_learning_methods.h \
    timetable_tabu_search.h \
    timetable_instance_generator.h \
    dialog_instance_generator.h

FORMS += \
        mainwindow.ui \
    help_menu.ui \
    menge_dialog_start_simulation.ui \
    menge_dialog_start_analysis.ui \
    menge_dialog_parameter_settings.ui \
    dialog_algorithm_settings.ui \
    dialog_start_algorithm.ui \
    dialog_compare_learning_methods.ui \
    dialog_instance_generator.ui

RESOURCES += \
    resources.qrc
