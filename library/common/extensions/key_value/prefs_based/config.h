#include "envoy/common/key_value_store.h"

#include "source/common/common/key_value_store_base.h"

#include "library/common/extensions/key_value/prefs_based/prefs.pb.h"
#include "library/common/extensions/key_value/prefs_based/prefs.pb.validate.h"

namespace Envoy {
namespace Extensions {
namespace KeyValue {

class PrefsInterface {
public:
  virtual ~PrefsInterface() {}
  virtual void flush(const std::string& key, const std::string& contents) PURE;
  virtual std::string read(const std::string& key) const PURE;
};

// A prefs file based key value store, which loads from and flushes to key of
// in the prefs file.
//
// All keys and values are flushed to a single entry as
// [length]\n[key][length]\n[value]
class PrefsBasedKeyValueStore : public KeyValueStoreBase {
public:
  PrefsBasedKeyValueStore(Event::Dispatcher& dispatcher, std::chrono::milliseconds flush_interval,
                          PrefsInterface& prefs_interface, const std::string& key);
  // KeyValueStore
  void flush() override;

private:
  PrefsInterface& prefs_interface_;
  const std::string key_;
};

class PrefsBasedKeyValueStoreFactory : public KeyValueStoreFactory {
public:
  PrefsBasedKeyValueStoreFactory() {}
  PrefsBasedKeyValueStoreFactory(PrefsInterface& prefs_interface)
      : prefs_interface_(prefs_interface) {}

  // KeyValueStoreFactory
  KeyValueStorePtr createStore(const Protobuf::Message& config,
                               ProtobufMessage::ValidationVisitor& validation_visitor,
                               Event::Dispatcher& dispatcher,
                               Filesystem::Instance& file_system) override;

  // TypedFactory
  ProtobufTypes::MessagePtr createEmptyConfigProto() override {
    return ProtobufTypes::MessagePtr{
        new envoymobile::extensions::key_value::prefs_based::PrefsBasedKeyValueStoreConfig()};
  }

  std::string name() const override { return "envoy.key_value.prefs_based"; }
  // TODO(alyssawilk, goaway) the default PrefsInterface should do up calls through Java and this
  // can be moved to a non-optional reference.
  OptRef<PrefsInterface> prefs_interface_;
};

} // namespace KeyValue
} // namespace Extensions
} // namespace Envoy
