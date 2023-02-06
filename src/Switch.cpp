#include "Switch.h"
#include "GlobalParams.h"

void Switch::readPort() {
    while(1) {
        wait();
        if (!checkCollision()) {
            if (self_send_flag == true)
                self_send_flag = false;
            else {
                if ((input_enable & LEFT_MASK) && left.event()) {
                    flit_data = left.read();
                    flit_data.hop_no++;
                    fq.push(flit_data);
                    if (GlobalParams::output_mode > NORMAL_MODE)
                    {
                        clog << "SW " << id << ": " << getCurrentTimeStamp() << ": R from left " << flit_data << endl;
                    }
                    feq.notify(GlobalParams::switch_delay_ps, SC_PS);
                }

                if ((input_enable & MID_MASK) && mid.event()) {
                    flit_data = mid.read();
                    flit_data.hop_no++;
                    fq.push(flit_data);
                    if (GlobalParams::output_mode > NORMAL_MODE)
                    {
                        clog << "SW " << id << ": " << getCurrentTimeStamp() << ": R from mid " << flit_data << endl;
                    }
                    feq.notify(GlobalParams::switch_delay_ps, SC_PS);                    
                }

                if ((input_enable & RIGHT_MASK) && right.event()){
                    flit_data = right.read();
                    flit_data.hop_no++;
                    fq.push(flit_data);
                    if (GlobalParams::output_mode > NORMAL_MODE)
                    {
                        clog << "SW " << id << ": " << getCurrentTimeStamp() << ": R from right " << flit_data << endl;
                    }
                    feq.notify(GlobalParams::switch_delay_ps, SC_PS);                            
                }
            }
        }
    }
}

void Switch::writePort() {
    while (1) {
        wait();
        if (!checkCollision()) {
            if (output_enable & RIGHT_MASK) {
                flit_data = fq.front();
                if (GlobalParams::output_mode > NORMAL_MODE)
                {
                        clog << "SW " << id << ": " << getCurrentTimeStamp() << ": S to right " << flit_data
                        // << " : " << sc_delta_count() 
                        << endl;                    
                }
                if (right.event())
                    clog << "Write COLLISION on switch " << id << " at time " << getCurrentTimeStamp() << endl;
                sc_spawn( [&](){
                    wait(GlobalParams::link_delay_ps, SC_PS);
                    right.write(flit_data);
                });             

            }

            if (output_enable & MID_MASK) {
                flit_data = fq.front(); 
                if (GlobalParams::output_mode > NORMAL_MODE)
                {
                        clog << "SW " << id << ": " << getCurrentTimeStamp() << ": S to mid " << flit_data
                        // << " : " << sc_delta_count() 
                        << endl;                    
                }               
                if (mid.event())
                    clog << "Write COLLISION on switch " << id << " at time " << getCurrentTimeStamp() << endl;
                sc_spawn( [&](){
                    wait(GlobalParams::link_delay_ps, SC_PS);
                    mid.write(flit_data);      
                });                                   
            }

            if (output_enable & LEFT_MASK) {
                flit_data = fq.front(); 
                if (GlobalParams::output_mode > NORMAL_MODE)
                {
                        clog << "SW " << id << ": " << getCurrentTimeStamp() << ": S to left " << flit_data
                        // << " : " << sc_delta_count() 
                        << endl;
                }           
                if (left.event())
                    clog << "Write COLLISION on switch " << id << " at time " << getCurrentTimeStamp() << endl;   
                sc_spawn( [&](){
                    wait(GlobalParams::link_delay_ps, SC_PS);
                    left.write(flit_data);  
                });                                                    
            }

            self_send_flag = true;
            fq.pop();
        }
    }
}

int Switch::getSwitchId() {
    return id;
}

bool Switch::checkCollision() {
    if (input_enable & output_enable) {
        clog << "Collision detected on switch " << id << " at time " << getCurrentTimeStamp() <<"!\n";
        return true;
    }
    else 
        return false;
}

void Switch::readControl() {
    while (1) {
        wait();

        input_enable = control_in.read().to_int();
        output_enable = control_out.read().to_int();
    
        // sc_bv<NUM_SW_PORTS*2> control_value;
        // control_value.range(NUM_SW_PORTS-1, 0) = control_in.read();
        // control_value.range(NUM_SW_PORTS*2-1, NUM_SW_PORTS) = control_out.read();
        // cq.push(control_value);
        // ceq.notify(CONTROL_DELAY_NS, SC_NS);
    }
}

void Switch::updateControl() {
    while (1) {
        wait();

        // sc_bv<NUM_SW_PORTS*2> control_value = cq.front();
        // input_enable = control_value.range(NUM_SW_PORTS-1, 0).to_int();
        // output_enable = control_value.range(NUM_SW_PORTS*2-1, NUM_SW_PORTS).to_int();
        // cq.pop();
        // deq.notify(SC_ZERO_TIME);
    }
}