//// ActiveStream will hold a reference to all filters and also implement the callback interfaces
//@objcMembers
//final class EnvoyStreamEmitter {
//  private let stream: EnvoyHTTPStream
//
//  init(stream: EnvoyHTTPStream) {
//    self.stream = stream
//  }
//
//  func sendData(_ data: Data) -> StreamEmitter {
//    // Filters go here
//    self.stream.send(data, close: false)
//    return self
//  }
//
//  func close(trailers: RequestHeaders) {
//    // Filters go here
//    self.stream.sendTrailers(trailers)
//  }
//
//  func close(data: Data) {
//    // Filters go here
//    self.stream.send(data, close: true)
//  }
//
//  func cancel() {
//    // Filters go here
//    _ = self.stream.cancel()
//  }
//}
