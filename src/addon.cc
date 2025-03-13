#include <napi.h>

Napi::Value CalculateNFP(const Napi::CallbackInfo& info);

Napi::Object Init(Napi::Env env, Napi::Object exports) {
  exports.Set("calculateNFP", Napi::Function::New(env, CalculateNFP));
  return exports;
}

NODE_API_MODULE(addon, Init)