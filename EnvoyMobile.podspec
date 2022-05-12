Pod::Spec.new do |s|
    s.name = 'EnvoyMobile'
    s.version = `cat VERSION`.chomp
    s.author = 'Envoy Mobile Project Authors'
    s.summary = "Multiplatform client HTTP/networking library built on the Envoy project's core networking layer"
    s.homepage = 'https://envoy-mobile.github.io'
    s.documentation_url = 'https://envoy-mobile.github.io/docs/envoy-mobile/latest/index.html'
    s.social_media_url = 'https://twitter.com/EnvoyProxy'
    s.license = { type: 'Apache-2.0', file: 'ios_framework_cocoapods/LICENSE' }
    s.platform = :ios, '12.0'
    s.swift_versions = ['5.5', '5.6']
    s.libraries = 'resolv.9', 'c++'
    s.frameworks = 'Network', 'SystemConfiguration', 'UIKit'
    s.source = { http: "https://github.com/envoyproxy/envoy-mobile/releases/download/v#{s.version}/ios_framework_cocoapods.zip" }
    s.vendored_frameworks = 'ios_framework_cocoapods/Envoy.xcframework'
    s.pod_target_xcconfig = { 'APPLICATION_EXTENSION_API_ONLY' => 'YES' }
end
