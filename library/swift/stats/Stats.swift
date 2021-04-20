import Foundation

struct Stats {
  /// Convert tags.
 static func convert(tags: [Tag]) -> [String: String] {
    var tagMap = [String: String]()
    for tag in tags {
        tagMap[tag.key] = tag.value
    }
    return tagMap
  }
}
