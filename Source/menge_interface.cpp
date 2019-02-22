#include "menge_interface.h"

#include <QTextStream>
#include <QFileDialog>
#include <QFile>
#include <QMessageBox>

#include "MengeCore/Agents/SimulatorInterface.h"
#include "MengeCore/Math/RandGenerator.h"
#include "MengeCore/PluginEngine/CorePluginEngine.h"
#include "MengeCore/ProjectSpec.h"
#include "MengeCore/Runtime/Logger.h"
#include "MengeCore/Runtime/os.h"
#include "MengeCore/Runtime/SimulatorDB.h"

#include "MengeVis/PluginEngine/VisPluginEngine.h"
#include "MengeVis/Runtime/AgentContext/BaseAgentContext.h"
#include "MengeVis/Runtime/EventInjectContext.h"
#include "MengeVis/Runtime/MengeContext.h"
#include "MengeVis/Runtime/SimSystem.h"
#include "MengeVis/SceneGraph/ContextSwitcher.h"
#include "MengeVis/SceneGraph/GLScene.h"
#include "MengeVis/SceneGraph/TextWriter.h"
#include "MengeVis/Viewer/GLViewer.h"
#include "MengeVis/Viewer/ViewConfig.h"

#include "thirdParty/tclap/CmdLine.h"

#include "logger.h"

#include <random>
#include <chrono>




namespace
{
    std::random_device randdev;
    std::seed_seq seedseq{ randdev(), randdev(), randdev(), randdev(), randdev(), randdev(), randdev(), randdev() };
    std::mt19937 generator(seedseq);

    constexpr double NANO = 1000000000.0;
}




using namespace Menge;
using Menge::PluginEngine::CorePluginEngine;
using Menge::ProjectSpec;
using MengeVis::PluginEngine::VisPluginEngine;


namespace ped
{
    menge_interface::menge_interface()
    {

    }

    menge_interface::~menge_interface()
    {

    }



    // VISUALIZATION
    double menge_interface::visualise_evacuation(const timetable::solution& sol, int timeslot)
    {
        VISUALIZE = true;

        // 1. MAKE XML FILES
        write_behavior_xml();
        write_scene_xml_evacuation(sol, timeslot);
        //write_view_xml();


        // 2. RUN MENGE
        double evactime = sim_main();
        return evactime;
    }

    double menge_interface::visualise_flows(const timetable::solution& sol, int first_timeslot)
    {
        VISUALIZE = true;

        // 1. MAKE XML FILES
        write_behavior_xml();
        write_scene_xml_travel(sol, first_timeslot);
        //write_view_xml();


        // 2. RUN MENGE
        double traveltime = sim_main();
        return traveltime;
    }



    // CALCULATIONS
    double menge_interface::calculate_evacuation_time(const timetable::solution& sol, int timeslot)
    {
        VISUALIZE = false;

        // 1. MAKE XML FILES
        write_behavior_xml();
        write_scene_xml_evacuation(sol, timeslot);
        //write_view_xml();


        // 2. RUN MENGE
        double evactime = sim_main();
        return evactime;
    }

    double menge_interface::calculate_flows_time(const timetable::solution& sol, int first_timeslot)
    {
        VISUALIZE = false;

        // 1. MAKE XML FILES
        write_behavior_xml();
        write_scene_xml_travel(sol, first_timeslot);
        //write_view_xml();


        // 2. RUN MENGE
        double traveltime = sim_main();
        return traveltime;
    }






    // SOLUTION ANALYSIS
    void menge_interface::start_calculations(const timetable::solution& sol, int replications)
    {
        try
        {
            VISUALIZE = false;

            m_replications = replications;

            evacuation_times.clear();
            travel_times.clear();
            evacuation_times_avg.clear();
            travel_times_avg.clear();
            evacuation_times_stddev.clear();
            travel_times_stddev.clear();

            evacuation_times.reserve(timetable::nb_timeslots*replications);
            travel_times.reserve(timetable::nb_timeslots*replications);
            evacuation_times_avg.reserve(timetable::nb_timeslots);
            travel_times_avg.reserve(timetable::nb_timeslots);
            evacuation_times_stddev.reserve(timetable::nb_timeslots);
            travel_times_stddev.reserve(timetable::nb_timeslots);



            total_number_of_calculations = replications * (2*timetable::nb_timeslots - 1);
            current_calculation_number = 0;
            stop = false;




            std::chrono::system_clock::time_point start_time = std::chrono::system_clock::now();

            for(int i = 0; i < replications; ++i)
            {
                for(int t = 0; t < timetable::nb_timeslots; ++t)
                {
                    if(stop)
                        return;

                    // calculate one evacuation
                    double evacuationtime = SIM_DURATION; int again = 0;
                    do {
                        evacuationtime = this->calculate_evacuation_time(sol, t);
                        ++again;
                    } while(evacuationtime > SIM_DURATION - 1.0f && again < 5); // recalculate maximum 5 times

                    if(evacuationtime < 0)
                        evacuationtime = 0.0;

                    evacuation_times.push_back(evacuationtime);
                    ++current_calculation_number;
                    emit(finished_one_calculation(current_calculation_number));


                    if(stop)
                        return;

                    // calculate one travel between events
                    if(t < timetable::nb_timeslots - 1)
                    {
                        double traveltime = SIM_DURATION; again = 0;
                        do { // if simulation gets stuck, don't count it
                            traveltime = this->calculate_flows_time(sol, t);
                            ++again;
                        } while(traveltime > SIM_DURATION - 1.0f && again < 5);

                        if(traveltime < 0)
                            traveltime = 0.0;

                        travel_times.push_back(traveltime);
                        ++current_calculation_number;
                        emit(finished_one_calculation(current_calculation_number));
                    }

                    if(current_calculation_number >= total_number_of_calculations)
                        emit(finished());
                }
            }

            std::chrono::nanoseconds calctime = std::chrono::system_clock::now() - start_time;

            QString output_text = "\nFinished analysing solution.\nElapsed computation time: ";
            output_text += QString::number(calctime.count() / NANO);
            output_text += " seconds";
            emit(signal_status(output_text));
            global::_logger << global::logger::log_type::INFORMATION << output_text;






            // calculate averages and stddev
            for(int t = 0; t < timetable::nb_timeslots; ++t)
            {
                double avg = 0;
                if(global::_logger.is_verbose())
                {
                    output_text = "Evacuations timeslot "; output_text += QString::number(t+1);
                    for(int r = 0; r < replications; ++r) {
                        output_text += "\n"; output_text += QString::number(evacuation_times[r*timetable::nb_timeslots+t]);
                        avg += evacuation_times[r*timetable::nb_timeslots+t] / replications;
                    }
                    global::_logger << global::logger::log_type::INFORMATION << output_text;
                }
                else
                {
                    for(int r = 0; r < replications; ++r)
                        avg += evacuation_times[r*timetable::nb_timeslots+t] / replications;
                }
                evacuation_times_avg.push_back(avg);

                double stddev = 0;
                for(int r = 0; r < replications; ++r)
                    stddev += (evacuation_times[r*timetable::nb_timeslots+t] - avg)
                              *(evacuation_times[r*timetable::nb_timeslots+t] - avg) / (replications - 1);
                stddev = sqrt(stddev);
                evacuation_times_stddev.push_back(stddev);

                output_text = "\nEvacuations timeslot "; output_text += QString::number(t+1);
                output_text += "\n    Observations = "; output_text += QString::number(replications);
                output_text += "\n    Average = "; output_text += QString::number(avg);
                output_text += "\n    Stddev = "; output_text += QString::number(stddev);
                global::_logger << global::logger::log_type::INFORMATION << output_text;
            }

            for(int t = 0; t < timetable::nb_timeslots - 1; ++t)
            {
                double avg = 0;
                if(global::_logger.is_verbose())
                {
                    output_text = "Travels timeslot "; output_text += QString::number(t+1);
                    for(int r = 0; r < replications; ++r) {
                        output_text += "\n"; output_text += QString::number(travel_times[r*(timetable::nb_timeslots-1)+t]);
                        avg += travel_times[r*(timetable::nb_timeslots-1)+t] / replications;
                    }
                    global::_logger << global::logger::log_type::INFORMATION << output_text;
                }
                else
                {
                    for(int r = 0; r < replications; ++r)
                        avg += travel_times[r*(timetable::nb_timeslots-1)+t] / replications;
                }
                travel_times_avg.push_back(avg);

                double stddev = 0;
                for(int r = 0; r < replications; ++r)
                    stddev += (travel_times[r*(timetable::nb_timeslots-1)+t] - avg)
                              *(travel_times[r*(timetable::nb_timeslots-1)+t] - avg) / (replications - 1);
                stddev = sqrt(stddev);
                travel_times_stddev.push_back(stddev);

                output_text = "\nTravels timeslot "; output_text += QString::number(t+1);
                output_text += "\n    Observations = "; output_text += QString::number(replications);
                output_text += "\n    Average = "; output_text += QString::number(avg);
                output_text += "\n    Stddev = "; output_text += QString::number(stddev);
                global::_logger << global::logger::log_type::INFORMATION << output_text;
            }

        }
        catch(const std::exception& ex)
        {
            QString message(ex.what());
            emit(signal_error(message));
        }
    }



    void menge_interface::halt()
    {
        stop = true;
        emit(finished_one_calculation(total_number_of_calculations));
    }



    double menge_interface::evacuation_time_CImin(int timeslot)
    {
        if(m_replications <= 10)
            return (evacuation_times_avg[timeslot] - student_t_values[m_replications]*evacuation_times_stddev[timeslot]/sqrt(m_replications));
        else
            return (evacuation_times_avg[timeslot] - 1.96*evacuation_times_stddev[timeslot]/sqrt(m_replications));
    }

    double menge_interface::travel_time_CImin(int timeslot)
    {
        if(m_replications <= 10)
            return (travel_times_avg[timeslot] - student_t_values[m_replications]*travel_times_stddev[timeslot]/sqrt(m_replications));
        else
            return (travel_times_avg[timeslot] - 1.96*travel_times_stddev[timeslot]/sqrt(m_replications));
    }

    double menge_interface::evacuation_time_CImax(int timeslot)
    {
        if(m_replications <= 10)
            return (evacuation_times_avg[timeslot] + student_t_values[m_replications]*evacuation_times_stddev[timeslot]/sqrt(m_replications));
        else
            return (evacuation_times_avg[timeslot] + 1.96*evacuation_times_stddev[timeslot]/sqrt(m_replications));
    }

    double menge_interface::travel_time_CImax(int timeslot)
    {
        if(m_replications <= 10)
            return (travel_times_avg[timeslot] + student_t_values[m_replications]*travel_times_stddev[timeslot]/sqrt(m_replications));
        else
            return (travel_times_avg[timeslot] + 1.96*travel_times_stddev[timeslot]/sqrt(m_replications));
    }






    // TO TRAIN SURROGATE
    double menge_interface::calculate_custom_evacuation_time(const std::vector<int> &nb_people_per_room)
    {
        VISUALIZE = false;

        // 1. MAKE XML FILES
        write_behavior_xml();
        write_scene_xml_evacuation(nb_people_per_room);
        //write_view_xml();


        // 2. RUN MENGE
        double evactime = sim_main();
        return evactime;
    }

    double menge_interface::calculate_custom_travel_time(const std::vector<int> &nb_people_from_room_to_room)
    {
        VISUALIZE = false;

        // 1. MAKE XML FILES
        write_behavior_xml();
        write_scene_xml_travel(nb_people_from_room_to_room);
        //write_view_xml();


        // 2. RUN MENGE
        double traveltime = sim_main();
        return traveltime;
    }













    // SIM MAIN
    double menge_interface::sim_main()
    {
        Menge::SimulatorDB simDB;

        logger.setFile( "log.html" );
        logger << Logger::INFO_MSG << "initialized logger";

        CorePluginEngine plugins( &simDB );
        logger.line();
        std::string pluginPath = "C:/Users/hendr/OneDrive/Documenten/Hendrik/PhD/QtProjects/Algorithm_Menge";
        logger << Logger::INFO_MSG << "Plugin path: " << pluginPath;
        plugins.loadPlugins( pluginPath );
        if ( simDB.modelCount() == 0 ) {
            logger << Logger::INFO_MSG << "There were no pedestrian models in the plugins folder\n";
            return -1;
        }


        Menge::Math::setDefaultGeneratorSeed(0);



        SimulatorDBEntry * simDBEntry = simDB.getDBEntry( MODEL );
        if ( simDBEntry == 0x0 ) {
            //std::cerr << "!!!  The specified model is not recognized: " << model << "\n";
            logger.close();
            return -1;
        }

        double result;
        if(timetable::nb_locations > 25)
             result = simulate( simDBEntry, "behavior.xml", "scene.xml",
                                       "", "", VISUALIZE, "officeV.xml", "" );
        else
             result = simulate( simDBEntry, "behavior.xml", "scene.xml",
                                       "", "", VISUALIZE, "officeV2.xml", "" );

        if ( std::fabs(result - 1) < 0.001 ) {
            //std::cerr << "Simulation terminated through error.  See error log for details.\n";
            return -1;
        }
        logger.close();

        return result;
    }

    double menge_interface::simulate( Menge::SimulatorDBEntry * dbEntry, const std::string & behaveFile,
                 const std::string & sceneFile, const std::string & outFile,
                 const std::string & scbVersion, bool visualize, const std::string & viewCfgFile,
                 const std::string & dumpPath )
    {
        size_t agentCount;

        if ( outFile != "" ) {
            logger << Logger::INFO_MSG << "Attempting to write scb file: " << outFile << "\n";
        }

        using Menge::Agents::SimulatorInterface;
        using MengeVis::Runtime::BaseAgentContext;
        using MengeVis::Runtime::EventInjectionContext;
        using MengeVis::Runtime::SimSystem;
        using MengeVis::SceneGraph::Context;
        using MengeVis::SceneGraph::ContextSwitcher;
        using MengeVis::SceneGraph::GLScene;
        using MengeVis::SceneGraph::TextWriter;
        using MengeVis::Viewer::GLViewer;
        using MengeVis::Viewer::ViewConfig;

        SimulatorInterface * sim = dbEntry->getSimulator( agentCount, TIME_STEP, SUB_STEPS,
                                                          SIM_DURATION, behaveFile, sceneFile, outFile,
                                                          scbVersion, VERBOSE );

        if ( sim == 0x0 ) {
            return -1;
        }

        if ( visualize ) {
            logger.line();
            logger << Logger::INFO_MSG << "Initializing visualization...";
            VisPluginEngine visPlugins;
            visPlugins.loadPlugins( "C:/Users/hendr/OneDrive/Documenten/Hendrik/PhD/QtProjects/Algorithm_Menge" );

            TextWriter::setDefaultFont( os::path::join( 2, ROOT.c_str(), "arial.ttf" ) );

            ViewConfig viewCfg;
            if ( VERBOSE ) {
                logger << Logger::INFO_MSG << "Using visualizer!";
            }
            if ( viewCfgFile == "" ) {
                if ( VERBOSE ) {
                    logger << Logger::INFO_MSG << "\tUsing default visualization settings.";
                }
            } else {
                // TODO: Error handling
                if ( viewCfg.readXML( viewCfgFile ) ) {
                    if ( VERBOSE ) {
                        logger << Logger::INFO_MSG << "\tUsing visualization from: " << viewCfgFile << "\n";
                        logger << Logger::INFO_MSG << viewCfg << "\n";
                    }
                } else {
                    logger << Logger::ERR_MSG << "Unable to read the specified view configuration (" << viewCfgFile << "). Terminating.";
                    return -1;
                }
            }
            GLViewer view( viewCfg );

            view.setDumpPath( dumpPath );

            std::string viewTitle = "Pedestrian Simulation - " + dbEntry->viewerName();

            if ( !view.initViewer( viewTitle ) ) {
                std::cerr << "Unable to initialize the viewer\n\n";
                visualize = false;
            } else {
                GLScene * scene = new GLScene();
                SimSystem * system = new SimSystem( sim );
                system->populateScene( scene );
                scene->addSystem( system );
                view.setScene( scene );

                view.setFixedStep( TIME_STEP );
                view.setBGColor( 0.1f, 0.1f, 0.1f );
                MengeVis::Runtime::MengeContext * ctx = new MengeVis::Runtime::MengeContext( sim );
                scene->setContext( new EventInjectionContext( ctx ) );
                view.newGLContext();
                logger.line();

                view.run();
            }
        }

        // no visualisation
        else
        {
            try {
                bool running = true;
                while ( running ) {
                    running = sim->step();
                }
            } catch(...) {
                return 1800.0;
            }
        }


        logger << Logger::INFO_MSG << "Simulation time: " << dbEntry->simDuration() << "\n";
        return dbEntry->simDuration();
    }



    // XMLs
    void menge_interface::write_behavior_xml()
    {
        QFile file("behavior.xml");
        if (file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            QTextStream stream(&file);

            // xml header
            stream << "<?xml version=\"1.0\"?>";

            // BFSM tags
            stream << "\n<BFSM>";



                // Goalset for rooms
                stream << "\n<GoalSet id=\"0\">";

                    // Goals = Targets
                    for(int i = 0; i < building::room_targets.size(); ++i)
                    {
                        stream << "\n<Goal capacity=\"1000000\" id=\"" << i << "\" type=\"point\" weight=\"1.00\" x=\""
                               << building::room_targets[i].x << "\" y=\"" << building::room_targets[i].y << "\"/>";
                    }

                stream << "\n</GoalSet>";


                // Goalset for exit
                stream << "\n<GoalSet id=\"1\">";

                    for(int i = 0; i < building::exit_targets.size(); ++i)
                    {
                        stream << "\n<Goal capacity=\"1000000\" id=\"" << i << "\" type=\"point\" weight=\"1.00\" x=\""
                                   << building::exit_targets[i].x << "\" y=\"" << building::exit_targets[i].y << "\"/>";
                    }

                stream << "\n</GoalSet>";



                // One final state
                stream << "\n<State name=\"Stop\" final=\"1\">";
                    stream << "\n<GoalSelector type=\"identity\"/>";
                    stream << "\n<VelComponent type=\"goal\"/>";
                stream << "\n</State>";


                // One teleport state exit destination
                stream << "\n<State name=\"ExitReached\" final=\"0\">";
                if(VISUALIZE)
                {
                    stream << "\n<Action type=\"teleport\" dist=\"u\" min_x=\"" << building::teleport_location_exit.x - 2
                               << "\" max_x=\"" << building::teleport_location_exit.x + 2 << "\" min_y=\""
                               << building::teleport_location_exit.y - 2 << "\" max_y=\"" << building::teleport_location_exit.y + 2 << "\"/>";
                }
                else
                {
                    stream << "\n<Action type=\"teleport\" dist=\"u\" min_x=\"" << 10000
                               << "\" max_x=\"" << 500000 << "\" min_y=\""
                               << 10000 << "\" max_y=\"" << 500000 << "\"/>";
                }
                stream << "\n</State>";

                // One state for exit destination
                stream << "\n<State name=\"WalkToExit\" final=\"0\">";
                    stream << "\n<GoalSelector type=\"nearest\" goal_set=\"1\" per_agent=\"1\" persistent=\"1\"/>";
                    stream << "\n<VelComponent type=\"road_map\" file_name=\"" << building::road_map_file_name << "\"/>";
                stream << "\n</State>";



                // One start state per destination
                for(int i = 0; i < building::room_targets.size(); ++i)
                {
                    stream << "\n<State name=\"WalkToTarget" << i << "\" final=\"0\">";
                        stream << "\n<GoalSelector type=\"explicit\" goal_set=\"0\" goal=\"" << i << "\"/>";
                        stream << "\n<VelComponent type=\"road_map\" file_name=\"" << building::road_map_file_name << "\"/>";
                    stream << "\n</State>";


                    stream << "\n<State name=\"TargetReached" << i << "\" final=\"0\">";
                    if(VISUALIZE)
                    {
                        stream << "\n<Action type=\"teleport\" dist=\"u\" min_x=\"" << building::teleport_locations_rooms[i].x - 1
                               << "\" max_x=\"" << building::teleport_locations_rooms[i].x + 1 << "\" min_y=\""
                               << building::teleport_locations_rooms[i].y - 1 << "\" max_y=\"" << building::teleport_locations_rooms[i].y + 1 << "\"/>";
                    }
                    else
                    {
                        stream << "\n<Action type=\"teleport\" dist=\"u\" min_x=\"" << 10000
                                   << "\" max_x=\"" << 500000 << "\" min_y=\""
                                   << 10000 << "\" max_y=\"" << 500000 << "\"/>";
                    }
                    stream << "\n</State>";
                }




                // One state per staircase
                for(int i = 0; i < building::stairs.size(); ++i) // stairs nows to where it should teleport
                {
                    stream << "\n<State name=\"Stairs" << i << "\" final=\"0\">";

                    stream << "\n<Action type=\"teleport\" dist=\"u\" min_x=\"" << building::stairs[i].to_x_min
                           << "\" max_x=\"" << building::stairs[i].to_x_max << "\" min_y=\""
                           << building::stairs[i].to_y_min << "\" max_y=\"" << building::stairs[i].to_y_max << "\"/>";

                    stream << "\n</State>";
                }



                // Transitions from goals to Stop
                // Exit
                stream << "\n<Transition from=\"WalkToExit\" to=\"ExitReached\">";
                    stream << "\n<Condition type=\"goal_reached\" distance=\"1.0\"/>";
                stream << "\n</Transition>";

                stream << "\n<Transition from=\"ExitReached\" to=\"Stop\">";
                    stream << "\n<Condition type=\"auto\"/>";
                stream << "\n</Transition>";

                // Rooms
                for(int i = 0; i < building::room_targets.size(); ++i)
                {
                    stream << "\n<Transition from=\"WalkToTarget" << i << "\" to=\"TargetReached" << i << "\">";
                        stream << "\n<Condition type=\"goal_reached\" distance=\"1.0\"/>";
                    stream << "\n</Transition>";

                    stream << "\n<Transition from=\"TargetReached" << i << "\" to=\"Stop\">";
                        stream << "\n<Condition type=\"auto\"/>";
                    stream << "\n</Transition>";
                }

                // Transition from states to Staircases and back
                for(int i = 0; i < building::stairs.size(); ++i)
                {
                    stream << "\n<Transition from=\"WalkToExit,";
                    for(int j = 0; j < building::room_targets.size(); ++j)
                    {
                        stream << "WalkToTarget" << j;

                        if(j < building::room_targets.size()-1)
                            stream << ",";
                    }
                    stream << "\" to=\"Stairs" << i << "\">";
                        stream << "\n<Condition type=\"AABB\" inside=\"1\" min_x=\"" << building::stairs[i].from_x_min
                               << "\" max_x=\"" << building::stairs[i].from_x_max << "\" min_y=\"" << building::stairs[i].from_y_min
                               << "\" max_y=\"" << building::stairs[i].from_y_max << "\"/>";
                    stream << "\n</Transition>";


                    stream << "\n<Transition from=\"Stairs" << i << "\">";
                        stream << "\n<Condition type=\"auto\"/>";
                        stream << "\n<Target type=\"return\"/>";
                    stream << "\n</Transition>";
                }


            stream << "\n</BFSM>";
        }
    }

    void menge_interface::write_scene_xml_evacuation(const timetable::solution &sol, int timeslot)
    {
        QFile file("scene.xml");
        if (file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            QTextStream stream(&file);

            // xml header
            stream << "<?xml version=\"1.0\"?>";

            // Experiment tag
            stream << "\n<Experiment version=\"2.0\">";


                stream << "<SpatialQuery type=\"kd-tree\" test_visibility=\"false\" />";


                // Behavioral models
                stream << "\n<OpenSteer max_force=\"" << _OpenSteer_max_force
                       << "\" leak_through=\"" << _OpenSteer_leak_through
                       << "\" reaction_time=\"" << _OpenSteer_reaction_time << "\"/>";
                stream << "\n<Common time_step=\"" << _Common_time_step << "\"/>";
                stream << "\n<GCF reaction_time=\"" << _GCF_reaction_time
                       << "\" max_agent_dist=\"" << _GCF_max_agent_dist
                       << "\" max_agent_force=\"" << _GCF_max_agent_force
                       << "\" agent_interp_width=\"" << _GCF_agent_interp_width
                       << "\" agent_force_strength=\"" << _GCF_agent_force_strength << "\"/>";
                stream << "\n<Helbing agent_scale=\"" << _Helbing_agent_scale
                       << "\" obstacle_scale=\"" << _Helbing_obstacle_scale
                       << "\" reaction_time=\"" << _Helbing_reaction_time
                       << "\" body_force=\"" << _Helbing_body_force
                       << "\" friction=\"" << _Helbing_friction
                       << "\" force_distance=\"" << _Helbing_force_distance << "\"/>";
                stream << "\n<Karamouzas orient_weight=\"" << _Karamouzas_orient_weight
                       << "\" fov=\"" << _Karamouzas_fov
                       << "\" reaction_time=\"" << _Karamouzas_reaction_time
                       << "\" wall_steepness=\"" << _Karamouzas_wall_steepness
                       << "\" wall_distance=\"" << _Karamouzas_wall_distance
                       << "\" colliding_count=\"" << _Karamouzas_colliding_count
                       << "\" d_min=\"" << _Karamouzas_d_min
                       << "\" d_mid=\"" << _Karamouzas_d_mid
                       << "\" d_max=\"" << _Karamouzas_d_max
                       << "\" agent_force=\"" << _Karamouzas_agent_force << "\"/>";
                stream << "\n<Zanlungo agent_scale=\"" << _Zanlungo_agent_scale
                       << "\" obstacle_scale=\"" << _Zanlungo_obstacle_scale
                       << "\" reaction_time=\"" << _Zanlungo_reaction_time
                       << "\" force_distance=\"" << _Zanlungo_force_distance << "\"/>";
                stream << "\n<Dummy stddev=\"" << _Dummy_stddev << "\"/>";



                // One agentprofile
                stream << "\n<AgentProfile name=\"group1\">";

                    stream << "\n<OpenSteer tau=\"" << _OpenSteer_tau
                           << "\" tauObst=\"" << _OpenSteer_tauObst << "\"/>";
                    stream << "\n<Common max_angle_vel=\"" << _Common_max_angle_vel
                           << "\" max_neighbors=\"" << _Common_max_neighbors
                           << "\" obstacleSet=\"" << _Common_obstacleSet
                           << "\" neighbor_dist=\"" << _Common_neighbor_dist
                           << "\" r=\"" << _Common_r
                           << "\" class=\"" << _Common_class
                           << "\" pref_speed=\"" << _Common_pref_speed
                           << "\" max_speed=\"" << _Common_max_speed
                           << "\" max_accel=\"" << _Common_max_accel << "\">"
                           << "\n<Property name=\"pref_speed\" dist=\"n\" mean=\"" << _Common_pref_speed << "\" stddev=\"" << _Common_pref_speed_stddev << "\"/>"
                           << "\n</Common>";
                    stream << "\n<PedVO factor=\"" << _PedVO_factor
                           << "\" buffer=\"" << _PedVO_buffer
                           << "\" tau=\"" << _PedVO_tau
                           << "\" tauObst=\"" << _PedVO_tauObst
                           << "\" turningBias=\"" << _PedVO_turningBias << "\"/>";
                    stream << "\n<GCF stand_depth=\"" << _GCF_stand_depth
                           << "\" move_scale=\"" << _GCF_move_scale
                           << "\" slow_width=\"" << _GCF_slow_width
                           << "\" sway_change=\"" << _GCF_sway_change << "\"/>";
                    stream << "\n<Helbing mass=\"" << _Helbing_mass << "\"/>";
                    stream << "\n<Karamouzas personal_space=\"" << _Karamouzas_personal_space
                           << "\" anticipation=\"" << _Karamouzas_anticipation << "\"/>";
                    stream << "\n<ORCA tau=\"" << _ORCA_tau
                           << "\" tauObst=\"" << _ORCA_tauObst << "\"/>";
                    stream << "\n<Zanlungo mass=\"" << _Zanlungo_mass
                           << "\" orient_weight=\"" << _Zanlungo_orient_weight << "\"/>";

                stream << "\n</AgentProfile>";



                // AgentGroups: one group per event-room
                for(int r = 0; r < timetable::nb_locations; ++r)
                {
                    int event = sol.timeslot_location(timeslot, r);
                    if(event != -1)
                    {
                        int nb_people_in_event = timetable::get_event_nb_people(event);

                        stream << "\n<AgentGroup>";
                            stream << "\n<ProfileSelector type=\"const\" name=\"group1\"/>";
                            stream << "\n<StateSelector type=\"const\" name=\"WalkToExit\"/>";
                            stream << "\n<Generator type=\"rect_grid\" "
                                   << "anchor_x=\"" << building::room_targets[r].x << "\" "
                                   << "anchor_y=\"" << building::room_targets[r].y << "\" "
                                   << "offset_x=\"0.1\" "
                                   << "offset_y=\"0.1\" "
                                   << "count_x=\"" << nb_people_in_event / 5.0 << "\" "
                                   << "count_y=\"" << 5 << "\" "
                                   << "displace_dist=\"u\" "
                                   << "displace_min=\"0.0\" "
                                   << "displace_max=\"0.05\""
                                   << "/>";
                        stream << "\n</AgentGroup>";
                    }
                }


                // Obstacles
                stream << "\n<ObstacleSet type=\"explicit\" class=\"1\">";

                    for(int i = 0; i < building::obstacles.size(); ++i)
                    {
                        stream << "\n<Obstacle closed=\"1\">";
                            for(int j = 0; j < building::obstacles[i].nb_vertices; ++j)
                            {
                                stream << "\n<Vertex p_x=\"" << building::obstacles[i].vertices_x[j] << "\" p_y=\"" << building::obstacles[i].vertices_y[j] << "\"/>";
                            }
                        stream << "\n</Obstacle>";
                    }

                    /*for(int i = 0; i < building::obstacles.size(); ++i)
                    {
                        stream << "\n<Obstacle closed=\"1\">";
                            stream << "\n<Vertex p_x=\"" << building::obstacles[i].x_1 << "\" p_y=\"" << building::obstacles[i].y_1 << "\"/>";
                            stream << "\n<Vertex p_x=\"" << building::obstacles[i].x_2 << "\" p_y=\"" << building::obstacles[i].y_2 << "\"/>";
                        stream << "\n</Obstacle>";
                    }*/

                stream << "\n</ObstacleSet>";



            stream << "\n</Experiment>";
        }

    }

    void menge_interface::write_scene_xml_travel(const timetable::solution &sol, int first_timeslot)
    {
        QFile file("scene.xml");
        if (file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            QTextStream stream(&file);

            // xml header
            stream << "<?xml version=\"1.0\"?>";

            // Experiment tag
            stream << "\n<Experiment version=\"2.0\">";


                stream << "<SpatialQuery type=\"kd-tree\" test_visibility=\"false\" />";


                // Behavioral models
                stream << "\n<OpenSteer max_force=\"" << _OpenSteer_max_force
                       << "\" leak_through=\"" << _OpenSteer_leak_through
                       << "\" reaction_time=\"" << _OpenSteer_reaction_time << "\"/>";
                stream << "\n<Common time_step=\"" << _Common_time_step << "\"/>";
                stream << "\n<GCF reaction_time=\"" << _GCF_reaction_time
                       << "\" max_agent_dist=\"" << _GCF_max_agent_dist
                       << "\" max_agent_force=\"" << _GCF_max_agent_force
                       << "\" agent_interp_width=\"" << _GCF_agent_interp_width
                       << "\" agent_force_strength=\"" << _GCF_agent_force_strength << "\"/>";
                stream << "\n<Helbing agent_scale=\"" << _Helbing_agent_scale
                       << "\" obstacle_scale=\"" << _Helbing_obstacle_scale
                       << "\" reaction_time=\"" << _Helbing_reaction_time
                       << "\" body_force=\"" << _Helbing_body_force
                       << "\" friction=\"" << _Helbing_friction
                       << "\" force_distance=\"" << _Helbing_force_distance << "\"/>";
                stream << "\n<Karamouzas orient_weight=\"" << _Karamouzas_orient_weight
                       << "\" fov=\"" << _Karamouzas_fov
                       << "\" reaction_time=\"" << _Karamouzas_reaction_time
                       << "\" wall_steepness=\"" << _Karamouzas_wall_steepness
                       << "\" wall_distance=\"" << _Karamouzas_wall_distance
                       << "\" colliding_count=\"" << _Karamouzas_colliding_count
                       << "\" d_min=\"" << _Karamouzas_d_min
                       << "\" d_mid=\"" << _Karamouzas_d_mid
                       << "\" d_max=\"" << _Karamouzas_d_max
                       << "\" agent_force=\"" << _Karamouzas_agent_force << "\"/>";
                stream << "\n<Zanlungo agent_scale=\"" << _Zanlungo_agent_scale
                       << "\" obstacle_scale=\"" << _Zanlungo_obstacle_scale
                       << "\" reaction_time=\"" << _Zanlungo_reaction_time
                       << "\" force_distance=\"" << _Zanlungo_force_distance << "\"/>";
                stream << "\n<Dummy stddev=\"" << _Dummy_stddev << "\"/>";



                // One agentprofile
                stream << "\n<AgentProfile name=\"group1\">";

                    stream << "\n<OpenSteer tau=\"" << _OpenSteer_tau
                           << "\" tauObst=\"" << _OpenSteer_tauObst << "\"/>";
                    stream << "\n<Common max_angle_vel=\"" << _Common_max_angle_vel
                           << "\" max_neighbors=\"" << _Common_max_neighbors
                           << "\" obstacleSet=\"" << _Common_obstacleSet
                           << "\" neighbor_dist=\"" << _Common_neighbor_dist
                           << "\" r=\"" << _Common_r
                           << "\" class=\"" << _Common_class
                           << "\" pref_speed=\"" << _Common_pref_speed
                           << "\" max_speed=\"" << _Common_max_speed
                           << "\" max_accel=\"" << _Common_max_accel << "\">"
                           << "\n<Property name=\"pref_speed\" dist=\"n\" mean=\"" << _Common_pref_speed << "\" stddev=\"" << _Common_pref_speed_stddev << "\"/>"
                           << "\n</Common>";
                    stream << "\n<PedVO factor=\"" << _PedVO_factor
                           << "\" buffer=\"" << _PedVO_buffer
                           << "\" tau=\"" << _PedVO_tau
                           << "\" tauObst=\"" << _PedVO_tauObst
                           << "\" turningBias=\"" << _PedVO_turningBias << "\"/>";
                    stream << "\n<GCF stand_depth=\"" << _GCF_stand_depth
                           << "\" move_scale=\"" << _GCF_move_scale
                           << "\" slow_width=\"" << _GCF_slow_width
                           << "\" sway_change=\"" << _GCF_sway_change << "\"/>";
                    stream << "\n<Helbing mass=\"" << _Helbing_mass << "\"/>";
                    stream << "\n<Karamouzas personal_space=\"" << _Karamouzas_personal_space
                           << "\" anticipation=\"" << _Karamouzas_anticipation << "\"/>";
                    stream << "\n<ORCA tau=\"" << _ORCA_tau
                           << "\" tauObst=\"" << _ORCA_tauObst << "\"/>";
                    stream << "\n<Zanlungo mass=\"" << _Zanlungo_mass
                           << "\" orient_weight=\"" << _Zanlungo_orient_weight << "\"/>";

                stream << "\n</AgentProfile>";



                // AgentGroups: one group per event-room
                {
                    for(int c = 0; c < timetable::nb_eventgroups; ++c)
                    {
                        // check the situation for eventgroup c:
                        // a. t and t+1
                        // b. t, not t+1
                        // c. not t, t+1
                        // d. not t, not t+1

                        int nb_people_in_group = timetable::get_eventgroup_nb_people(c);

                        int event_first_timeslot = -1;
                        int room_first_timeslot = -1;
                        int event_second_timeslot = -1;
                        int room_second_timeslot = -1;

                        for(int l = 0; l < timetable::nb_events; ++l)
                        {
                            if(timetable::get_eventgroup_event(c,l))
                            {
                                // first timeslot
                                if(sol.event_timeslot(l) == first_timeslot)
                                {
                                    event_first_timeslot = l;

                                    for(int r = 0; r < timetable::nb_locations; ++r)
                                    {
                                        if(sol.event_location(l) == r)
                                        {
                                            room_first_timeslot = r;
                                            break;
                                        }
                                    }
                                }

                                // second timeslot
                                if(sol.event_timeslot(l) == first_timeslot + 1)
                                {
                                    event_second_timeslot = l;

                                    for(int r = 0; r < timetable::nb_locations; ++r)
                                    {
                                        if(sol.event_location(l) == r)
                                        {
                                            room_second_timeslot = r;
                                            break;
                                        }
                                    }
                                }
                            }
                        }

                        // A. class time t, class time t+1
                        if(event_first_timeslot >= 0 && event_second_timeslot >= 0)
                        {
                            stream << "\n<AgentGroup>";
                                stream << "\n<ProfileSelector type=\"const\" name=\"group1\"/>";
                                stream << "\n<StateSelector type=\"const\" name=\"WalkToTarget" << room_second_timeslot << "\"/>";
                                stream << "\n<Generator type=\"rect_grid\" "
                                       << "anchor_x=\"" << building::room_targets[room_first_timeslot].x << "\" "
                                       << "anchor_y=\"" << building::room_targets[room_first_timeslot].y << "\" "
                                       << "offset_x=\"0.1\" "
                                       << "offset_y=\"0.1\" "
                                       << "count_x=\"" << nb_people_in_group / 5.0 << "\" "
                                       << "count_y=\"" << 5 << "\" "
                                       << "displace_dist=\"u\" "
                                       << "displace_min=\"0.0\" "
                                       << "displace_max=\"0.05\""
                                       << "/>";
                            stream << "\n</AgentGroup>";
                        }

                        // B. class time t, no class time t+1
                        else if(event_first_timeslot >= 0)
                        {
                            stream << "\n<AgentGroup>";
                                stream << "\n<ProfileSelector type=\"const\" name=\"group1\"/>";
                                stream << "\n<StateSelector type=\"const\" name=\"WalkToExit\"/>";
                                stream << "\n<Generator type=\"rect_grid\" "
                                       << "anchor_x=\"" << building::room_targets[room_first_timeslot].x << "\" "
                                       << "anchor_y=\"" << building::room_targets[room_first_timeslot].y << "\" "
                                       << "offset_x=\"0.1\" "
                                       << "offset_y=\"0.1\" "
                                       << "count_x=\"" << nb_people_in_group / 5.0 << "\" "
                                       << "count_y=\"" << 5 << "\" "
                                       << "displace_dist=\"u\" "
                                       << "displace_min=\"0.0\" "
                                       << "displace_max=\"0.05\""
                                       << "/>";
                            stream << "\n</AgentGroup>";
                        }

                        // C. no class time t, class time t+1
                        else if(event_second_timeslot >= 0)
                        {
                            std::uniform_int_distribution<int> dist_target(0, building::exit_targets.size() - 1);
                            int target_index = dist_target(generator);

                            stream << "\n<AgentGroup>";
                                stream << "\n<ProfileSelector type=\"const\" name=\"group1\"/>";
                                stream << "\n<StateSelector type=\"const\" name=\"WalkToTarget" << room_second_timeslot << "\"/>";
                                stream << "\n<Generator type=\"rect_grid\" "
                                       << "anchor_x=\"" << building::exit_targets[target_index].x << "\" "
                                       << "anchor_y=\"" << building::exit_targets[target_index].y << "\" "
                                       << "offset_x=\"0.1\" "
                                       << "offset_y=\"0.1\" "
                                       << "count_x=\"" << nb_people_in_group / 5.0 << "\" "
                                       << "count_y=\"" << 5 << "\" "
                                       << "displace_dist=\"u\" "
                                       << "displace_min=\"0.0\" "
                                       << "displace_max=\"0.05\""
                                       << "/>";
                            stream << "\n</AgentGroup>";
                        }

                        // D. no classes at time t or time t+1
                        // Do nothing

                    }
                }



                // Obstacles
                stream << "\n<ObstacleSet type=\"explicit\" class=\"1\">";

                    for(int i = 0; i < building::obstacles.size(); ++i)
                    {
                        stream << "\n<Obstacle closed=\"1\">";
                            for(int j = 0; j < building::obstacles[i].nb_vertices; ++j)
                            {
                                stream << "\n<Vertex p_x=\"" << building::obstacles[i].vertices_x[j] << "\" p_y=\"" << building::obstacles[i].vertices_y[j] << "\"/>";
                            }
                        stream << "\n</Obstacle>";
                    }

                    /*
                    for(int i = 0; i < building::obstacles.size(); ++i)
                    {
                        stream << "\n<Obstacle closed=\"1\">";
                            stream << "\n<Vertex p_x=\"" << building::obstacles[i].x_1 << "\" p_y=\"" << building::obstacles[i].y_1 << "\"/>";
                            stream << "\n<Vertex p_x=\"" << building::obstacles[i].x_2 << "\" p_y=\"" << building::obstacles[i].y_2 << "\"/>";
                        stream << "\n</Obstacle>";
                    }
                    */

                stream << "\n</ObstacleSet>";





            stream << "\n</Experiment>";
        }

    }

    void menge_interface::write_view_xml()
    {
        // calculate center of view
        double xpos, xmax = -1000000, xmin = 1000000;
        double ypos, ymax = -1000000, ymin = 1000000;
        // obstacles
        /*for(auto&& obs: building::obstacles)
        {
            // x coordinate
            if(obs.x_1 < xmin) {
                xmin = obs.x_1;
            }
            if(obs.x_2 < xmin) {
                xmin = obs.x_2;
            }
            if(obs.x_1 > xmax) {
                xmax = obs.x_1;
            }
            if(obs.x_2 > xmax) {
                xmax = obs.x_2;
            }

            // y coordinate
            if(obs.y_1 < ymin) {
                ymin = obs.y_1;
            }
            if(obs.y_2 < ymin) {
                ymin = obs.y_2;
            }
            if(obs.y_1 > ymax) {
                ymax = obs.y_1;
            }
            if(obs.y_2 > ymax) {
                ymax = obs.y_2;
            }
        }*/

        xpos = xmin + (xmax - xmin) / 2;
        ypos = ymin + (ymax - ymin) / 2;

        double zpos = 60;
        double ztgt = 0.0064413;



        QFile file("view.xml");
        if (file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            QTextStream stream(&file);

            // xml header
            stream << "<?xml version=\"1.0\"?>";

            // view tag
            stream << "\n<View width=\"640\" height=\"480\" z_up=\"1\">";

                // camera
                stream << "\n<Camera xpos=\"" << xpos << "\" ypos=\"" << ypos << "\" zpos=\"" << zpos << "\" "
                       << "xtgt=\"" << xpos << "\" ytgt=\"" << ypos << "\" ztgt=\"" << ztgt << "\" "
                       << "far=\"500\" near=\"0.01\" fov=\"0.0\" orthoScale=\"0.411132\"/>";

                // light
                stream << "\n<Light x=\"1\" y=\"0\" z=\"-1\" type=\"directional\" space=\"camera\" diffR=\"1\" diffG=\"0.8\" diffB=\"0.8\"/>";
                stream << "\n<Light x=\"-1\" y=\"0\" z=\"-1\" type=\"directional\" space=\"camera\" diffR=\"0.8\" diffG=\"0.8\" diffB=\"1\"/>";
                stream << "\n<Light x=\"0\" y=\"0\" z=\"1\" type=\"directional\" space=\"world\" diffR=\"0.8\" diffG=\"0.8\" diffB=\"0.8\"/>";

            stream << "\n</View>";
        }
    }



    void menge_interface::write_scene_xml_evacuation(const std::vector<int> &nb_people_per_room)
    {
        QFile file("scene.xml");
        if (file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            QTextStream stream(&file);

            // xml header
            stream << "<?xml version=\"1.0\"?>";

            // Experiment tag
            stream << "\n<Experiment version=\"2.0\">";


                stream << "<SpatialQuery type=\"kd-tree\" test_visibility=\"false\" />";


                // Behavioral models
                stream << "\n<OpenSteer max_force=\"" << _OpenSteer_max_force
                       << "\" leak_through=\"" << _OpenSteer_leak_through
                       << "\" reaction_time=\"" << _OpenSteer_reaction_time << "\"/>";
                stream << "\n<Common time_step=\"" << _Common_time_step << "\"/>";
                stream << "\n<GCF reaction_time=\"" << _GCF_reaction_time
                       << "\" max_agent_dist=\"" << _GCF_max_agent_dist
                       << "\" max_agent_force=\"" << _GCF_max_agent_force
                       << "\" agent_interp_width=\"" << _GCF_agent_interp_width
                       << "\" agent_force_strength=\"" << _GCF_agent_force_strength << "\"/>";
                stream << "\n<Helbing agent_scale=\"" << _Helbing_agent_scale
                       << "\" obstacle_scale=\"" << _Helbing_obstacle_scale
                       << "\" reaction_time=\"" << _Helbing_reaction_time
                       << "\" body_force=\"" << _Helbing_body_force
                       << "\" friction=\"" << _Helbing_friction
                       << "\" force_distance=\"" << _Helbing_force_distance << "\"/>";
                stream << "\n<Karamouzas orient_weight=\"" << _Karamouzas_orient_weight
                       << "\" fov=\"" << _Karamouzas_fov
                       << "\" reaction_time=\"" << _Karamouzas_reaction_time
                       << "\" wall_steepness=\"" << _Karamouzas_wall_steepness
                       << "\" wall_distance=\"" << _Karamouzas_wall_distance
                       << "\" colliding_count=\"" << _Karamouzas_colliding_count
                       << "\" d_min=\"" << _Karamouzas_d_min
                       << "\" d_mid=\"" << _Karamouzas_d_mid
                       << "\" d_max=\"" << _Karamouzas_d_max
                       << "\" agent_force=\"" << _Karamouzas_agent_force << "\"/>";
                stream << "\n<Zanlungo agent_scale=\"" << _Zanlungo_agent_scale
                       << "\" obstacle_scale=\"" << _Zanlungo_obstacle_scale
                       << "\" reaction_time=\"" << _Zanlungo_reaction_time
                       << "\" force_distance=\"" << _Zanlungo_force_distance << "\"/>";
                stream << "\n<Dummy stddev=\"" << _Dummy_stddev << "\"/>";



                // One agentprofile
                stream << "\n<AgentProfile name=\"group1\">";

                    stream << "\n<OpenSteer tau=\"" << _OpenSteer_tau
                           << "\" tauObst=\"" << _OpenSteer_tauObst << "\"/>";
                    stream << "\n<Common max_angle_vel=\"" << _Common_max_angle_vel
                           << "\" max_neighbors=\"" << _Common_max_neighbors
                           << "\" obstacleSet=\"" << _Common_obstacleSet
                           << "\" neighbor_dist=\"" << _Common_neighbor_dist
                           << "\" r=\"" << _Common_r
                           << "\" class=\"" << _Common_class
                           << "\" pref_speed=\"" << _Common_pref_speed
                           << "\" max_speed=\"" << _Common_max_speed
                           << "\" max_accel=\"" << _Common_max_accel << "\">"
                           << "\n<Property name=\"pref_speed\" dist=\"n\" mean=\"" << _Common_pref_speed << "\" stddev=\"" << _Common_pref_speed_stddev << "\"/>"
                           << "\n</Common>";
                    stream << "\n<PedVO factor=\"" << _PedVO_factor
                           << "\" buffer=\"" << _PedVO_buffer
                           << "\" tau=\"" << _PedVO_tau
                           << "\" tauObst=\"" << _PedVO_tauObst
                           << "\" turningBias=\"" << _PedVO_turningBias << "\"/>";
                    stream << "\n<GCF stand_depth=\"" << _GCF_stand_depth
                           << "\" move_scale=\"" << _GCF_move_scale
                           << "\" slow_width=\"" << _GCF_slow_width
                           << "\" sway_change=\"" << _GCF_sway_change << "\"/>";
                    stream << "\n<Helbing mass=\"" << _Helbing_mass << "\"/>";
                    stream << "\n<Karamouzas personal_space=\"" << _Karamouzas_personal_space
                           << "\" anticipation=\"" << _Karamouzas_anticipation << "\"/>";
                    stream << "\n<ORCA tau=\"" << _ORCA_tau
                           << "\" tauObst=\"" << _ORCA_tauObst << "\"/>";
                    stream << "\n<Zanlungo mass=\"" << _Zanlungo_mass
                           << "\" orient_weight=\"" << _Zanlungo_orient_weight << "\"/>";

                stream << "\n</AgentProfile>";



                // AgentGroups: one group per event-room
                for(int r = 0; r < timetable::nb_locations; ++r)
                {
                    stream << "\n<AgentGroup>";
                        stream << "\n<ProfileSelector type=\"const\" name=\"group1\"/>";
                        stream << "\n<StateSelector type=\"const\" name=\"WalkToExit\"/>";
                        stream << "\n<Generator type=\"rect_grid\" "
                               << "anchor_x=\"" << building::room_targets[r].x << "\" "
                               << "anchor_y=\"" << building::room_targets[r].y << "\" "
                               << "offset_x=\"0.1\" "
                               << "offset_y=\"0.1\" "
                               << "count_x=\"" << nb_people_per_room[r] / 5.0 << "\" "
                               << "count_y=\"" << 5 << "\" "
                               << "displace_dist=\"u\" "
                               << "displace_min=\"0.0\" "
                               << "displace_max=\"0.05\""
                               << "/>";
                    stream << "\n</AgentGroup>";
                }


                // Obstacles
                stream << "\n<ObstacleSet type=\"explicit\" class=\"1\">";

                    for(int i = 0; i < building::obstacles.size(); ++i)
                    {
                        stream << "\n<Obstacle closed=\"1\">";
                            for(int j = 0; j < building::obstacles[i].nb_vertices; ++j)
                            {
                                stream << "\n<Vertex p_x=\"" << building::obstacles[i].vertices_x[j] << "\" p_y=\"" << building::obstacles[i].vertices_y[j] << "\"/>";
                            }
                        stream << "\n</Obstacle>";
                    }

                    /*
                    for(int i = 0; i < building::obstacles.size(); ++i)
                    {
                        stream << "\n<Obstacle closed=\"1\">";
                            stream << "\n<Vertex p_x=\"" << building::obstacles[i].x_1 << "\" p_y=\"" << building::obstacles[i].y_1 << "\"/>";
                            stream << "\n<Vertex p_x=\"" << building::obstacles[i].x_2 << "\" p_y=\"" << building::obstacles[i].y_2 << "\"/>";
                        stream << "\n</Obstacle>";
                    }
                    */

                stream << "\n</ObstacleSet>";



            stream << "\n</Experiment>";
        }
    }

    void menge_interface::write_scene_xml_travel(const std::vector<int>& nb_people_from_room_to_room)
    {
        QFile file("scene.xml");
        if (file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            QTextStream stream(&file);

            // xml header
            stream << "<?xml version=\"1.0\"?>";

            // Experiment tag
            stream << "\n<Experiment version=\"2.0\">";


                stream << "<SpatialQuery type=\"kd-tree\" test_visibility=\"false\" />";


                // Behavioral models
                stream << "\n<OpenSteer max_force=\"" << _OpenSteer_max_force
                       << "\" leak_through=\"" << _OpenSteer_leak_through
                       << "\" reaction_time=\"" << _OpenSteer_reaction_time << "\"/>";
                stream << "\n<Common time_step=\"" << _Common_time_step << "\"/>";
                stream << "\n<GCF reaction_time=\"" << _GCF_reaction_time
                       << "\" max_agent_dist=\"" << _GCF_max_agent_dist
                       << "\" max_agent_force=\"" << _GCF_max_agent_force
                       << "\" agent_interp_width=\"" << _GCF_agent_interp_width
                       << "\" agent_force_strength=\"" << _GCF_agent_force_strength << "\"/>";
                stream << "\n<Helbing agent_scale=\"" << _Helbing_agent_scale
                       << "\" obstacle_scale=\"" << _Helbing_obstacle_scale
                       << "\" reaction_time=\"" << _Helbing_reaction_time
                       << "\" body_force=\"" << _Helbing_body_force
                       << "\" friction=\"" << _Helbing_friction
                       << "\" force_distance=\"" << _Helbing_force_distance << "\"/>";
                stream << "\n<Karamouzas orient_weight=\"" << _Karamouzas_orient_weight
                       << "\" fov=\"" << _Karamouzas_fov
                       << "\" reaction_time=\"" << _Karamouzas_reaction_time
                       << "\" wall_steepness=\"" << _Karamouzas_wall_steepness
                       << "\" wall_distance=\"" << _Karamouzas_wall_distance
                       << "\" colliding_count=\"" << _Karamouzas_colliding_count
                       << "\" d_min=\"" << _Karamouzas_d_min
                       << "\" d_mid=\"" << _Karamouzas_d_mid
                       << "\" d_max=\"" << _Karamouzas_d_max
                       << "\" agent_force=\"" << _Karamouzas_agent_force << "\"/>";
                stream << "\n<Zanlungo agent_scale=\"" << _Zanlungo_agent_scale
                       << "\" obstacle_scale=\"" << _Zanlungo_obstacle_scale
                       << "\" reaction_time=\"" << _Zanlungo_reaction_time
                       << "\" force_distance=\"" << _Zanlungo_force_distance << "\"/>";
                stream << "\n<Dummy stddev=\"" << _Dummy_stddev << "\"/>";



                // One agentprofile
                stream << "\n<AgentProfile name=\"group1\">";

                    stream << "\n<OpenSteer tau=\"" << _OpenSteer_tau
                           << "\" tauObst=\"" << _OpenSteer_tauObst << "\"/>";
                    stream << "\n<Common max_angle_vel=\"" << _Common_max_angle_vel
                           << "\" max_neighbors=\"" << _Common_max_neighbors
                           << "\" obstacleSet=\"" << _Common_obstacleSet
                           << "\" neighbor_dist=\"" << _Common_neighbor_dist
                           << "\" r=\"" << _Common_r
                           << "\" class=\"" << _Common_class
                           << "\" pref_speed=\"" << _Common_pref_speed
                           << "\" max_speed=\"" << _Common_max_speed
                           << "\" max_accel=\"" << _Common_max_accel << "\">"
                           << "\n<Property name=\"pref_speed\" dist=\"n\" mean=\"" << _Common_pref_speed << "\" stddev=\"" << _Common_pref_speed_stddev << "\"/>"
                           << "\n</Common>";
                    stream << "\n<PedVO factor=\"" << _PedVO_factor
                           << "\" buffer=\"" << _PedVO_buffer
                           << "\" tau=\"" << _PedVO_tau
                           << "\" tauObst=\"" << _PedVO_tauObst
                           << "\" turningBias=\"" << _PedVO_turningBias << "\"/>";
                    stream << "\n<GCF stand_depth=\"" << _GCF_stand_depth
                           << "\" move_scale=\"" << _GCF_move_scale
                           << "\" slow_width=\"" << _GCF_slow_width
                           << "\" sway_change=\"" << _GCF_sway_change << "\"/>";
                    stream << "\n<Helbing mass=\"" << _Helbing_mass << "\"/>";
                    stream << "\n<Karamouzas personal_space=\"" << _Karamouzas_personal_space
                           << "\" anticipation=\"" << _Karamouzas_anticipation << "\"/>";
                    stream << "\n<ORCA tau=\"" << _ORCA_tau
                           << "\" tauObst=\"" << _ORCA_tauObst << "\"/>";
                    stream << "\n<Zanlungo mass=\"" << _Zanlungo_mass
                           << "\" orient_weight=\"" << _Zanlungo_orient_weight << "\"/>";

                stream << "\n</AgentProfile>";




                // Agents
                for(int r1 = 0; r1 < timetable::nb_locations + 1; ++r1)
                {
                    for(int r2 = 0; r2 < timetable::nb_locations + 1; ++r2)
                    {
                        int nb_people_in_group = nb_people_from_room_to_room[r1 * (timetable::nb_locations + 1) + r2];


                        if(r1 == timetable::nb_locations && r2 == timetable::nb_locations)
                        {
                            // nothing
                        }
                        else if(r1 == timetable::nb_locations)
                        {
                            std::uniform_int_distribution<int> dist_target(0, building::exit_targets.size() - 1);
                            int target_index = dist_target(generator);

                            // people going inside from outside
                            stream << "\n<AgentGroup>";
                                stream << "\n<ProfileSelector type=\"const\" name=\"group1\"/>";
                                stream << "\n<StateSelector type=\"const\" name=\"WalkToTarget" << r2 << "\"/>";
                                stream << "\n<Generator type=\"rect_grid\" "
                                       << "anchor_x=\"" << building::exit_targets[target_index].x << "\" "
                                       << "anchor_y=\"" << building::exit_targets[target_index].y << "\" "
                                       << "offset_x=\"0.1\" "
                                       << "offset_y=\"0.1\" "
                                       << "count_x=\"" << nb_people_in_group / 5.0 << "\" "
                                       << "count_y=\"" << 5 << "\" "
                                       << "displace_dist=\"u\" "
                                       << "displace_min=\"0.0\" "
                                       << "displace_max=\"0.05\""
                                       << "/>";
                            stream << "\n</AgentGroup>";
                        }
                        else if(r2 == timetable::nb_locations)
                        {
                            // people going outside
                            stream << "\n<AgentGroup>";
                                stream << "\n<ProfileSelector type=\"const\" name=\"group1\"/>";
                                stream << "\n<StateSelector type=\"const\" name=\"WalkToExit\"/>";
                                stream << "\n<Generator type=\"rect_grid\" "
                                       << "anchor_x=\"" << building::room_targets[r1].x << "\" "
                                       << "anchor_y=\"" << building::room_targets[r1].y << "\" "
                                       << "offset_x=\"0.1\" "
                                       << "offset_y=\"0.1\" "
                                       << "count_x=\"" << nb_people_in_group / 5.0 << "\" "
                                       << "count_y=\"" << 5 << "\" "
                                       << "displace_dist=\"u\" "
                                       << "displace_min=\"0.0\" "
                                       << "displace_max=\"0.05\""
                                       << "/>";
                            stream << "\n</AgentGroup>";
                        }
                        else
                        {
                            // regular
                            stream << "\n<AgentGroup>";
                                stream << "\n<ProfileSelector type=\"const\" name=\"group1\"/>";
                                stream << "\n<StateSelector type=\"const\" name=\"WalkToTarget" << r2 << "\"/>";
                                stream << "\n<Generator type=\"rect_grid\" "
                                       << "anchor_x=\"" << building::room_targets[r1].x << "\" "
                                       << "anchor_y=\"" << building::room_targets[r1].y << "\" "
                                       << "offset_x=\"0.1\" "
                                       << "offset_y=\"0.1\" "
                                       << "count_x=\"" << nb_people_in_group / 5.0 << "\" "
                                       << "count_y=\"" << 5 << "\" "
                                       << "displace_dist=\"u\" "
                                       << "displace_min=\"0.0\" "
                                       << "displace_max=\"0.05\""
                                       << "/>";
                            stream << "\n</AgentGroup>";
                        }

                    }
                }




                // Obstacles
                stream << "\n<ObstacleSet type=\"explicit\" class=\"1\">";

                    for(int i = 0; i < building::obstacles.size(); ++i)
                    {
                        stream << "\n<Obstacle closed=\"1\">";
                            for(int j = 0; j < building::obstacles[i].nb_vertices; ++j)
                            {
                                stream << "\n<Vertex p_x=\"" << building::obstacles[i].vertices_x[j] << "\" p_y=\"" << building::obstacles[i].vertices_y[j] << "\"/>";
                            }
                        stream << "\n</Obstacle>";
                    }

                    /*
                    for(int i = 0; i < building::obstacles.size(); ++i)
                    {
                        stream << "\n<Obstacle closed=\"1\">";
                            stream << "\n<Vertex p_x=\"" << building::obstacles[i].x_1 << "\" p_y=\"" << building::obstacles[i].y_1 << "\"/>";
                            stream << "\n<Vertex p_x=\"" << building::obstacles[i].x_2 << "\" p_y=\"" << building::obstacles[i].y_2 << "\"/>";
                        stream << "\n</Obstacle>";
                    }
                    */

                stream << "\n</ObstacleSet>";



            stream << "\n</Experiment>";
        }

    }



}
