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

#include <string>
#include <cmath>
#include <iostream>
#include <fstream>
#include <cstdlib>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Residential_Scenario");

void PopulateARPcache ();

int
main (int argc, char *argv[])
{
  uint32_t nFloors = 2;
  uint32_t xnFlats = 2;
  uint32_t ynFlats = 2;
  uint32_t nSta = 1;
  uint32_t RngRun = 0;

  CommandLine cmd;
  cmd.AddValue("nFloors", "Number of floors", nFloors);
  cmd.AddValue("xnFlats", "Number of flats per floor in x axis", xnFlats);
  cmd.AddValue("ynFlats", "Number of flats per floor in y axis", ynFlats);
  cmd.AddValue("nSta", "Number of stations per floor", nSta);
  cmd.AddValue("RngRun", "Number of stations per floor", RngRun);
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

  YansWifiChannelHelper channel = YansWifiChannelHelper::Default();
  channel.AddPropagationLoss("ns3::HybridBuildingsPropagationLossModel");
  YansWifiPhyHelper phy = YansWifiPhyHelper::Default();
  phy.SetPcapDataLinkType (YansWifiPhyHelper::DLT_IEEE802_11_RADIO);
  phy.SetChannel(channel.Create());

  WifiHelper wifi;
  wifi.SetStandard(WIFI_PHY_STANDARD_80211n_5GHZ);
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode", StringValue ("HtMcs7"), "ControlMode", StringValue("HtMcs0"));
  WifiMacHelper mac;

  NetDeviceContainer apDevices[nFlats];
  NetDeviceContainer staDevices[nFlats][nSta];
  Ssid ssid;
  for(uint32_t i=0;i<nFlats;i++){
    ssid = Ssid("network_" + std::to_string(i));
    phy.Set("ChannelNumber", UintegerValue(36));

    mac.SetType("ns3::ApWifiMac", "Ssid", SsidValue(ssid), "BeaconGeneration", BooleanValue(true));
    apDevices[i] = wifi.Install(phy, mac, wifiApNodes.Get(i));

    for(uint32_t j=0;j<nSta;j++){
      mac.SetType("ns3::StaWifiMac", "Ssid", SsidValue(ssid));
      staDevices[i][j] = wifi.Install(phy, mac, wifiStaNodes.Get(0));
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
  //TODO: Implement building aware pathloss model

  //TODO: Add layer 3 (Internet Stack)
  InternetStackHelper stack;
  stack.Install(wifiApNodes);
  for(uint32_t i = 0; i < nFlats; i++){
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

  //TODO: Setup applications (e.g. UdpClient)

  //TODO: Save pcap files

  //TODO: Perform simulation


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
