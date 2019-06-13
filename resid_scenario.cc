#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/wifi-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/buildings-module.h"
#include "ns3/buildings-propagation-loss-model.h"
#include "ns3/building.h"
#include "ns3/buildings-helper.h"
#include "ns3/flow-monitor-module.h"

#include <string>
#include <cmath>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <iomanip>

using namespace std;
using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Residential_Scenario");

void installTrafficGenerator(Ptr<ns3::Node> fromNode, Ptr<ns3::Node> toNode, int port, std::string offeredLoad, int packetSize, int simulationTime, int warmupTime);
void PopulateARPcache ();

int
main (int argc, char *argv[])
{
  uint32_t nFloors = 2;
  uint32_t xnFlats = 2;
  uint32_t ynFlats = 2;
  uint32_t nSta = 1;
  uint32_t RngRun = 0;
  std::string offeredLoad = "1"; //Mbps
	uint32_t simulationTime = 10;
	uint32_t warmupTime = 1;
	uint32_t packetSize = 1472;

  CommandLine cmd;
  cmd.AddValue("nFloors", "Number of floors", nFloors);
  cmd.AddValue("xnFlats", "Number of flats per floor in x axis", xnFlats);
  cmd.AddValue("ynFlats", "Number of flats per floor in y axis", ynFlats);
  cmd.AddValue("nSta", "Number of stations per floor", nSta);
  cmd.AddValue("RngRun", "Number of stations per floor", RngRun);
  cmd.AddValue("offeredLoad", "", offeredLoad);
  cmd.AddValue("simulationTime", "", simulationTime);
  cmd.AddValue("warmupTime", "", warmupTime);
  cmd.AddValue("packetSize", "", packetSize);
  cmd.Parse (argc, argv);

  double x_min = 0.0;
  double x_max = xnFlats * 10.0;
  double y_min = 0.0;
  double y_max = ynFlats * 10.0;
  double z_min = 0.0;
  double z_max = nFloors * 3.0;
  Ptr<Building> b = CreateObject <Building> ();
  b->SetBoundaries (Box (x_min, x_max, y_min, y_max, z_min, z_max));
  b->SetBuildingType (Building::Residential);
  b->SetExtWallsType (Building::ConcreteWithWindows);
  b->SetNFloors (nFloors);
  b->SetNRoomsX (xnFlats);
  b->SetNRoomsY (ynFlats);

  uint32_t nFlats = xnFlats * ynFlats * nFloors;
  NodeContainer wifiStaNodes;
  wifiStaNodes.Create(nFlats * nSta);
  NodeContainer wifiApNodes;
  wifiApNodes.Create(nFlats);

  YansWifiChannelHelper channel;
  channel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
  channel.AddPropagationLoss("ns3::HybridBuildingsPropagationLossModel");

  YansWifiPhyHelper phy = YansWifiPhyHelper::Default();
  phy.SetPcapDataLinkType (YansWifiPhyHelper::DLT_IEEE802_11_RADIO);
  phy.SetChannel(channel.Create());
  phy.Set("ChannelNumber", UintegerValue(36));

  WifiHelper wifi;
  wifi.SetStandard(WIFI_PHY_STANDARD_80211n_5GHZ);
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode", StringValue ("HtMcs7"), "ControlMode", StringValue("HtMcs0"));
  WifiMacHelper mac;

  NetDeviceContainer apDevices[nFlats];
  NetDeviceContainer staDevices[nFlats][nSta];
  Ssid ssid;
  for(uint32_t i=0;i<nFlats;i++){
    ssid = Ssid("network_" + std::to_string(i));
    mac.SetType("ns3::ApWifiMac", "Ssid", SsidValue(ssid), "BeaconGeneration", BooleanValue(true));
    apDevices[i] = wifi.Install(phy, mac, wifiApNodes.Get(i));

    for(uint32_t j=0;j<nSta;j++){
      mac.SetType("ns3::StaWifiMac", "Ssid", SsidValue(ssid));
      staDevices[i][j] = wifi.Install(phy, mac, wifiStaNodes.Get(i*nSta+j));
    }
  }

  MobilityHelper mobility;
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install(wifiApNodes);
  mobility.Install(wifiStaNodes);
  BuildingsHelper::Install (wifiApNodes);
  BuildingsHelper::Install (wifiStaNodes);
  Ptr<ConstantPositionMobilityModel> cpmmAp[nFlats];
  Ptr<ConstantPositionMobilityModel> cpmmSta[nFlats][nSta];
  srand(RngRun);
  uint32_t xApRand, yApRand, xStaRand, yStaRand;
  std::ofstream csv_file;
  csv_file.open("nodes.csv");
  csv_file << "xAP;yAP;zAP;";
  for(uint32_t j=0;j<nSta;j++){
    csv_file << "xSTA" << j << ";ySTA" << j << ";zSTA" << j << ";";
  }
  csv_file << std::endl;
  for(uint32_t i=0;i<nFlats;i++){
    xApRand = (rand() % 9) + 1;
    yApRand = (rand() % 9) + 1;
    cpmmAp[i] = wifiApNodes.Get(i)->GetObject<ConstantPositionMobilityModel> ();
    cpmmAp[i]->SetPosition(Vector((xApRand+i*10) % (xnFlats*10), int(yApRand+10*ceil(i/xnFlats)) % (ynFlats*10), 1.5 + 3*int(ceil(i/(xnFlats*ynFlats)))));
    csv_file << cpmmAp[i]->GetPosition().x << ";" << cpmmAp[i]->GetPosition().y << ";" << cpmmAp[i]->GetPosition().z << ";";
    for(uint32_t j=0;j<nSta;j++){
        xStaRand = (rand() % 9) + 1;
        yStaRand = (rand() % 9) + 1;
        cpmmSta[i][j] = wifiStaNodes.Get(i)->GetObject<ConstantPositionMobilityModel> ();
        cpmmSta[i][j]->SetPosition(Vector((xStaRand+i*10) % (xnFlats*10), int(yStaRand+10*ceil(i/xnFlats)) % (ynFlats*10), 1.5 + 3*int(ceil(i/(xnFlats*ynFlats)))));
        csv_file << cpmmSta[i][j]->GetPosition().x << ";" << cpmmSta[i][j]->GetPosition().y << ";" << cpmmSta[i][j]->GetPosition().z << ";";
    }
    csv_file << std::endl;
  }
  csv_file.close();

  BuildingsHelper::MakeMobilityModelConsistent ();
  //Building aware pathloss model

  //Layer 3 (Internet Stack)
  InternetStackHelper stack;
  stack.Install(wifiApNodes);
  for(uint32_t i = 0; i < (nFlats * nSta); i++){
    stack.Install(wifiStaNodes.Get(i));
  }

	Ipv4AddressHelper address;

  for(uint32_t i = 0; i < nFlats; i++){
    std::string addrString;
    addrString = "10.1." + std::to_string(i) + ".0";
    const char *cstr = addrString.c_str(); //convert to constant char
    address.SetBase(Ipv4Address(cstr), "255.255.255.0");
    address.Assign(apDevices[i]);

    for(uint32_t j = 0; j < nSta; j++){
      address.Assign(staDevices[i][j]);
    }
  }

  PopulateARPcache ();

  //Applications (e.g. UdpClient)

  uint32_t port = 9;
	for(uint32_t i = 0; i < nFlats; i++){
		for(uint32_t j = 0; j < nSta; j++){
			installTrafficGenerator(wifiStaNodes.Get(i*nSta + j), wifiApNodes.Get(i), port++, offeredLoad, packetSize, simulationTime, warmupTime);
    }
	}

  for(uint32_t i=0;i<nFlats;i++){
    phy.EnablePcap("pcap_ap", apDevices[i]);
	// for(uint32_t j=0;j<nSta;j++){
	// 	  phy.EnablePcap("pcap_sta", staDevices[i][j]);
	// }
  }

  FlowMonitorHelper flowmon;
	Ptr<FlowMonitor> monitor = flowmon.InstallAll();

  Simulator::Stop(Seconds(simulationTime + 1));
  Simulator::Run ();
  // Simulator::Destroy ();

  /* Calculate results */
	double flowThr;
	double flowDel;

	ofstream myfile;
	myfile.open ("resid_scenario.csv", ios::app);


	Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
	std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats ();
	for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i) {
		auto time = std::time(nullptr); //Get timestamp
		auto tm = *std::localtime(&time);
		Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
		flowThr=i->second.rxBytes * 8.0 / (i->second.timeLastRxPacket.GetSeconds () - i->second.timeFirstTxPacket.GetSeconds ()) / 1024 / 1024;
		flowDel=i->second.delaySum.GetSeconds () / i->second.rxPackets;
		NS_LOG_UNCOND ("Flow " << i->first  << " (" << t.sourceAddress << " -> " << t.destinationAddress << ")\tThroughput: " <<  flowThr  << " Mbps\tTime: " << i->second.timeLastRxPacket.GetSeconds () - i->second.timeFirstTxPacket.GetSeconds () << "\t Delay: " << flowDel << " s \n");
		myfile << std::put_time(&tm, "%Y-%m-%d %H:%M") << "," << offeredLoad << "," << RngSeedManager::GetRun() << "," << t.sourceAddress << "," << t.destinationAddress << "," << flowThr << "," << flowDel;
		myfile << std::endl;
	}
	myfile.close();


  return 0;
}

void PopulateARPcache () {
	Ptr<ArpCache> arp = CreateObject<ArpCache> ();
	arp->SetAliveTimeout (Seconds (3600 * 24 * 365) );

	for (NodeList::Iterator i = NodeList::Begin (); i != NodeList::End (); ++i)
	{
		Ptr<Ipv4L3Protocol> ip = (*i)->GetObject<Ipv4L3Protocol> ();
		NS_ASSERT (ip !=0);
		ObjectVectorValue interfaces;
		ip->GetAttribute ("InterfaceList", interfaces);

		for (ObjectVectorValue::Iterator j = interfaces.Begin (); j != interfaces.End (); j++)
		{
			Ptr<Ipv4Interface> ipIface = (*j).second->GetObject<Ipv4Interface> ();
			NS_ASSERT (ipIface != 0);
			Ptr<NetDevice> device = ipIface->GetDevice ();
			NS_ASSERT (device != 0);
			Mac48Address addr = Mac48Address::ConvertFrom (device->GetAddress () );

			for (uint32_t k = 0; k < ipIface->GetNAddresses (); k++)
			{
				Ipv4Address ipAddr = ipIface->GetAddress (k).GetLocal();
				if (ipAddr == Ipv4Address::GetLoopback ())
					continue;

				ArpCache::Entry *entry = arp->Add (ipAddr);
				Ipv4Header ipv4Hdr;
				ipv4Hdr.SetDestination (ipAddr);
				Ptr<Packet> p = Create<Packet> (100);
				entry->MarkWaitReply (ArpCache::Ipv4PayloadHeaderPair (p, ipv4Hdr));
				entry->MarkAlive (addr);
			}
		}
	}

	for (NodeList::Iterator i = NodeList::Begin (); i != NodeList::End (); ++i)
	{
		Ptr<Ipv4L3Protocol> ip = (*i)->GetObject<Ipv4L3Protocol> ();
		NS_ASSERT (ip !=0);
		ObjectVectorValue interfaces;
		ip->GetAttribute ("InterfaceList", interfaces);

		for (ObjectVectorValue::Iterator j = interfaces.Begin (); j != interfaces.End (); j ++)
		{
			Ptr<Ipv4Interface> ipIface = (*j).second->GetObject<Ipv4Interface> ();
			ipIface->SetAttribute ("ArpCache", PointerValue (arp) );
		}
	}
}

void installTrafficGenerator(Ptr<ns3::Node> fromNode, Ptr<ns3::Node> toNode, int port, std::string offeredLoad, int packetSize, int simulationTime, int warmupTime ) {

	Ptr<Ipv4> ipv4 = toNode->GetObject<Ipv4> (); // Get Ipv4 instance of the node
	Ipv4Address addr = ipv4->GetAddress (1, 0).GetLocal (); // Get Ipv4InterfaceAddress of xth interface.

	ApplicationContainer sourceApplications, sinkApplications;

	uint8_t tosValue = 0x70; //AC_BE

	//Add random fuzz to app start time
	double min = 0.0;
	double max = 1.0;
	Ptr<UniformRandomVariable> fuzz = CreateObject<UniformRandomVariable> ();
	fuzz->SetAttribute ("Min", DoubleValue (min));
	fuzz->SetAttribute ("Max", DoubleValue (max));

	InetSocketAddress sinkSocket (addr, port);
	sinkSocket.SetTos (tosValue);
	OnOffHelper onOffHelper ("ns3::UdpSocketFactory", sinkSocket);
	onOffHelper.SetConstantRate (DataRate (offeredLoad + "Mbps"), packetSize);
	sourceApplications.Add (onOffHelper.Install (fromNode)); //fromNode
	PacketSinkHelper packetSinkHelper ("ns3::UdpSocketFactory", sinkSocket);
	sinkApplications.Add (packetSinkHelper.Install (toNode)); //toNode

	sinkApplications.Start (Seconds (warmupTime));
	sinkApplications.Stop (Seconds (simulationTime));
	sourceApplications.Start (Seconds (warmupTime+fuzz->GetValue ()));
	sourceApplications.Stop (Seconds (simulationTime));
}
