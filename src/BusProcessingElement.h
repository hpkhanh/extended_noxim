#ifndef __BUSPROCESSINGELEMENT_H__
#define __BUSPROCESSINGELEMENT_H__


#include <queue>
#include <systemc.h>

#include "DataStructs.h"
#include "GlobalTrafficTable.h"
#include "GlobalTrafficTrace.h"
#include "Utils.h"

typedef enum TransceiveMode_e
{
    BLOCKED_MODE    = 0,
    READ_MODE       = 1,
    WRITE_MODE      = 2,
} TransceiveMode;

using namespace std;

SC_MODULE(BusProcessingElement)
{

    // I/O Ports
    sc_in_clk clock;		// The input clock for the PE
    sc_in < bool > reset;	// The reset signal for the PE

    sc_inout < Flit > flit_rtx;	// The input output channel

    // Registers
    int local_id;		// Unique identification number
    queue < Packet > packet_queue;	// Local queue of packets
    bool transmittedAtPreviousCycle;	// Used for distributions with memory
    bool selfSendFlag;
    sc_event_queue eq;
    queue <Flit> flit_queue;

    // Functions
    void rxProcess();		// The receiving process
    void txProcess();		// The transmitting process
    void txProcessLink();
    bool canShot(Packet & packet);	// True when the packet must be shot
    Flit nextFlit();	// Take the next flit of the current packet
    Packet trafficTest();	// used for testing traffic
    Packet trafficRandom();	// Random destination distribution
    Packet trafficTranspose1();	// Transpose 1 destination distribution
    Packet trafficTranspose2();	// Transpose 2 destination distribution
    Packet trafficBitReversal();	// Bit-reversal destination distribution
    Packet trafficShuffle();	// Shuffle destination distribution
    Packet trafficButterfly();	// Butterfly destination distribution
    Packet trafficULocal();	// Random with locality

    GlobalTrafficTable *traffic_table;	// Reference to the Global traffic Table
    GlobalTrafficTrace *global_traffic_trace;	// Reference to the Global traffic Table

    bool never_transmit;	// true if the PE does not transmit any packet 
    //  (valid only for the table based traffic)

    TransceiveMode rtx_mode;

    void fixRanges(const Coord, Coord &);	// Fix the ranges of the destination
    int randInt(int min, int max);	// Extracts a random integer number between min and max
    int getRandomSize();	// Returns a random size in flits for the packet
    void setBit(int &x, int w, int v);
    int getBit(int x, int w);
    double log2ceil(double x);

    int roulette();
    int findRandomDestination(int local_id,int hops);
    unsigned int getQueueSize() const;

    void printRxFlit(Flit & flit);
    void printTxFlit(Flit & flit);

    // Constructor
    SC_CTOR(BusProcessingElement) 
    {
        SC_METHOD(rxProcess);
        sensitive << reset;
        sensitive << flit_rtx;
        // dont_initialize();
        // sensitive << clock.pos();

        SC_METHOD(txProcess);
        sensitive << reset;
        sensitive << clock.pos();
        // dont_initialize();

        SC_THREAD(txProcessLink);
        sensitive << eq;
        // dont_initialize();
    }

};

#endif /* __BUSPROCESSINGELEMENT_H__ */
