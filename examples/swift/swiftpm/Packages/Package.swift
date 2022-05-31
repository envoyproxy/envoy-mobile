// swift-tools-version:5.5

import PackageDescription

// swiftlint:disable line_length

let package = Package(
    name: "NetworkClient",
    platforms: [.iOS(.v15)],
    products: [
        .library(
            name: "NetworkClient",
            targets: ["NetworkClient"]),
    ],
    targets: [
        .target(
            name: "NetworkClient",
            dependencies: ["Envoy"],
            linkerSettings: [
                .linkedLibrary("c++"),
                .linkedLibrary("Network"),
                .linkedFramework("SystemConfiguration"),
            ]
        ),
        // Local xcframework
        .binaryTarget(
            name: "Envoy",
            path: "Envoy.xcframework"
        ),
        // GitHub Releases xcframework
        // .binaryTarget(
        //     name: "Envoy",
        //     url: "https://github.com/envoyproxy/envoy-mobile/releases/download/v0.4.6.20220530/Envoy.xcframework.zip",
        //     checksum: "b73ce19128f314a43bce735f057d7ae57eef92e646d1df93723e8c71a86e5479"
        // ),
    ]
)
