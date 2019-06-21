import Envoy
import UIKit

private enum ConfigLoadError: Error {
  case noFileAtPath
}

@UIApplicationMain
final class AppDelegate: UIResponder, UIApplicationDelegate {
  var envoy: Envoy?
  var window: UIWindow?

  func application(
    _ application: UIApplication,
    didFinishLaunchingWithOptions launchOptions: [UIApplication.LaunchOptionsKey: Any]?) -> Bool
  {
    let configYaml = try! self.loadEnvoyConfig()
    NSLog("Loading config:\n\(configYaml)")
    self.envoy = Envoy(config: configYaml)

    let window = UIWindow(frame: UIScreen.main.bounds)
    window.rootViewController = ViewController()
    window.makeKeyAndVisible()
    self.window = window
    NSLog("Finished launching!")

    return true
  }

  private func loadEnvoyConfig() throws -> String {
    guard let configFile = Bundle.main.path(forResource: "config", ofType: "yaml") else {
      throw ConfigLoadError.noFileAtPath
    }

    return try String(contentsOfFile: configFile, encoding: .utf8)
  }
}
