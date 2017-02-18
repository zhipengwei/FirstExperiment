#include <iostream>
#include <fstream>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/internet-module.h"

#include "ns3/point-to-point-module.h"
#include "ns3/net-device.h"

#include "ns3/traffic-control-module.h"

#include "config.h"
#include <vector>

using namespace ns3;
using namespace std;
NS_LOG_COMPONENT_DEFINE ("CsmaBridgeExample");

// This is the application defined in another class, this definition will allow us to hook the congestion window.
class MyApp : public Application 
{
public:

  MyApp ();
  virtual ~MyApp();

  void Setup (Ptr<Socket> socket, Address address, uint32_t packetSize, uint32_t nPackets, DataRate dataRate, uint32_t numberPacketsPerFlow, double mean, double bound);

private:
  virtual void StartApplication (void);
  virtual void StopApplication (void);

  void ScheduleTx (void);
  void SendPacket (void);

  Ptr<Socket>     m_socket;
  Address         m_peer;
  uint32_t        m_packetSize;
  uint32_t        m_nPackets;
  DataRate        m_dataRate;
  EventId         m_sendEvent;
  bool            m_running;
  uint32_t        m_packetsSent;

  uint32_t 	  m_numberPacketsPerFlow;
  uint32_t        m_numberPacketsPerFlowCnt;
  Ptr<ExponentialRandomVariable> x;
};

MyApp::MyApp ()
  : m_socket (0), 
    m_peer (), 
    m_packetSize (0), 
    m_nPackets (0), 
    m_dataRate (0), 
    m_sendEvent (), 
    m_running (false), 
    m_packetsSent (0)
{
	x = CreateObject<ExponentialRandomVariable> ();
        m_numberPacketsPerFlowCnt = 0;
}

MyApp::~MyApp()
{
  m_socket = 0;
}

void
MyApp::Setup (Ptr<Socket> socket, Address address, uint32_t packetSize, uint32_t nPackets, DataRate dataRate, uint32_t numberPacketsPerFlow, double mean, double bound)
{
  m_socket = socket;
  m_peer = address;
  m_packetSize = packetSize;
  m_nPackets = nPackets;
  m_dataRate = dataRate;

  x->SetAttribute ("Mean", DoubleValue (mean));
  //x->SetAttribute ("Bound", DoubleValue(bound));

  m_numberPacketsPerFlow = numberPacketsPerFlow;
}

void
MyApp::StartApplication (void)
{
  m_running = true;
  m_packetsSent = 0;
  m_socket->Bind ();
  m_socket->Connect (m_peer);
  SendPacket ();
}

void 
MyApp::StopApplication (void)
{
  m_running = false;

  if (m_sendEvent.IsRunning ())
    {
      Simulator::Cancel (m_sendEvent);
    }

  if (m_socket)
    {
      m_socket->Close ();
    }
}

void 
MyApp::SendPacket (void)
{
  Ptr<Packet> packet = Create<Packet> (m_packetSize);
  m_socket->Send (packet);

  if (m_running)
    {
      ScheduleTx ();
    }
}

void 
MyApp::ScheduleTx (void)
{

  // After a certain number of packets are sent, an interval is inserted.
  if (m_running)
    {
      Time tNext; 
      // The interval includes the transmission time of the packet;
      //tNext = Seconds (m_packetSize * 8 / static_cast<double> (m_dataRate.GetBitRate ()) + x->GetValue());
      // The interval only includes the part which follows exponential distribution;
      tNext = Seconds (x->GetValue());
      cout << "Interval time " << tNext << endl;
      // Time is used to denote delay until the next event should execute.
      m_sendEvent = Simulator::Schedule (tNext, &MyApp::SendPacket, this);
    }
   // cout << "couter " << m_numberPacketsPerFlowCnt << " " << m_running << endl;
   // cout << "start time " << m_startTime << " " << m_stopTime << endl;
}

//static void
//CwndChange (uint32_t oldCwnd, uint32_t newCwnd)
//{
//  NS_LOG_UNCOND ( index << Simulator::Now ().GetSeconds () << "\t" << newCwnd);
//}

static void
CwndChange (std::string context, uint32_t oldCwnd, uint32_t newCwnd)
{
  NS_LOG_UNCOND (context << "\t" << Simulator::Now ().GetSeconds () << "\t" << newCwnd);
}

//static void
//RxDrop (Ptr<const Packet> p)
//{
//  NS_LOG_UNCOND ("RxDrop at " << Simulator::Now ().GetSeconds ());
//}

// The following are queue related tracing functions.
// Packet drop event
static void 
AsciiDropEvent (std::string path, Ptr<const QueueItem> packet)
{
  NS_LOG_UNCOND ("PacketDrop:\t" << Simulator::Now ().GetMicroSeconds () << "\t" << path << "\t" << *packet);
//  cout << "aaa" << endl;
//  *os << "d " << Simulator::Now ().GetSeconds () << " ";
//  *os << path << " " << *packet << std::endl;
}
// Enqueue event
static void 
AsciiEnqueueEvent (std::string path, Ptr<const QueueItem> packet)
{
  // NS_LOG_UNCOND ("Enqueue\t" << Simulator::Now ().GetMicroSeconds () << "\t" << *(packet->GetPacket()) );
  NS_LOG_UNCOND ("Enqueue\t" << Simulator::Now ().GetMicroSeconds () << "\t" << *packet << *(packet->GetPacket()) );
 // *os << "+ " << Simulator::Now ().GetSeconds () << " ";
 // *os << path << " " << *packet << std::endl;
}

// Dequeue event
static void 
AsciiDequeueEvent (std::string path, Ptr<const QueueItem> packet)
{
  NS_LOG_UNCOND ("Dequeue\t" << Simulator::Now ().GetMicroSeconds () << "\t" << *(packet->GetPacket()) );
 // *os << "+ " << Simulator::Now ().GetSeconds () << " ";
 // *os << path << " " << *packet << std::endl;
}

static void 
AsciiPacketsInQueue (std::string path, uint32_t oldValue, uint32_t newValue) 
{
  NS_LOG_UNCOND ("BytesInQueue\t" << Simulator::Now ().GetSeconds () << "\t" << newValue);
 // *os << "+ " << Simulator::Now ().GetSeconds () << " ";
 // *os << path << " " << *packet << std::endl;
}

static void 
AsciiPacketsInQueueNetDevice (std::string path, uint32_t oldValue, uint32_t newValue) 
{
  NS_LOG_UNCOND ("BytesInQueueNetDevice\t" << Simulator::Now ().GetSeconds () << "\t" << newValue);
 // *os << "+ " << Simulator::Now ().GetSeconds () << " ";
 // *os << path << " " << *packet << std::endl;
}

const char * IpBaseGenerator (int index) {
	int second = 0, third = 0;
	third = index;
	second = third / 255;
	third = third % 255;

	ostringstream oss;
	oss << "10." << second << "." << third << ".0";
	cout << oss.str() << endl;
	return oss.str().c_str();
}


int 
main (int argc, char *argv[])
{
  //
  // Users may find it convenient to turn on explicit debugging
  // for selected modules; the below lines suggest how to do this
  //
#if 0 
  LogComponentEnable ("CsmaBridgeExample", LOG_LEVEL_INFO);
#endif

  //
  // Allow the user to override any of the defaults and the above Bind() at
  // run-time, via command-line arguments
  //
  CommandLine cmd;
  cmd.Parse (argc, argv);

  //
  // Explicitly create the nodes required by the topology (shown above).
  //

  Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(1448));
  Config::SetDefault("ns3::TcpSocket::SndBufSize", UintegerValue(1310720));

  int numberOfTerminals = CONFIG_NUMBER_OF_TERMINALS;
  NS_LOG_INFO ("Create sender nodes.");
  NodeContainer terminals;
  terminals.Create (numberOfTerminals);

  NS_LOG_INFO ("Create server node.");
  NodeContainer servers;
  servers.Create (1);

  NodeContainer router;
  router.Create (1);

  // Add internet stack to the terminals
  InternetStackHelper internet;
  internet.Install (terminals);
  internet.Install (servers);
  internet.Install (router);

  NS_LOG_INFO ("Build Topology");
  // The terminal link
  PointToPointHelper p2p;

  p2p.SetDeviceAttribute ("DataRate", StringValue (CONFIG_SENDER_LINK_DATA_RATE));
  p2p.SetChannelAttribute ("Delay", StringValue (CONFIG_SENDER_LINK_DELAY));
  p2p.SetQueue("ns3::DropTailQueue", "MaxBytes", UintegerValue(CONFIG_INPUT_BUFFER_SIZE_BYTES), "Mode", EnumValue (DropTailQueue::QUEUE_MODE_BYTES));

  // Create the point to point links from each terminal to the router
  NetDeviceContainer terminalDevices;
  NetDeviceContainer routerDevices;

  vector<Ipv4InterfaceContainer> TerminalIpv4Interface;
  Ipv4AddressHelper ipv4;
  for (int i = 0; i < numberOfTerminals; i++) {
    NetDeviceContainer link = p2p.Install (NodeContainer (terminals.Get (i), router));
    terminalDevices.Add (link.Get (0));
    routerDevices.Add (link.Get (1));
    
    ipv4.SetBase (IpBaseGenerator (i+1), "255.255.255.0");
    TerminalIpv4Interface.push_back (ipv4.Assign (link));
  }

  // The server link
  // UintegerValue, holds an unsigned integer type.
  //p2p.SetQueue("ns3::DropTailQueue", "MaxBytes", UintegerValue(CONFIG_OUTPUT_BUFFER_SIZE_BYTES), "Mode", EnumValue (DropTailQueue::QUEUE_MODE_BYTES));
  p2p.SetQueue("ns3::DropTailQueue", "MaxBytes", UintegerValue (15000), "Mode", EnumValue (DropTailQueue::QUEUE_MODE_BYTES));
  p2p.SetDeviceAttribute ("DataRate", DataRateValue (CONFIG_SERVER_LINK_DATA_RATE));
  p2p.SetChannelAttribute ("Delay", StringValue (CONFIG_SERVER_LINK_DELAY));

  // Create point to point link, from the server to the bridge
  NetDeviceContainer serverDevices;
  NetDeviceContainer linkServer = p2p.Install (NodeContainer (servers.Get (0), router));
  serverDevices.Add (linkServer.Get (0));
  routerDevices.Add (linkServer.Get (1));

  // We've got the "hardware" in place.  Now we need to add IP addresses.
  NS_LOG_INFO ("Assign IP Addresses.");
  ipv4.SetBase (IpBaseGenerator (numberOfTerminals + 1), "255.255.255.0");
  ipv4.Assign (linkServer);

  Ipv4InterfaceContainer serverIpv4; 
  serverIpv4.Add(ipv4.Assign (serverDevices));


  // Set the size of the TC layer queue
  TrafficControlHelper tch;
  uint16_t handle = tch.SetRootQueueDisc ("ns3::PfifoFastQueueDisc");
  tch.AddInternalQueues (handle, 3, "ns3::DropTailQueue", "MaxPackets", UintegerValue (CONFIG_OUTPUT_BUFFER_SIZE_BYTES / 1500 - 10));

  // Create router nodes.
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  // Add the trace callback function.
  ostringstream oss;
  // oss << "/NodeList/" << router.Get (0) -> GetId () << "/DeviceList/" << linkServer.Get (1)->GetIfIndex() << "/$ns3::PointToPointNetDevice/TxQueue/Enqueue";
  oss << "/NodeList/" << router.Get (0) -> GetId () << "/$ns3::TrafficControlLayer/RootQueueDiscList/" << linkServer.Get (1)->GetIfIndex() << "/Enqueue";
  cout << oss.str() << endl;
  Config::Connect (oss.str(), MakeCallback (&AsciiEnqueueEvent));
  //servers.Get (0)->TraceConnect ("Enqueue", oss.str(), MakeCallback (&AsciiEnqueueEvent));

  oss.str("");
  oss.clear();
  // oss << "/NodeList/" << router.Get (0) -> GetId () << "/DeviceList/" << linkServer.Get (1)->GetIfIndex() << "/$ns3::PointToPointNetDevice/TxQueue/Dequeue";
  oss << "/NodeList/" << router.Get (0) -> GetId () << "/$ns3::TrafficControlLayer/RootQueueDiscList/" << linkServer.Get (1)->GetIfIndex() << "/Dequeue";
  cout << oss.str() << endl;
  Config::Connect (oss.str(), MakeCallback (&AsciiDequeueEvent));
  //servers.Get (0)->TraceConnect ("Enqueue", oss.str(), MakeCallback (&AsciiEnqueueEvent));

  oss.str("");
  oss.clear();
  // oss << "/NodeList/" << router.Get (0) -> GetId () << "/DeviceList/" << linkServer.Get (1)->GetIfIndex() << "/$ns3::PointToPointNetDevice/TxQueue/Drop";
  oss << "/NodeList/" << router.Get (0) -> GetId () << "/$ns3::TrafficControlLayer/RootQueueDiscList/" << linkServer.Get (1)->GetIfIndex() << "/Drop";
  cout << oss.str() << endl;
  Config::Connect (oss.str(), MakeCallback (&AsciiDropEvent));

  oss.str("");
  oss.clear();
  // oss << "/NodeList/" << router.Get (0) -> GetId () << "/DeviceList/" << linkServer.Get (1)->GetIfIndex() << "/$ns3::PointToPointNetDevice/TxQueue/BytesInQueue";
  oss << "/NodeList/" << router.Get (0) -> GetId () << "/$ns3::TrafficControlLayer/RootQueueDiscList/" << linkServer.Get (1)->GetIfIndex() << "/BytesInQueue";
  cout << oss.str() << endl;
  Config::Connect (oss.str(), MakeCallback (&AsciiPacketsInQueue));

  // This is to log down the number of bytes in the queue on the net device.
  oss.str("");
  oss.clear();
  oss << "/NodeList/" << router.Get (0) -> GetId () << "/DeviceList/" << linkServer.Get (1)->GetIfIndex() << "/$ns3::PointToPointNetDevice/TxQueue/BytesInQueue";
  //oss << "/NodeList/" << router.Get (0) -> GetId () << "/$ns3::TrafficControlLayer/RootQueueDiscList/" << linkServer.Get (1)->GetIfIndex() << "/BytesInQueue";
  cout << oss.str() << endl;
  Config::Connect (oss.str(), MakeCallback (&AsciiPacketsInQueueNetDevice));

   // Create a sink application on the server node to receive these applications. 
   uint16_t port = 50000;
   Address sinkLocalAddress (InetSocketAddress (serverIpv4.GetAddress(0), port));
   PacketSinkHelper sinkHelper ("ns3::TcpSocketFactory", sinkLocalAddress);
   ApplicationContainer sinkApp = sinkHelper.Install (servers.Get (0));
   sinkApp.Start (Seconds (CONFIG_START_TIME - 1));
   sinkApp.Stop (Seconds (CONFIG_STOP_TIME));

   // Create all the sockets.
   vector<Ptr<Socket> > SocketVector(numberOfTerminals);
   for (vector<Ptr<Socket> >::iterator it = SocketVector.begin(); it < SocketVector.end(); it++) {
     int nodeIndex = it - SocketVector.begin();
     *it = Socket::CreateSocket (terminals.Get (nodeIndex), TcpSocketFactory::GetTypeId ());
     ostringstream oss;
     oss << "/NodeList/" << terminals.Get (nodeIndex)->GetId () << "/$ns3::TcpL4Protocol/SocketList/0/CongestionWindow";
     //cout << oss.str() << endl;
     (*it)->TraceConnect ("CongestionWindow", oss.str(), MakeCallback (&CwndChange));
   }

   ApplicationContainer clientApps;
   vector<Ptr<MyApp> > ApplicationVector(numberOfTerminals);
   for(uint32_t i=0; i<terminals.GetN (); ++i)
   {
      Address sinkAddress (InetSocketAddress (serverIpv4.GetAddress (0), port)); 

      ApplicationVector[i] = CreateObject<MyApp> ();
      // void Setup (Ptr<Socket> socket, Address address, uint32_t packetSize, uint32_t nPackets, DataRate dataRate);
      // number of packets is not used here.

      // construct a string to denote the rate
      ApplicationVector[i]->Setup (SocketVector[i], sinkAddress, CONFIG_SENDER_PACKET_SIZE, 1000, DataRate (string (CONFIG_SENDER_LINK_DATA_RATE)), CONFIG_SENDER_PACKETS_PER_SHORT_FLOW, CONFIG_SENDER_INTERVAL_MEAN, CONFIG_SENDER_INTERVAL_BOUND);
      terminals.Get (i)->AddApplication (ApplicationVector[i]);
      clientApps.Add (ApplicationVector[i]);
   }
   clientApps.Start (Seconds (CONFIG_START_TIME));
   clientApps.Stop (Seconds (CONFIG_STOP_TIME));


  NS_LOG_INFO ("Configure Tracing.");

  //
  // Configure tracing of all enqueue, dequeue, and NetDevice receive events.
  // Trace output will be sent to the file "csma-bridge.tr"
  //
  AsciiTraceHelper ascii;
  p2p.EnableAsciiAll (ascii.CreateFileStream ("csma-bridge.tr"));

  //
  // Also configure some tcpdump traces; each interface will be traced.
  // The output files will be named:
  //     csma-bridge-<nodeId>-<interfaceId>.pcap
  // and can be read by the "tcpdump -r" command (use "-tt" option to
  // display timestamps correctly)
  //
  p2p.EnablePcapAll ("csma-bridge", false);
  Simulator::Stop (Seconds (CONFIG_STOP_TIME));
  //
  // Now, do the actual simulation.
  //
  NS_LOG_INFO ("Run Simulation.");
  Simulator::Run ();
  Simulator::Destroy ();
  NS_LOG_INFO ("Done.");
}




 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 

 
 







 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
