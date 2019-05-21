#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/wifi-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/buildings-module.h"

#include <string>
#include <cmath>
#include <iostream>
#include <fstream>
#include <cstdlib>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Residential_Scenario");

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
  double y_max = ynFlats * 20.0;
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
  InternetStackHelper stack[nFlats];

  for(uint32_t i=0;i<nFlats;i++){
    stack[i].Install (wifiApNodes.Get(i));
    
    Ipv4AddressHelper address;
    address.SetBase ("192.168.0.0", "255.255.255.0");
    Ipv4InterfaceContainer apInterface;
    apInterface = address.Assign (apDevices[i]);
    
    for(uint32_t j=0;j<nSta;j++){
      stack[i].Install (wifiStaNodes.Get(j));

      Ipv4InterfaceContainer staInterface;
      staInterface = address.Assign (staDevices[i][j]);

      Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
    }
  }

  //TODO: Setup applications (e.g. UdpClient)

  //TODO: Save pcap files

  //TODO: Perform simulation


  return 0;
}

