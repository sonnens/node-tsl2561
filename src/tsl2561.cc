#include <node.h>
#include "tsl2561.h"

using namespace v8;

void InitAll(Handle<Object> exports) {
  Tsl2561::Init(exports);
}

NODE_MODULE(tsl2561, InitAll)
