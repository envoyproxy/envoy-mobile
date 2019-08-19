import Envoy
import UIKit

private let kCellID = "cell-id"
private let kRequestAuthority = "s3.amazonaws.com"
private let kRequestPath = "/api.lyft.com/static/demo/hello_world.txt"

final class ViewController: UITableViewController {
  private var envoy: Envoy?
  private var requestCount = 0
  private var results = [Result<Response, RequestError>]()
  private var timer: Timer?

  override func viewDidLoad() {
    super.viewDidLoad()
    do {
      self.envoy = try EnvoyBuilder()
        .addLogLevel(.trace)
        .build()
    } catch let error {
      NSLog("Starting Envoy failed: \(error)")
    }

    self.startRequests()
  }

  deinit {
    self.timer?.invalidate()
  }

  // MARK: - Requests

  private func startRequests() {
    // Note that the first delay will give Envoy time to start up.
    self.timer = .scheduledTimer(withTimeInterval: 1.0, repeats: true) { [weak self] _ in
      self?.performRequest()
    }
  }

  private func performRequest() {
    self.requestCount += 1
    let requestID = self.requestCount

    NSLog("Starting request to '\(kRequestPath)'")
    let request = RequestBuilder(method: .get, scheme: "http", authority: kRequestAuthority,
                                 path: kRequestPath).build()
    var status = -1
    let handler = ResponseHandler()
      .onHeaders { [weak self] headers, statusCode, _ in
        status = statusCode
        NSLog("Response headers (\(requestID)):\nStatus: \(statusCode)\n\(headers)")
        self?.add(result: .success(Response(id: requestID, body: "",
                                            serverHeader: headers["Server"]?.first ?? "")))
      }
      .onData { data, _ in
        NSLog("Response data (\(requestID)): \(data.count) bytes")
      }
      .onError { [weak self] in
        NSLog("Error (\(requestID)) - request failed")
        self?.add(result: .failure(RequestError(id: requestID, message: "failed, status: \(status)")))
    }

    let stream = self.envoy?.startStream(with: request, handler: handler)
    _ = stream // Not sending additional data up the stream
  }

  private func add(result: Result<Response, RequestError>) {
    self.results.insert(result, at: 0)
    self.tableView.reloadData()
  }

  // MARK: - UITableView

  override func numberOfSections(in tableView: UITableView) -> Int {
    return 1
  }

  override func tableView(_ tableView: UITableView, numberOfRowsInSection section: Int) -> Int {
    return self.results.count
  }

  override func tableView(_ tableView: UITableView, cellForRowAt indexPath: IndexPath)
    -> UITableViewCell
  {
    let cell = tableView.dequeueReusableCell(withIdentifier: kCellID) ??
      UITableViewCell(style: .subtitle, reuseIdentifier: kCellID)

    let result = self.results[indexPath.row]
    switch result {
    case .success(let response):
      cell.textLabel?.text = "[\(response.id)] \(response.body)"
      cell.detailTextLabel?.text = "'Server' header: \(response.serverHeader)"

      cell.textLabel?.textColor = .black
      cell.detailTextLabel?.textColor = .black
      cell.contentView.backgroundColor = .white

    case .failure(let error):
      cell.textLabel?.text = "[\(error.id)]"
      cell.detailTextLabel?.text = error.message

      cell.textLabel?.textColor = .white
      cell.detailTextLabel?.textColor = .white
      cell.contentView.backgroundColor = .red
    }

    return cell
  }
}
