#include "library/common/jni/android_log.h"

#include <iostream>
#include <stdarg.h>

// NOLINT(namespace-envoy)

// Log methods for tests only.

int __android_log_write(int prio, const char *tag, const char *text) {
  std::cout << "prio: " << prio << " tag: " << tag << " text: " << text << "\n";
  return 0;
}

int __android_log_print(int prio, const char *tag, const char *fmt, ...) {
  va_list ap;
  char buf[LOG_BUF_SIZE];
  va_start(ap, fmt);
  vsnprintf(buf, LOG_BUF_SIZE, fmt, ap);
  va_end(ap);
  return __android_log_write(prio, tag, buf);
}
