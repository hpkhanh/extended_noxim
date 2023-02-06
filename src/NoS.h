#ifndef __NOS_H__
#define __NOS_H__

#include <systemc.h>
#include "BusProcessingElement.h"
#include "GlobalTrafficTrace.h"
#include "Switch.h"
#include "SwitchTest.h"
#include "PETest.h"

#define NUM_PES             3
#define NUM_SWITCHES        (NUM_PES*2 + 1)

SC_MODULE(NoS) {

    sc_in_clk clk;
	sc_in<bool> reset;

    Switch *sw[NUM_SWITCHES];
    // SwitchTest *sw;
    BusProcessingElement * pe1;
    BusProcessingElement * pe2;
    BusProcessingElement * pe[NUM_PES];
    // PETest * pe1;
    // PETest * pe2;
    // sc_signal<bool> left_in, left_out, mid_in, mid_out, right_in, right_out;
    sc_signal<Flit> main_bus[NUM_SWITCHES+1];
    sc_signal<Flit> side_bus[NUM_SWITCHES];
    sc_signal<Flit> left, mid, right;
    sc_signal <sc_bv<NUM_SW_PORTS>> control_in[NUM_SWITCHES];
    sc_signal <sc_bv<NUM_SW_PORTS>> control_out[NUM_SWITCHES];

    GlobalTrafficTrace gtr_trace;

	// Methods
	void buildNetwork();
    void buildNetwork2();

    // Constructor
    SC_CTOR(NoS) {
        buildNetwork2();
    }
};

#endif /* __NOS_H__ */
