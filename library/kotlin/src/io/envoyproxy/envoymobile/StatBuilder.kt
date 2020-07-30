package io.envoyproxy.envoymobile

import java.lang.ref.WeakReference

import io.envoyproxy.envoymobile.engine.EnvoyEngine

/**
 * A builder to build a {@link Stat}
 */
class StatBuilder constructor(private val envoyEngine: WeakReference<EnvoyEngine>) {
    private lateinit var elements: List<String>

    fun withElements(elements: List<String>): StatBuilder {
        this.elements = elements
        return this
    }

    fun build(): Stat {
        return Stat(envoyEngine, elements)
    }
}