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
// n15  n14  n13  n3 -------------- n2   n10  n11  n12
//                                   |
//                                   |
//                                   |       10.1.2.0
//                                   |----> ponto-a ponto 
//   Wifi 10.1.4.0                   |
//                 Ap                |
//  *    *    *    *                 |
//  |    |    |    |    10.1.1.0     |
// n6   n5   n4   n0 -------------- n1   n7   n8   n9
//                   ponto-a-ponto   |    |    |    |
//                                   ================
//                                     LAN 10.1.5.0
// 
//
// Descrição: Esse sistema possui duas redes wifi (802.11x) e duas redes Ethernet (802.3).
// As redes se comunicam por meios dos nós n0, n1, n2, n3.
// A simulação será feita entre os nós n6 (10.1.4.4) e n15 (10.1.7.4). Onde o nó "n6" é o  
// receptor e o nó n15 é o transmissor. A simulação está configurada para os nós trocarem 
// pacotes durante 30s.
// 


using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("ProjetoTR1");

struct CSMAContainer {
    NodeContainer nodeContainer;
    NetDeviceContainer deviceContainer;
    Ipv4InterfaceContainer interfaceContainer;
};

struct WifiContainer {
    NodeContainer nodeApContainer;
    NetDeviceContainer deviceApContainer;
    Ipv4InterfaceContainer interfaceApContainer;
    

    NodeContainer nodeStaContainer;
    NetDeviceContainer deviceStaContainer;
    Ipv4InterfaceContainer interfaceStaContainer;
};

CSMAContainer criaDispositivoCSMA(CsmaHelper csma, Ptr<Node> no, uint32_t nNosExtras);
WifiContainer criaDispositivoWifi(YansWifiPhyHelper phy, Ptr<Node> noAp, uint32_t nNosExtras);

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

    /* Cria três dispositivos P2P que serviram para ligar todo sistema */
    NetDeviceContainer dispositivosP2PUm = pontoAPonto.Install(p2pNos.Get(0), p2pNos.Get(1));
    NetDeviceContainer dispositivosP2PDois = pontoAPonto.Install(p2pNos.Get(1), p2pNos.Get(2));
    NetDeviceContainer dispositivosP2PTres = pontoAPonto.Install(p2pNos.Get(2), p2pNos.Get(3));

    /* Cria dois dispositivos CSMA */
    CsmaHelper csma;
    csma.SetChannelAttribute ("DataRate", StringValue ("100Mbps"));
    csma.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (6560)));
    CSMAContainer CSMAUm = criaDispositivoCSMA(csma, p2pNos.Get(1), nCsma);
    CSMAContainer CSMADois = criaDispositivoCSMA(csma, p2pNos.Get(2), nCsma);

    /* Cria dois dispositivos Wifi */
    YansWifiPhyHelper phy = YansWifiPhyHelper::Default();
    WifiContainer WifiUm = criaDispositivoWifi(phy, p2pNos.Get(0), nWifi);
    WifiContainer WifiDois = criaDispositivoWifi(phy, p2pNos.Get(3), nWifi);

    // Cria um pilha para os protocolos
    InternetStackHelper stack;
    stack.Install(CSMAUm.nodeContainer);
    stack.Install(CSMADois.nodeContainer);
    stack.Install(WifiUm.nodeApContainer);
    stack.Install(WifiUm.nodeStaContainer);
    stack.Install(WifiDois.nodeApContainer);
    stack.Install(WifiDois.nodeStaContainer);

    // Configura endereços
    Ipv4AddressHelper address;

    // Endereços das conexões ponto-a-ponto
    address.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer p2pInterfacesUm = address.Assign(dispositivosP2PUm);
    address.SetBase("10.1.2.0", "255.255.255.0");
    Ipv4InterfaceContainer p2pInterfacesDois = address.Assign(dispositivosP2PDois);
    address.SetBase("10.1.3.0", "255.255.255.0");
    Ipv4InterfaceContainer p2pInterfacesTres = address.Assign(dispositivosP2PTres);

    // Endereço da primeira rede Wifi (Base: 10.1.4.0)
    address.SetBase("10.1.4.0", "255.255.255.0");
    WifiUm.interfaceApContainer = address.Assign(WifiUm.deviceApContainer);
    WifiUm.interfaceStaContainer = address.Assign(WifiUm.deviceStaContainer);

    // Endereço da primeira rede CSMA (Base: 10.1.5.0)
    address.SetBase("10.1.5.0", "255.255.255.0");
    CSMAUm.interfaceContainer = address.Assign(CSMAUm.deviceContainer);

    // Endereço da segunda rede CSMA (Base: 10.1.6.0)
    address.SetBase("10.1.6.0", "255.255.255.0");
    CSMADois.interfaceContainer = address.Assign(CSMADois.deviceContainer);

    // Endereço da segunda rede Wifi (Base: 10.1.7.0)
    address.SetBase("10.1.7.0", "255.255.255.0");
    WifiDois.interfaceApContainer = address.Assign(WifiDois.deviceApContainer);
    WifiDois.interfaceStaContainer = address.Assign(WifiDois.deviceStaContainer);

    // Configura nó receptor
    UdpEchoServerHelper echoServer(9);
    ApplicationContainer serverApps = echoServer.Install(WifiUm.nodeStaContainer.Get(nWifi - 1));
    serverApps.Start (Seconds (1.0));
    serverApps.Stop (Seconds (31.0));

    // Configura nó transmissor
    UdpEchoClientHelper echoClient (WifiUm.interfaceStaContainer.GetAddress(nWifi - 1), 9);
    echoClient.SetAttribute ("MaxPackets", UintegerValue (30));
    echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
    echoClient.SetAttribute ("PacketSize", UintegerValue (1024));
    ApplicationContainer clientApps = echoClient.Install(WifiDois.nodeStaContainer.Get(nWifi - 1));
    clientApps.Start(Seconds(2.0));
    clientApps.Stop(Seconds(31.0));

    // Popula rotas
    Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

    // Indica quanto tempo terá a simulação
    Simulator::Stop (Seconds (31.0));

    if (tracing) {
        pontoAPonto.EnablePcapAll("main");
        phy.EnablePcap("main", WifiUm.deviceApContainer.Get(0));
        phy.EnablePcap("main", WifiDois.deviceApContainer.Get(0));
        csma.EnablePcap("main", CSMAUm.deviceContainer.Get(0), true);
        csma.EnablePcap("main", CSMADois.deviceContainer.Get(0), true);
    }

    AnimationInterface::SetConstantPosition(WifiUm.nodeApContainer.Get(0), 4, 4);
    AnimationInterface::SetConstantPosition(WifiDois.nodeApContainer.Get(0), 4, 11);
    AnimationInterface::SetConstantPosition(CSMAUm.nodeContainer.Get(0), 8, 5);
    AnimationInterface::SetConstantPosition(CSMAUm.nodeContainer.Get(1), 10, 5);
    AnimationInterface::SetConstantPosition(CSMAUm.nodeContainer.Get(2), 12, 5);
    AnimationInterface::SetConstantPosition(CSMAUm.nodeContainer.Get(3), 14, 5);
    AnimationInterface::SetConstantPosition(CSMADois.nodeContainer.Get(0), 8, 10);
    AnimationInterface::SetConstantPosition(CSMADois.nodeContainer.Get(1), 10, 10);
    AnimationInterface::SetConstantPosition(CSMADois.nodeContainer.Get(2), 12, 10);
    AnimationInterface::SetConstantPosition(CSMADois.nodeContainer.Get(3), 14, 10);

    AnimationInterface anim ("animation.xml");

    /* Finaliza simulação */
    Simulator::Run();
    Simulator::Destroy();

    return 0;
}

// Essa função irá criar um dispositivo CSMA e irá retornar um container com esse
// dispositivo e os nós correspondentes.
CSMAContainer criaDispositivoCSMA(CsmaHelper csma, Ptr<Node> noP2P, uint32_t quantNosExtras) {

    // Cria os nós
    NodeContainer nosCSMA;
    nosCSMA.Add(noP2P);
    nosCSMA.Create(quantNosExtras);

    // Armazena os nós no container
    CSMAContainer csmaContainer;
    csmaContainer.nodeContainer = nosCSMA;

    // Armazena no container o novo dispositivo CSMA
    csmaContainer.deviceContainer = csma.Install(nosCSMA);

    return csmaContainer;
}

// Essa função irá criar dois dispositivos Wifi (Ap e Sta) e irá retornar um container
// com esses dispositivos e os nós correspondentes.
WifiContainer criaDispositivoWifi(YansWifiPhyHelper phy, Ptr<Node> noAp, uint32_t nNosExtras) {

    // Cria os nós
    NodeContainer wifiSta;
    wifiSta.Create(nNosExtras);

    // Armazena os nós no container
    WifiContainer wifiContainer;
    wifiContainer.nodeApContainer = noAp;
    wifiContainer.nodeStaContainer = wifiSta;
    
    // Configura ChannelHelpers e o PHY
    YansWifiChannelHelper canal = YansWifiChannelHelper::Default();
    phy.SetChannel(canal.Create());
    
    // Configura MAC
    WifiHelper wifi;
    wifi.SetRemoteStationManager("ns3::AarfWifiManager");
    WifiMacHelper mac;

    // Configura o SSID da infraestrutura da rede
    Ssid ssid = Ssid ("ns-3-ssid");

    // Armazena no container o novo dispositivo Wifi Ap
    mac.SetType ("ns3::ApWifiMac",
                    "Ssid", SsidValue (ssid));
    wifiContainer.deviceApContainer = wifi.Install (phy, mac, noAp);

    // Armazena no container os novos dispositivos Wifi STA
    mac.SetType ("ns3::StaWifiMac",
            "Ssid", SsidValue (ssid),
            "ActiveProbing", BooleanValue (false));
    wifiContainer.deviceStaContainer = wifi.Install (phy, mac, wifiSta);
    
    // Configura mobilidade geral dos nós
    MobilityHelper mobility;
    mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                    "MinX", DoubleValue (0.0),
                                    "MinY", DoubleValue (0.0),
                                    "DeltaX", DoubleValue (5.0),
                                    "DeltaY", DoubleValue (10.0),
                                    "GridWidth", UintegerValue (3),
                                    "LayoutType", StringValue ("RowFirst"));
    
    // Configura nó Ap para ter não ter mobilidade, ou seja, esteja estático em sua posição
    mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
    mobility.Install (wifiContainer.nodeApContainer);
    
    // Configura nós Sta para ter mobilidade randômica
    mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
                                "Bounds", RectangleValue (Rectangle (-50, 50, -50, 50)));
    mobility.Install (wifiContainer.nodeStaContainer);    

    return wifiContainer;
}