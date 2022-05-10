/*
 * Noxim - the NoC Simulator
 *
 * (C) 2005-2015 by the University of Catania
 * For the complete list of authors refer to file ../doc/AUTHORS.txt
 * For the license applied to these sources refer to file ../doc/LICENSE.txt
 *
 * This file contains the implementation of the global traffic trace
 */

#include "GlobalTrafficTrace.h"
#include "GlobalParams.h"
#include <math.h>

GlobalTrafficTrace::GlobalTrafficTrace()
{
  // Clear the Global Traffic
  global_trace_map.clear();
}

bool GlobalTrafficTrace::load(const char *fname)
{
  // Open file
  ifstream fin(fname, ios::in);
  if (!fin)
    return false;

  TraceCommunication communication;
  GlobalTrafficTraceMap_t::iterator it;

  // Read file
  for(std::string line; getline(fin, line);)
  {
      if ((line.length() != 0) || (line[0] != '%')) 
      {
          int time, src, dst, num_flit;	// Mandatory data

          int params = sscanf(line.c_str(), "%d %d %d %d", &time, &src, &dst, &num_flit);
          if (params >= NUM_TRACE_PARAMETERS)
          {
              // Create a communication from the parameters read on the line

              if (params == NUM_TRACE_PARAMETERS)
                  num_flit = 1;
              // Mandatory fields
              communication.time = time;
              communication.num_flit = num_flit;
              communication.src = src;
              communication.dst = dst;

              it = global_trace_map.find(src);

              // Add this communication to the vector of communications
              if (it == global_trace_map.end())
              {
                vector<TraceCommunication> trace_vector;
                trace_vector.push_back(communication);
                global_trace_map.insert({src, trace_vector});
              }
              else
              {
                vector<TraceCommunication>* ptr_trace_vector = &(it->second);
                ptr_trace_vector->push_back(communication);
              }
          }
      }
  }

  fin.close();
  
  return true;
}

/**
 * Get the trace communication data for a specific processing element
 * @param[in] src_id  The ID of the source processing element
 * 
 * @return  A pointer to a vector of Trace Communication with the specified 
 *          processing element as source
 */
vector<TraceCommunication>* GlobalTrafficTrace::getTrace(const int src_id)
{
  GlobalTrafficTraceMap_t::iterator it;
  it = global_trace_map.find(src_id);

  if (it == global_trace_map.end())
    return NULL;
  else
    return &(it->second);
}

int GlobalTrafficTrace::occurrencesAsSource(const int src_id)
{
  int count = 0;
  GlobalTrafficTraceMap_t::iterator it;

  it = global_trace_map.find(src_id);
  if (it != global_trace_map.end())
    count = it->second.size();
  else 
    count = 0;

  return count;
}
