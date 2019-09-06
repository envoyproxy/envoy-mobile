import EnvoyInterfaces
import Foundation

/// <#function description#>
@objcMembers
public final class GRPCClient: NSObject {
    private let client: Client

    // MARK: - Internal

    init(with client: Client) {
        self.client = client
    }

    // MARK: - Public

    /// <#function description#>
    ///
    /// - parameter request: <#request description#>
    /// - parameter handler: <#handler description#>
    ///
    /// - returns: <#return value description#>
    public func send(_ request: Request, handler: GRPCResponseHandler) -> GRPCStreamEmitter {
        let emitter = self.client.send(request, handler: handler.underlyingHandler)
        return GRPCStreamEmitter(emitter: emitter)
    }
}
