#include <node.h>
#include <string>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/fcntl.h>
#include <linux/i2c-dev.h>
#include <cmath>
#include "tsl2561.h"

using namespace v8;
using namespace std;

Persistent<Function> Tsl2561::constructor;

Tsl2561::Tsl2561(char* device, int addr) {
  fd = open(device, O_RDWR);
  if (fd < 0) {
    ThrowException(Exception::Error(String::New("Failed to open the i2c bus")));
  }
  if (ioctl(fd, I2C_SLAVE, addr) < 0) {
    ThrowException(Exception::Error(String::New("Failed to acquire bus access and/or talk to slave.")));
  }

  unsigned char timing;
  if (!readByte(fd, TSL2561_REG_TIMING,&timing))
    ThrowException(Exception::Error(String::New("Failed to read timing on device")));
  timing |= 0x10; // low gain
  timing &= ~0x03;
  timing |= (1&0x03); // 101ms timing
  if (!writeByte(fd, TSL2561_REG_TIMING,timing))
    ThrowException(Exception::Error(String::New("Failed to set timing on device")));

  if(!writeByte(fd, TSL2561_REG_CONTROL, TSL2561_CMD_POWERON))
    ThrowException(Exception::Error(String::New("Failed to power on device")));
}

Tsl2561::~Tsl2561() {
  if(!writeByte(fd, TSL2561_REG_CONTROL, TSL2561_CMD_POWEROFF))
    ThrowException(Exception::Error(String::New("Failed to power off device")));
  close(fd);
}

bool
Tsl2561::readByte(int file, unsigned char address, unsigned char *value) {

  char byte  = ((address & 0x0F ) | TSL2561_CMD);

  if (write(file,&byte,1) != 1)
    return (false);

  int i = read(file, value, 1);
  if (i !=1 )
    return (false);

  return (true);
}

bool
Tsl2561::writeByte(int file, unsigned char address, unsigned char value) {
  char byte[2];

  byte[0] = ((address & 0x0F ) | TSL2561_CMD);
  byte[1] = value;

  if (write(file,byte,2) != 2) {
    return (false);
  }
  return (true);
}

bool
Tsl2561::readUint(int file, unsigned char address, unsigned int *value) {

  char byte  = ((address & 0x0F ) | TSL2561_CMD);

  if (write(file,&byte,1) != 1)
    return (false);

  char buf[2];
  int i = read(file, buf, 2);
  if (i !=2 )
    return (false);
  *value = (buf[1] << 8) + buf[0];
  return (true);
}

void Tsl2561::Init(Handle<Object> exports) {
  Local<FunctionTemplate> tpl = FunctionTemplate::New(New);
  tpl->SetClassName(String::NewSymbol("Tsl2561"));
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  tpl->PrototypeTemplate()->Set(String::NewSymbol("lux"),
      FunctionTemplate::New(Lux)->GetFunction());

  tpl->PrototypeTemplate()->Set(String::NewSymbol("id"),
      FunctionTemplate::New(Id)->GetFunction());

  constructor = Persistent<Function>::New(tpl->GetFunction());
  exports->Set(String::NewSymbol("Tsl2561"), constructor);
}

Handle<Value>
Tsl2561::Lux(const v8::Arguments& args) {
  HandleScope scope;
  Tsl2561* obj = ObjectWrap::Unwrap<Tsl2561>(args.This());
  double ratio, d0, d1, lux;
  unsigned int data0, data1;
  obj->readUint(obj->fd, TSL2561_REG_DATA_0,&data0);
  obj->readUint(obj->fd, TSL2561_REG_DATA_1,&data1);
  if (data0 == 0xFFFF || data1 == 0xFFFF)
    ThrowException(Exception::Error(String::New("Device Saturated")));
  d0 =  data0;
  d1 =  data1;

  ratio = d1/d0;

  d0 *= (402.0/101.0);
  d1 *= (402.0/101.0);
  d0 *= 16.0;
  d1 *= 16.0;
  if (ratio < 0.5) {
    lux = 0.0304 * d0 - 0.062 * d0 * pow(ratio,1.4);
  } else if (ratio < 0.61) {
    lux = 0.0224 * d0 - 0.031 * d1;
  } else if (ratio < 0.8) {
    lux = 0.0128 * d0 - 0.0153 * d1;
  } else if (ratio < 1.30) {
     lux = 0.00146 * d0 - 0.00112 * d1;
  } else {
    lux = 0.0;
  }

  return scope.Close(Number::New(lux));
}

Handle<Value>
Tsl2561::Id(const v8::Arguments& args) {

  HandleScope scope;
  Tsl2561* obj = ObjectWrap::Unwrap<Tsl2561>(args.This());
  unsigned char id;
  if (!obj->readByte(obj->fd, TSL2561_REG_ID, &id))
    ThrowException(Exception::Error(String::New("Failed to read from device")));
  return scope.Close(Number::New(id));
}

Handle<Value>
Tsl2561::New(const Arguments& args) {
  HandleScope scope;

  if (args.IsConstructCall()) {
    Local<String> device = args[0]->IsUndefined() ? String::New("/dev/i2c-1") : args[0]->ToString();
    int address = args[1]->IsUndefined() ? TSL2561_ADDR : args[1]->IntegerValue();

    Tsl2561* obj = new Tsl2561(*v8::String::AsciiValue(device), address);

    obj->Wrap(args.This());
    return args.This();
  } else {
    // Invoked as plain function `Tsl2561(...)`, turn into construct call.
    const int argc = 2;
    Local<Value> argv[argc] = { args[0], args[1] };
    return scope.Close(constructor->NewInstance(argc, argv));
  }
}
