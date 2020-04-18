public enum DataStatus {
    // Continue filter chain iteration. If headers have not yet been sent to the next filter, they
    // will be sent first via decodeHeaders()/encodeHeaders(). If data has previously been buffered,
    // the data in this callback will be added to the buffer before the entirety is sent to the next
    // filter.
    case `continue`(Data)

    // Do not iterate to any of the remaining filters in the chain, and buffer body data for later
    // dispatching. Returning FilterDataStatus::Continue from decodeData()/encodeData() or calling
    // continueDecoding()/continueEncoding() MUST be called if continued filter iteration is desired.
    //
    // This should be called by filters which must parse a larger block of the incoming data before
    // continuing processing and so can not push back on streaming data via watermarks.
    //
    // If buffering the request causes buffered data to exceed the configured buffer limit, a 413 will
    // be sent to the user. On the response path exceeding buffer limits will result in a 500.
    case stopIterationAndBuffer(Data)

    // Do not iterate to any of the remaining filters in the chain, but do not buffer any of the
    // body data for later dispatching. Returning FilterDataStatus::Continue from
    // decodeData()/encodeData() or calling continueDecoding()/continueEncoding() MUST be called if
    // continued filter iteration is desired.
    case stopIteration(Data) // Envoy: stopIterationNoBuffer
}
