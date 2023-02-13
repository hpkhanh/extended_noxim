#ifndef __SWITCHCONTROLLER_H__
#define __SWITCHCONTROLLER_H__

#include <systemc.h>
#include "DataStructs.h"


SC_MODULE (SwitchController) {
    sc_out <sc_bv<NUM_SW_PORTS>> * sw_in, * sw_out;
    sc_in_clk clock;		// The input clock

    SC_HAS_PROCESS(SwitchController);
    SwitchController (sc_module_name module_name, int _num_sw);

    private:
        int num_sw;

        void updatePort();

};

#endif /* __SWITCHCONTROLLER_H__ */
