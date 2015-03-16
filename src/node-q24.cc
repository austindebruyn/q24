// hello.cc
#include <v8.h>
#include <node.h>
#include <nan.h>
#include "q24.h"

using namespace v8;

static void convertHashToHexString(char *output, uint8_t *hash) {
	for (int i = 0; i < HASH_LENGTH; i++)
		sprintf(&output[i*2], "%02x", hash[i]);
}

class HashWorker : public NanAsyncWorker {

	public:
		HashWorker(NanCallback *callback, const uint8_t *entropy, int entropy_length)
			: NanAsyncWorker(callback)
			, entropy(entropy)
			, entropy_length(entropy_length) {
				output = new char[48];
			}

		~HashWorker() {}

		// Executed inside the worker-thread.
		// It is not safe to access V8, or V8 data structures
		// here, so everything we need for input and output
		// should go on `this`.
		void Execute () {

			uint8_t *hash = compute(entropy, entropy_length, 0);
			convertHashToHexString(output, hash);
		}

		// Executed when the async work is complete
		// this function will be run inside the main event loop
		// so it is safe to use V8 again
		void HandleOKCallback () {
			NanScope();

			Local<Value> argv[] = {
				NanNull()
				, NanNew<String>(output)
			};

			callback->Call(2, argv);
		};

	private:
		const uint8_t *entropy;
		char *output;
		int entropy_length;
};

NAN_METHOD(HashAsync) {
	NanScope();

	std::string inputString(*v8::String::Utf8Value(args[0]->ToString()));

	const uint8_t *entropy = (const uint8_t *)inputString.c_str();
	int entropyLength = inputString.length();

	NanCallback *callback = new NanCallback(args[1].As<Function>());

	NanAsyncQueueWorker(new HashWorker(callback, entropy, entropyLength));
	NanReturnUndefined();
}

NAN_METHOD(HashSync) {
	NanScope();

	std::string inputString(*v8::String::Utf8Value(args[0]->ToString()));

	const uint8_t *entropy = (const uint8_t *)inputString.c_str();
	int entropyLength = inputString.length();

	char *output = new char[48];

	uint8_t *hash = compute(entropy, entropyLength, 0);
	convertHashToHexString(output, hash);

	NanReturnValue(NanNew<String>(output));
}

void InitAll(Handle<Object> exports) {
	exports->Set(NanNew<String>("hashAsync"),
		NanNew<FunctionTemplate>(HashAsync)->GetFunction());
	exports->Set(NanNew<String>("hashSync"),
		NanNew<FunctionTemplate>(HashSync)->GetFunction());
}

NODE_MODULE(q24, InitAll)

