#ifndef TSL2561_H
#define TSL2561_H
#define BUILDIN_NODE_EXTENSION

#include <node.h>
using namespace v8;
using namespace std;

#define TSL2561_ADDR_0 0x29 // address with '0' shorted on board
#define TSL2561_ADDR 0x39 // default address
#define TSL2561_ADDR_1 0x49 // address with '1' shorted on board

// TSL2561 registers
#define TSL2561_CMD 0x80
#define TSL2561_CMD_CLEAR 0xC0

#define TSL2561_CMD_POWERON 0x03
#define TSL2561_CMD_POWEROFF 0x00

#define TSL2561_REG_CONTROL 0x00
#define TSL2561_REG_TIMING 0x01
#define TSL2561_REG_THRESH_L 0x02
#define TSL2561_REG_THRESH_H 0x04
#define TSL2561_REG_INTCTL 0x06
#define TSL2561_REG_ID 0x0A
#define TSL2561_REG_DATA_0 0x0C
#define TSL2561_REG_DATA_1 0x0E

class Tsl2561 : public node::ObjectWrap {
 public:
  static void Init(v8::Handle<v8::Object> exports);

 private:
  explicit Tsl2561(char* device, int address);
  ~Tsl2561();

  static v8::Handle<v8::Value> New(const v8::Arguments& args);
  static v8::Handle<v8::Value> Lux(const v8::Arguments& args);
  static v8::Handle<v8::Value> Id(const v8::Arguments& args);

  bool readByte(int file, unsigned char address, unsigned char *value);
  bool writeByte(int file, unsigned char address, unsigned char value);
  bool readUint(int file, unsigned char address, unsigned int *value);

  static v8::Persistent<v8::Function> constructor;
  int fd;
};

#endif
