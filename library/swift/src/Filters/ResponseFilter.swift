protocol ResponseFilter : Filter {

    /**
     * Called by the filter manager once to initialize the filter callbacks that the filter should
     * use. Callbacks will not be invoked by the filter after onDestroy() is called.
     */
    func setResponseFilterCallbacks(callbacks: ResponseFilterCallback)

    /**
     * Called once when headers are received.
     *
     * Filters may mutate or delay the headers.
     *
     * For the response to continue, each filter MUST call `callback.onHeaders()` at some point.
     * Alternatively, filters may end the stream by calling one or more response callbacks.
     *
     * @param headers    The inbound headers.
     * @param statusCode The HTTP status code.
     * @param endStream  Whether this response is headers-only. If true, no body should be expected.
     * @return The header status.
     */
    func onResponseHeaders(headers: inout ResponseHeaders, endStream: Bool) -> HeaderStatus

    /**
     * Called any number of times whenever body data is received.
     *
     * Filters may mutate or buffer (defer and concatenate) the data.
     *
     * For the response to continue, each filter MUST call `callback.onResponseData()` at some point.
     * Alternatively, filters may end the stream by calling one or more response callbacks.
     *
     * @param body      The inbound body data chunk.
     * @param endStream Whether this represents the end of a stream/request. If false, the stream will
     *                        continue to remain open.
     * @return The data status.
     */
    func onResponseData(body: inout Data, endStream: Bool) -> DataStatus

    /**
     * Called at most once when the request is closed from the server with trailers.
     *
     * Filters may mutate or delay the trailers.
     *
     * For the response to continue, each filter MUST call `callback.onTrailers()` at some point.
     * Alternatively, filters may call one or more other response callbacks instead.
     *
     * @param trailers The inbound trailers.
     * @return The trailer status.
     */
    func onResponseTrailers(trailers: inout Dictionary<String, Array<String>>) -> TrailerStatus

    /**
     * Called at most once when an error within Envoy occurs.
     *
     * For the response to continue, each filter MUST call `callback.onError()` at some point.
     * Alternatively, filters may call one or more other response callbacks instead.
     *
     * @param error The error that occurred within Envoy.
     */
    func onError(error: inout EnvoyError)

    /**
     * Called at most once when the client cancels a request.
     *
     * For the response to continue, each filter MUST call `callback.onCancel()` at some point.
     * Alternatively, filters may call one or more other response callbacks instead.
     */
    func onCanceled()
}
