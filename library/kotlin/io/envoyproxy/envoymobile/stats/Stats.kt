package io.envoyproxy.envoymobile

/** Interface for stats */
interface Stats {

  fun convert(tags: List<Tag>): Map<String, String> {
    val tagMap = mutableMapOf<String, String>()
    tags.forEach { it ->
      tagMap[it.key] = it.value
    }
    return tagMap
  }
}
