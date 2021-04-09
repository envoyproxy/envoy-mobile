package test.kotlin

class Main {
  init {
    System.loadLibrary("main_lib")
  }

  fun main(args: Array<String>) {
    println(foo())
  }

  external fun foo(): Int
}
