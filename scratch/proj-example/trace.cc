#include "trace.h"


using namespace ns3;
extern std::string dir;
// Check the queue size
void CheckQueueSize (Ptr<QueueDisc> qd)
{
  uint32_t qsize = qd->GetCurrentSize ().GetValue ();
  Simulator::Schedule (Seconds (0.2), &CheckQueueSize, qd);
  std::ofstream q (dir + "/queueSize.dat", std::ios::out | std::ios::app);
  q << Simulator::Now ().GetSeconds () << " " << qsize << std::endl;
  q.close ();
}

//check throuhput
void TraceThroughputFromTcp(uint32_t nodeId, uint32_t socketId)
{
  static uint64_t lastTotalDelievered = 0;
  auto container = Config::LookupMatches("/NodeList/" + std::to_string (nodeId) + "/$ns3::TcpL4Protocol/SocketList/" + std::to_string (socketId));
  Ptr<TcpSocketBase> socket = dynamic_cast<TcpSocketBase*>(PeekPointer(container.Get(0)));
  uint64_t curTotalDelievered = socket->GetTotalDeliveredBytes();
  std::cout << Simulator::Now().GetSeconds() << " Delivered Rate: " << (curTotalDelievered - lastTotalDelievered) * 8 / 1000000.0 / 0.1 << "; Retransmit: " << socket->GetTotalRetransBytes() << std::endl;
  lastTotalDelievered = curTotalDelievered;
  Simulator::Schedule (Seconds (0.1), &TraceThroughputFromTcp, nodeId, socketId);
}


// Trace congestion window
static void CwndTracer (Ptr<OutputStreamWrapper> stream, uint32_t oldval, uint32_t newval)
{
  *stream->GetStream () << Simulator::Now ().GetSeconds () << " " << newval / 1448.0 << std::endl;
}

void TraceCwnd (uint32_t nodeId, uint32_t socketId)
{
  AsciiTraceHelper ascii;
  Ptr<OutputStreamWrapper> stream = ascii.CreateFileStream (dir + "/cwnd.dat");
  Config::ConnectWithoutContext ("/NodeList/" + std::to_string (nodeId) + "/$ns3::TcpL4Protocol/SocketList/" + std::to_string (socketId) + "/CongestionWindow", MakeBoundCallback (&CwndTracer, stream));
}


// Trace BBR mode

std::string lut[4] = {"BBR_STARTUP", "BBR_DRAIN", "BBR_PROBE_BW", "BBR_PROBE_RTT"};

static void BbrmodeTracer(Ptr<OutputStreamWrapper> stream, TcpBbr::BbrMode_t oldval, TcpBbr::BbrMode_t newval)
{
  *stream->GetStream() << Simulator::Now().GetSeconds() << " BBR mode: " << lut[oldval] << " -> " << lut[newval]<< std::endl;
}

void TraceBbrMode (uint32_t nodeId, uint32_t socketId)
{
  AsciiTraceHelper ascii;
  Ptr<OutputStreamWrapper> stream = ascii.CreateFileStream (dir + "/bbrMode.txt");
  Config::ConnectWithoutContext ("/NodeList/" + std::to_string (nodeId) + "/$ns3::TcpL4Protocol/SocketList/" + std::to_string (socketId) + "/CongestionOps/BbrState", MakeBoundCallback (&BbrmodeTracer, stream));
}


// Trace pacing rate

static void PacingRateTracer (Ptr<OutputStreamWrapper> stream, DataRate oldval, DataRate newval)
{
    *stream->GetStream() << Simulator::Now().GetSeconds() << " " << newval.GetBitRate() / 1000000.0 << "Mbps" << std::endl;
}

void TracePacingRate (uint32_t nodeId, uint32_t socketId)
{
    AsciiTraceHelper ascii;
    Ptr<OutputStreamWrapper> stream = ascii.CreateFileStream (dir + "/pacingRate.dat");
    Config::ConnectWithoutContext ("/NodeList/" + std::to_string (nodeId) + "/$ns3::TcpL4Protocol/SocketList/" + std::to_string (socketId) + "/PacingRate", MakeBoundCallback (&PacingRateTracer, stream));
}


// Trace PaicingGain

static double PACING_GAIN[] = {1.25, 0.75, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0};

static void PacingGainTracer (Ptr<OutputStreamWrapper> stream, uint32_t oldval, uint32_t newval)
{
    *stream->GetStream() << Simulator::Now().GetSeconds() << " Cycle Index: " << newval << " Pacing gain: " << PACING_GAIN[newval] << std::endl;
}

void TracePacingGain (uint32_t nodeId, uint32_t socketId)
{
    AsciiTraceHelper ascii;
    Ptr<OutputStreamWrapper> stream = ascii.CreateFileStream (dir + "/pacingGain.dat");
    Config::ConnectWithoutContext ("/NodeList/" + std::to_string (nodeId) + "/$ns3::TcpL4Protocol/SocketList/" + std::to_string (socketId) + "/CongestionOps/CycleIdx", MakeBoundCallback (&PacingGainTracer, stream));
}


// Trace Min RTT

static void MinRttTracer (Ptr<OutputStreamWrapper> stream, Time oldval, Time newval)
{
    *stream->GetStream() << Simulator::Now().GetSeconds() << " " << newval.GetMilliSeconds() << std::endl;
}

void TraceMinRtt (uint32_t nodeId, uint32_t socketId)
{
    AsciiTraceHelper ascii;
    Ptr<OutputStreamWrapper> stream = ascii.CreateFileStream (dir + "/minRtt.dat");
    Config::ConnectWithoutContext ("/NodeList/" + std::to_string (nodeId) + "/$ns3::TcpL4Protocol/SocketList/" + std::to_string (socketId) + "/CongestionOps/MinRtt", MakeBoundCallback (&MinRttTracer, stream));
}