package io.envoyproxy.envoymobile

/*
 * Base builder class used to construct `Headers` instances.
 * See `{Request|Response}HeadersBuilder` for usage.
 */
open class HeadersContainer {
  data class Header(val name: String, var value: MutableList<String>) {
    constructor(name: String) : this(name, mutableListOf()) {}

    fun add(value: List<String>) {
      this.value.addAll(value)
    }

    fun add(value: String) {
      this.value.add(value)
    }
  }

  protected val headers: MutableMap<String, Header>

  internal constructor(headers: Map<String, MutableList<String>>) {
    this.headers = headers.mapValues { Header(it.key, it.value) }.toMutableMap()
  }

  companion object {
    fun create(headers: Map<String, List<String>>) : HeadersContainer {
      return HeadersContainer(headers.mapValues { it.value.toMutableList() })
    }
  }

  // static fun create(headers: Map<String, List<String>>): HeadersContainer {
  //   return HeadersContainer(headers.mapValues { it.value.toMutableList() })
  // }

  fun set(name: String, value: List<String>) {
    headers[name.lowercase()] = Header(name, value.toMutableList())
  }

  fun add(name: String, value: String) {
    val lowercased = name.lowercase()
    headers[lowercased]?.let { it.add(value) } ?: run {
      headers.put(lowercased, Header(name, mutableListOf(value)))
    }
    // if (existingValue != null) {
    //   headers[lowercased]
    // }

    // headers.getOrDefault(name.lowercase(), Header(name)).add(value)
  }

  fun remove(name: String) {
    headers.remove(name)
  }

  fun value(name: String): List<String>? {
    return headers[name.lowercase()]?.value
  }

  fun allHeaders(): Map<String, List<String>> {
    var caseSensitiveHeaders = mutableMapOf<String, List<String>>()
    headers.forEach {
      caseSensitiveHeaders.put(it.value.name, it.value.value)
    } 

    return caseSensitiveHeaders
  }
}