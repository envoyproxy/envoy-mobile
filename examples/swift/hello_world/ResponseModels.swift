/// Represents a response from the server.
struct Response {
  let body: String
  let serverHeader: String
}

/// Error that was encountered when executing a request.
struct RequestError: Error {
  let description: String
}
