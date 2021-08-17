package org.chromium.net.impl;

import org.chromium.net.CallbackException;

/** An implementation of {@link CallbackException}. */
public final class CallbackExceptionImpl extends CallbackException {
  CallbackExceptionImpl(String message, Throwable cause) { super(message, cause); }
}
