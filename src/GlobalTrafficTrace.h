/*
 * GlobalTrafficTrace.h
 *
 *  Created on: Mar 22, 2016
 *      Author: khanh
 */

#ifndef __NOXIMGLOBALTRAFFICTRACE_H_
#define __NOXIMGLOBALTRAFFICTRACE_H_

#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <map>
#include "DataStructs.h"

using namespace std;

typedef map<int, vector<TraceCommunication>> GlobalTrafficTraceMap_t;
#define NUM_TRACE_PARAMETERS      3

class GlobalTrafficTrace 
{
  public:

    GlobalTrafficTrace();

    // Load traffic trace from file. Returns true if ok, false otherwise
    bool load(const char *fname);

    vector<TraceCommunication>* getTrace(const int src_id);

    // Returns the number of occurrences of source src_id in the traffic
    // trace
    int occurrencesAsSource(const int src_id);

  private:

     GlobalTrafficTraceMap_t global_trace_map;
};


#endif /* __NOXIMGLOBALTRAFFICTRACE_H_ */
