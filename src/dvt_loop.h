#ifndef __DVT_LOOP_H__
#define __DVT_LOOP_H__

#include <ev++.h>



namespace divert {
  enum {
    EVENT_READ,
    EVENT_WRITE
  };
  class EventLoop;

  class Event{
    public:
      virtual void setLoop(ev::dynamic_loop &loop) = 0;
  };


  template<class K,void (K::*method)(int)>
    void eventfile_cb(ev::io &w,int rv){
      (static_cast<K *>(w.data)->*method)(w.fd);
    }


  template<class K,void (K::*method)()>
    void eventtimer_cb(ev::timer &w,int rv){
      (static_cast<K *>(w.data)->*method)();
    }

  class EventTimer: public Event{
    public:
      virtual void setLoop(ev::dynamic_loop &loop){
        timer_.set(loop);
      }

      template<class K,void (K::*method)()>
      void bind(K *object){
            timer_.set< &eventtimer_cb<K,method> >(object);
      }

      void start(double timeout,double repeat){
        timer_.start(timeout,repeat);
      }

    private:
      ev::timer timer_;
  };

  class EventFile: public Event{
    public:
      virtual void setLoop(ev::dynamic_loop &loop){
        ior_.set(loop);
        iow_.set(loop);
      }

      void setFileDescriptor(int fd){
        fd_ = fd;
      }

      template<class K,void (K::*method)(int)>
      void bind(int event, K *object){
          if(event == EVENT_READ){
            ior_.set< &eventfile_cb<K,method> >(object);
          }else if(event == EVENT_WRITE){
            iow_.set< &eventfile_cb<K,method> >(object);
          }
      }

      void start(int event){
        if(event == EVENT_READ){
          ior_.start(fd_,ev::READ);
        }else if(event == EVENT_WRITE){
          iow_.start(fd_,ev::WRITE);
        }
      }


    private:
      ev::io ior_; //read
      int fd_;
      ev::io iow_; //write
  };

  class EventLoop{
    public:
      EventLoop();
      void stop();
      void start();
      void once();
      EventFile *newEventFile(){
        EventFile *f = new EventFile;
        add(f);
        return f;
      }
      EventTimer *newEventTimer(){
        EventTimer *f = new EventTimer;
        add(f);
        return f;
      }
    private:
      void add(Event *event_producer){
        event_producer->setLoop(loop_);
      }
      ev::dynamic_loop loop_;
  };
};

#endif
