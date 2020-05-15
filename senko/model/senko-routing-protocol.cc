/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2009 IITP RAS
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */
#define Dmin 400 //速度判定に至る距離の差の最小値
#define Vmin 120 //速度のしきい値
#define NS_LOG_APPEND_CONTEXT                                   \
  if (m_ipv4) { std::clog << "[node " << m_ipv4->GetObject<Node> ()->GetId () << "] "; }

#define ReceiveLog 1 //1 log 0 lognothing
#define ImageRange 40 //画像処理の距離
#define NodeNum 110//ノード数
#define NotransTime 1//NotransTime 止まっていたら通信をやめさせる  5秒
#define transTime 5  //Transtime以上動いていたら通信を可能にさせる
#define CancelAngle 20 //canselAngle　度内に同じノードがあれば近い方だけ取得する
#define DangerSpeed 120 //危険車両とみなすスピード
#define SimTime 750 //シミュレーション時間
#define SimMilliTime 750000//シミュレーション時間×1000
#define SimDeciTime 7500 //シミュレーション時間×10
#define Resendbroad 1//１のときresendxbroadcast関数を呼び出す
#define waitmax 1000000 //microsecond 単位で待ち時間の最大値
#define TransLange 200//通信レンジ
#define HopMax 8 //hop max
#define sv 10 //sv以上のIDを持つ車両は違反車両


#include "senko-routing-protocol.h"
#include "ns3/log.h"
#include "ns3/boolean.h"
#include "ns3/random-variable-stream.h"
#include "ns3/inet-socket-address.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/udp-socket-factory.h"
#include "ns3/udp-l4-protocol.h"
#include "ns3/udp-header.h"
#include "ns3/wifi-net-device.h"
#include "ns3/adhoc-wifi-mac.h"
#include "ns3/string.h"
#include "ns3/pointer.h"
#include <algorithm>
#include <limits>
#include <stdio.h>
#include <map>
#include <cmath>

#include "ns3/mobility-module.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("SenkoRoutingProtocol");

namespace senko {
NS_OBJECT_ENSURE_REGISTERED (RoutingProtocol);

/// UDP Port for SENKO control traffic
const uint32_t RoutingProtocol::SENKO_PORT = 654;




RoutingProtocol::RoutingProtocol ()
{
 
}

TypeId
RoutingProtocol::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::senko::RoutingProtocol")
    .SetParent<Ipv4RoutingProtocol> ()
    .SetGroupName ("Senko")
    .AddConstructor<RoutingProtocol> ()
    .AddAttribute ("UniformRv",
                   "Access to the underlying UniformRandomVariable",
                   StringValue ("ns3::UniformRandomVariable"),
                   MakePointerAccessor (&RoutingProtocol::m_uniformRandomVariable),
                   MakePointerChecker<UniformRandomVariable> ())
  ;
  return tid;
}



RoutingProtocol::~RoutingProtocol ()
{
}

void
RoutingProtocol::DoDispose ()
{

}

void
RoutingProtocol::PrintRoutingTable (Ptr<OutputStreamWrapper> stream, Time::Unit unit) const
{
  *stream->GetStream () << "Node: " << m_ipv4->GetObject<Node> ()->GetId ()
                        << "; Time: " << Now ().As (unit)
                        << ", Local time: " << GetObject<Node> ()->GetLocalTime ().As (unit)
                        << ", SENKO Routing table" << std::endl;

  //Print routing table here.
}

uint64_t
RoutingProtocol::AssignStreams (uint64_t stream)
{
  NS_LOG_FUNCTION (this << stream);
  m_uniformRandomVariable->SetStream (stream);
  return 1;
}



Ptr<Ipv4Route>
RoutingProtocol::RouteOutput (Ptr<Packet> p, const Ipv4Header &header,
                              Ptr<NetDevice> oif, Socket::SocketErrno &sockerr)
{

  //std::cout<<"Route Ouput Node: "<<m_ipv4->GetObject<Node> ()->GetId ()<<"\n";
  Ptr<Ipv4Route> route;

  if (!p)
    {
	  std::cout << "loopback occured! in routeoutput";
	  return route;// LoopbackRoute (header,oif);

	}

  if (m_socketAddresses.empty ())
    {
	  sockerr = Socket::ERROR_NOROUTETOHOST;
	  NS_LOG_LOGIC ("No zeal interfaces");
	  std::cout << "RouteOutput No zeal interfaces!!, packet drop\n";

	  Ptr<Ipv4Route> route;
	  return route;
    }





  
  return route;
}



bool
RoutingProtocol::RouteInput (Ptr<const Packet> p, const Ipv4Header &header,
                             Ptr<const NetDevice> idev, UnicastForwardCallback ucb,
                             MulticastForwardCallback mcb, LocalDeliverCallback lcb, ErrorCallback ecb)
{
  
  std::cout<<"Route Input Node: "<<m_ipv4->GetObject<Node> ()->GetId ()<<"\n";
  return true;
}



void
RoutingProtocol::SetIpv4 (Ptr<Ipv4> ipv4)
{
  NS_ASSERT (ipv4 != 0);
  NS_ASSERT (m_ipv4 == 0);
  m_ipv4 = ipv4;
}

void
RoutingProtocol::NotifyInterfaceUp (uint32_t i)
{
  NS_LOG_FUNCTION (this << m_ipv4->GetAddress (i, 0).GetLocal ()
                        << " interface is up");
  Ptr<Ipv4L3Protocol> l3 = m_ipv4->GetObject<Ipv4L3Protocol> ();
  Ipv4InterfaceAddress iface = l3->GetAddress (i,0);
  if (iface.GetLocal () == Ipv4Address ("127.0.0.1"))
    {
      return;
    }
  // Create a socket to listen only on this interface
  Ptr<Socket> socket;

  socket = Socket::CreateSocket (GetObject<Node> (),UdpSocketFactory::GetTypeId ());
  NS_ASSERT (socket != 0);
  socket->SetRecvCallback (MakeCallback (&RoutingProtocol::RecvSenko,this));
  socket->BindToNetDevice (l3->GetNetDevice (i));
  socket->Bind (InetSocketAddress (iface.GetLocal (), SENKO_PORT));
  socket->SetAllowBroadcast (true);
  socket->SetIpRecvTtl (true);
  m_socketAddresses.insert (std::make_pair (socket,iface));


    // create also a subnet broadcast socket
  socket = Socket::CreateSocket (GetObject<Node> (),
                                 UdpSocketFactory::GetTypeId ());
  NS_ASSERT (socket != 0);
  socket->SetRecvCallback (MakeCallback (&RoutingProtocol::RecvSenko, this));
  socket->BindToNetDevice (l3->GetNetDevice (i));
  socket->Bind (InetSocketAddress (iface.GetBroadcast (), SENKO_PORT));
  socket->SetAllowBroadcast (true);
  socket->SetIpRecvTtl (true);
  m_socketSubnetBroadcastAddresses.insert (std::make_pair (socket, iface));


  if (m_mainAddress == Ipv4Address ())
    {
      m_mainAddress = iface.GetLocal ();
    }

  NS_ASSERT (m_mainAddress != Ipv4Address ());


/*  for (int32_t i = 0; i < m_ipv4->GetNInterfaces (); i++)
        {

          // Use primary address, if multiple
          Ipv4Address addr = m_ipv4->GetAddress (i, 0).GetLocal ();
        //  std::cout<<"############### "<<addr<<" |ninterface "<<m_ipv4->GetNInterfaces ()<<"\n";
       

              TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
              Ptr<Node> theNode = GetObject<Node> ();
              Ptr<Socket> socket = Socket::CreateSocket (theNode,tid);
              InetSocketAddress inetAddr (m_ipv4->GetAddress (i, 0).GetLocal (), SENKO_PORT);
              if (socket->Bind (inetAddr))
                {
                  NS_FATAL_ERROR ("Failed to bind() ZEAL socket");
                }
              socket->BindToNetDevice (m_ipv4->GetNetDevice (i));
              socket->SetAllowBroadcast (true);
              socket->SetRecvCallback (MakeCallback (&RoutingProtocol::RecvSenko, this));
              //socket->SetAttribute ("IpTtl",UintegerValue (1));
              socket->SetRecvPktInfo (true);

              m_socketAddresses[socket] = m_ipv4->GetAddress (i, 0);

              //  NS_LOG_DEBUG ("Socket Binding on ip " << m_mainAddress << " interface " << i);

              break;
           
        }
*/

}

void
RoutingProtocol::NotifyInterfaceDown (uint32_t i)
{
  
}

void
RoutingProtocol::NotifyAddAddress (uint32_t i, Ipv4InterfaceAddress address)
{
 
}

void
RoutingProtocol::NotifyRemoveAddress (uint32_t i, Ipv4InterfaceAddress address)
{
 
}



void
RoutingProtocol::DoInitialize (void)     //ノードの数だけnode0から呼び出される関数 
{
  int32_t id = m_ipv4->GetObject<Node> ()->GetId ();
  if(id == 0)
  {
    SetPushBack();
    for(int i=1; i<SimTime; i++)
      {
        Simulator::Schedule(Seconds(i), &RoutingProtocol::PrintTime, this);
      }

  }
      for(int i=1; i<SimTime; i++){
          Simulator::Schedule(Seconds(i), &RoutingProtocol::GetGod, this);
          //Simulator::Schedule(Seconds(i), &RoutingProtocol::HelloNodeId, this);
          Simulator::Schedule(Seconds(i), &RoutingProtocol::SetMyPos, this);

          //getareagod メソッドで作られるマップの定期的なクリア
          Simulator::Schedule(Seconds(i), &RoutingProtocol::EraseAngle, this);

          //画像処理内にいるノードだけのIDと角度を保存
          Simulator::Schedule(Seconds(i), &RoutingProtocol::GetAreaGod, this);
          //時間をプリント
      }
      for(int i=1; i<SimMilliTime; i++){
        Simulator::Schedule(MilliSeconds(i), &RoutingProtocol::SetGodCall, this);
      }
}
void
RoutingProtocol::SetGodCall()
{
   Ptr<MobilityModel> mobility = m_ipv4->GetObject<Node> () ->GetObject<MobilityModel>();
      Vector mypos = mobility->GetPosition ();
  SetGodposition(mypos.x, mypos.y);
}


void
RoutingProtocol::RecvSenko (Ptr<Socket> socket)
{
  //std::cout<<"recv\n";
  Address sourceAddress;
  Ptr<Packet> packet = socket->RecvFrom (sourceAddress);
  int32_t id = m_ipv4->GetObject<Node> ()->GetId ();
  //std::cout<<"packet size"<<packet->GetSize()<<"\n";
  double x,y; //パケットを受け取った時点のノードの座標
  Ptr<MobilityModel> mobility = m_ipv4->GetObject<Node> () ->GetObject<MobilityModel>();
      Vector mypos = mobility->GetPosition ();
      x = mypos.x;
      y = mypos.y;

 int32_t recvtime = Simulator::Now().GetMicroSeconds();//このパケットを受け取った時間 

 if(id ==7000)
 std::cout<<x<<y<<recvtime;

  TypeHeader tHeader (SENKOTYPE_HELLOID);//default
  packet->RemoveHeader (tHeader);
  
  if (!tHeader.IsValid ())
    {
      NS_LOG_DEBUG ("Senko protocol message " << packet->GetUid () << " with unknown type received: " << tHeader.Get () << ". Drop");
      return; // drop
    }
  switch (tHeader.Get ())
    {
    case SENKOTYPE_HELLOID:
    {
      /*
      if(mtran[id] == 1){//通信可能ノードのみ

      std::cout<<"In recv hellloid (Node "<< id<<")------------------------------------------------------\n";
      IdHeader Idheader;
      packet->RemoveHeader(Idheader);
      int32_t helloid = Idheader.GetHelloId ();
      int32_t helloxpoint = Idheader.GetHelloXpoint ();
      int32_t helloypoint = Idheader.GetHelloYpoint ();
      int32_t recvidtime = Simulator::Now().GetMicroSeconds();
      //int32_t recvidtime = Simulator::Now().GetSeconds();
      std::cout<<"reveive  time"<< recvidtime<<"\n\n";


      int intrecvidtime = recvidtime;  //８ビットをintに変換
      
      if(mxpoint.count(helloid) == 0){//受け取ったIDを持っているかどうか

        std::cout<<"mxpoint doesn't have"<< helloid<<"."<<"\n";
        //SendXBroadcast(helloid,helloxpoint,helloypoint,0,recvidtime,0);

      }else{//持っている場合

        std::cout<<"m has"<<helloid<<"."<<"\n";
        std::cout<<"スピード違反の疑いがある id="<<helloid<<"\n";
        std::cout<<" その車両のX座標は"<<mxpoint[helloid]<<"\n";
        std::cout<<" その車両のY座標は"<<mypoint[helloid]<<"\n";
        std::cout<<" その車両時間は"<<mtime[helloid]<<"\n";
        std::cout<<"その車両の警戒値は"<<mdanger[helloid]<<"\n";
        //---------------------------------------------------------------------------------speed measument
        double dist = std::sqrt((mxpoint[helloid] - helloxpoint) * (mxpoint[helloid] - helloxpoint) 
        + (mypoint[helloid] - helloypoint) * (mypoint[helloid] - helloypoint));
        std::cout<<" dist="<<dist<<"\n";
        if(dist>Dmin) //Dminより位置の差が大きい場合
        {
        std::cout<<" 速度を測定します"<<"\n";
        int timelag = intrecvidtime - mtime[helloid];
        double cotimelag = timelag/100000;
        std::cout<<" cotimelag="<<cotimelag<<"\n";
        double speed = dist/cotimelag*10;
        std::cout<<"その車両の速度は"<<speed<<"m/s"<<"\n";
        double cospeed = speed*3.6;
        std::cout<<"その車両の速度は"<<cospeed<<"km/h"<<"\n";
        if(cospeed>Vmin){//速度違反なら
          std::cout<<"速度違反者がいました　そのIDは"<<helloid<<"\n";
          mdanger[helloid]++; //警戒値の更新
          mxpoint[helloid]=helloxpoint; //速度違反に伴い
          mypoint[helloid]=helloypoint; //判定した位置情報と時間を
          mtime[helloid]=recvidtime; //更新する
          //SendXBroadcast(helloid,helloxpoint,helloypoint,0,recvidtime,mdanger[helloid]);
          std::cout<<"mdanger[helloid]"<<mdanger[helloid]<<"\n";
        }else{//速度違反じゃないなら
          
        }

        }else{//Dminより位置の差が小さい場合
          std::cout<<"Dminより小さかったです"<<"\n";
          //SendXBroadcast(helloid,helloxpoint,helloypoint,0,recvidtime,0);
        }
        //---------------------------------------------------------------------------------speed measument
      }//持っている場合
      
      break;
      }
      */
    }//SENKOTYPE_HELLOID
    case SENKOTYPE_DANGER:
      {
        if(trans[id] == 1 ){//通信可能ノードで違反車両ではい場合

        DangerHeader dangerheader;
        packet->RemoveHeader(dangerheader);
        
        int32_t recvid = dangerheader.GetNodeId();
        int32_t posx = dangerheader.GetPosX();
        int32_t posy = dangerheader.GetPosY();
        int8_t hopcount = dangerheader.GetHopCount();
        int32_t detectiontime = dangerheader.GetRecvTime();
        int8_t danger = dangerheader.GetDanger();
        int32_t sourcex = dangerheader.GetMyPosX();
        int32_t sourcey = dangerheader.GetMyPosY();
       /////////////////////////////////////////////////////////////////////////////////////
       //後方からのパケットかどうかを判定
      double dist = GetDist(x, y, (double)sourcex , (double)sourcey);
      double past_dist = GetDist(mypastxpoint[id], mypastypoint[id], 
      (double)sourcex , (double)sourcey);
      if(dist > past_dist)//後方からのパケットのみ
      {
      


       ///////////////////////////////////////////////////////////////////////////////////

        if(mxpoint.count(recvid) == 0){//受け取ったIDを持っているかどうか
          //std::cout<<"そのIDのマップを持っていないのでセットします\n";
          ////////////////////////////////////////////////////////////////////パケットMidをマップに保存
          SaveXMap(recvid,posx);//速度判定対象車両のIDとx座標
          SaveYMap(recvid,posy);//速度判定対象車両のIDとｙ座標
          SaveTMap(recvid,detectiontime);//速度判定対象車両のIDと検知された時間
          SaveDMap(recvid, danger);//速度判定対象車両のIDと警戒値
          if(hopcount ==  1)
        {
        
        //std::cout<<"GetBroadcast value-------------------------------------------------\n";
        std::cout<<"In recv broadcast (Node "<< id<<"Time"<<Simulator::Now().GetMicroSeconds()<<")------------------------------------------------------\n";
        //std::cout<<"broadcast reveive hellonode id"<< recvid<<"\n\n";
        //std::cout<<"broadcast reveive point"<< posx<<"\n\n";
        //std::cout<<"broadcast reveive point"<< posy<<"\n\n";
        std::cout<<"broadcast reveive hopcount"<< (unsigned)hopcount<<"\n\n";
        //std::cout<<"broadcast reveive detectiontime"<< detectiontime<<"\n\n";
        std::cout<<"broadcast reveive danger"<< (unsigned)danger<<"\n\n";
        //std::cout<<"broadcast reveive source xposition"<< myposx<<"\n\n";
        //std::cout<<"broadcast reveive source yposition"<< myposy<<"\n\n";
        //std::cout<<"broadcast reveive source past  yposition"<< pastmyposx<<"\n\n";
        //std::cout<<"broadcast reveive source past yposition"<< pastmyposy<<"\n\n";
       // std::cout<<"broadcast reveive time"<< recvtime<<"\n\n";
        
        //std::cout<<"----------------------------------------------------------------\n\n";
        //hopcount = 2;
        //hopcount++;
        //if(id == 12){
          //PauseTime();
          //double pastx = mypastxpoint[id];
          //double pasty = mypastypoint[id];
          //Simulator::ScheduleNow (&RoutingProtocol::ReSendXBroadcast,1.0,1.0,1.0,1.0,1.0,
          //1.0,1.0,1.0,1.0,1.0);//６個までは送れる？
          // Simulator::ScheduleNow(&RoutingProtocol::GetAngle,this,1.0,1.0,1.0,mypastxpoint[id],1.0);
          //&RoutingProtocol::SetMyPos, this
          if(Resendbroad == 1)
          {
            ReSendXBroadcast(recvid, posx, posy, hopcount, recvtime, danger,(int)x, (int) y);
          //ここでは自分の位置情報をいれない

          }

          

        //}
        }else if(hopcount <=HopMax)
        {
          rebroadid[recvid] = hopcount;//リブロードキャストで受け取ったIDとホップカウントを保存
          rebroad_danger[recvid] = danger;
          std::cout<<"In recv rebroadcast (Node "<< id<<"Time"<<Simulator::Now().GetMicroSeconds()<<")------------------------------------------------------\n";
          //std::cout<<"GetReBroadcast value-------------------------------------------------\n";
          
          //std::cout<<"rebroadcast reveive hellonode id"<< recvid<<"\n\n";
          //std::cout<<"rebroadcast reveive point"<< posx<<"\n\n";
          //std::cout<<"rebroadcast reveive point"<< posy<<"\n\n";
          std::cout<<"rebroadcast reveive hopcount"<< (unsigned)hopcount<<"\n\n";
          //std::cout<<"rebroadcast reveive detectiontime"<< detectiontime<<"\n\n";
          std::cout<<"rebroadcast reveive danger"<< (unsigned)danger<<"\n\n";
          //std::cout<<"rebroadcast reveive source xposition"<< myposx<<"\n\n";
          //std::cout<<"rebroadcast reveive source yposition"<< myposy<<"\n\n";
          //std::cout<<"rebroadcast reveive source past  yposition"<< pastmyposx<<"\n\n";
          //std::cout<<"rebroadcast reveive source past yposition"<< pastmyposy<<"\n\n";
          //std::cout<<"rebroadcast reveive time"<< recvtime<<"\n\n";
        
          std::cout<<"----------------------------------------------------------------\n\n";
          
         std::cout<<"rebroadcast reveive hopcount"<< (unsigned)hopcount<<"\n\n";

         //hopcount = hopcount+1;
         if(Resendbroad == 1)//1notoki rebroadcast
         {
           ReSendXBroadcast(recvid, posx, posy, hopcount, recvtime, danger,(int)x, (int) y);
          //ここでは自分の位置情報をいれない
         }
         

        }////受け取ったIDを持っているかどうかのif
          ////////////////////////////////////////////////////////////////////////
        }else{//持っている場合

          if(mdanger[recvid]<danger){//保持している警戒値と受け取ったWidの警戒値が異なっていたら
            //std::cout<<"警戒値がましているパケットを受け取ったので更新します\n";
            mdanger[recvid]=danger;
            mxpoint[recvid]=posx;
            mypoint[recvid]=posy;
            mtime[recvid]=detectiontime;

            if(hopcount ==  1)
        {
        
        //std::cout<<"すでに持ってるGetBroadcast value-------------------------------------------------\n";
        std::cout<<"update In recv broadcast (Node "<< id<<"Time"<<Simulator::Now().GetMicroSeconds()<<")------------------------------------------------------\n";
        //std::cout<<"broadcast reveive hellonode id"<< recvid<<"\n\n";
        //std::cout<<"broadcast reveive point"<< posx<<"\n\n";
        //std::cout<<"broadcast reveive point"<< posy<<"\n\n";
        std::cout<<"broadcast reveive hopcount"<< (unsigned)hopcount<<"\n\n";
        //std::cout<<"broadcast reveive detectiontime"<< detectiontime<<"\n\n";
        std::cout<<"broadcast reveive danger"<< (unsigned)danger<<"\n\n";
        //std::cout<<"broadcast reveive source xposition"<< myposx<<"\n\n";
        //std::cout<<"broadcast reveive source yposition"<< myposy<<"\n\n";
        //std::cout<<"broadcast reveive source past  yposition"<< pastmyposx<<"\n\n";
        //std::cout<<"broadcast reveive source past yposition"<< pastmyposy<<"\n\n";
        //std::cout<<"broadcast reveive time"<< recvtime<<"\n\n";
        
        //std::cout<<"----------------------------------------------------------------\n\n";
        
        //hopcount = 2;
       // hopcount++;
        //ReSendXBroadcast(recvid, posx, posy, hopcount, recvtime, danger,
         //x, y, myxpoint[id], myypoint[id]);
         if(Resendbroad == 1)//1notoki rebroadcast
         {
           ReSendXBroadcast(recvid, posx, posy, hopcount, recvtime, danger,(int)x, (int) y);
          //ここでは自分の位置情報をいれない
         }
        }else if(hopcount <=HopMax)
        {
          rebroadid[recvid] = hopcount;//リブロードで受け取ったIDとホップカウントを保存
          rebroad_danger[recvid] = danger;
          //std::cout<<"In recv rebroadcast (Node "<< id<<")------------------------------------------------------\n";
          std::cout<<" update In recv rebroadcast (Node "<< id<<"Time"<<Simulator::Now().GetMicroSeconds()<<")------------------------------------------------------\n";
          
          //std::cout<<"GetReBroadcast value-------------------------------------------------\n";

          //std::cout<<"broadcast reveive hellonode id"<< recvid<<"\n\n";
          //std::cout<<"broadcast reveive point"<< posx<<"\n\n";
          //std::cout<<"broadcast reveive point"<< posy<<"\n\n";
          std::cout<<"broadcast reveive hopcount"<< (unsigned)hopcount<<"\n\n";
          //std::cout<<"broadcast reveive detectiontime"<< detectiontime<<"\n\n";
          std::cout<<"broadcast reveive danger"<< (unsigned)danger<<"\n\n";
          //std::cout<<"broadcast reveive source xposition"<< myposx<<"\n\n";
          //std::cout<<"broadcast reveive source yposition"<< myposy<<"\n\n";
          //std::cout<<"broadcast reveive source past  yposition"<< pastmyposx<<"\n\n";
          //std::cout<<"broadcast reveive source past yposition"<< pastmyposy<<"\n\n";
          //std::cout<<"broadcast reveive time"<< recvtime<<"\n\n";
          //hopcount = hopcount+1;
          //std::cout<<"----------------------------------------------------------------\n\n";
          if(Resendbroad == 1)//1notoki rebroadcast
         {
           ReSendXBroadcast(recvid, posx, posy, hopcount, recvtime, danger,(int)x, (int) y);
          //ここでは自分の位置情報をいれない
         }
        }
      }else{//すでに情報を受け取った場合
          //rebroadid[recvid] = hopcount;//リブロードキャストで受け取ったIDとホップカウントを保存
          //rebroad_danger[recvid] = danger;
       }
      }//持っている場合
      }// kouhoukarano  packet nomi

      }//通信可能ノードのみ
    





      }//case SENKO_DANGER
    }//switch
 
 


  
}

void
RoutingProtocol::SendXBroadcast (int32_t recvid, int32_t posx ,int32_t posy, int8_t hopcount,
int32_t time,int8_t danger, int sourcex, int sourcey)
{
  
  for (std::map<Ptr<Socket>, Ipv4InterfaceAddress>::const_iterator j = m_socketAddresses.begin (); j
       != m_socketAddresses.end (); ++j)
    {
      //int id = m_ipv4->GetObject<Node> ()->GetId ();
      //broadcount[0]++;
      Ptr<Socket> socket = j->first;
      Ipv4InterfaceAddress iface = j->second;
      Ptr<Packet> packet = Create<Packet> ();
      /////////////////////////////////////////////////////////////////////////////
     // dangerheader sendcodes
      
      //DangerHeader dangerHeader(id,11,11,11,11);
      DangerHeader dangerHeader(recvid,posx,posy,hopcount,time,danger,sourcex,sourcey);
      packet->AddHeader (dangerHeader);
      TypeHeader tHeader (SENKOTYPE_DANGER);
      packet->AddHeader (tHeader);
      // Send to all-hosts broadcast if on /32 addr, subnet-directed otherwise
      Ipv4Address destination;
      
      if (iface.GetMask () == Ipv4Mask::GetOnes ())
        {
          destination = Ipv4Address ("255.255.255.255");
        }
      else
        {
          destination = iface.GetBroadcast ();
        }
      Time Jitter = Time (MicroSeconds (m_uniformRandomVariable->GetInteger (0,150)));
      //socket->SendTo (packet, 0, InetSocketAddress (destination, SENKO_PORT));
      //if(id == 8)
      Simulator::Schedule (Jitter, &RoutingProtocol::SendTo,this, socket, packet,destination,
      hopcount, recvid);
     }
}
/*

void
RoutingProtocol::HelloNodeId(void)
{
 for (std::map<Ptr<Socket>, Ipv4InterfaceAddress>::const_iterator j = m_socketAddresses.begin (); j
       != m_socketAddresses.end (); ++j)
    {
      int32_t id = m_ipv4->GetObject<Node> ()->GetId ();//doinitialize  のIDと同じID
      if(mtran[id] == 1){//通信可能ノードのみ
      std::cout<< "HelloNodeId method id   " <<id<<"\n\n";
      int posx,posy;
     
      //位置情報を取得
      Ptr<MobilityModel> mobility = m_ipv4->GetObject<Node> () ->GetObject<MobilityModel>();
      Vector mypos = mobility->GetPosition ();
      posx = mypos.x;
      posy = mypos.y;

      Ptr<Socket> socket = j->first;
      Ipv4InterfaceAddress iface = j->second;
      Ptr<Packet> packet = Create<Packet> ();
    
      IdHeader idheader(id,posx,posy);
      packet->AddHeader (idheader);
      TypeHeader tHeader (SENKOTYPE_HELLOID);  
      packet->AddHeader (tHeader);
      
      // Send to all-hosts broadcast if on /32 addr, subnet-directed otherwise
      Ipv4Address destination;
      
      if (iface.GetMask () == Ipv4Mask::GetOnes ())
        {
          destination = Ipv4Address ("255.255.255.255");
        }
      else
        {
          destination = iface.GetBroadcast ();
        }
      std::cout<<Simulator::Now().GetMicroSeconds()<<"\n";  
      Time Jitter = Time (MicroSeconds (m_uniformRandomVariable->GetInteger (0,510)));
      //socket->SendTo (packet, 0, InetSocketAddress (destination, SENKO_PORT));
      Simulator::Schedule (Jitter, &RoutingProtocol::SendTo,this, socket, packet,destination);
     }
    }
}//Hellonodeid

*/

void
RoutingProtocol::SendTo(Ptr<Socket> socket, Ptr<Packet> packet, Ipv4Address destination,
int hopcount, int sendid){
  int32_t id = m_ipv4->GetObject<Node> ()->GetId ();//doinitialize  のIDと同じID
    if(hopcount == 1)
    {
      broadcount[0]++;
      std::cout<<"send broadcast id"<<id<<"send id"<<sendid<<"\n";
    }else
    {
      rebroadcount[0]++;
      std::cout<<"send rebroadcast id"<<id<<"send id"<<sendid<<"\n";
    }
    std::cout<<"  time  "<<Simulator::Now().GetMicroSeconds()<<"\n";
    socket->SendTo (packet, 0, InetSocketAddress (destination, SENKO_PORT));
}


void
RoutingProtocol::ReSendXBroadcast (int32_t id, int32_t posx ,
          int32_t posy, int8_t hopcount, int32_t time,int8_t danger,int sourcex, int sourcey)
{
 for (std::map<Ptr<Socket>, Ipv4InterfaceAddress>::const_iterator j = m_socketAddresses.begin (); j
       != m_socketAddresses.end (); ++j)
    {
      //int32_t broad_recv_id = m_ipv4->GetObject<Node> ()->GetId ();
      //std::cout<<" rebroadcast id"<<broad_recv_id<<"\n";
      //broadcast を受け取ったid
      //Ptr<MobilityModel> mobility = m_ipv4->GetObject<Node> () ->GetObject<MobilityModel>();
      //Vector mypos = mobility->GetPosition ();
      //double x = mypos.x; //現在の自分のx座標
      //double y = mypos.y; 
      //std::cout<<"id"<<broad_recv_id<<"xposition"<<x<<"send xposition"<<myposx<<"\n";


      Ptr<Socket> socket = j->first;
      Ipv4InterfaceAddress iface = j->second;
      Ptr<Packet> packet = Create<Packet> ();
      /////////////////////////////////////////////////////////////////////////////
      //dangerheader sendcode
      // std::cout<<"rebroadcast send"<<(unsigned)hopcount<<"\n";
      hopcount++; //ブロードキャストする前に受け取ったホップカウントに１足す
      DangerHeader dangerHeader(id, posx, posy, hopcount, time, danger,
      sourcex,sourcey);
      //ここで自分の位置情報と過去の位置情報をパケットにのせる
      packet->AddHeader (dangerHeader);  
      TypeHeader tHeader (SENKOTYPE_DANGER);  
      packet->AddHeader (tHeader);
      // Send to all-hosts broadcast if on /32 addr, subnet-directed otherwise
      Ipv4Address destination;
      
      if (iface.GetMask () == Ipv4Mask::GetOnes ())
        {
          destination = Ipv4Address ("255.255.255.255");
        }
      else
        {
          destination = iface.GetBroadcast ();
        }
      std::cout<<Simulator::Now().GetMicroSeconds()<<"\n";  
      //rebroadcount[0]++;
      //Time Jitter = Time (MicroSeconds (m_uniformRandomVariable->GetInteger (0,510)));
      Time Jitter = Time (MicroSeconds (m_uniformRandomVariable->GetInteger (0,150)));
      Simulator::Schedule (Jitter, &RoutingProtocol::SendTo,this, socket, packet,destination,
      hopcount,id);
      }
}//resendxbroadcast


//////////////////////////////////////////////////////////////////////////////mapmethod
void
RoutingProtocol::SaveXMap(int mapid, int mapxpoint)
{
 
mxpoint[mapid] = mapxpoint;
if(mxpoint.find(1) == mxpoint.end()){
  //std::cout<<"not found"<<std::endl;
}else{
  //std::cout<<"found"<<std::endl;
}


}//savemap

int
RoutingProtocol::GetXMap(int getid)
{
  return mxpoint[getid];
}

void
RoutingProtocol::SaveYMap(int mapid, int mapypoint)
{
 
mypoint[mapid] = mapypoint;
if(mypoint.find(1) == mypoint.end()){
  //std::cout<<"not found"<<std::endl;
}else{
 //std::cout<<"found"<<std::endl;
}


}//savemap

int
RoutingProtocol::GetYMap(int getid)
{
  return mypoint[getid];
}

void
RoutingProtocol::SaveTMap(int mapid, int maptime)
{
 
mtime[mapid] = maptime;
if(mtime.find(1) == mtime.end()){
  //std::cout<<"not found"<<std::endl;
}else{
  //std::cout<<"found"<<std::endl;
}


}//savemap

int
RoutingProtocol::GetTMap(int getid)
{
  return mtime[getid];
}

void
RoutingProtocol::SaveDMap(int mapid, int mapdanger)
{
 
mdanger[mapid] = mapdanger;
if(mdanger.find(1) == mdanger.end()){
  //std::cout<<"not found"<<std::endl;
}else{
  //std::cout<<"found"<<std::endl;
}


}//savemap

int
RoutingProtocol::GetDMap(int getid)
{
  return mdanger[getid];
}



////////////////////////////////////////////////////////mapmethod
/*
int GetSpeed(int newx,int newy, int newtime, int x,int y, int time){

  double dist = std::sqrt((newx - x) * (newx - x) + (newy - y) * (newy - y));
  double timelag = newtime -  time;
  double speed = dist/timelag;
  return speed;
}
*/
bool has_key_using_count(std::map<int,int> &mxpoint,int n)
{
  if (mxpoint.count(n) == 0){
        std::cout << "m doesn't have " << n << "." << std::endl;
        return false;
    }
    else{
        std::cout << "m has " << n << "." <<std::endl;
        return true;
    }
}
//// ノードが自分の速度を自分で図る　ｘ秒ごとの自分のポジションを保存

void
RoutingProtocol::SetMyPos()//自分の位置を取得してマップに保存
{
  int id = m_ipv4->GetObject<Node> ()->GetId ();
  int myposx,myposy;
  Ptr<MobilityModel> mobility = m_ipv4->GetObject<Node> () ->GetObject<MobilityModel>();
   Vector mypos = mobility->GetPosition ();
   myposx = mypos.x;//現在の自分の座標
   myposy = mypos.y;
   //std::cout<<"trans"<<trans[id]<<"\n";
  if(trans[id] != 2){//目的地に到着してなかったら

  if(myxpoint.count(id) == 0){//すでに自分の前の位置がセットされていないとき
   myxpoint[id] = myposx;
   myypoint[id] = myposy;
  }else{//前の座標のデータがあるとき
  mypastxpoint[id] = myxpoint[id];//過去のマップに残ってる座標を過去用のマップに保存
  mypastypoint[id] = myypoint[id];

double mydist =0;
    mydist = std::sqrt((myxpoint[id] - myposx) * (myxpoint[id] - myposx) 
        + (myypoint[id] - myposy) * (myypoint[id] - myposy));//ｘ秒前の位置と現在の自分の位置の距離
        ////////////////////////////////////x=1のときmydistは秒速に値する
        //std::cout<<"１秒前からの移動"<<mydist<<"\n";
        double myspeed = mydist*60*60/1000;
        if(myspeed > DangerSpeed){
          //std::cout<<"速い速度で走っています。車両のIDは"<<id<<"その速度は"<<myspeed<<"\n";
          vehiclecount[id]++;
        }

        if(mydist > 0 && trans[id] == 0  ){//車両が動き出したら　
          //trans[id]=1;//通信可能にする
          myposcount[id]++;
        }

        if(myposcount[id] > transTime && trans[id] == 0)
        {
          trans[id] = 1; //通信可能にする
          std::cout<<"通信可能になりましたid="<<id<<"trans[id]"<<trans[id]<<"\n";
          myposcount[id] = 0;
        }

        if(mydist == 0 && trans[id] == 1)//通信可能車両が止まっていたら
        {
          myposcount[id]++;
        }
        //if(mydist > 0 && myposcount[id] > 0)//何秒か止まっていたのに再び動き出したら
        //{
          //myposcount[id]=0;//止まっていた時間を初期化する
        //}
        //時速に変換
        if(myposcount[id] > NotransTime && trans[id] == 1){//通信可能ノードが一定期間停止
          trans[id]=2;
          std::cout<<"通信不可能になりました"<<id<<"\n";
        }
   m_myspeed[id] =  myspeed;//自分の速度をマップに保存
   myxpoint[id] = myposx;//位置情報をマップに保存
   myypoint[id] = myposy;
   //if(id == 10)
   //std::cout<<"myxpoint"<<mypoint[id]<<"mypastxpoint"<<mypastxpoint[id]<<"\n";
  }

  }//目的地に到着してなかったら

}

void
RoutingProtocol::GetAngle(double x, double y, double x2, double y2, double recvid)
{

  double radian = atan2(y2 - y,x2 - x);
  double angle = radian * 180 / 3.14159265;
  //int32_t id = m_ipv4->GetObject<Node> ()->GetId ();
 //std::cout<<"画像処理しているノードのIDは"<<id<<"\n";
  //std::cout<<"角度が求まりました　角度="<<angle<<"\n";
  //imageangleに角度を保存 keyはRecvid
  imageangle[recvid] = angle;
}

void
RoutingProtocol::EraseAngle()
{
  //std::cout<<"EraseAngle\n";
  areaangle.clear();
  //
}

double
RoutingProtocol::GetDist(double x, double y, double x2, double y2)
{
  double dist = std::sqrt((x2 - x) * (x2 - x) 
        + (y2 - y) * (y2 - y));
        return dist;
}

void
RoutingProtocol::GetGod() //画像処理内にいるノードだけをmapに保存
{
  int32_t id = m_ipv4->GetObject<Node> ()->GetId ();
  Ptr<MobilityModel> mobility = m_ipv4->GetObject<Node> () ->GetObject<MobilityModel>();
      Vector mypos = mobility->GetPosition ();
      double x = mypos.x; 
      double y = mypos.y; 
  for(int i=0; i < NodeNum; i++){
    double dist = std::sqrt( (x - GodXposition[i])*(x - GodXposition[i])
    +(y - GodYposition[i])*(y - GodYposition[i]) );
    if(id == 300){
      std::cout<<" get god id = "<<id<<"と id = "<<i<<",dist="<<dist<<"\n";
    }
  }
}

void
RoutingProtocol::GetAreaGod()//ループ発生関数
{
  int id = m_ipv4->GetObject<Node> ()->GetId ();
  Ptr<MobilityModel> mobility = m_ipv4->GetObject<Node> () ->GetObject<MobilityModel>();
      Vector mypos = mobility->GetPosition ();
      double x = mypos.x; //現在の自分のx座標
      double y = mypos.y; 
      double dist;
      //std::cout<<"--------------画像処理ノードはid="<<id<<"xpoint="<<x<<"ypoint"<<y<<"\n\n";
      //std::cout<<Simulator::Now().GetMicroSeconds()<<"\n";  
      //std::cout<<"１秒前の自分のx座標の移動距離は"<<x-myxpoint[id];
      //現在の自分の座標と１秒前の自分の座標

  //int32_t id = m_ipv4->GetObject<Node> ()->GetId ();
 //std::cout<<"画像処理しているノードのIDは"<<id<<"\n";
  //std::cout<<"角度が求まりました　角度="<<angle<<"\n";
  //imageangleに角度を保存 keyはRecvid

  for(int i=0; i<NodeNum; i++){
    if(id != i){
      dist = std::sqrt((GodXposition[i] - x) * (GodXposition[i] - x) + 
        (GodYposition[i] - y) * (GodYposition[i] - y));
      //std::cout<<"id ="<<id<<"と　id = "<<i<<"との距離は"<<dist<<"\n";
      //std::cout<<"id ="<<i<<"のx座標は"<<GodXposition[i]<<"y座標は"<<GodYposition[i]<<"\n";

      double radian = atan2(GodYposition[i] - y,GodXposition[i] - x);
      double angle = radian * 180 / 3.14159265;
      //int32_t id = m_ipv4->GetObject<Node> ()->GetId ();
      //std::cout<<"画像処理しているノードのIDは"<<id<<"\n";
      //std::cout<<"角度が求まりました　角度="<<angle<<"\n";
      //imageangleに角度を保存 keyはRecvid
      if(dist < ImageRange)
      {
        if(trans[id] == 1 && trans[i] == 1 )//画像処理ノードとそれに写ってるノードが通信可能のとき
        {
          areaangle[i] = angle;
          areadist[i] = dist;
        }
        
        //std::cout<<"画像処理内に存在する id ="<<i<<"で角度は "<<angle<<"\n";
      }
    }
  }
  ////////////円を
  ////////////2 1
  ////////////3 4の領域に分ける   1-2○　3-4○　1-3と2-4は差を取る必要なし 1-4○ 2-3だけ差をそのままでは取れない
//std::cout<<"size"<<areaangle.size()<<"\n";
int areasize = areaangle.size();
  for(auto itr = areaangle.begin(); itr != areaangle.end(); itr++) {
    
    if(areasize>3)//ループ簡易措置
{
  break;
}
       // std::cout << "画像処理内にあるID = " << itr->first           // キーを表示
                       // << ", 角度 = " << itr->second << "\n";    // 値を表示
            for(auto itr2 = areaangle.begin(); itr2 != areaangle.end(); itr2++)//S2重ループ
            {

              if(itr->first != itr2->first)//同じIDは比較しない
              {
                double dif;
                if(itr->second*itr2->second>=0)//符号が同じ場合 または角度0がいる場合 1-2 or 3-4
                {
                  dif = itr->second - itr2->second;
                  if(dif < 0)//引いてマイナスの場合プラスに変換
                    dif=dif*(-1.0);
                }else //S符号が違ったら
                {
                 if(x > GodXposition[itr->first] && x>GodXposition[itr2->first])
                 //比較対象が2と3の領域にいる場合
                 {
                   if(GodXposition[itr->first] >= y)//itr->firstが3にいるとき itr2は2にいる
                   {
                     dif = (itr2->second + 360) - itr->second;
                     if(dif < 0)//引いてマイナスの場合プラスに変換
                      dif=dif*(-1.0);
                   }else{//itr2が3にいる
                     dif = (itr->second + 360) - itr2->second;
                     if(dif < 0)//引いてマイナスの場合プラスに変換
                      dif=dif*(-1.0);
                   }
                 }else{//比較対象が符号が違うが2と3の比較にならない場合
                   dif = itr->second - itr2->second;
                  if(dif < 0)//引いてマイナスの場合プラスに変換
                    dif=dif*(-1.0);
                 }
                 //std::cout<<"broadcast reveive source yposition"<< pastmyposx<<"\n\n";
        //std::cout<<"broadcast reveive source yposition"<< pastmyposy<<"\n\n";
                }//F符号が違ったら

                if(dif < CancelAngle)//角度の差がしきい値より小さければ
                {
                  double distdif;
                  distdif = areadist[itr->first] - areadist[itr2->first];
                  //std::cout<<"id "<<itr->first<<"とid "<<itr2->first<<"の２つのノードの角度が近い\n";
                  if(distdif > 0)//itr->first のIDとを持つノードのほうが距離が長い
                  {
                    areaangle.erase(itr->first);
                    areadist.erase(itr->first);
                    //std::cout<<"id "<<itr->first<<"は実際には見えないので削除します\n";
                  }else{
                    areaangle.erase(itr2->first);
                    areadist.erase(itr2->first);
                    //std::cout<<"id "<<itr2->first<<"は実際には見えないので削除します\n";
                  }
                }

              }//F同じIDは比較しない
            }//F２重ループ
    }
    for(auto itr = areaangle.begin(); itr != areaangle.end(); itr++) {
        //std::cout << "id"<<id<<"の画像処理内にあるID = " << itr->first           // キーを表示
                       // << ", 角度 = " << itr->second << "\n";    // 値を表示
                        //x秒ごとに取得した近隣車両のIDと角度
      //画像処理内にいるIDを回している
      int32_t time = Simulator::Now().GetMicroSeconds();
      


      /////////////////////////////////////////////////////////////////////////////////
      //画像処理内にいるIDをブロードキャスト
      ////////////////////////////////////////////////////////////////////////////////
      if(Resendbroad == 1 && trans[id] == 1){//通信可能ノードのみ
       if(sendid[itr->first]  == 0)//おなじIDの車両のブロードキャストは二度行わない
       {
        // if(mxpoint.count(recvid) == 0){//受け取ったIDを持っているかどうか
          //std::cout<<"そのIDのマップを持っていないのでセットします\n";
        
          //std::cout<<"画像処理のブロードキャスト\n";
          if(mxpoint.count(itr->first) == 0 && trans[itr->first] == 1 ){//すでにもっていたIDじゃなかったら
            SendXBroadcast(itr->first, GodXposition[itr->first], GodYposition[itr->first],
          1,time, 0,(int)x,(int)y);
          sendid[itr->first]=1;//すでに送ったIDのマップを１に変換
          }else{
           // std::cout<<"すでに受け取ったIDなのでブロードキャストしません image id"<<id<<"speed vehicle id"<<itr->first<<"\n";
          }
        
       }
       //////////////////////////////////////////////////////////////////////////////////
       //危険車両との遭遇
       ////////////////////////////////////////////////////////////////
       auto itr3 = mxpoint.find(itr->first);
       if(itr3 != mxpoint.end()){//通信で受け取った違反車両IDが画像処理内にあったら
       //std::cout<<"違反車両の疑いがありますその車両のIDは"<<itr->first
       //<<"見つけた車両は"<<id<<"\n";
       double dist2 = GetDist(mxpoint[itr->first],mypoint[itr->first],
       GodXposition[itr->first],GodYposition[itr->first]);
       if(dist < Dmin)//速度判定には距離が足りない場合
       {
         std::cout<<"速度を図るには距離が足りません id"<<id<<"dist"<<dist<<"\n";
       }else{//速度判定するのに十分な距離があるとき
       int32_t time = Simulator::Now().GetMicroSeconds(); //速度判定対象車両を見つけた時間
         double timelag = (double)time - (double)mtime[itr->first];
         timelag = timelag/1000000; //microをsecondに変換
         timelag = timelag/3600;//秒を時間を変換
         dist2 = dist2/1000; //m -> km
         double speed = dist2/(double)timelag;
         if(sendid[itr->first]  == 0){
           if(speed > Vmin && trans[itr->first] == 1 )//違反車両だったら
         {
           std::cout<<"速度違反車両がいました　IDは"<<itr->first<<"speed"<<speed<<"detect veihcle id"<<id<<"\n";
           mdanger[itr->first]++;
           SendXBroadcast(itr->first, GodXposition[itr->first],GodYposition[itr->first],
           1, time,mdanger[itr->first], (int)x, (int)y);
           sendid[itr->first]=1;//すでに送ったIDのマップを１に変換

           if(detectdanger[itr->first] == 0)
              {
                  detectdanger[itr->first] = mdanger[itr->first];
              }else if(detectdanger[itr->first] < mdanger[itr->first])//警戒地よりdetectdangerが小さければ値を更新する
              {
                  detectdanger[itr->first] = mdanger[itr->first];
                }
         }else{//違反してなかったら
         //if(itr->first == 30)
           //std::cout<<"速度違反してませんでしたspeed"<<speed<<"id"<<itr->first<<"\n";
           //std::cout<<"aaaaaaaaa\n";
         }
         }
         
       }
       //std::cout<<"aaaaaaaaa\n";
       }
      }


  }//for(auto itr = areaangle.begin(); itr != areaangle.end(); ++itr) {


}//std::cout<<"getareagod の終わり\n";

void
RoutingProtocol::Direction(double current_x, double current_y,
double source_x, double source_y)
{
  //int32_t id = m_ipv4->GetObject<Node> ()->GetId ();
  //double past_x = GodXposition[id];
  //double past_y = GodYposition[id];
  //std::cout<<"direction"<<"\n";
}

void
RoutingProtocol::PrintTime()
{
   std::cout<<Simulator::Now().GetMicroSeconds()<<"\n";//このパケットを受け取った時間 
   if( Simulator::Now().GetSeconds() == SimTime - 1)
   {
     std::cout<<"broadcast の回数"<<broadcount[0]<<"\n";
     std::cout<<"rebroadcast の回数"<<rebroadcount[0]<<"\n";
     for(int i=0;i<NodeNum+1;i++)
     {
       if(vehiclecount[i] > 40)
       {
         std::cout<<"id"<<i<<"違反回数"<<vehiclecount[i]<<"\n\n";
       }
       if(detectdanger[i] > 0)
       {
         std::cout<<"id"<<i<<"違反を検知して加算した警戒値は"<<detectdanger[i]<<"\n\n";
       }
     }
   }
}


std::vector<double> RoutingProtocol::GodXposition;
std::vector<double> RoutingProtocol::GodYposition;
std::vector<int> RoutingProtocol::trans;
std::vector<int> RoutingProtocol::broadcount;
std::vector<int> RoutingProtocol::rebroadcount;
std::vector<int> RoutingProtocol::vehiclecount;
std::vector<int> RoutingProtocol::detectdanger;


} //namespace senko
} //namespace ns3
