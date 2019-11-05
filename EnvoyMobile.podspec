Pod::Spec.new do |s|
    s.name              = 'EnvoyMobile'
    s.version           = '0.2.0'
    s.author            = 'Lyft, Inc.'
    s.summary           = 'Client networking libraries based on the Envoy project'
    s.homepage          = 'https://envoy-mobile.github.io'
    s.documentation_url = 'https://envoy-mobile.github.io/docs/envoy-mobile/latest/index.html'
    s.social_media_url  = 'https://twitter.com/EnvoyProxy'
    s.license           = { :type => 'Apache-2.0', :file => 'LICENSE' }
    s.platform          = :ios
    s.source            = { :http => "https://github.com/lyft/envoy-mobile/releases/download/v#{s.version}/envoy_ios_framework.zip",
                            :sha256 => 'efb14c917286daccd26aecb999c7a0ef3f547742156662f2545bcdd71484646f' }
    s.libraries         = 'resolv.9', 'c++'
    s.ios.framework     = 'SystemConfiguration'
    s.ios.vendored_frameworks = 'Envoy.framework'
end
