#include "dvt_loop.h"
#include "ev.c"

using namespace divert;

EventLoop::EventLoop(){
}

void EventLoop::stop(){
  loop_.unloop();
}

void EventLoop::start(){
  loop_.loop();
}

void EventLoop::once(){
  loop_.loop(ev::ONCE);
}

