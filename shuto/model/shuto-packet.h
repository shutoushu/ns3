/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2018 Alberto Gallegos
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
 * Author: Alberto Gallegos <ramonet@fc.ritsumei.ac.jp>
 *         Ritsumeikan University, Shiga, Japan
 */
#ifndef SHUTOPACKET_H
#define SHUTOPACKET_H

#include <iostream>
#include "ns3/header.h"
#include "ns3/enum.h"
#include "ns3/ipv4-address.h"
#include <map>
#include "ns3/nstime.h"

namespace ns3 {
namespace shuto {

/**
* \ingroup shuto
* \brief MessageType enumeration
*/
enum MessageType
{
  //SHUTOTYPE_RREQ  = 1,   //!< SHUTOTYPE_RREQ
  //SHUTOTYPE_RREP  = 2,   //!< SHUTOTYPE_RREP
  //SHUTOTYPE_RERR  = 3,   //!< SHUTOTYPE_RERR
  //SHUTOTYPE_RREP_ACK = 4, //!< SHUTOTYPE_RREP_ACK
  SHUTOTYPE_DANGER = 5,
  SHUTOTYPE_HELLOID = 6
};

/**
* \ingroup shuto
* \brief SHUTO types
*/
class TypeHeader : public Header
{
public:
  /**
   * constructor
   * \param t the SHUTO RREQ type
   */
  //TypeHeader (MessageType t = SHUTOTYPE_RREQ);
  //TypeHeader (MessageType t = SHUTOTYPE_DANGER);
  TypeHeader (MessageType t = SHUTOTYPE_HELLOID);
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId ();
  TypeId GetInstanceTypeId () const;
  uint32_t GetSerializedSize () const;
  void Serialize (Buffer::Iterator start) const;
  uint32_t Deserialize (Buffer::Iterator start);
  void Print (std::ostream &os) const;

  /**
   * \returns the type
   */
  MessageType Get () const
  {
    return m_type;
  }
  /**
   * Check that type if valid
   * \returns true if the type is valid
   */
  bool IsValid () const
  {
    return m_valid;
  }
  /**
   * \brief Comparison operator
   * \param o header to compare
   * \return true if the headers are equal
   */
  bool operator== (TypeHeader const & o) const;
private:
  MessageType m_type; ///< type of the message
  bool m_valid; ///< Indicates if the message is valid
};

/**
  * \brief Stream output operator
  * \param os output stream
  * \return updated stream
  */
std::ostream & operator<< (std::ostream & os, TypeHeader const & h);

////////////////////////////////////////////////////////////////////////////////////////////////////
////id
/*
* \ingroup shuto
* \brief idheader
  \verbatim
  0                   1                   2                   3
  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |     Type      |R|A|    Reserved     |Prefix Sz|   Hop Count   |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |                     Destination IP address                    |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |                  Destination Sequence Number                  |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |                    Originator IP address                      |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |                           Lifetime                            |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  \endverbatim
*/

class IdHeader : public Header
{
public:
  /**
   * constructor
   *
   * \param id  
   */

  IdHeader (uint32_t id = 0 ,uint32_t xpoint = 0, uint32_t ypoint = 0);
  //IdHeader ();
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId ();
  TypeId GetInstanceTypeId () const;
  uint32_t GetSerializedSize () const;
  void Serialize (Buffer::Iterator start) const;
  uint32_t Deserialize (Buffer::Iterator start);
  void Print (std::ostream &os) const;

  
  void SetHelloId (uint32_t id)
  {
    m_helloid = id;
  }
  uint32_t GetHelloId () const
  {
    return m_helloid;
  }

  void SetHelloXpoint (uint32_t xpoint)
  {
    m_helloxpoint = xpoint;
  }
  uint32_t GetHelloXpoint () const
  {
    return m_helloxpoint;
  }

  void SetHelloYpoint (uint32_t ypoint)
  {
    m_helloypoint = ypoint;
  }
  uint32_t GetHelloYpoint () const
  {
    return m_helloypoint;
  }


bool operator== (IdHeader const & o) const;
private:
  uint32_t m_helloid;
  uint32_t m_helloxpoint;
  uint32_t m_helloypoint;
};
//////////////////////////////////////////////////////////////////////////








///////////////////////////////////////////////////////////////////////////////////////////////
//danger
/**
* \ingroup shuto
* \brief dangerheader
  \verbatim
  0                   1                   2                   3
  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |     Type      |R|A|    Reserved     |Prefix Sz|   Hop Count   |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |                     Destination IP address                    |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |                  Destination Sequence Number                  |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |                    Originator IP address                      |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |                           Lifetime                            |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  \endverbatim
*/


class DangerHeader : public Header
{
public:
/*
nodeid 定期的なIDの伝搬に使うID
point IDを受け取った車両の位置情報
hopCount ブロードキャストされた回数
recvtime 車両がIDを受け取った時間
danger 　警戒値
*/
DangerHeader (uint32_t nodeid = 0, uint32_t posx = 0, 
uint32_t posy = 0, uint8_t hopcount = 0,uint32_t recvtime = 0, uint8_t danger = 0);

static TypeId GetTypeId ();
TypeId GetInstanceTypeId () const;
uint32_t GetSerializedSize () const;
void Serialize (Buffer::Iterator start) const;
uint32_t Deserialize (Buffer::Iterator start);
void Print (std::ostream &os) const;


void SetNodeId (uint32_t id)  //IDをセットする
  {
    m_nodeid = id;
  }

uint32_t GetNodeId ()  const//IDを返す
  {
    return m_nodeid;
  }
void SetPosX (uint32_t p)    //座標をセットする
  {  //座標をセットする
    m_posx = p;
  }

uint32_t GetPosX ()  //座標の値を返す
  {
    return m_posx;
  }
void SetPosY (uint32_t p)    //座標をセットする
  {  //座標をセットする
    m_posy = p;
  }

uint32_t GetPosY()  //座標の値を返す
  {
    return m_posy;
  }
  void SetHopCount (uint8_t count) //hopcountを足していく関数
  {
    m_hopcount = count;
  }

uint8_t GetHopCount () const  //hopcount の値を返す
  {
    return m_hopcount;
  }

void SetDanger (uint8_t dan) //警戒地をセットする
  {
    m_danger = dan;
  }

uint8_t GetDanger ()  //警戒値を返す
  {
    return m_danger;
  }

void SetRecvTime (uint32_t recvtime)
{
  m_recvtime = recvtime;
}

//Time GetRecvTime () const;
uint32_t GetRecvTime() 
{
  return m_recvtime;
}
  bool operator== (DangerHeader const & o);
private:
  uint32_t     m_nodeid;    //ノードID
  uint32_t     m_posx;     //座標
  uint32_t     m_posy;
  uint8_t      m_hopcount;  //hop数
  uint32_t     m_recvtime;  //IDを受け取った時間
  uint8_t      m_danger;    //警戒値
};










































/**
* \ingroup shuto
* \brief Route Reply (RREP) Message Format
  \verbatim
  0                   1                   2                   3
  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |     Type      |R|A|    Reserved     |Prefix Sz|   Hop Count   |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |                     Destination IP address                    |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |                  Destination Sequence Number                  |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |                    Originator IP address                      |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |                           Lifetime                            |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  \endverbatim
*/
class RrepHeader : public Header
{
public:
  /**
   * constructor
   *
   * \param prefixSize the prefix size (0)
   * \param hopCount the hop count (0)
   * \param dst the destination IP address
   * \param dstSeqNo the destination sequence number
   * \param origin the origin IP address
   * \param lifetime the lifetime
   */
  RrepHeader (uint8_t prefixSize = 0, uint8_t hopCount = 0, Ipv4Address dst =
                Ipv4Address (), uint32_t dstSeqNo = 0, Ipv4Address origin =
                Ipv4Address (), Time lifetime = MilliSeconds (0));
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId ();
  TypeId GetInstanceTypeId () const;
  uint32_t GetSerializedSize () const;
  void Serialize (Buffer::Iterator start) const;
  uint32_t Deserialize (Buffer::Iterator start);
  void Print (std::ostream &os) const;

  // Fields
  /**
   * \brief Set the hop count
   * \param count the hop count
   */
  void SetHopCount (uint8_t count)
  {
    m_hopCount = count;
  }
  /**
   * \brief Get the hop count
   * \return the hop count
   */
  uint8_t GetHopCount () const
  {
    return m_hopCount;
  }
  /**
   * \brief Set the destination address
   * \param a the destination address
   */
  void SetDst (Ipv4Address a)
  {
    m_dst = a;
  }
  /**
   * \brief Get the destination address
   * \return the destination address
   */
  Ipv4Address GetDst () const
  {
    return m_dst;
  }
  /**
   * \brief Set the destination sequence number
   * \param s the destination sequence number
   */
  void SetDstSeqno (uint32_t s)
  {
    m_dstSeqNo = s;
  }
  /**
   * \brief Get the destination sequence number
   * \return the destination sequence number
   */
  uint32_t GetDstSeqno () const
  {
    return m_dstSeqNo;
  }
  /**
   * \brief Set the origin address
   * \param a the origin address
   */
  void SetOrigin (Ipv4Address a)
  {
    m_origin = a;
  }
  /**
   * \brief Get the origin address
   * \return the origin address
   */
  Ipv4Address GetOrigin () const
  {
    return m_origin;
  }
  /**
   * \brief Set the lifetime
   * \param t the lifetime
   */
  void SetLifeTime (Time t);
  /**
   * \brief Get the lifetime
   * \return the lifetime
   */
  Time GetLifeTime () const;

  // Flags
  /**
   * \brief Set the ack required flag
   * \param f the ack required flag
   */
  void SetAckRequired (bool f);
  /**
   * \brief get the ack required flag
   * \return the ack required flag
   */
  bool GetAckRequired () const;
  /**
   * \brief Set the prefix size
   * \param sz the prefix size
   */
  void SetPrefixSize (uint8_t sz);
  /**
   * \brief Set the pefix size
   * \return the prefix size
   */
  uint8_t GetPrefixSize () const;

  /**
   * Configure RREP to be a Hello message
   *
   * \param src the source IP address
   * \param srcSeqNo the source sequence number
   * \param lifetime the lifetime of the message
   */
  void SetHello (Ipv4Address src, uint32_t srcSeqNo, Time lifetime);

  /**
   * \brief Comparison operator
   * \param o RREP header to compare
   * \return true if the RREP headers are equal
   */
  bool operator== (RrepHeader const & o) const;
private:
  uint8_t       m_flags;                  ///< A - acknowledgment required flag
  uint8_t       m_prefixSize;         ///< Prefix Size
  uint8_t       m_hopCount;         ///< Hop Count
  Ipv4Address   m_dst;              ///< Destination IP Address
  uint32_t      m_dstSeqNo;         ///< Destination Sequence Number
  Ipv4Address     m_origin;           ///< Source IP Address
  uint32_t      m_lifeTime;         ///< Lifetime (in milliseconds)
};

/**
  * \brief Stream output operator
  * \param os output stream
  * \return updated stream
  */
std::ostream & operator<< (std::ostream & os, RrepHeader const &);
std::ostream & operator<< (std::ostream & os, DangerHeader const &);
std::ostream & operator<< (std::ostream & os, IdHeader const &);
//std::ostream & operator<< (std::ostream & os, IdHeader  &);





}  // namespace shuto
}  // namespace ns3

#endif /* SHUTOPACKET_H */
