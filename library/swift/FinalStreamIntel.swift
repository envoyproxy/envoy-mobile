@_implementationOnly import EnvoyEngine
import Foundation

/// Exposes one time HTTP stream metrics, context, and other details.
@objcMembers
public final class FinalStreamIntel: NSObject, Error {
  /// The time the request started, in ms since the epoch.
  public let requestStartMs: UInt64
  /// The time the DNS resolution for this request started, in ms since the epoch.
  public let dnsStartMs: UInt64
  /// The time the DNS resolution for this request completed, in ms since the epoch.
  public let dnsEndMs: UInt64
  /// The time the upstream connection started, in ms since the epoch. (1)
  public let connectStartMs: UInt64
  /// The time the upstream connection completed, in ms since the epoch. (1)
  public let connectEndMs: UInt64
  /// The time the SSL handshake started, in ms since the epoch. (1)
  public let sslStartMs: UInt64
  /// The time the SSL handshake completed, in ms since the epoch. (1)
  public let sslEndMs: UInt64
  /// The time the first byte of the request was sent upstream, in ms since the epoch.
  public let sendingStartMs: UInt64
  /// The time the last byte of the request was sent upstream, in ms since the epoch.
  public let sendingEndMs: UInt64
  /// The time the first byte of the response was received, in ms since the epoch.
  public let responseStartMs: UInt64
  /// The time the last byte of the request was received, in ms since the epoch.
  public let requestEndMs: UInt64
  /// True if the upstream socket had been used previously.
  public let socketReused: Bool
  /// The number of bytes sent upstream.
  public let sentByteCount: UInt64
  /// The number of bytes received from upstream.
  public let receivedByteCount: UInt64

  // NOTE(1): These fields may not be set if socket_reused is false.

  public init(
    requestStartMs: UInt64,
    dnsStartMs: UInt64,
    dnsEndMs: UInt64,
    connectStartMs: UInt64,
    connectEndMs: UInt64,
    sslStartMs: UInt64,
    sslEndMs: UInt64,
    sendingStartMs: UInt64,
    sendingEndMs: UInt64,
    responseStartMs: UInt64,
    requestEndMs: UInt64,
    socketReused: Bool,
    sentByteCount: UInt64,
    receivedByteCount: UInt64
  ) {
    self.requestStartMs = requestStartMs
    self.dnsStartMs = dnsStartMs
    self.dnsEndMs = dnsEndMs
    self.connectStartMs = connectStartMs
    self.connectEndMs = connectEndMs
    self.sslStartMs = sslStartMs
    self.sslEndMs = sslEndMs
    self.sendingStartMs = sendingStartMs
    self.sendingEndMs = sendingEndMs
    self.responseStartMs = responseStartMs
    self.requestEndMs = requestEndMs
    self.socketReused = socketReused
    self.sentByteCount = sentByteCount
    self.receivedByteCount = receivedByteCount
  }
}

extension FinalStreamIntel {
  internal convenience init(_ cStruct: EnvoyFinalStreamIntel) {
    self.init(
      requestStartMs: cStruct.request_start_ms,
      dnsStartMs: cStruct.dns_start_ms,
      dnsEndMs: cStruct.dns_end_ms,
      connectStartMs: cStruct.connect_start_ms,
      connectEndMs: cStruct.connect_end_ms,
      sslStartMs: cStruct.ssl_start_ms,
      sslEndMs: cStruct.ssl_end_ms,
      sendingStartMs: cStruct.sending_start_ms,
      sendingEndMs: cStruct.sending_end_ms,
      responseStartMs: cStruct.response_start_ms,
      requestEndMs: cStruct.request_end_ms,
      socketReused: cStruct.socket_reused != 0,
      sentByteCount: cStruct.sent_byte_count,
      receivedByteCount: cStruct.received_byte_count
    )
  }
}
