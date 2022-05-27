package io.envoyproxy.envoymobile.android

import android.content.SharedPreferences

import io.envoyproxy.envoymobile.KeyValueStore

class SharedPreferencesStore(sharedPreferences: SharedPreferences) : KeyValueStore {
  val preferences = sharedPreferences
  val editor = sharedPreferences.edit()

  override fun read(key: String): String? {
    return preferences.getString(key, null)
  }

  override fun remove(key: String) {
    editor.remove(key)
    editor.apply()
  }

  override fun save(key: String, value: String) {
    editor.putString(key, value)
    editor.apply()
  }
}
