package io.envoyproxy.envoymobile

import java.util.regex.Pattern

/**
 * Tag for a stats.
 *
 * Element values must conform to the [Tag.TAG_REGEX].
 */

class Tag(internal val name: String, internal val value: String) {
  init {
    require(TAG_PATTERN.matcher(name).matches()) {
      "Tag names must conform to the regex $TAG_REGEX"
    }
    require(TAG_PATTERN.matcher(value).matches()) {
      "Tag values must conform to the regex $TAG_REGEX"
    }
  }
  
  companion object {
    private const val TAG_REGEX = "^({){0,1}[0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{12}(}){0,1}"
    private val TAG_PATTERN = Pattern.compile(TAG_REGEX)
  }
}
