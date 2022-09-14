#pragma once

#include <stddef.h>

// This file specifies types and interfaces used between the dynamic stat sink and the shared
// library that receives the stats. Note that we avoid using existing EM types to provide a
// delineation between internal EM interfaces and once that need to adhere to stricter ABI
// guarantees.

// A string view type, representing a view into a C++ std::string.
typedef struct {
  const char* data;
  size_t size;
} em_string_view;

// A stingle metric tag, specified two string views forming a key-value pair.
typedef struct {
  em_string_view key;
  em_string_view value;
} em_metric_tag;

// A handle to a implementation of em_record_counter. This is called for each counter whose value
// has changed since the last stat flush.
//
// @param name the name of the counter to record.
// @param tags a pointer to an array of tags.
// @param tag_count the number of elements in the tag array.
// @param delta the counter delta since the last flush. This should always be greater than 0.
typedef void* (*em_record_counter_t)(const em_string_view name, const em_metric_tag* tags,
                                     size_t tag_count, size_t delta);
