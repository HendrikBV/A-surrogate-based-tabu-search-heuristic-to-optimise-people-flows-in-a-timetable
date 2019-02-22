#include "help_menu.h"
#include "ui_help_menu.h"

help_menu::help_menu(QWidget *parent) :  QDialog(parent), ui(new Ui::help_menu)
{
    ui->setupUi(this);

    connect(ui->pushButton, SIGNAL(clicked(bool)), this, SLOT(close()));

    QString document;
    document = QStringLiteral(
        "<a name = top><H1>How to use the Egress Time and Travel Time Optimiser</H1></a>"
        "This document provides information on how to use the Egress Time and Travel Time Optimiser."
        "<br><H2>Contents</H2>"
        "<UL>"
        "<LI><a href=#data>Editing data</a>"
        "<LI><a href=#settings>Changing run settings</a>"
        "<LI><a href=#runningalg>Running the algorithm</a>"
        "</UL>"
        "<br><a name = data><H2>Editing data</H2></a>"
        "Importing data can be done through the Data Menu."
        "Data consist of three seperate parts. <p>The first part consists of the timetable data. The data are imported from a .txt file. "
        "First, the basic information has to be provided, in sequence: the number of events, the number of eventgroups, the number of "
        "eventblocks, the number of timeslots, and finally the number of locations. Next, the names of the events, the names of the "
        "eventgroups and the names of the locations (rooms) need to be listed. Third, matrices for eventgroup_event, event_locations_possible, "
        "event_block, block_events, event_nb_people, and finally eventgroup_nb_people need to be specified."
        "<p>The second part consists of the building data. The data are again imported from a .txt file. "
        "First, the number of obstacles, followed by the x1, y1, x2, and y2 coordinates of the obstacles is listed. "
        "Next, the number of rooms followed by the x1, y1, x2, y2, x3, y3, x4, y4 coordinates and the number of doors and their x,y coordinates (for the door targets) "
        "is given. Third, the number of final (exit) targets is given, followed by the x,y coordinates. Finally, the number of intermediary targets "
        "is specified, followed by their x,y coordinates."
        "<p>The third part consists of a timetable solution. Importing a solution can only be done after the timetable and building data have been read in. "
        "The data are again read in from a .txt file. For each event e, first the timeslot is given in which the event is planned, "
        "and second the location to which the event is assigned."
        "<p>You can also import observations from a file to train the neural networks, so that it is not necessary to generate new data is time the algorithm "
        "is executed for a known building layout. During algorithm run, generated observations are automatically save to a file so that they can be reused later."
        "<p>Data that have been imported, can be deleted again using the Clear Data actions in the Data Menu."
        "<br><a name = settings><H2>Changing run settings</H2></a>"
        "<p>In the Settings Menu, you can choose to enter the parameters of the social-force model or of the Tabu Search heuristic. "
        "If you don't change any of these settings, standard settings will be used."
        "<br><a name = runningalg><H2>Running the algorithm</H2></a>"
        "Different options for analysis or optimisation are available. A first possibility is to calculate the egress times and travel times for every timeslot "
        "without visualisation. Before the simulation starts, a dialog appears asking the user to specify: (1) the wayfinding model of the pedestrians; "
        "(2) the numerical technique used to approximate the system of differential equations in the social-force model; (3) the time step used "
        "in the calculations of the social-force model; and (4) the number of replications used. The results of the social-force model are stochastic because "
        "of two reasons. First, the start positions of pedestrians in each room are chosen at random. Second, the preferred free walking speed of each pedestrian "
        "is drawn from a normal distribution. Therefore, the user can specify how many replications should be used to construct confidence intervals for the expected "
        "egress or travel time in each timeslot. The results for the evacuation times are shown in the fourth tab in a bar chart. The horizontal axis denotes the "
        "different timeslots. For every timeslot, there are three bars that show the lower 95% confidence interval, the average, and the upper 95% confidence interval "
        "for the evacuation time, respectively, unless there are less than 3 replications, in which case only the average is shown. In the same way, the results for "
        "the travel times are shown in the fifth tab."
        "<p>A second possibility is to simulate the evacuation process for a specific timeslot of the timetable. This allows a more in-depth analysis for timeslots "
        "with a high evacuation time. Visualisation is a powerful tool to gain insight into complex processes such as evacuations. Again, a dialog is shown that asks "
        "the user to specify: (1) for which timeslot the evacuation is to be simulated; (2) the time step to be used in the numerical solution of the social-force model; "
        "(3) the speed factor of the simulation (for a speed factor of 1, 1 second in real time corresponds to 1 second in the simulation; for a speed factor of 2, "
        "1 second in real time corresponds to 2 seconds in the simulation; etc.), so that the visualisation can be sped up; (4) the wayfinding method of the pedestrians; "
        "and (5) the numerical technique used to approximate the system of differential equations in the social-force model. If the configuration has been specified, "
        "the simulation starts. A new screen is shown in which the building is drawn and every pedestrian is represented by a circle. "
        "Next, at every time step, the positions of the pedestrians are updated according to the social-force model. If the evacuation ends, a messagebox appears with "
        "the maximum egress time."
        "<p>Aside from evacuations, a third possibility is to simulate the process of travelling from lectures at time t to lectures at time t+1. Again, the same dialog "
        "is shown for the user to specify the different settings before the simulation is started. Here, the timeslot refers to the first timeslot t where the people "
        "flows originate. At the start of the simulation, in the same way a new screen is shown in which the building is drawn and every pedestrian is represented by a "
        "circle. Again, a messagebox appears with the maximum travel time if the simulation ends."
        "<p>Finally, the Tabu Search algorithm can be executed to find a better assignmennt of events to locations so that either or both the egress times and travel "
        "times are improved. You can choose the allowed computation time as well as the relative importance (alpha) of either egress times or evacuations times. "
        "During the algorithm run the TextEdit panel at the bottom of the screen prints the status of the algorithm, while the statusbar shows a timer with the "
        "elapsed computation time. Each time a new best solution is found, the solution view is updated, where the changed assignments are indicated in green. "
        "<p><br><a href=#top>Go back to top.</a>"
        );


    ui->textBrowser->setHtml(document);
}

help_menu::~help_menu()
{
    delete ui;
}
