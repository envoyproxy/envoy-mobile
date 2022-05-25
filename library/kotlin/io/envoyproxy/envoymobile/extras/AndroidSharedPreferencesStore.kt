package io.envoyproxy.envoymobile.extras

import android.content.SharedPreferences

import io.envoyproxy.envoymobile.KeyValueStore

class AndroidSharedPreferencesStore(String preferencesName) {
  val editor = getSharedPreferences(perferencesName, Context.MODE_PRIVATE).edit()
} : KeyValueStore {
  override fun read(key: String): String? {
    return editor.getString(key, null)
  }

  override fun remove(key: String) {
    editor.remove(key)
    editor.apply()
  }

  override fun save(key: String, value: String) {
    editor.putString(key, value)
    editor.apply
  }
}
