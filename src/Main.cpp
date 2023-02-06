/*
 * Noxim - the NoC Simulator
 *
 * (C) 2005-2018 by the University of Catania
 * For the complete list of authors refer to file ../doc/AUTHORS.txt
 * For the license applied to these sources refer to file ../doc/LICENSE.txt
 *
 * This file contains the implementation of the top-level of Noxim
 */

#include "ConfigurationManager.h"
#include "NoC.h"
#include "NoS.h"
#include "GlobalStats.h"
#include "DataStructs.h"
#include "GlobalParams.h"

#include <csignal>

using namespace std;

// need to be globally visible to allow "-volume" simulation stop
unsigned int drained_volume;
NoC *n;
NoS * m;

void signalHandler( int signum )
{
    cout << "\b\b  " << endl;
    cout << endl;
    cout << "Current Statistics:" << endl;
    cout << "(" << sc_time_stamp().to_double() / GlobalParams::clock_period_ps << " sim cycles executed)" << endl;
    GlobalStats gs(n);
    gs.showStats(std::cout, GlobalParams::detailed);
}

int sc_main(int arg_num, char *arg_vet[])
{
    signal(SIGQUIT, signalHandler);  

    // TEMP
    drained_volume = 0;

    // Handle command-line arguments
    cout << "\t--------------------------------------------" << endl; 
    cout << "\t\tNoxim - the NoC Simulator" << endl;
    cout << "\t\t(C) University of Catania" << endl;
    cout << "\t--------------------------------------------" << endl; 

    cout << "Catania V., Mineo A., Monteleone S., Palesi M., and Patti D. (2016) Cycle-Accurate Network on Chip Simulation with Noxim. ACM Trans. Model. Comput. Simul. 27, 1, Article 4 (August 2016), 25 pages. DOI: https://doi.org/10.1145/2953878" << endl;
    cout << endl;
    cout << endl;

    configure(arg_num, arg_vet);

    
    sc_time_unit sim_unit;
    int reset_time;
    int simulation_time;
    double clock_period_log = log10(GlobalParams::clock_period_ps);


    if (clock_period_log < 3) {
        sim_unit = SC_PS;
        reset_time = GlobalParams::reset_time * GlobalParams::clock_period_ps;
        simulation_time = GlobalParams::simulation_time * GlobalParams::clock_period_ps;
    }
    else if (clock_period_log < 6) {
        sim_unit = SC_NS;
        reset_time = (int)((double)GlobalParams::reset_time * (double)(GlobalParams::clock_period_ps/1000));
        simulation_time = (int)((double)GlobalParams::simulation_time * (double)(GlobalParams::clock_period_ps/1000));
    }
    else if (clock_period_log < 9) {
        sim_unit = SC_US;
        reset_time = (int)((double)GlobalParams::reset_time * (double)(GlobalParams::clock_period_ps/10^6));
        simulation_time = (int)((double)GlobalParams::simulation_time * (double)(GlobalParams::clock_period_ps/10^6));
    }
    else if (clock_period_log < 12) {
        sim_unit = SC_MS;
        reset_time = (int)((double)GlobalParams::reset_time * (double)(GlobalParams::clock_period_ps/10^9));
        simulation_time = (int)((double)GlobalParams::simulation_time * (double)(GlobalParams::clock_period_ps/10^9));
    }

    // Signals
    sc_clock clock("clock", GlobalParams::clock_period_ps, SC_PS);
    sc_signal <bool> reset;

    if (GlobalParams::topology != TOPOLOGY_DSB) {
        // NoC instance
        n = new NoC("NoC");
        n->clock(clock);
        n->reset(reset);
    }
    else {
        m = new NoS("NoS");
        m->clk(clock);
        m->reset(reset);
    }

    // Change clog stream to output file
    ofstream output;
    streambuf * clogbuf = clog.rdbuf();
    output.open(GlobalParams::output_filename.c_str(), ios::trunc);
    clog.rdbuf(output.rdbuf());

    // Trace signals
    sc_trace_file *tf = NULL;
    if (GlobalParams::topology != TOPOLOGY_DSB) {
        if (GlobalParams::trace_mode) {
            tf = sc_create_vcd_trace_file(GlobalParams::trace_filename.c_str());
            sc_trace(tf, reset, "reset");
            sc_trace(tf, clock, "clock");

            for (int i = 0; i < GlobalParams::mesh_dim_x; i++) {
                for (int j = 0; j < GlobalParams::mesh_dim_y; j++) {
                    char label[64];

                    sprintf(label, "req(%02d)(%02d).east", i, j);
                    sc_trace(tf, n->req[i][j].east, label);
                    sprintf(label, "req(%02d)(%02d).west", i, j);
                    sc_trace(tf, n->req[i][j].west, label);
                    sprintf(label, "req(%02d)(%02d).south", i, j);
                    sc_trace(tf, n->req[i][j].south, label);
                    sprintf(label, "req(%02d)(%02d).north", i, j);
                    sc_trace(tf, n->req[i][j].north, label);

                    sprintf(label, "ack(%02d)(%02d).east", i, j);
                    sc_trace(tf, n->ack[i][j].east, label);
                    sprintf(label, "ack(%02d)(%02d).west", i, j);
                    sc_trace(tf, n->ack[i][j].west, label);
                    sprintf(label, "ack(%02d)(%02d).south", i, j);
                    sc_trace(tf, n->ack[i][j].south, label);
                    sprintf(label, "ack(%02d)(%02d).north", i, j);
                    sc_trace(tf, n->ack[i][j].north, label);
                }
            }
        }
    }
    // Reset the chip and run the simulation
    reset.write(1);
    cout << "Reset for " << (int)(GlobalParams::reset_time) << " cycles... ";
    srand(GlobalParams::rnd_generator_seed);
    sc_start(reset_time, sim_unit);

    reset.write(0);
    cout << " done! " << endl;
    cout << "Now running for " << GlobalParams:: simulation_time << " cycles..." << endl;
    sc_start(simulation_time, sim_unit);

    // Close the simulation
    if (GlobalParams::trace_mode) sc_close_vcd_trace_file(tf);
    cout << "Noxim simulation completed.";
    cout << " (" << sc_time_stamp().to_double() / GlobalParams::clock_period_ps << " cycles executed)" << endl;
    cout << endl;
    //assert(false);
    if (GlobalParams::topology != TOPOLOGY_DSB) {
        // Show statistics
        GlobalStats gs(n);
        gs.showStats(std::cout, GlobalParams::detailed);
    }


    if ((GlobalParams::max_volume_to_be_drained > 0) &&
	(sc_time_stamp().to_double() / GlobalParams::clock_period_ps - GlobalParams::reset_time >=
	 GlobalParams::simulation_time)) {
	cout << endl
         << "WARNING! the number of flits specified with -volume option" << endl
	     << "has not been reached. ( " << drained_volume << " instead of " << GlobalParams::max_volume_to_be_drained << " )" << endl
         << "You might want to try an higher value of simulation cycles" << endl
	     << "using -sim option." << endl;

#ifdef TESTING
	cout << endl
         << " Sum of local drained flits: " << gs.drained_total << endl
	     << endl
         << " Effective drained volume: " << drained_volume;
#endif

    }

#ifdef DEADLOCK_AVOIDANCE
	cout << "***** WARNING: DEADLOCK_AVOIDANCE ENABLED!" << endl;
#endif
    // Restor clog stream buffer
    clog.rdbuf(clogbuf);
    output.close();

    return 0;
}
