#pragma once

// NOLINT(namespace-envoy)

// Mimics android/log.h - tests only.

#define LOG_BUF_SIZE 1024

#define ANDROID_LOG_DEFAULT 1
#define ANDROID_LOG_VERBOSE 2
#define ANDROID_LOG_DEBUG 3
#define ANDROID_LOG_INFO 4
#define ANDROID_LOG_WARN 5
#define ANDROID_LOG_ERROR 6
#define ANDROID_LOG_FATAL 7

int __android_log_write(int prio, const char *tag, const char *text);

int __android_log_print(int prio, const char *tag, const char *fmt, ...);
