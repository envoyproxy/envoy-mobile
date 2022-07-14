struct ProxySettings {
  static ProxySettings empty() { return ProxySettings{"", ""}; }
  bool isEmpty() { return hostname_.empty() && address_.empty(); }

  std::string hostname_;
  std::string address_;
};
