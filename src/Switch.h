#pragma once
#ifndef __SWITCH_H__
#define __SWITCH_H__

#define SC_INCLUDE_DYNAMIC_PROCESSES

#include <systemc.h>
#include <queue>
#include "DataStructs.h"
#include "Utils.h"
#include "BusSignal.h"


#define CONTROL_DELAY_NS	1
#define LEFT_MASK 0b100
#define MID_MASK 0b010
#define RIGHT_MASK 0b001


SC_MODULE(Switch) {

	SC_HAS_PROCESS(Switch);

	sc_inout<Flit> left, mid, right;
	bus_signal<Flit> *left_bus, *mid_bus, *right_bus;
	// sc_in<Flit> left_in, mid_in, right_in;
	// sc_out<Flit> left_out, mid_out, right_out;
	sc_in <sc_bv<NUM_SW_PORTS>> control_in, control_out;
	Flit flit_data;

	int input_enable;
	int output_enable;
	int id;

	bool self_send_flag;
	bool control_update_flag;
	bool left_drive, mid_drive, right_drive;

	void readPort();
	void writePort();

	void readControl();
	void updateControl();

	int getSwitchId();
	bool checkCollision();
	void releaseBus();

	std::queue<Flit> fq;
	std::queue<sc_bv<NUM_SW_PORTS*2>> cq;
	sc_event_queue feq, ceq, deq;

	// Constructor
	Switch(sc_module_name module_name, int _id): sc_module(module_name) {

		id = _id;
		output_enable = 0b000;
		input_enable = 0b000;
		self_send_flag = false;
		control_update_flag = false;

		SC_THREAD(readPort);
        sensitive << left << mid << right << ceq;
		// dont_initialize();
		SC_THREAD(writePort);
		sensitive << feq;
		// dont_initialize();

		SC_THREAD(readControl);
		sensitive << control_in << control_out;

		SC_THREAD(updateControl);
		sensitive << ceq;

	}
};

#endif /* __SWITCH_H__ */
