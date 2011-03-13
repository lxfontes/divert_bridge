#include "dvt_loop.h"
#include "dvt_device.h"
#include "dvt_v8.h"
#include "dvt_packet.h"
#include "Parser.h"
#include <iostream>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>


class Bridge{
  public:
    Bridge(divert::EventLoop *loop):loop_(loop),timer_cb_name_(""){
      engine_.prepare();
      ev_timer_ = loop_->newEventTimer();
      ev_timer_->bind<Bridge,&Bridge::timer_cb>(this);
      ev_timer_->start(1.0,1.0); //every second
    }

    bool singleDevice(const char *inout){
      dev_in_.prepare(inout);
      if(dev_in_.open() == false){
        return false;
      }
      ev_in_ = loop_->newEventFile();
      ev_in_->setFileDescriptor(dev_in_.fd());
      ev_in_->bind<Bridge,&Bridge::read_single>(divert::EVENT_READ,this);
      ev_in_->start(divert::EVENT_READ);
    }
    bool dualDevice(const char *in,const char *out){
      dev_in_.prepare(in);
      dev_out_.prepare(out);
      if(dev_in_.open() == false){
        return false;
      }else if(dev_out_.open() == false){
        dev_in_.close();
        return false;
      }
      ev_in_ = loop_->newEventFile();
      ev_in_->setFileDescriptor(dev_in_.fd());
      ev_in_->bind<Bridge,&Bridge::read_in>(divert::EVENT_READ,this);
      ev_in_->start(divert::EVENT_READ);

      ev_out_ = loop_->newEventFile();
      ev_out_->setFileDescriptor(dev_out_.fd());
      ev_out_->bind<Bridge,&Bridge::read_out>(divert::EVENT_READ,this);
      ev_out_->start(divert::EVENT_READ);

      return true;
    }

    void read_in(int fd){
      bridge(&dev_in_,&dev_out_);
    }

    void read_out(int fd){
      bridge(&dev_out_,&dev_in_);
    }
    void read_single(int fd){
      bridge(&dev_in_,&dev_in_);
    }

    void timer_cb(){
      if(timer_cb_name_.size() > 0)
        engine_.runNoArgs(timer_cb_name_.c_str());
    }

    void bridge(divert::Device *dev_in,divert::Device *dev_out){
      divert::IPPacket packet;
      int n;
      n = read(dev_in->fd(),packet.buf(),DIVERT_MAX_MTU);
      if(n <= 0){
      std::cout << "IN bad" << std::endl;
        //something bad happened
        return;
      }

      packet.load(n);
      int pkt_type = packet.getPacketType();
      divert::ScriptPacket spacket;
      spacket.ip = &packet;
      spacket.device = dev_in->name();
      if(pkt_type == divert::PACKET_TYPE_TCP){
        divert::TCPPacket tcp(&packet);
        spacket.tcp = &tcp;
        engine_.runCallback(&spacket);
      }else if(pkt_type == divert::PACKET_TYPE_UDP){
        divert::UDPPacket udp(&packet);
        spacket.udp = &udp;
        engine_.runCallback(&spacket);
      }else{
        //some other random packet, icmp or whatever
        engine_.runCallback(&spacket);
      }
      if(spacket.verdict == divert::PACKET_VERDICT_PASS){
        write(dev_out->fd(),packet.buf(),n);
      }
    }

    bool parseAndRun(const char *scr){
      return engine_.parseAndRun(scr);
    }
    bool setCallback(const char *name){
      return engine_.setCallback(name);
    }
    void setTimerCallback(const char *name){
      timer_cb_name_ = name;
    }

  private:
    divert::EventLoop *loop_;
    divert::EventFile *ev_in_;
    divert::EventFile *ev_out_;
    divert::EventTimer *ev_timer_;
    divert::Device dev_in_;
    divert::Device dev_out_;
    divert::ScriptEngine engine_;
    std::string timer_cb_name_;
};

int main(int argc,char **argv){
  divert::EventLoop loop;
  StringOption jsfile ('s', "script", true , "javascript logic file");
  StringOption jscallback ('c', "callback", true , "javascript callback (receive packet,interface_name)");
  StringOption js_timer_callback ('t', "timer_callback", false, "javascript callback called every second");
  StringOption tap_one ('i', "interface_client", true , "tap device for client side (ex: tap0)");
  StringOption tap_two ('I', "interface_server", false, "tap device for server side (ex: tap1)");

  Parser parser;

  parser.addOption(jsfile)
    .addOption(jscallback)
    .addOption(js_timer_callback)
    .addOption(tap_one)
    .addOption(tap_two);

  std::vector<std::string> otherArguments = parser.parse(argc, argv);

  Bridge bridge(&loop);
  if(tap_two.getValue().size() > 0){
    bridge.dualDevice(tap_one.getValue().c_str(),tap_two.getValue().c_str());
  }else{
    bridge.singleDevice(tap_one.getValue().c_str());
  }
  bridge.parseAndRun(jsfile.getValue().c_str());
  bridge.setCallback(jscallback.getValue().c_str());
  if(js_timer_callback.getValue().size() > 0){
    bridge.setTimerCallback(js_timer_callback.getValue().c_str());
  }
  loop.start();
  return(0);
}
