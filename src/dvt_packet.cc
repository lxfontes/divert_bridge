#include "dvt_packet.h"
#include <iostream>
#include <string.h>
#include <stdio.h>
#include <arpa/inet.h>

namespace divert {
  void printMac(unsigned char *ptr){
    int i = ETHER_ADDR_LEN;
    do{
      printf("%s%02x",(i == ETHER_ADDR_LEN) ? " " : ":",*ptr++);
    }while(--i>0);
    printf("\n");
  }
  IPPacket::IPPacket():packet_size_(0),ip_data_size_(0) {
    ether_header_ = (struct ether_header *)buf_;
    ip_header_ = (struct ip *)(buf_ + ETHER_HDR_LEN);
  }

  void IPPacket::load(int size){
    packet_size_ = size;
    int tmp_size = htons(ip_header_->ip_len);
  }

  void IPPacket::macFlip(){
    unsigned char tmp[ETHER_ADDR_LEN];
    printf("Before\n");
    printMac(ether_header_->ether_shost);
    printMac(ether_header_->ether_dhost);
    memcpy(tmp,ether_header_->ether_shost,ETHER_ADDR_LEN);
    memcpy(ether_header_->ether_shost,ether_header_->ether_dhost,ETHER_ADDR_LEN);
    memcpy(ether_header_->ether_dhost,tmp,ETHER_ADDR_LEN);
    printf("After\n");
    printMac(ether_header_->ether_shost);
    printMac(ether_header_->ether_dhost);
  }

  int IPPacket::getPacketType(){
    uint16_t pkt_type = ntohs(ether_header_->ether_type);

    if(pkt_type != ETHERTYPE_IP)
      return PACKET_TYPE_NONE;


    if(ip_header_->ip_p == IPPROTO_TCP){
      return PACKET_TYPE_TCP;
    }else if(ip_header_->ip_p == IPPROTO_UDP){
      return PACKET_TYPE_UDP;
    }

      return PACKET_TYPE_NONE;
    }

  int IPPacket::l3offset(){
    return ( ETHER_HDR_LEN + (ip_header_->ip_hl * 4) );
  }


  TCPPacket::TCPPacket(IPPacket *ip){
    unsigned char *buf = ip->buf();
    struct ip *iph = ip->ipHeader();
    uint32_t data_pos;
    tcp_header_ = reinterpret_cast<struct tcphdr *>(buf + ip->l3offset());
    tcp_data_size_ = ntohs(iph->ip_len) - IP_LEN - (tcp_header_->th_off * 4);
    data_pos = ip->l3offset() + (tcp_header_->th_off * 4);
    tcp_data_ = buf + data_pos;
  }

  UDPPacket::UDPPacket(IPPacket *ip){
    unsigned char *buf = ip->buf();
    struct ip *iph = ip->ipHeader();
    uint32_t data_pos;
    udp_header_ = reinterpret_cast<struct udphdr *>(buf + ip->l3offset());
    udp_data_size_ = ntohs(iph->ip_len) - IP_LEN - ntohs(udp_header_->uh_ulen);
    data_pos = ip->l3offset() + sizeof(struct udphdr);
    udp_data_ = buf + data_pos;
  }

};
