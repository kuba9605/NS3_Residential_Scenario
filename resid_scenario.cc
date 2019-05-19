#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/wifi-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include <ns3/buildings-module.h>

#include <string>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Residential_Scenario");

int
main (int argc, char *argv[])
{
  uint32_t nFloors = 3;
  uint32_t xnFlats = 4;
  uint32_t ynFlats = 4;
  uint32_t nSta = 1;

  CommandLine cmd;
  cmd.AddValue("nFloors", "Number of floors", nFloors);
  cmd.AddValue("xnFlats", "Number of flats per floor in x axis", xnFlats);
  cmd.AddValue("ynFlats", "Number of flats per floor in y axis", ynFlats);
  cmd.AddValue("nSta", "Number of stations per floor", nSta);
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

  return 0;
}
