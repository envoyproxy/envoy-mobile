package io.envoyproxy.envoymobile

import java.nio.ByteBuffer

/*
 * Status returned by filters when transmitting or receiving trailers.
 */
sealed class FilterResumeStatus<T : Headers, U : Headers> {
  /**
   * Resume previously-stopped iteration, possibly forwarding headers and data, if iteration was
   * previously stopped during an on*Headers or on*Data invocation.
   *
   * It is an error to return ResumeIteration if iteration is not currently stopped, and it is
   * an error to include headers if headers have already been forwarded to the next filter
   * (i.e. iteration was stopped during an on*Data invocation instead of on*Headers).
   */
  class ResumeIteration<T : Headers, U : Headers>(val headers: T?, val data: ByteBuffer?, val trailers: U?) : FilterResumeStatus<T, U>()
}
