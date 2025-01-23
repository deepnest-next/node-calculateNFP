#include <napi.h>

Napi::Value calculateNFP(const Napi::CallbackInfo& info);

Napi::Object Init(Napi::Env env, Napi::Object exports) {
  exports.Set(
    Napi::String::New(env, "calculateNFP"),
    Napi::Function::New(env, calculateNFP)
  );

  return exports;
}

NODE_API_MODULE(addon, Init)