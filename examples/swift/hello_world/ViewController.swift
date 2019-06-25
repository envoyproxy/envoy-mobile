import UIKit

private let kCellID = "cell-id"
// swiftlint:disable:next force_unwrapping
private let kURL = URL(string: "http://localhost:9001/api.lyft.com/static/demo/hello_world.txt")!

final class ViewController: UITableViewController {
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
    // Note that the request is sent to the envoy thread listening locally on port 9001.
    let request = URLRequest(url: kURL)
    NSLog("Starting request to '\(kURL.path)'")
    let task = URLSession.shared.dataTask(with: request) { [weak self] data, response, error in
      self?.handle(response: response, with: data, error: error)
    }

    task.resume()
  }

  private func handle(response: URLResponse?, with data: Data?, error: Error?) {
    if let error = error {
      return self.add(result: .failure(RequestError(description: "\(error)")))
    }

    guard let response = response as? HTTPURLResponse, let data = data else {
      return self.add(result: .failure(RequestError(description: "Missing response data")))
    } 

    guard response.statusCode == 200 else {
      return self.add(result: .failure(RequestError(description: "failed with status \(response.statusCode)")))
    }

    guard let body = String(data: data, encoding: .utf8) else {
      return self.add(result: .failure(RequestError(description: "Failed to deserialize body")))
    }

    let untypedHeaders = response.allHeaderFields
    let headers = Dictionary(uniqueKeysWithValues: untypedHeaders.map
    { header -> (String, String) in
      return (header.key as? String ?? String(describing: header.key), "\(header.value)")
    })

    NSLog("Response:\n\(data.count) bytes\n\(body)\n\(headers)")

    // Deserialize the response, which will include a `Server` header set by Envoy.
    let value = Response(body: body, serverHeader: headers["Server"] ?? "")
    self.add(result: .success(value))
  }
  
  private func add(result: Result<Response, RequestError>) {
    DispatchQueue.main.async {
      self.results.insert(result, at: 0)
      self.tableView.reloadData()
    }
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
        cell.textLabel?.text = "[\(indexPath.row + 1)] Response: \(response.body)"
        cell.detailTextLabel?.text = "'Server' header: \(response.serverHeader)"
        cell.textLabel?.textColor = .black
        cell.contentView.backgroundColor = .white
      case .failure(let error):
        cell.textLabel?.text = "[\(indexPath.row + 1)] \(error.description)"
        cell.detailTextLabel?.text = nil
        cell.textLabel?.textColor = .white
        cell.contentView.backgroundColor = .red
    }
    return cell
  }
}
