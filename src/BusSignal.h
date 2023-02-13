#ifndef __BUSSIGNAL_H__
#define __BUSSIGNAL_H__

#include <systemc.h>
#include "DataStructs.h"

template<class T, sc_writer_policy POL = SC_DEFAULT_WRITER_POLICY>
class bus_signal : public sc_signal<T, POL> {

    public:
        bus_signal<T, POL>():sc_signal<T, POL>(), is_driven(0) {
        }
        int write_drive(const T &t, bool &succeed);
        int read_check(T &t, bool &valid);
        int release_drive();

    private:
        sc_semaphore is_driven;

};


template<class T, sc_writer_policy POL>
int bus_signal<T, POL>::write_drive(const T &t, bool &succeed) {
    // clog << t << endl;
    // T t_copy = t;
    is_driven.post();
    if (is_driven.get_value() >= 2) {
        succeed = false;
        is_driven.trywait();
    } 
    else {
        succeed = true;
        sc_spawn( [&](){
            wait(GlobalParams::link_delay_ps, SC_PS);
            sc_signal<T>::write(t);
            // clog << t_copy << endl;
        }); 
    }
    return is_driven.get_value();
}

template<class T, sc_writer_policy POL>
int bus_signal<T, POL>::read_check(T &t, bool &valid) {
    if (is_driven.get_value() >= 1) {
        valid = true;
        t = sc_signal<T>::read();
        // clog << t << endl;
    }
    else {
        valid = false;
    }
    return is_driven.get_value();
}

template<class T,  sc_writer_policy POL>
int bus_signal<T, POL>::release_drive()  {
    is_driven.trywait();
    return is_driven.get_value();
}

#endif /* __BUSSIGNAL_H__ */
