#ifndef __DVT_PACKET_H__
#define __DVT_PACKET_H__

#define DIVERT_MAX_MTU 1980

#include <stdint.h>
#define __FAVOR_BSD
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/ether.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>


namespace divert{
  enum {
    IP_LEN = 20,
    MAC_LEN = 6,
    PACKET_TYPE_TCP = 1,
    PACKET_TYPE_UDP = 2,
    PACKET_TYPE_NONE = 3
  };
  class IPPacket {
    public:
      IPPacket();
      unsigned char *buf(){return buf_;}
      int bufSize(){ return packet_size_; }
      void load(int size);
      struct ip *ipHeader(){return ip_header_;}
      struct ether_header *ethHeader(){return ether_header_;}
      int getPacketType();
      int l3offset();
      void macFlip();
    private:
      unsigned char buf_[DIVERT_MAX_MTU];
      uint32_t ip_data_size_;
      uint32_t packet_size_;
      char dst_ip_[16];
      char src_ip_[16];
      struct ip *ip_header_;
      struct ether_header *ether_header_;
      unsigned char *ip_data_;
  };

  class TCPPacket{
    public:
      TCPPacket(IPPacket *ip);
      struct tcphdr *tcpHeader(){return tcp_header_;}
      unsigned char *data(){return tcp_data_;}
      uint32_t dataSize(){return tcp_data_size_;}
    private:
      struct tcphdr *tcp_header_;
      unsigned char *tcp_data_;
      uint32_t tcp_data_size_;
  };


  class UDPPacket {
    public:
      UDPPacket(IPPacket *ip);
      struct udphdr *udpHeader();
      unsigned char *data(){return udp_data_;}
      uint32_t dataSize(){return udp_data_size_;}
    private:
      struct udphdr *udp_header_;
      unsigned char *udp_data_;
      uint32_t udp_data_size_;
  };

};
#endif
