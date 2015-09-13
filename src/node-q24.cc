// hello.cc
#include <v8.h>
#include <node.h>
#include <nan.h>
#include "q24.h"

using v8::Function;
using v8::FunctionTemplate;
using v8::Handle;
using v8::Object;
using v8::String;
using v8::Local;
using v8::Number;
using v8::Value;
using Nan::AsyncQueueWorker;
using Nan::AsyncWorker;
using Nan::Callback;
using Nan::HandleScope;
using Nan::New;
using Nan::Null;
using Nan::To;
using Nan::Set;
using Nan::GetFunction;

static void convertHashToHexString(char *output, uint8_t *hash) {
	for (int i = 0; i < HASH_LENGTH; i++)
		sprintf(&output[i*2], "%02x", hash[i]);
}

class HashAsyncWorker : public AsyncWorker {
 public:
  HashAsyncWorker(Callback *callback, const uint8_t *entropy, int entropyLength)
    : AsyncWorker(callback)
    , entropy(entropy)
    , entropyLength(entropyLength) {
      output = new char[48];
    }
  ~HashAsyncWorker() {}

  // Executed inside the worker-thread.
  // It is not safe to access V8, or V8 data structures
  // here, so everything we need for input and output
  // should go on `this`.
  void Execute () {
	  uint8_t *hash = compute(entropy, entropyLength, 0);
	  convertHashToHexString(output, hash);
  }

  // Executed when the async work is complete
  // this function will be run inside the main event loop
  // so it is safe to use V8 again
  void HandleOKCallback () {
    Nan::HandleScope scope;

    Local<Value> argv[] = {
        Null()
      , New<String>(output).ToLocalChecked()
    };

    callback->Call(2, argv);
  }

 private:
  const uint8_t *entropy;
  char *output;
  int entropyLength;
};

// Asynchronous access to the `Estimate()` function
NAN_METHOD(HashAsync) {
	std::string inputString(*v8::String::Utf8Value(info[0]->ToString()));
  const uint8_t *entropy = (const uint8_t *)inputString.c_str();
  int entropyLength = inputString.length();

  Callback *callback = new Callback(info[1].As<Function>());

  AsyncQueueWorker(new HashAsyncWorker(callback, entropy, entropyLength));
}

NAN_METHOD(HashSync) {
	std::string inputString(*v8::String::Utf8Value(info[0]->ToString()));
  const uint8_t *entropy = (const uint8_t *)inputString.c_str();
  int entropyLength = inputString.length();

  char *output = new char[48];
  uint8_t *hash = compute(entropy, entropyLength, 0);
  convertHashToHexString(output, hash);

  info.GetReturnValue().Set(New<String>(output).ToLocalChecked());
}

NAN_MODULE_INIT(InitAll) {
  Nan::Set(target, New<String>("hashAsync").ToLocalChecked(),
    GetFunction(New<FunctionTemplate>(HashAsync)).ToLocalChecked());
  Nan::Set(target, Nan::New("hashSync").ToLocalChecked(),
    Nan::GetFunction(Nan::New<FunctionTemplate>(HashSync)).ToLocalChecked());
}

NODE_MODULE(q24, InitAll)

