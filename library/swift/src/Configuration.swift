import Foundation

@objc
enum ConfigurationError: Int, Swift.Error {
    case missingDefaultConfigFile
    case invalidTemplateKey
    case unresolvedTemplateKey
}

/// Builder used for creating configurations for new Envoy instances.
@objcMembers
public final class Configuration: NSObject {
    /// The address at which Envoy should listen for requests.
    public private(set) var address: String = "0.0.0.0"

    /// The port at which Envoy should listen for requests.
    public private(set) var port: String = "9001"

    /// Timeout to apply to connections.
    public private(set) var connectTimeoutSeconds: UInt = 30

    /// Interval at which DNS should be refreshed.
    public private(set) var dnsRefreshSeconds: UInt = 60

    /// Interval at which stats should be flushed.
    public private(set) var statsFlushSeconds: UInt = 60

    public func withAddress(_ address: String) -> Configuration {
        self.address = address
        return self
    }

    public func withPort(_ port: String) -> Configuration {
        self.port = port
        return self
    }

    public func withConnectTimeoutSeconds(_ connectTimeoutSeconds: UInt) -> Configuration {
        self.connectTimeoutSeconds = connectTimeoutSeconds
        return self
    }

    public func withDNSRefreshSeconds(_ dnsRefreshSeconds: UInt) -> Configuration {
        self.dnsRefreshSeconds = dnsRefreshSeconds
        return self
    }

    public func withStatsFlushSeconds(_ statsFlushSeconds: UInt) -> Configuration {
        self.statsFlushSeconds = statsFlushSeconds
        return self
    }

    // MARK: - Internal

    /// Converts the structure into a file by replacing values in a default template configuration.
    ///
    /// - returns: A file that may be used for starting an instance of Envoy.
    public func build() throws -> String {
        let bundle = Bundle(for: type(of: self))
        guard let configFile = bundle.path(forResource: "config", ofType: "yaml") else {
            NSLog("**Unable to load config.yaml")
            throw ConfigurationError.missingDefaultConfigFile
        }

        var template = try String(contentsOfFile: configFile, encoding: .utf8)
        let templateKeysToValues: [String: String] = [
            "address": self.address,
            "port_value": self.port,
            "connect_timeout": "\(self.connectTimeoutSeconds)s",
            "dns_refresh_rate": "\(self.dnsRefreshSeconds)s",
            "stats_flush_interval": "\(self.statsFlushSeconds)s",
        ]

        for (templateKey, value) in templateKeysToValues {
            guard let range = template.range(of: "{{ \(templateKey) }}") else {
                throw ConfigurationError.invalidTemplateKey
            }

            template = template.replacingCharacters(in: range, with: value)
        }

        if template.contains("{{") {
            throw ConfigurationError.unresolvedTemplateKey
        }

        return template
    }
}
