package io.envoyproxy.envoymobile.filter

enum class HeaderStatus {
  // Continue filter chain iteration.
  Continue,
  // Do not iterate to any of the remaining filters in the chain. Returning
  // FilterDataStatus::Continue from decodeData()/encodeData() or calling
  // continueDecoding()/continueEncoding() MUST be called if continued filter iteration is desired.
  StopIteration,

  // Continue iteration to remaining filters, but ignore any subsequent data or trailers. This
  // results in creating a header only request/response.
  ContinueAndEndStream,

  // Do not iterate for headers as well as data and trailers for the current filter and the filters
  // following, and buffer body data for later dispatching. ContinueDecoding() MUST
  // be called if continued filter iteration is desired.
  //
  // Used when a filter wants to stop iteration on data and trailers while waiting for headers'
  // iteration to resume.
  //
  // If buffering the request causes buffered data to exceed the configured buffer limit, a 413 will
  // be sent to the user. On the response path exceeding buffer limits will result in a 500.
  StopAllIterationAndBuffer,
}

enum class DataStatus {
  // Continue filter chain iteration. If headers have not yet been sent to the next filter, they
  // will be sent first via decodeHeaders()/encodeHeaders(). If data has previously been buffered,
  // the data in this callback will be added to the buffer before the entirety is sent to the next
  // filter.
  Continue,

  // Do not iterate to any of the remaining filters in the chain, and buffer body data for later
  // dispatching. Returning FilterDataStatus::Continue from decodeData()/encodeData() or calling
  // continueDecoding()/continueEncoding() MUST be called if continued filter iteration is desired.
  //
  // This should be called by filters which must parse a larger block of the incoming data before
  // continuing processing and so can not push back on streaming data via watermarks.
  //
  // If buffering the request causes buffered data to exceed the configured buffer limit, a 413 will
  // be sent to the user. On the response path exceeding buffer limits will result in a 500.
  StopIterationAndBuffer,

  // Do not iterate to any of the remaining filters in the chain, but do not buffer any of the
  // body data for later dispatching. Returning FilterDataStatus::Continue from
  // decodeData()/encodeData() or calling continueDecoding()/continueEncoding() MUST be called if
  // continued filter iteration is desired.
  StopIterationNoBuffer
}

enum class TrailerStatus {
  // Continue filter chain iteration.
  Continue,

  // Do not iterate to any of the remaining filters in the chain. Calling
  // continueDecoding()/continueEncoding() MUST be called if continued filter iteration is desired.
  StopIteration
}