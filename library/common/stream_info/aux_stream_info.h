#pragma once

#include "envoy/stream_info/stream_info.h"

#include "library/common/types/c_types.h"

namespace Envoy {
namespace StreamInfo {

struct AuxStreamInfo {
  envoy_stream_t stream_id_;
  absl::optional<uint16_t> configuration_key_;
};

using AuxStreamInfoPtr = std::unique_ptr<AuxStreamInfo>;

// TODO(goaway): convert to singleton. Long-term it might be preferabale to store AuxStreamInfo
// directly on StreamInfo as an extension. This would simplify management considerably.
class AuxProvider {
public:
  /**
   * Retrieve auxiliary stream info for a stream.
   * @param stream_info, the canonical stream info for the stream.
   * @return AuxStreamInfo&, auxiliary stream info for the stream.
   */
  static AuxStreamInfo& get(const StreamInfo& stream_info);

  /**
   * Create auxiliary stream info for a stream.
   * @param stream_info, the canonical stream info for the stream.
   */
  static AuxStreamInfo& create(const StreamInfo&);

  /**
   * Clear auxiliary stream info for a stream.
   * @param stream_info, the canonical stream info for the stream.
   */
  static void clear(const StreamInfo&);
};

} // namespace StreamInfo
} // namespace Envoy
