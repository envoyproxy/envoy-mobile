#include "library/common/stream_info/aux_stream_info.h"

#include "source/common/common/assert.h"

namespace Envoy {
namespace StreamInfo {

namespace {
// TODO(goaway): This is a hack; it would preferable to use a unique key on StreamInfo.
// Long-term it might be preferabale to store AuxStreamInfo directly on StreamInfo as an
// extension. This would simplify management considerably.
// struct StreamInfoHash {
//  size_t operator()(const StreamInfo& object) const {
//    return reinterpret_cast<size_t>(std::addressof(object));
//  }
//};
//
// inline bool operator == (const StreamInfo& lhs, const StreamInfo& rhs) {
//    return &lhs == &rhs;
//}

} // namespace

// static std::unordered_map<std::reference_wrapper<const StreamInfo>, AuxStreamInfoPtr,
// std::hash<StreamInfoHash>> internal_map_{};

static std::unordered_map<const StreamInfo*, AuxStreamInfoPtr> internal_map_{};

AuxStreamInfo& AuxProvider::get(const StreamInfo& stream_info) {
  auto it = internal_map_.find(&stream_info);
  RELEASE_ASSERT(it != internal_map_.end(), "aux stream info not present on get");
  return *it->second.get();
}

AuxStreamInfo& AuxProvider::create(const StreamInfo& stream_info) {
  internal_map_[&stream_info] = std::make_unique<AuxStreamInfo>();
  return *internal_map_.at(&stream_info);
}

void AuxProvider::clear(const StreamInfo& stream_info) {
  auto erased = internal_map_.erase(&stream_info);
  RELEASE_ASSERT(erased == 1, "aux stream info not present for erase");
}

} // namespace StreamInfo
} // namespace Envoy
