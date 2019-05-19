#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/wifi-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include <ns3/buildings-module.h>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Residential_Scenario");

int
main (int argc, char *argv[])
{
  uint32_t nFloors = 3;
  uint32_t xnFlats = 4;
  uint32_t ynFlats = 4;

  CommandLine cmd;
  cmd.AddValue("nFloors", "Number of floors", nFloors);
  cmd.AddValue("xnFlats", "Number of flats per floor in x axis", xnFlats);
  cmd.AddValue("ynFlats", "Number of flats per floor in y axis", ynFlats);
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

  return 0;
}
