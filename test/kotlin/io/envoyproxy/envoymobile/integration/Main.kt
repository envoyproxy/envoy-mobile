package test.kotlin.io.envoyproxy.envoymobile.integration

class Main {
  init {
    System.loadLibrary("main_lib")
  }

  external fun foo(): Int
}
