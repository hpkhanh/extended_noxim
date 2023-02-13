#include "Switch.h"
#include "GlobalParams.h"

void Switch::readPort() {
    while(1) {
        wait();
        if (!checkCollision()) {
            if (self_send_flag == true)
                self_send_flag = false;
            else {
                if ((input_enable & LEFT_MASK) && (left.event() || control_update_flag)) {
                    if (control_update_flag)
                        control_update_flag = false;
                    bool valid;
                    Flit flit;
                    int lock_value = left_bus->read_check(flit, valid);
                    // clog << "SW " << id << ": " << getCurrentTimeStamp() << ": R lock value left " << lock_value << endl;
                    if (valid) {
                        flit.hop_no++;
                        fq.push(flit);
                        if (GlobalParams::output_mode > NORMAL_MODE)
                        {
                            clog << "SW " << id << ": " << getCurrentTimeStamp() << ": R from left " << flit << endl;
                        }
                        feq.notify(GlobalParams::switch_delay_ps, SC_PS);
                    }
                }

                if ((input_enable & MID_MASK) && (mid.event() || control_update_flag)) {
                    if (control_update_flag)
                        control_update_flag = false;                    
                    bool valid;
                    Flit flit;
                    int lock_value = mid_bus->read_check(flit, valid);
                    // clog << "SW " << id << ": " << getCurrentTimeStamp() << ": R lock value mid " << lock_value << endl;
                    if (valid) {
                        flit.hop_no++;
                        fq.push(flit);
                        if (GlobalParams::output_mode > NORMAL_MODE)
                        {
                            clog << "SW " << id << ": " << getCurrentTimeStamp() << ": R from mid " << flit << endl;
                        }
                        feq.notify(GlobalParams::switch_delay_ps, SC_PS);  
                    }                  
                }

                if ((input_enable & RIGHT_MASK) && (right.event() || control_update_flag)){
                    if (control_update_flag)
                        control_update_flag = false;                    
                    bool valid;
                    Flit flit;
                    int lock_value = right_bus->read_check(flit, valid);
                    // clog << "SW " << id << ": " << getCurrentTimeStamp() << ": R lock value right " << lock_value << endl;
                    if (valid) {
                        flit.hop_no++;
                        fq.push(flit);                   
                        if (GlobalParams::output_mode > NORMAL_MODE)
                        {
                            clog << "SW " << id << ": " << getCurrentTimeStamp() << ": R from right " << flit << endl;
                        }
                        feq.notify(GlobalParams::switch_delay_ps, SC_PS); 
                    }                           
                }
            }
        }
    }
}

void Switch::writePort() {
    while (1) {
        wait();
        if (!checkCollision()) {
            if (output_enable & LEFT_MASK) {
                flit_data = fq.front();
                if (GlobalParams::output_mode > NORMAL_MODE)
                {
                        clog << "SW " << id << ": " << getCurrentTimeStamp() << ": S to left " << flit_data
                        // << " : " << sc_delta_count() 
                        << endl;                    
                }

                bool succeed;
                if (left_drive)
                    left_bus->release_drive();                
                int lock_value = left_bus->write_drive(flit_data, succeed);
                // clog << "SW " << id << ": " << getCurrentTimeStamp() << ": S lock value left " << lock_value << endl;
                if (!succeed)
                    clog << "Write COLLISION on switch " << id << " at time " << getCurrentTimeStamp() << endl;      
                else {
                    left_drive = true;
                }     
            }

            if (output_enable & MID_MASK) {
                flit_data = fq.front(); 
                if (GlobalParams::output_mode > NORMAL_MODE)
                {
                        clog << "SW " << id << ": " << getCurrentTimeStamp() << ": S to mid " << flit_data
                        // << " : " << sc_delta_count() 
                        << endl;                    
                }               

                bool succeed;
                if (mid_drive)
                    mid_bus->release_drive();                
                int lock_value = mid_bus->write_drive(flit_data, succeed);
                // clog << "SW " << id << ": " << getCurrentTimeStamp() << ": S lock value mid " << lock_value << endl;
                if (!succeed)
                    clog << "Write COLLISION on switch " << id << " at time " << getCurrentTimeStamp() << endl;  
                else {
                    mid_drive = true;
                }                                  
            }

            if (output_enable & RIGHT_MASK) {
                flit_data = fq.front(); 
                if (GlobalParams::output_mode > NORMAL_MODE)
                {
                        clog << "SW " << id << ": " << getCurrentTimeStamp() << ": S to right " << flit_data
                        // << " : " << sc_delta_count() 
                        << endl;
                }           

                bool succeed;
                if (right_drive)
                    right_bus->release_drive();
                int lock_value = right_bus->write_drive(flit_data, succeed);
                // clog << "SW " << id << ": " << getCurrentTimeStamp() << ": S lock value right " << lock_value << endl;
                if (!succeed)
                    clog << "Write COLLISION on switch " << id << " at time " << getCurrentTimeStamp() << endl;     
                else {
                    right_drive = true;
                }                                                
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
    int left_in = (input_enable & LEFT_MASK) >> 2;
    int mid_in = (input_enable & MID_MASK) >> 1;
    int right_in = (input_enable & RIGHT_MASK);

    if ((input_enable & output_enable) || (left_in & mid_in) ||
        (left_in & right_in) || (right_in & mid_in)){
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
        control_update_flag = true;
        releaseBus();

        ceq.notify(SC_ZERO_TIME);
    
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

void Switch::releaseBus() {
    // if (!(output_enable & LEFT_MASK) && left_drive) {
    //     left_bus->release_drive();
    //     left_drive = false;
    //     clog << "Switch " << id << " release left bus at time " << getCurrentTimeStamp() <<"!\n";
    // }
    // if (!(output_enable & MID_MASK) && mid_drive) {
    //     mid_bus->release_drive();
    //     mid_drive = false;
    //     clog << "Switch " << id << " release mid bus at time " << getCurrentTimeStamp() <<"!\n";
    // }
    // if (!(output_enable & RIGHT_MASK) && right_drive) {
    //     right_bus->release_drive();
    //     right_drive = false;
    //     clog << "Switch " << id << " release right bus at time " << getCurrentTimeStamp() <<"!\n";
    // }
    left_bus->release_drive();
    left_drive = false;
    mid_bus->release_drive();
    mid_drive = false;
    right_bus->release_drive();
    right_drive = false;
    clog << "Switch " << id << " release bus at time " << getCurrentTimeStamp() <<"!\n";
}