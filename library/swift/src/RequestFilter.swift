import Foundation

public protocol RequestFilter: Filter {
    /**
     * Called by the filter manager once to initialize the filter callbacks that the filter should
     * use. Callbacks will not be invoked by the filter after onDestroy() is called.
     */
    func setRequestFilterCallbacks(callbacks: RequestFilterCallback)

    /**
     * Called once when the request is initiated.
     *
     * Filters may mutate or delay the request.
     *
     * For the request to continue, each filter MUST call `callback.onRequest()` at some point.
     * Alternatively, filters may cancel the request by calling one or more response callbacks.
     *
     * @param request The outbound request.
     * @return The header status.
     */
    func onRequestHeaders(request: RequestHeaders) -> HeaderStatus<RequestHeaders>

    /**
     * Called any number of times whenever body data is sent.
     *
     * Filters may mutate or buffer (defer and concatenate) the data.
     *
     * For the request to continue, each filter MUST call `callback.onRequestData()` at some point.
     * Alternatively, filters may end the stream by calling one or more response callbacks.
     *
     * @param body      The outbound body data chunk.
     * @param endStream Whether this represents the end of a stream/request. If false, the stream will
     *                        continue to remain open after this data is sent.
     * @return The data status.
     */
    func onRequestData(body: Data, endStream: Bool) -> DataStatus

    /**
     * Called at most once when the request is closed from the client with trailers.
     *
     * Filters may mutate or delay the trailers.
     *
     * For the request to continue, each filter MUST call `callback.onTrailers()` at some point.
     * Alternatively, filters may cancel the request by calling one or more response callbacks.
     *
     * @param trailers The outbound trailers.
     * @return The trailer status
     */
    func onRequestTrailers(trailers: RequestHeaders) -> TrailerStatus<RequestHeaders>
}
