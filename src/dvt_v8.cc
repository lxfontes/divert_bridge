#include "dvt_v8.h"
#include "dvt_packet.h"
#include <iostream>

using namespace v8;
using namespace std;
namespace divert {
  Handle<Value> dvt_packet_pass(const v8::Arguments& args) {
    Local<Object> self(args[0]->ToObject());
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();
    ScriptPacket *pkt = static_cast<ScriptPacket*>(ptr);
    pkt->verdict = PACKET_VERDICT_PASS;
    return v8::Boolean::New(true);
  }


  Handle<Value> dvt_packet_mac_flip(const v8::Arguments& args) {
    Local<Object> self(args[0]->ToObject());
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();
    ScriptPacket *pkt = static_cast<ScriptPacket*>(ptr);
    pkt->ip->macFlip();
    return v8::Boolean::New(true);
  }

  Handle<Value> dvt_packet_block(const v8::Arguments& args) {
    Local<Object> self(args[0]->ToObject());
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();
    ScriptPacket *pkt = static_cast<ScriptPacket*>(ptr);
    pkt->verdict = PACKET_VERDICT_DROP;
    return v8::Boolean::New(true);
  }

  Handle<Value> dvt_packet_inject(const v8::Arguments& args) {
  }

  Handle<Value> pkt_v8_type(Local<String> property,
      const AccessorInfo &info) {
    Local<Object> self = info.Holder();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();
    ScriptPacket *pkt = static_cast<ScriptPacket*>(ptr);

    int typ = pkt->ip->getPacketType();
    if (typ == PACKET_TYPE_TCP){
      return String::New("tcp");
    }else if(typ == PACKET_TYPE_UDP){
      return String::New("udp");
    }else {
      return String::New("unknown");
    }
  }


  Handle<Value> pkt_v8_raw(Local<String> property,
      const AccessorInfo &info) {
    Local<Object> self = info.Holder();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();
    ScriptPacket *pkt = static_cast<ScriptPacket*>(ptr);

    unsigned char *data = pkt->ip->buf();
    int bsize = pkt->ip->bufSize();
      return String::New((const char *)data,bsize);
  }


  Handle<Value> pkt_v8_tcp_raw(Local<String> property,
      const AccessorInfo &info) {
    Local<Object> self = info.Holder();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();
    ScriptPacket *pkt = static_cast<ScriptPacket*>(ptr);

    if(pkt->tcp != NULL){
      unsigned char *data = pkt->tcp->data();
      int bsize = pkt->tcp->dataSize();
      return String::New((const char *)data,bsize);
    }else{
      return v8::Undefined();
    }
  }



  Handle<Value> pkt_v8_udp_raw(Local<String> property,
      const AccessorInfo &info) {
    Local<Object> self = info.Holder();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();
    ScriptPacket *pkt = static_cast<ScriptPacket*>(ptr);

    if(pkt->udp != NULL){
      unsigned char *data = pkt->udp->data();
      int bsize = pkt->udp->dataSize();
      return String::New((const char *)data,bsize);
    }else{
      return v8::Undefined();
    }
  }

  Handle<ObjectTemplate> dvt_packet_template() {
    HandleScope handle_scope;
    Handle<ObjectTemplate> result = ObjectTemplate::New();
    result->SetInternalFieldCount(1);
    result->SetAccessor(String::New("pkt_type"), pkt_v8_type);
    result->SetAccessor(String::New("pkt_raw"), pkt_v8_raw);
    result->SetAccessor(String::New("tcp_data"), pkt_v8_tcp_raw);
    result->SetAccessor(String::New("udp_data"), pkt_v8_udp_raw);
    return handle_scope.Close(result);
  }



  Handle<Value> dvt_print(const v8::Arguments& args) {
    bool first = true;
    for (int i = 0; i < args.Length(); i++) {
      v8::HandleScope handle_scope;
      if (first) {
        first = false;
      } else {
        cout << " ";
      }
      v8::String::Utf8Value str(args[i]);
      cout << *str;
    }
    cout << endl;
    return v8::Undefined();
  }

  Handle<ObjectTemplate> dvt_script_global_funcs() {
    HandleScope handle_scope;

    Handle<ObjectTemplate> result = ObjectTemplate::New();
    result->Set(String::New("print"), FunctionTemplate::New(dvt_print));
    result->Set(String::New("divert_pass"), FunctionTemplate::New(dvt_packet_pass));
    result->Set(String::New("divert_block"), FunctionTemplate::New(dvt_packet_block));
    result->Set(String::New("divert_flip"), FunctionTemplate::New(dvt_packet_mac_flip));
//    result->Set(String::New("divert_inject"), FunctionTemplate::New(dvt_packet_inject));
    return handle_scope.Close(result);
  }


  v8::Handle<v8::String> dvt_script_read(const char* name) {
    FILE* file = fopen(name, "rb");
    if (file == NULL) return v8::Handle<v8::String > ();

    fseek(file, 0, SEEK_END);
    int size = ftell(file);
    rewind(file);

    char* chars = new char[size + 1];
    chars[size] = '\0';
    for (int i = 0; i < size;) {
      int read = fread(&chars[i], 1, size - i, file);
      i += read;
    }
    fclose(file);
    v8::Handle<v8::String> result = v8::String::New(chars, size);
    delete[] chars;
    return result;
  }

  bool ScriptEngine::prepare(){
    HandleScope handle_scope;
    Handle<ObjectTemplate> funcs = dvt_script_global_funcs();
    global_ = Persistent<ObjectTemplate>::New(funcs);
    Handle<Context> lcontext = Context::New(NULL, global_);
    context_ = Persistent<Context>::New(lcontext);
    tpl_packet_ = Persistent<ObjectTemplate>::New(dvt_packet_template());
    return true;
  }

  bool ScriptEngine::parseAndRun(const char *file){
    HandleScope handle_scope;
    Context::Scope context_scope(context_);
    Handle<String> v8script = dvt_script_read(file);
    Handle<Script> script = Script::Compile(v8script, String::New(file));
    if (script.IsEmpty()) {
      return false;
    }
    Handle<Value> result = script->Run();
    return true;
  }

  bool ScriptEngine::setCallback(const char *name){
    HandleScope handle_scope;
    Context::Scope context_scope(context_);
    Handle<String> process_name = String::New(name);
    Handle<Value> process_val = context_->Global()->Get(process_name);

    if (!process_val->IsFunction()) {
      return false;
    }
    Handle<Function> hfun = Handle<Function>::Cast(process_val);
    jscallback_ = Persistent<Function>::New(hfun);
    return true;
  }

  bool ScriptEngine::runNoArgs(const char *name){
    HandleScope handle_scope;
    Context::Scope context_scope(context_);

    Handle<String> process_name = String::New(name);
    Handle<Value> process_val = context_->Global()->Get(process_name);

    if (!process_val->IsFunction()) {
      return false;
    }
    Handle<Function> hfun = Handle<Function>::Cast(process_val);
    {
      TryCatch trycatch;
      Local<String> reqObj = String::New("z");
      const int argc = 1;
      Handle<Value> argv[argc] = {reqObj};
      Handle<Value> result = hfun->Call(context_->Global(), argc, argv);
      if(result.IsEmpty()){
        Handle<Value> exception = trycatch.Exception();
        String::AsciiValue exception_str(exception);
        cout << "Exception:" << *exception_str << endl;
        return false;
      }else{
        return true;
      }
    }
  }
  bool ScriptEngine::runCallback(ScriptPacket *pkt){
    bool retb = false;
    HandleScope handle_scope;
    Context::Scope context_scope(context_);
    Local<String> device_v8 = String::New(pkt->device);
    Local<Object> pkt_v8 = tpl_packet_->NewInstance();
    pkt_v8->SetInternalField(0, External::New(pkt));
    const int argc = 2;
    Handle<Value> argv[argc] = {pkt_v8,device_v8};
    Handle<Value> result = jscallback_->Call(context_->Global(), argc, argv);
    if (!result->IsUndefined()) {
    }

    return retb;
  }

};
