#ifndef __DVT_DEVICE_H__
#define __DVT_DEVICE_H__
#include <string>


namespace divert {
  class Device {
    public:
      bool prepare(const char *name);
      bool open();
      void close();
      int fd(){return fd_;}
      const char *name(){return name_.c_str();}
    private:
      std::string name_;
      int fd_;
  };
};

#endif
