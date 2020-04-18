public enum HeaderStatus<T: Headers> {
    // Continue filter chain iteration.
    case `continue`(T)

    // Do not iterate to any of the remaining filters in the chain. Returning
    // FilterDataStatus::Continue from decodeData()/encodeData() or calling
    // continueDecoding()/continueEncoding() MUST be called if continued filter iteration is desired.
    case stopIteration(T)

    // Continue iteration to remaining filters, but ignore any subsequent data or trailers. This
    // results in creating a header only request/response.
    // case continueAndEndStream // Skipped for v1

    // Do not iterate for headers as well as data and trailers for the current filter and the filters
    // following, and buffer body data for later dispatching. ContinueDecoding() MUST
    // be called if continued filter iteration is desired.
    //
    // Used when a filter wants to stop iteration on data and trailers while waiting for headers'
    // iteration to resume.
    //
    // If buffering the request causes buffered data to exceed the configured buffer limit, a 413 will
    // be sent to the user. On the response path exceeding buffer limits will result in a 500.
    case stopIterationAndBuffer(T) // Envoy: stopAllIterationAndBuffer
}
