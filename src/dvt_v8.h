#ifndef __DVT_V8_H__
#define __DVT_V8_H__
#include <v8.h>
#include "dvt_packet.h"

using namespace v8;
namespace divert{
Handle<ObjectTemplate> dvt_script_global_funcs();


enum {
  PACKET_VERDICT_PASS,
  PACKET_VERDICT_DROP
};

struct ScriptPacket {
  ScriptPacket():ip(NULL),tcp(NULL),udp(NULL),device(NULL),verdict(PACKET_VERDICT_PASS){}
  IPPacket *ip;
  TCPPacket *tcp;
  UDPPacket *udp;
  const char *device;
  int verdict;
};

class ScriptEngine{
  public:
    bool prepare();
    bool parseAndRun(const char *file);
    bool runNoArgs(const char *name);
    bool setCallback(const char *name);
    bool runCallback(ScriptPacket *packet);
  private:
     Handle<Context> context_;
     Persistent<ObjectTemplate> global_;
     Persistent<Function> jscallback_;
     Persistent<ObjectTemplate> tpl_packet_;
};


};
#endif
