package test

object App {
  // For running as main
  @JvmStatic
  fun main(args: Array<String>) {
    val f = f(123)
    if (f != 123) {
      throw RuntimeException()
    }
  }

  external fun f(x: Int): Int

  init {
    System.loadLibrary("native")
  }
}
