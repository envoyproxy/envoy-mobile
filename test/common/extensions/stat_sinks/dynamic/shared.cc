#include "library/common/extensions/stat_sinks/dynamic/dynamic_sink.h"

// NOLINT(namespace-envoy)

__attribute__((__used__)) extern "C" void em_record_counter(em_string_view, em_metric_tag*, size_t,
                                                            size_t) {}
