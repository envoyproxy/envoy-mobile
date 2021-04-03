package test

object App {
  // For running as main
  @JvmStatic
  fun main(args: Array<String>) {
    f(123)
  }

  external fun f(x: Int): Int

  init {
    System.loadLibrary("native")
  }
}