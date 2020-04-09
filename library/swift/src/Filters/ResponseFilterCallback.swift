public protocol ResponseFilterCallback {
    /**
     * Continue iterating through the filter chain with buffered headers and body data. This routine
     * can only be called if the filter has previously returned StopIteration from onHeaders() AND
     * one of StopIterationAndBuffer, StopIterationAndWatermark, or StopIterationNoBuffer
     * from each previous call to onData().
     *
     * The connection manager will dispatch headers and any buffered body data to the next filter in
     * the chain. Further note that if the response is not complete, this filter will still receive
     * onData() calls and must return an appropriate status code depending on what the filter
     * needs to do.
     */
    func continueResponse()

    /**
     * @return const Data the currently buffered data as buffered by this filter or
     *         previous ones in the filter chain. May be nullptr if nothing has been buffered yet.
     */
    func responseBuffer() -> Data

    /**
     * Allows modifying the encoding buffer. May only be called before any data has been continued
     * past the calling filter.
     */
    func modifyResponseBuffer(callback: (inout Data) -> Void)

    /**
     * Adds response trailers. May only be called in onData when end_stream is set to true.
     * If called in any other context, an assertion will be triggered.
     *
     * When called in onData, the trailers Dictionary will be initialized to an empty Dictionary and returned by
     * reference. Calling this function more than once is invalid.
     *
     * @return a reference to the newly created trailers Dictionary.
     */
    func addResponseTrailers() -> ResponseHeaders
}
