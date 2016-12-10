#include "ns3/core-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/netanim-module.h"


// Topologia da simulação
//
//   Wifi 10.1.7.0
//                 Ap                  LAN 10.1.6.0
//  *    *    *    *     10.1.3.0    ================
//  |    |    |    |  ponto a ponto  |    |    |    |
// n15  n14  n13  n3 -------------- n2   n12  n11  n10
//                                   |
//                                   |
//                                   |       10.1.2.0
//                                   |----> ponto-a ponto 
//   Wifi 10.1.4.0                   |
//                 Ap                |
//  *    *    *    *                 |
//  |    |    |    |    10.1.1.0     |
// n4   n5   n6   n0 -------------- n1   n7   n8   n9
//                   ponto-a-ponto   |    |    |    |
//                                   ================
//                                     LAN 10.1.5.0
// 
//
// Descrição: Simula uma rede de uma empresa que...
// 
//


using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("ProjetoTR1");

CsmaHelper csma;
YansWifiPhyHelper phy;

NodeContainer nosCSMAUm;
NodeContainer nosCSMADois;
NodeContainer nosWifiApUm;
NodeContainer nosWifiStaUm;
NodeContainer nosWifiApDois;
NodeContainer nosWifiStaDois;
NetDeviceContainer deviceAp;
NetDeviceContainer deviceSta;

NetDeviceContainer criaDispositivoCSMA(Ptr<Node> no, uint32_t nNosExtras, bool isFirst);
void criaDispositivoWifi(Ptr<Node> noAp, uint32_t nNosExtras, bool isFirst);

int main(int argc, char *argv[]) {
    bool verbose = true;
    bool tracing = true;
    uint32_t nCsma = 3;
    uint32_t nWifi = 3;

    CommandLine cmd;
    cmd.AddValue("nCsma", "Número de \"extra\" CSMA nós/dispositivos", nCsma);
    cmd.AddValue("nWifi", "Número de wifi STA dispositivos", nWifi);
    cmd.AddValue("verbose", "Comunica às aplicações echo para fazer o log se verdadeiro", verbose);
    cmd.AddValue("tracing", "Habilitar o trace do tráfego", tracing);

    cmd.Parse (argc, argv);

    /* Habilita logs */
    if (verbose) {
         LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
         LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);
    }
   

    /* Cria os nós */
    NodeContainer p2pNos;
    p2pNos.Create(4);

    /* Cria o meio utilizando um helper */
    PointToPointHelper pontoAPonto;
    pontoAPonto.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    pontoAPonto.SetChannelAttribute("Delay", StringValue("2ms"));

    NetDeviceContainer dispositivosP2PUm;
    dispositivosP2PUm = pontoAPonto.Install(p2pNos.Get(0), p2pNos.Get(1));

    NetDeviceContainer dispositivosP2PDois;
    dispositivosP2PDois = pontoAPonto.Install(p2pNos.Get(1), p2pNos.Get(2));

    NetDeviceContainer dispositivosP2PTres;
    dispositivosP2PTres = pontoAPonto.Install(p2pNos.Get(2), p2pNos.Get(3));

    NetDeviceContainer dispositivosCSMAUm = criaDispositivoCSMA(p2pNos.Get(1), nCsma, true);
    NetDeviceContainer dispositivosCSMADois = criaDispositivoCSMA(p2pNos.Get(2), nCsma, false);

    criaDispositivoWifi(p2pNos.Get(0), nWifi, true);
    NetDeviceContainer dispositivosWifiUm_Ap = deviceAp;
    NetDeviceContainer dispositivosWifiUm_Sta = deviceSta;

    criaDispositivoWifi(p2pNos.Get(3), nWifi, false);
    NetDeviceContainer dispositivosWifiDois_Ap = deviceAp;
    NetDeviceContainer dispositivosWifiDois_Sta = deviceSta;

    InternetStackHelper stack;
    stack.Install(nosCSMAUm);
    stack.Install(nosCSMADois);
    stack.Install(nosWifiApUm);
    stack.Install(nosWifiStaUm);
    stack.Install(nosWifiApDois);
    stack.Install(nosWifiStaDois);

    Ipv4AddressHelper address;

    address.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer p2pInterfacesUm;
    p2pInterfacesUm = address.Assign(dispositivosP2PUm);

    address.SetBase("10.1.2.0", "255.255.255.0");
    Ipv4InterfaceContainer p2pInterfacesDois;
    p2pInterfacesDois = address.Assign(dispositivosP2PDois);

    address.SetBase("10.1.3.0", "255.255.255.0");
    Ipv4InterfaceContainer p2pInterfacesTres;
    p2pInterfacesTres = address.Assign(dispositivosP2PTres);

    address.SetBase("10.1.4.0", "255.255.255.0");
    address.Assign(dispositivosWifiUm_Sta);
    address.Assign(dispositivosWifiUm_Ap);

    address.SetBase("10.1.5.0", "255.255.255.0");
    Ipv4InterfaceContainer csmaInterfacesUm;
    csmaInterfacesUm = address.Assign(dispositivosCSMAUm);

    address.SetBase("10.1.6.0", "255.255.255.0");
    Ipv4InterfaceContainer csmaIntefacesDois;
    csmaIntefacesDois = address.Assign(dispositivosCSMADois);

    address.SetBase("10.1.7.0", "255.255.255.0");
    address.Assign(dispositivosWifiDois_Sta);
    address.Assign(dispositivosWifiDois_Ap);

    UdpEchoServerHelper echoServer(9);

    ApplicationContainer serverApps = echoServer.Install(nosCSMAUm.Get(0));
    serverApps.Start (Seconds (1.0));
    serverApps.Stop (Seconds (30.0));

    UdpEchoClientHelper echoClient (csmaInterfacesUm.GetAddress(0), 9);
    echoClient.SetAttribute ("MaxPackets", UintegerValue (30));
    echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
    echoClient.SetAttribute ("PacketSize", UintegerValue (1024));

    ApplicationContainer clientApps = echoClient.Install(nosWifiStaDois.Get(nWifi - 1));
    clientApps.Start(Seconds(2.0));
    clientApps.Stop(Seconds(31.0));

    Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

    Simulator::Stop (Seconds (31.0));

    if (tracing) {
        pontoAPonto.EnablePcapAll("main");
        phy.EnablePcap("main", dispositivosWifiUm_Ap.Get(0));
        phy.EnablePcap("main", dispositivosWifiDois_Ap.Get(0));
        csma.EnablePcap("main", dispositivosCSMAUm.Get(0), true);
        csma.EnablePcap("main", dispositivosCSMADois.Get(0), true);

        AnimationInterface::SetConstantPosition(nosWifiApUm.Get(0), 2, 2);
        AnimationInterface::SetConstantPosition(nosWifiApDois.Get(0), 2, 7);
        AnimationInterface::SetConstantPosition(nosCSMAUm.Get(0), 4, 2);
        AnimationInterface::SetConstantPosition(nosCSMAUm.Get(1), 5, 2);
        AnimationInterface::SetConstantPosition(nosCSMAUm.Get(2), 6, 2);
        AnimationInterface::SetConstantPosition(nosCSMAUm.Get(3), 7, 2);
        AnimationInterface::SetConstantPosition(nosCSMADois.Get(0), 4, 7);
        AnimationInterface::SetConstantPosition(nosCSMADois.Get(1), 5, 7);
        AnimationInterface::SetConstantPosition(nosCSMADois.Get(2), 6, 7);
        AnimationInterface::SetConstantPosition(nosCSMADois.Get(3), 7, 7);

        AnimationInterface anim ("animation.xml");
    }


    /* Finaliza simulação */
    Simulator::Run();
    Simulator::Destroy();

    return 0;
}

NetDeviceContainer criaDispositivoCSMA(Ptr<Node> no, uint32_t nNosExtras, bool isFirst) {
    NodeContainer nosCSMA;
    nosCSMA.Add(no);
    nosCSMA.Create(nNosExtras);

    if (isFirst) {
        nosCSMAUm = nosCSMA;
    } else {
        nosCSMADois = nosCSMA;
    }

    csma.SetChannelAttribute ("DataRate", StringValue ("100Mbps"));
    csma.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (6560)));

    return csma.Install(nosCSMA);
}

void criaDispositivoWifi(Ptr<Node> noAp, uint32_t nNosExtras, bool isFirst) {
    NodeContainer nosWifiSta;
    nosWifiSta.Create(nNosExtras);
    NodeContainer noWifiAp = noAp;

    if (isFirst) {
        nosWifiStaUm = nosWifiSta;
        nosWifiApUm = noAp;
    } else {
        nosWifiStaDois = nosWifiSta;
        nosWifiApDois = noAp;
    }
    

    YansWifiChannelHelper canal = YansWifiChannelHelper::Default();
    phy = YansWifiPhyHelper::Default();
    phy.SetChannel(canal.Create());

    WifiHelper wifi;
    wifi.SetRemoteStationManager("ns3::AarfWifiManager");

    WifiMacHelper mac;
    Ssid ssid = Ssid ("ns-3-ssid");
    mac.SetType ("ns3::StaWifiMac",
            "Ssid", SsidValue (ssid),
            "ActiveProbing", BooleanValue (false));

    NetDeviceContainer staDevices;
    staDevices = wifi.Install (phy, mac, nosWifiSta);

    mac.SetType ("ns3::ApWifiMac",
                    "Ssid", SsidValue (ssid));

    NetDeviceContainer apDevices;
    apDevices = wifi.Install (phy, mac, noWifiAp);
    
    MobilityHelper mobility;

    mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                    "MinX", DoubleValue (0.0),
                                    "MinY", DoubleValue (0.0),
                                    "DeltaX", DoubleValue (5.0),
                                    "DeltaY", DoubleValue (10.0),
                                    "GridWidth", UintegerValue (3),
                                    "LayoutType", StringValue ("RowFirst"));

    mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
                                "Bounds", RectangleValue (Rectangle (-50, 50, -50, 50)));
    mobility.Install (nosWifiSta);

    mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
    mobility.Install (noWifiAp);

    deviceAp = apDevices;
    deviceSta = staDevices;   
}