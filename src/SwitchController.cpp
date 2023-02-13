#include "SwitchController.h"
#include "Utils.h"

// SC_HAS_PROCESS(SwitchController);

SwitchController::SwitchController (sc_module_name module_name, int _num_sw) : sc_module(module_name) {
    assert (_num_sw >= 0);

    num_sw = _num_sw;
    sw_in = new sc_out <sc_bv<NUM_SW_PORTS>>[num_sw];
    sw_out = new sc_out <sc_bv<NUM_SW_PORTS>>[num_sw];

    SC_METHOD(updatePort);
    sensitive << clock.pos();
}

void SwitchController::updatePort() {
    if ((int)getCurrentTimeStamp() % 20 == 19) {
        sw_in[1].write(0b001);
        sw_out[1].write(0b010);
        sw_in[2].write(0b001);
        sw_out[2].write(0b100);    
        sw_in[3].write(0b010);
        sw_out[3].write(0b101);
        // sw_in[4].write(0b100);
        // sw_out[4].write(0b001); 
        // sw_in[5].write(0b100);
        // sw_out[5].write(0b010); 
    } 

}
