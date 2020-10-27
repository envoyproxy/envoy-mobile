import Envoy
import Foundation

class AsyncDemoFilter: AsyncResponseFilter {
  private var internalCallbacks: ResponseFilterCallbacks?
  private lazy var callbacks: ResponseFilterCallbacks = {
    internalCallbacks! //swiftlint:disable:this force_unwrapping
  }()

  func onResponseHeaders(_ headers: ResponseHeaders, endStream: Bool)
    -> FilterHeadersStatus<ResponseHeaders>
  {
    return .stopIteration
  }

  func onResponseData(_ body: Data, endStream: Bool) -> FilterDataStatus<ResponseHeaders> {
    if endStream {
      callbacks.resumeResponse()
    }
    return .stopIterationAndBuffer
  }

  func onResponseTrailers(
    _ trailers: ResponseTrailers
  ) -> FilterTrailersStatus<ResponseHeaders, ResponseTrailers> {
    callbacks.resumeResponse()
    return .stopIteration
  }

  func setResponseFilterCallbacks(_ callbacks: ResponseFilterCallbacks) {
    self.internalCallbacks = callbacks
  }

  func onResumeResponse(
    headers: ResponseHeaders?,
    data: Data?,
    trailers: ResponseTrailers?,
    endStream: Bool
  ) -> FilterResumeStatus<ResponseHeaders, ResponseTrailers> {
    guard let headers = headers else {
      // Iteration stopped on headers, so headers must be present.
      fatalError("Filter behavior violation!")
    }
    let builder = headers.toResponseHeadersBuilder()
    builder.add(name: "async-filter-demo", value: "1")
    return .resumeIteration(headers: builder.build(), data: data, trailers: trailers)
  }

  func onError(_ error: EnvoyError) {}

  func onCancel() {}
}
