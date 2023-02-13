/*
 * Noxim - the NoC Simulator
 *
 * (C) 2005-2018 by the University of Catania
 * For the complete list of authors refer to file ../doc/AUTHORS.txt
 * For the license applied to these sources refer to file ../doc/LICENSE.txt
 *
 * This file contains the implementation of the processing element
 */

#include "BusProcessingElement.h"
#include <vector>

int BusProcessingElement::randInt(int min, int max)
{
    return min +
	(int) ((double) (max - min + 1) * rand() / (RAND_MAX + 1.0));
}

void BusProcessingElement::rxProcess()
{    
    if (reset.read()) {
        rtx_bus->release_drive();
        return;
    } 

    if (selfSendFlag) {
        selfSendFlag = false;
        return;
    }

    if (drive_bus) {
        rtx_bus->release_drive();
        drive_bus = false;
    }
    if (flit_rtx.event()) {
        bool valid;
        Flit flit_tmp;
        rtx_bus->read_check(flit_tmp, valid);
        if (valid)
            printRxFlit(flit_tmp); 
    }           
}

void BusProcessingElement::txProcess()
{
    if (reset.read()) {
	    transmittedAtPreviousCycle = false;
        return;
    } 

    if (drive_bus) {
        rtx_bus->release_drive();
        drive_bus = false;
    }

    Packet packet;
    if (canShot(packet)) {
        packet_queue.push(packet);
        transmittedAtPreviousCycle = true;
    } else
        transmittedAtPreviousCycle = false;

    if (!packet_queue.empty()) {
        selfSendFlag = true;
        Flit flit = nextFlit();	// Generate a new flit
        flit_queue.push(flit);
        eq.notify(SC_ZERO_TIME);
        printTxFlit(flit);
        // bool succeed;
        // rtx_bus->write_drive(flit, succeed);
        // if (!succeed)
        //     clog << "Write COLLISION on PE " << local_id << " at time " << getCurrentTimeStamp() << endl;

        // }
    }

}

void BusProcessingElement::txProcessLink() {
    while (1) {
        wait();
        Flit flit = flit_queue.front();
        bool succeed;
        rtx_bus->write_drive(flit, succeed);
        if (!succeed)
            clog << "Write COLLISION on PE " << local_id << " at time " << getCurrentTimeStamp() << endl;
        else 
            drive_bus = true;
        flit_queue.pop();
    }
}

Flit BusProcessingElement::nextFlit()
{
    Flit flit;
    Packet packet = packet_queue.front();

    flit.src_id = packet.src_id;
    flit.dst_id = packet.dst_id;
    flit.vc_id = packet.vc_id;
    flit.timestamp = packet.timestamp;
    flit.sequence_no = packet.size - packet.flit_left;
    flit.sequence_length = packet.size;
    flit.hop_no = 0;
    //  flit.payload     = DEFAULT_PAYLOAD;

    flit.hub_relay_node = NOT_VALID;

    if (packet.size == 1)
    {
        flit.flit_type = FLIT_TYPE_SINGLE;
    }
    else
    {
        if (packet.size == packet.flit_left)
        flit.flit_type = FLIT_TYPE_HEAD;
        else if (packet.flit_left == 1)
        flit.flit_type = FLIT_TYPE_TAIL;
        else
        flit.flit_type = FLIT_TYPE_BODY;
    }

    packet_queue.front().flit_left--;
    if (packet_queue.front().flit_left == 0)
	    packet_queue.pop();

    return flit;
}

bool BusProcessingElement::canShot(Packet & packet)
{
   // assert(false);
    if(never_transmit) 
        return false;

#ifdef DEADLOCK_AVOIDANCE
    if (local_id%2==0)
	return false;
#endif
    bool shot = false;
    double threshold;

    double now = sc_time_stamp().to_double() / GlobalParams::clock_period_ps;

    if ((GlobalParams::traffic_distribution != TRAFFIC_TABLE_BASED) && 
        (GlobalParams::traffic_distribution != TRAFFIC_TRACE_BASED))
    {
        if (!transmittedAtPreviousCycle)
            threshold = GlobalParams::packet_injection_rate;
        else
            threshold = GlobalParams::probability_of_retransmission;

        shot = (((double) rand()) / RAND_MAX < threshold);
        if (shot) {
            if (GlobalParams::traffic_distribution == TRAFFIC_RANDOM)
                packet = trafficRandom();
            else if (GlobalParams::traffic_distribution == TRAFFIC_TRANSPOSE1)
                packet = trafficTranspose1();
            else if (GlobalParams::traffic_distribution == TRAFFIC_TRANSPOSE2)
                packet = trafficTranspose2();
            else if (GlobalParams::traffic_distribution == TRAFFIC_BIT_REVERSAL)
                packet = trafficBitReversal();
            else if (GlobalParams::traffic_distribution == TRAFFIC_SHUFFLE)
                packet = trafficShuffle();
            else if (GlobalParams::traffic_distribution == TRAFFIC_BUTTERFLY)
                packet = trafficButterfly();
            else if (GlobalParams::traffic_distribution == TRAFFIC_ULOCAL)
                packet = trafficULocal();
            else {
                cout << "Invalid traffic distribution: " << GlobalParams::traffic_distribution << endl;
                exit(-1);
            }
        }
    } 
    else 
    {			// Table based or trace based communication traffic
        if (never_transmit)
            return false;
        if (GlobalParams::traffic_distribution == TRAFFIC_TABLE_BASED)
        {
            bool use_pir = (transmittedAtPreviousCycle == false);
            vector < pair < int, double > > dst_prob;
            double threshold =
                traffic_table->getCumulativePirPor(local_id, (int) now, use_pir, dst_prob);

            double prob = (double) rand() / RAND_MAX;
            shot = (prob < threshold);
            if (shot) {
                for (unsigned int i = 0; i < dst_prob.size(); i++) {
                if (prob < dst_prob[i].second) 
                {
                    int vc = randInt(0,GlobalParams::n_virtual_channels-1);
                    packet.make(local_id, dst_prob[i].first, vc, now, getRandomSize());
                    break;
                }
                }
            }
        }
        else
        {
            // Traffic trace based
            vector<TraceCommunication>* traffic_trace = global_traffic_trace->getTrace(local_id);
            vector<TraceCommunication>::iterator it;
            // clog << "PE " << local_id << " checking traffic trace at time " << getCurrentTimeStamp() << endl;
            if (traffic_trace != NULL) {
                for (it  = traffic_trace->begin(); it < traffic_trace->end(); it++)
                {
                    if (now >= it->time)
                    {
                        shot = true;
                        int vc = randInt(0,GlobalParams::n_virtual_channels-1);
                        packet.make(local_id, it->dst, vc, now, it->num_flit);
                        traffic_trace->erase(it);
                        break;
                    }
                    else 
                        shot = false;
                }
            }
        }
    }

    return shot;
}


int BusProcessingElement::findRandomDestination(int id, int hops)
{
    assert(GlobalParams::topology == TOPOLOGY_MESH);

    int inc_y = rand()%2?-1:1;
    int inc_x = rand()%2?-1:1;
    
    Coord current =  id2Coord(id);
    
    for (int h = 0; h<hops; h++)
    {

	if (current.x==0)
	    if (inc_x<0) inc_x=0;

	if (current.x== GlobalParams::mesh_dim_x-1)
	    if (inc_x>0) inc_x=0;

	if (current.y==0)
	    if (inc_y<0) inc_y=0;

	if (current.y==GlobalParams::mesh_dim_y-1)
	    if (inc_y>0) inc_y=0;

	if (rand()%2)
	    current.x +=inc_x;
	else
	    current.y +=inc_y;
    }
    return coord2Id(current);
}


int BusProcessingElement::roulette()
{
    int slices = GlobalParams::mesh_dim_x + GlobalParams::mesh_dim_y -2;


    double r = rand()/(double)RAND_MAX;


    for (int i=1;i<=slices;i++)
    {
	if (r< (1-1/double(2<<i)))
	{
	    return i;
	}
    }
    assert(false);
    return 1;
}


Packet BusProcessingElement::trafficULocal()
{
    Packet p;
    p.src_id = local_id;

    int target_hops = roulette();

    p.dst_id = findRandomDestination(local_id,target_hops);

    p.timestamp = sc_time_stamp().to_double() / GlobalParams::clock_period_ps;
    p.size = p.flit_left = getRandomSize();
    p.vc_id = randInt(0,GlobalParams::n_virtual_channels-1);

    return p;
}

Packet BusProcessingElement::trafficRandom()
{
    Packet p;
    p.src_id = local_id;
    double rnd = rand() / (double) RAND_MAX;
    double range_start = 0.0;
    int max_id;

    if (GlobalParams::topology == TOPOLOGY_MESH)
	max_id = (GlobalParams::mesh_dim_x * GlobalParams::mesh_dim_y) - 1; //Mesh 
    else    // other delta topologies
	max_id = GlobalParams::n_delta_tiles-1; 

    // Random destination distribution
    do {
	p.dst_id = randInt(0, max_id);

	// check for hotspot destination
	for (size_t i = 0; i < GlobalParams::hotspots.size(); i++) {

	    if (rnd >= range_start && rnd < range_start + GlobalParams::hotspots[i].second) {
		if (local_id != GlobalParams::hotspots[i].first ) {
		    p.dst_id = GlobalParams::hotspots[i].first;
		}
		break;
	    } else
		range_start += GlobalParams::hotspots[i].second;	// try next
	}
#ifdef DEADLOCK_AVOIDANCE
	assert((GlobalParams::topology == TOPOLOGY_MESH));
	if (p.dst_id%2!=0)
	{
	    p.dst_id = (p.dst_id+1)%256;
	}
#endif

    } while (p.dst_id == p.src_id);

    p.timestamp = sc_time_stamp().to_double() / GlobalParams::clock_period_ps;
    p.size = p.flit_left = getRandomSize();
    p.vc_id = randInt(0,GlobalParams::n_virtual_channels-1);

    return p;
}
// TODO: for testing only
Packet BusProcessingElement::trafficTest()
{
    Packet p;
    p.src_id = local_id;
    p.dst_id = 10;

    p.timestamp = sc_time_stamp().to_double() / GlobalParams::clock_period_ps;
    p.size = p.flit_left = getRandomSize();
    p.vc_id = randInt(0,GlobalParams::n_virtual_channels-1);

    return p;
}

Packet BusProcessingElement::trafficTranspose1()
{
    assert(GlobalParams::topology == TOPOLOGY_MESH);
    Packet p;
    p.src_id = local_id;
    Coord src, dst;

    // Transpose 1 destination distribution
    src.x = id2Coord(p.src_id).x;
    src.y = id2Coord(p.src_id).y;
    dst.x = GlobalParams::mesh_dim_x - 1 - src.y;
    dst.y = GlobalParams::mesh_dim_y - 1 - src.x;
    fixRanges(src, dst);
    p.dst_id = coord2Id(dst);

    p.vc_id = randInt(0,GlobalParams::n_virtual_channels-1);
    p.timestamp = sc_time_stamp().to_double() / GlobalParams::clock_period_ps;
    p.size = p.flit_left = getRandomSize();

    return p;
}

Packet BusProcessingElement::trafficTranspose2()
{
    assert(GlobalParams::topology == TOPOLOGY_MESH);
    Packet p;
    p.src_id = local_id;
    Coord src, dst;

    // Transpose 2 destination distribution
    src.x = id2Coord(p.src_id).x;
    src.y = id2Coord(p.src_id).y;
    dst.x = src.y;
    dst.y = src.x;
    fixRanges(src, dst);
    p.dst_id = coord2Id(dst);

    p.vc_id = randInt(0,GlobalParams::n_virtual_channels-1);
    p.timestamp = sc_time_stamp().to_double() / GlobalParams::clock_period_ps;
    p.size = p.flit_left = getRandomSize();

    return p;
}

void BusProcessingElement::setBit(int &x, int w, int v)
{
    int mask = 1 << w;

    if (v == 1)
	x = x | mask;
    else if (v == 0)
	x = x & ~mask;
    else
	assert(false);
}

int BusProcessingElement::getBit(int x, int w)
{
    return (x >> w) & 1;
}

inline double BusProcessingElement::log2ceil(double x)
{
    return ceil(log(x) / log(2.0));
}

Packet BusProcessingElement::trafficBitReversal()
{

    int nbits =
	(int)
	log2ceil((double)
		 (GlobalParams::mesh_dim_x *
		  GlobalParams::mesh_dim_y));
    int dnode = 0;
    for (int i = 0; i < nbits; i++)
	setBit(dnode, i, getBit(local_id, nbits - i - 1));

    Packet p;
    p.src_id = local_id;
    p.dst_id = dnode;

    p.vc_id = randInt(0,GlobalParams::n_virtual_channels-1);
    p.timestamp = sc_time_stamp().to_double() / GlobalParams::clock_period_ps;
    p.size = p.flit_left = getRandomSize();

    return p;
}

Packet BusProcessingElement::trafficShuffle()
{

    int nbits =
	(int)
	log2ceil((double)
		 (GlobalParams::mesh_dim_x *
		  GlobalParams::mesh_dim_y));
    int dnode = 0;
    for (int i = 0; i < nbits - 1; i++)
	setBit(dnode, i + 1, getBit(local_id, i));
    setBit(dnode, 0, getBit(local_id, nbits - 1));

    Packet p;
    p.src_id = local_id;
    p.dst_id = dnode;

    p.vc_id = randInt(0,GlobalParams::n_virtual_channels-1);
    p.timestamp = sc_time_stamp().to_double() / GlobalParams::clock_period_ps;
    p.size = p.flit_left = getRandomSize();

    return p;
}

Packet BusProcessingElement::trafficButterfly()
{

    int nbits = (int) log2ceil((double)
		 (GlobalParams::mesh_dim_x *
		  GlobalParams::mesh_dim_y));
    int dnode = 0;
    for (int i = 1; i < nbits - 1; i++)
	setBit(dnode, i, getBit(local_id, i));
    setBit(dnode, 0, getBit(local_id, nbits - 1));
    setBit(dnode, nbits - 1, getBit(local_id, 0));

    Packet p;
    p.src_id = local_id;
    p.dst_id = dnode;

    p.vc_id = randInt(0,GlobalParams::n_virtual_channels-1);
    p.timestamp = sc_time_stamp().to_double() / GlobalParams::clock_period_ps;
    p.size = p.flit_left = getRandomSize();

    return p;
}

void BusProcessingElement::fixRanges(const Coord src,
				       Coord & dst)
{
    // Fix ranges
    if (dst.x < 0)
	dst.x = 0;
    if (dst.y < 0)
	dst.y = 0;
    if (dst.x >= GlobalParams::mesh_dim_x)
	dst.x = GlobalParams::mesh_dim_x - 1;
    if (dst.y >= GlobalParams::mesh_dim_y)
	dst.y = GlobalParams::mesh_dim_y - 1;
}

int BusProcessingElement::getRandomSize()
{
    return randInt(GlobalParams::min_packet_size,
		   GlobalParams::max_packet_size);
}

unsigned int BusProcessingElement::getQueueSize() const
{
    return packet_queue.size();
}

void BusProcessingElement::printRxFlit(Flit & flit)
{
    if (GlobalParams::output_mode > NORMAL_MODE)
    {
        clog << "PE " << local_id << ": " << sc_time_stamp().to_double() / GlobalParams::clock_period_ps << ": R " << flit << ". Route: ";
        for (auto it = flit.route_time.begin(); it < flit.route_time.end(); it++)
        {
            clog << it->first <<":" << it->second << "->";
        }
        clog << endl;
    }
}

void BusProcessingElement::printTxFlit(Flit & flit)
{
    if (GlobalParams::output_mode > NORMAL_MODE)
    {
        clog << "PE " << local_id << ": " << sc_time_stamp().to_double() / GlobalParams::clock_period_ps << ": S " << flit 
            // << " delta count " << sc_delta_count() 
            << endl;
    }
}
