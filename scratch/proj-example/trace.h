#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/traffic-control-module.h"
#include "ns3/flow-monitor-module.h"


void CheckQueueSize (Ptr<QueueDisc> qd);
void TraceThroughputFromTcp(uint32_t nodeId, uint32_t socketId);
void TraceCwnd (uint32_t nodeId, uint32_t socketId);
void TraceBbrMode (uint32_t nodeId, uint32_t socketId);
void TracePacingRate (uint32_t nodeId, uint32_t socketId);
void TracePacingGain (uint32_t nodeId, uint32_t socketId);
void TraceMinRtt (uint32_t nodeId, uint32_t socketId);