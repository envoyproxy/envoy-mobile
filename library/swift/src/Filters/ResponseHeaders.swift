final class ResponseHeaders {
    private var headers = [String: [String]]()

    func addHeader() {} // ...

    func removeHeader() {} // ...

    var statusCode: Int? {
        return self.headers[":status"]?.first.flatMap(Int.init)
    }
}
