package io.envoyproxy.envoymobile

import java.util.regex.Pattern

/**
 * Element for stats.
 */
class Element(val element: String) {
  init {
    if (!Pattern.compile("^[A-Za-z_]+\$").matcher(element).matches()) {
      throw IllegalArgumentException(
        "Element values must conform to the regex /^[A-Za-z_]+$/"
      )
    }
  }
}
