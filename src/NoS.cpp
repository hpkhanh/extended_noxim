#include "NoS.h"
#include "GlobalParams.h"

void NoS::buildNetwork() {
    // if (GlobalParams::traffic_distribution == TRAFFIC_TRACE_BASED)
	// {
	// 	cout << "Loading Traffic Trace File ...";
	// 	assert(gtr_trace.load(GlobalParams::traffic_trace_filename.c_str()));
	// 	cout << "Done" << endl;
	// }

    // sw = new Switch("sw1", 0);
    // // sw = new SwitchTest("sw1");
    // pe1 = new BusProcessingElement("pe1");
    // pe2 = new BusProcessingElement("pe2");
    // pe1->global_traffic_trace = &gtr_trace;
    // pe2->global_traffic_trace = &gtr_trace;
    // pe1 = new PETest("pe1");
    // pe2 = new PETest("pe2");
    // sc_signal<Flit, SC_MANY_WRITERS> rtx_pe1, rtx_pe2, rtx_end;

    // pe1->clock(clk);
    // pe1->reset(reset);
    // pe1->flit_rtx(left);
    // pe1->local_id = 0;

    // pe2->clock(clk);
    // pe2->reset(reset);
    // pe2->flit_rtx(right);
    // pe2->local_id = 1;

    // sw->left(left);
    // sw->right(right);
    // sw->mid(mid);

    // sw->control_in(control_in);
    // sw->control_out(control_out);

    // control_in.write(0b100);
    // control_out.write(0b001);

    // sw->input_enable = 0b100;
    // sw->output_enable = 0b001;

    // sw->in1(left_in);
    // sw->in2(right_in);
    // sw->in3(mid_in);
    // sw->out1(left_out);
    // sw->out2(right_out);
    // sw->out3(mid_out);

    // left_in = 0;
    // right_in = 0;
    // mid_in = 1;
    // pe1->rtx_mode = WRITE_MODE;
    // pe2->rtx_mode = READ_MODE;
}

void NoS::buildNetwork2() {
    if (GlobalParams::traffic_distribution == TRAFFIC_TRACE_BASED)
	{
		cout << "Loading Traffic Trace File ...";
		assert(gtr_trace.load(GlobalParams::traffic_trace_filename.c_str()));
		cout << "Done" << endl;
	}

    swc = new SwitchController("swc_0", NUM_SWITCHES);
    
    swc->clock(clk);

    // Connect the switches to the bus
    for (int i = 0; i < NUM_SWITCHES; i++) {
        char sw_name[32];
        sprintf(sw_name, "PE_%d", i);
        sw[i] = new Switch(sw_name, i);

        sw[i]->control_in(control_in[i]);
        sw[i]->control_out(control_out[i]);
        sw[i]->left(main_bus[i]);
        sw[i]->right(main_bus[i+1]);
        sw[i]->mid(side_bus[i]);
        sw[i]->left_bus = &main_bus[i];
        sw[i]->right_bus = &main_bus[i+1];
        sw[i]->mid_bus = &side_bus[i];

        swc->sw_in[i](control_in[i]);
        swc->sw_out[i](control_out[i]);
    }

    for (int i = 0; i < NUM_PES; i++) {
        char pe_name[32];
        sprintf(pe_name, "BPE_%d", i);
        pe[i] = new BusProcessingElement(pe_name);

        pe[i]->local_id = i;
        pe[i]->clock(clk);
        pe[i]->reset(reset);
        pe[i]->flit_rtx(side_bus[i*2+1]);
        pe[i]->rtx_bus = &side_bus[i*2+1];
        pe[i]->global_traffic_trace = &gtr_trace;
    }

    // 0 to 1 & 2
    // sw[1]->input_enable = 0b010;
    // sw[1]->output_enable = 0b001;
    // sw[2]->input_enable = 0b100;
    // sw[2]->output_enable = 0b001;    
    // sw[3]->input_enable = 0b100;
    // sw[3]->output_enable = 0b011;
    // sw[4]->input_enable = 0b100;
    // sw[4]->output_enable = 0b001; 
    // sw[5]->input_enable = 0b100;
    // sw[5]->output_enable = 0b010;

    // 0 to 1
    sw[1]->input_enable = 0b010;
    sw[1]->output_enable = 0b001;
    sw[2]->input_enable = 0b100;
    sw[2]->output_enable = 0b001;    
    sw[3]->input_enable = 0b100;
    sw[3]->output_enable = 0b010;



}