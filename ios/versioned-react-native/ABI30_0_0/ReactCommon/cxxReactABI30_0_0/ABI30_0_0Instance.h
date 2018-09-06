// Copyright 2004-present Facebook. All Rights Reserved.

#pragma once

#include <condition_variable>
#include <memory>

#include <cxxReactABI30_0_0/ABI30_0_0NativeToJsBridge.h>
#include <ABI30_0_0jschelpers/ABI30_0_0Value.h>

#ifndef RN_EXPORT
#define RN_EXPORT __attribute__((visibility("default")))
#endif

namespace folly {
struct dynamic;
}

namespace facebook {
namespace ReactABI30_0_0 {

class JSBigString;
class JSExecutorFactory;
class MessageQueueThread;
class ModuleRegistry;
class RAMBundleRegistry;

struct InstanceCallback {
  virtual ~InstanceCallback() {}
  virtual void onBatchComplete() {}
  virtual void incrementPendingJSCalls() {}
  virtual void decrementPendingJSCalls() {}
};

class RN_EXPORT Instance {
public:
  ~Instance();
  void initializeBridge(std::unique_ptr<InstanceCallback> callback,
                        std::shared_ptr<JSExecutorFactory> jsef,
                        std::shared_ptr<MessageQueueThread> jsQueue,
                        std::shared_ptr<ModuleRegistry> moduleRegistry);

  void setSourceURL(std::string sourceURL);

  void loadScriptFromString(std::unique_ptr<const JSBigString> string,
                            std::string sourceURL, bool loadSynchronously);
  static bool isIndexedRAMBundle(const char *sourcePath);
  void loadRAMBundleFromFile(const std::string& sourcePath,
                             const std::string& sourceURL,
                             bool loadSynchronously);
  void loadRAMBundle(std::unique_ptr<RAMBundleRegistry> bundleRegistry,
                     std::unique_ptr<const JSBigString> startupScript,
                     std::string startupScriptSourceURL, bool loadSynchronously);
  bool supportsProfiling();
  void setGlobalVariable(std::string propName,
                         std::unique_ptr<const JSBigString> jsonValue);
  void *getJavaScriptContext();
  bool isInspectable();
  void callJSFunction(std::string &&module, std::string &&method,
                      folly::dynamic &&params);
  void callJSCallback(uint64_t callbackId, folly::dynamic &&params);

  // This method is experimental, and may be modified or removed.
  void registerBundle(uint32_t bundleId, const std::string& bundlePath);

  const ModuleRegistry &getModuleRegistry() const;
  ModuleRegistry &getModuleRegistry();

#ifdef WITH_JSC_MEMORY_PRESSURE
  void handleMemoryPressure(int pressureLevel);
#endif

private:
  void callNativeModules(folly::dynamic &&calls, bool isEndOfBatch);
  void loadApplication(std::unique_ptr<RAMBundleRegistry> bundleRegistry,
                       std::unique_ptr<const JSBigString> startupScript,
                       std::string startupScriptSourceURL);
  void loadApplicationSync(std::unique_ptr<RAMBundleRegistry> bundleRegistry,
                           std::unique_ptr<const JSBigString> startupScript,
                           std::string startupScriptSourceURL);

  std::shared_ptr<InstanceCallback> callback_;
  std::unique_ptr<NativeToJsBridge> nativeToJsBridge_;
  std::shared_ptr<ModuleRegistry> moduleRegistry_;

  std::mutex m_syncMutex;
  std::condition_variable m_syncCV;
  bool m_syncReady = false;
};

} // namespace ReactABI30_0_0
} // namespace facebook