#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/traffic-control-module.h"
#include "ns3/flow-monitor-module.h"
#include "trace.h"
#include <iomanip> // For std::setprecision
#include <fstream>
using namespace ns3;

NS_LOG_COMPONENT_DEFINE("ScratchSimulator");


std::string dir;
std::string tcpTypeId = "TcpBbr";
std::string queueDisc = "FifoQueueDisc";
double bufferSize = 0.25;  // Defaultly set buffer size to 0.25 * BDP
double OBBRU = 0.5;
bool useOBbr = false;
uint32_t delAckCount = 2;
double lossRate = 0.0;
Time stopTime = Seconds (100);
std::string dataRate = "100Mbps", delay = "40ms";
uint64_t totalDeliveredBytes, totalLossBytes;

std::string doubleToStringWithPrecision(double value, int precision) {
    std::stringstream stream;
    stream << std::fixed << std::setprecision(precision) << value;
    return stream.str();
}


void GetTrace()
{
  // Hook trace source after application starts
  Simulator::Schedule (Seconds (0.1) + MilliSeconds (1), &TraceCwnd, 0, 0);
  Simulator::Schedule (Seconds (0.1) + MilliSeconds (1), &TraceThroughputFromTcp, 0, 0);
  Simulator::Schedule (Seconds (0.1) + MilliSeconds (1), &TracePacingRate, 0, 0);
  Simulator::Schedule (Seconds (0.1) + MilliSeconds (1), &TraceBbrMode, 0, 0);
  Simulator::Schedule (Seconds (0.1) + MilliSeconds (1), &TracePacingGain, 0, 0);
  Simulator::Schedule (Seconds (0.1) + MilliSeconds (1), &TraceMinRtt, 0, 0);
}

void parseArg(int argc, char *argv[])
{
  CommandLine cmd (__FILE__);
  cmd.AddValue ("tcpTypeId", "Transport protocol to use: TcpNewReno, TcpBbr", tcpTypeId);
  cmd.AddValue ("delAckCount", "Delayed ACK count", delAckCount);
  cmd.AddValue ("stopTime", "Stop time for applications / simulation time will be stopTime + 1", stopTime);
  cmd.AddValue ("enableOBbr", "Enable OBBR", useOBbr);
  cmd.AddValue ("bufferSize", "Buffer size as a fraction of BDP", bufferSize);
  cmd.AddValue ("dataRate", "Data rate of the link", dataRate);
  cmd.AddValue ("delay", "Delay of the link", delay);
  cmd.AddValue ("OBBRU", "OBBR argue mu", OBBRU);
  cmd.AddValue ("lossRate", "Loss rate of the link", lossRate); 
  cmd.Parse (argc, argv);
}

void setDefaultAttri()
{
  queueDisc = std::string ("ns3::") + queueDisc;
  Config::SetDefault ("ns3::TcpL4Protocol::SocketType", StringValue ("ns3::" + tcpTypeId));
  Config::SetDefault ("ns3::TcpBbr::EnableOBBR", BooleanValue (useOBbr));
  Config::SetDefault ("ns3::TcpSocket::SndBufSize", UintegerValue (67108864));
  Config::SetDefault ("ns3::TcpSocket::RcvBufSize", UintegerValue (67108864));
  Config::SetDefault ("ns3::TcpSocket::InitialCwnd", UintegerValue (10));
  Config::SetDefault ("ns3::TcpSocket::DelAckCount", UintegerValue (delAckCount));
  Config::SetDefault ("ns3::TcpSocket::SegmentSize", UintegerValue (1448));
  Config::SetDefault ("ns3::DropTailQueue<Packet>::MaxSize", QueueSizeValue (QueueSize ("1p")));
  Config::SetDefault (queueDisc + "::MaxSize", QueueSizeValue (QueueSize ("10MB")));
  Config::SetDefault ("ns3::TcpBbr::OBbrU", DoubleValue (OBBRU));
  Config::SetDefault ("ns3::RateErrorModel::ErrorRate", DoubleValue (lossRate));
  Config::SetDefault ("ns3::RateErrorModel::ErrorUnit", StringValue ("ERROR_UNIT_PACKET"));
}

void logInfo(){
  std::fstream f(dir + "info.txt", std::ios::out | std::ios::app);
  f << "tcpTypeId: " << tcpTypeId << std::endl;
  f << "delAckCount: " << delAckCount << std::endl;
  f << "stopTime: " << stopTime << std::endl;
  f << "enableOBbr: " << useOBbr << std::endl;
  f << "bufferSize: " << bufferSize << std::endl;
  f << "dataRate: " << dataRate << std::endl;
  f << "delay: " << delay << std::endl;
  f << "OBBRU: " << OBBRU << std::endl;
  f << "lossRate: " << lossRate << std::endl;
  f.close();
}

int main (int argc, char *argv [])
{
  // Naming the output directory using local system time
  time_t rawtime;
  struct tm * timeinfo;
  char buffer [80];
  time (&rawtime);
  timeinfo = localtime (&rawtime);
  strftime (buffer, sizeof (buffer), "%d-%m-%Y-%I-%M-%S", timeinfo);
  std::string currentTime (buffer);


  
  bool bql = true;

  parseArg(argc, argv);
  setDefaultAttri();  


  NodeContainer sender, receiver;
  // NodeContainer routers;
  sender.Create (1);
  receiver.Create (1);

  // Create the point-to-point link helpers
  PointToPointHelper p2pHelper;
  p2pHelper.SetDeviceAttribute  ("DataRate", StringValue (dataRate));
  p2pHelper.SetChannelAttribute ("Delay", StringValue (delay));

  NetDeviceContainer link = p2pHelper.Install (sender.Get (0), receiver.Get (0));

  
  // Install Stack
  InternetStackHelper internet;
  internet.Install (sender);
  internet.Install (receiver);

  // Configure the root queue discipline
  TrafficControlHelper tch;
  tch.SetRootQueueDisc (queueDisc);

  if (bql)
    {
      tch.SetQueueLimits ("ns3::DynamicQueueLimits", "HoldTime", StringValue ("1000ms"));
    }

  tch.Install (link.Get(1));


  // Assign IP addresses
  Ipv4AddressHelper ipv4;
  ipv4.SetBase ("10.0.0.0", "255.255.255.0");
  Ipv4InterfaceContainer interfaces = ipv4.Assign (link);




  // Populate routing tables
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  // Select sender side port
  uint16_t port = 50001;

  // Install application on the sender
  BulkSendHelper source ("ns3::TcpSocketFactory", InetSocketAddress (interfaces.GetAddress (1), port));
  source.SetAttribute ("MaxBytes", UintegerValue (0));
  ApplicationContainer sourceApps = source.Install (sender.Get (0));
  sourceApps.Start (Seconds (0.1));
  sourceApps.Stop (stopTime);

  GetTrace();

  // Install application on the receiver
  PacketSinkHelper sink ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), port));
  ApplicationContainer sinkApps = sink.Install (receiver.Get (0));
  sinkApps.Start (Seconds (0.0));
  sinkApps.Stop (stopTime);

  // Create a new directory to store the output of the program
  dir = "/home/ybxiao/cc/ns-3-dev/results";
  std::string bufstr = doubleToStringWithPrecision(bufferSize, 2) + "-buffer/";
  std::string lossRateStr = doubleToStringWithPrecision(lossRate, 3) + "-loss/";
  if(tcpTypeId == "TcpBbr")
  {
    if(useOBbr)
      dir += "/obbr-results/" + bufstr + lossRateStr + doubleToStringWithPrecision(OBBRU, 2) + "-OBBRU/" + currentTime + "/";
    else
      dir += "/bbr-results/" + bufstr + lossRateStr + currentTime + "/";
  }
  else
    dir = "/home/ybxiao/cc/ns-3-dev/ + " + tcpTypeId + "-results/" + bufstr + lossRateStr + currentTime + "/";
  std::string dirToSave = "mkdir -p " + dir;
  if (system (dirToSave.c_str ()) == -1)
    {
      exit (1);
    }

  if(lossRate > 0)
  {
    //log
    std::fstream f(dir + "info.txt", std::ios::out | std::ios::app);
    f << "Use a loss link. Loss rate: " << lossRate << std::endl;
    f.close();
    Ptr<RateErrorModel> em = CreateObject<RateErrorModel> ();
    link.Get (1)->SetAttribute ("ReceiveErrorModel", PointerValue (em));
  }

 
  tch.Uninstall (link.Get(0));
  //Set shadow buffer and trace the queue size
  
  Config::SetDefault (queueDisc + "::MaxSize", QueueSizeValue (QueueSize (QueueSizeUnit::BYTES, (uint32_t)(DataRate(dataRate) * Time(delay) * bufferSize / 8))));
  QueueDiscContainer qd;
  qd = tch.Install (link.Get(0));
  Simulator::ScheduleNow (&CheckQueueSize, qd.Get (0));
  
  logInfo();

  Simulator::Stop (stopTime + TimeStep (1));
  Simulator::Run ();
  Simulator::Destroy ();

  std::fstream f(dir + "info.txt", std::ios::out | std::ios::app);
  f << "Total Delivered: " << totalDeliveredBytes << std::endl;
  f << "Total Loss: " << totalLossBytes << std::endl;
  f.close();
  return 0;
}
