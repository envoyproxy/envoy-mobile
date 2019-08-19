import Envoy
import UIKit

private let kCellID = "cell-id"
private let kRequestAuthority = "s3.amazonaws.com"
private let kRequestPath = "api.lyft.com/static/demo/hello_world.txt"

final class ViewController: UITableViewController {
  private let envoy = try! EnvoyBuilder().build()
  private var requestCount = 0
  private var results = [Result<Response, RequestError>]()
  private var timer: Timer?

  override func viewDidLoad() {
    super.viewDidLoad()
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
    let handler = ResponseHandler()
      .onHeaders { headers, statusCode, _ in
        NSLog("Response:\nStatus: \(statusCode)\n\(headers)")
      }

    let stream = self.envoy.startStream(with: request, handler: handler)
    _ = stream // Not sending additional data up the stream
  }

  private func handle(response: URLResponse?, with data: Data?, error: Error?, id: Int) {
    if let error = error {
      return self.add(result: .failure(RequestError(id: id, message: "\(error)")))
    }

    guard let response = response as? HTTPURLResponse, let data = data else {
      return self.add(result: .failure(RequestError(id: id, message: "missing response data")))
    }

    guard response.statusCode == 200 else {
      let error = RequestError(id: id, message: "failed with status \(response.statusCode)")
      return self.add(result: .failure(error))
    }

    guard let body = String(data: data, encoding: .utf8) else {
      return self.add(result: .failure(RequestError(id: id, message: "failed to deserialize body")))
    }

    let untypedHeaders = response.allHeaderFields
    let headers = Dictionary(uniqueKeysWithValues: untypedHeaders.map
    { header -> (String, String) in
      return (header.key as? String ?? String(describing: header.key), "\(header.value)")
    })

    // Deserialize the response, which will include a `Server` header set by Envoy.
    NSLog("Response:\n\(data.count) bytes\n\(body)\n\(headers)")
    self.add(result: .success(Response(id: id, body: body, serverHeader: headers["Server"] ?? "")))
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
