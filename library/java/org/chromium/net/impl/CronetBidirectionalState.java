package org.chromium.net.impl;

import androidx.annotation.IntDef;

import org.chromium.net.RequestFinishedInfo;
import org.chromium.net.impl.RequestFinishedInfoImpl.FinishedReason;

import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.util.concurrent.atomic.AtomicInteger;

/**
 *  Holder the the current state associated to a bidirectional stream. The main goal is to provide
 *  a mean to determine what should be the next action for a given event by considering the
 *  current state. This class uses "CAS" logic (https://en.wikipedia.org/wiki/Compare-and-swap): the
 *  next state is saved with {@code AtomicInteger.compareAndSet()}.
 *
 *  <p>All methods in this class are Thread Safe.
 */
final class CronetBidirectionalState {

  /**
   * Enum of the events altering the global state. There are 3 types of events: User induced
   * (prefixed with USER_), EM Callbacks (prefixed with ON_), and internal events (the remaining
   * ones).
   */
  @IntDef({Event.USER_START,
           Event.USER_START_WITH_HEADERS,
           Event.USER_START_READ_ONLY,
           Event.USER_START_WITH_HEADERS_READ_ONLY,
           Event.USER_WRITE,
           Event.USER_LAST_WRITE,
           Event.USER_FLUSH,
           Event.USER_READ,
           Event.USER_CANCEL,
           Event.ERROR,
           Event.READY_TO_FLUSH,
           Event.FLUSH_DATA_COMPLETED,
           Event.LAST_FLUSH_DATA_COMPLETED,
           Event.WRITE_COMPLETED,
           Event.READ_COMPLETED,
           Event.LAST_WRITE_COMPLETED,
           Event.LAST_READ_COMPLETED,
           Event.READY_TO_FINISH,
           Event.ON_HEADERS,
           Event.ON_HEADERS_END_STREAM,
           Event.ON_DATA,
           Event.ON_DATA_END_STREAM,
           Event.ON_COMPLETE,
           Event.ON_CANCEL,
           Event.ON_ERROR})
  @Retention(RetentionPolicy.SOURCE)
  @interface Event {
    int USER_START = 0; // Ready. Don't send request headers yet. There will be a request body.
    int USER_START_WITH_HEADERS = 1; // Ready to send request headers. There will be a request body.
    int USER_START_READ_ONLY = 2;    // Ready. Don't send request headers yet. No request body.
    int USER_START_WITH_HEADERS_READ_ONLY = 3; // Ready to send request headers. No request body.
    int USER_WRITE = 4;      // User adding a ByteBuffer in the pending queue - not the last one.
    int USER_LAST_WRITE = 5; // User adding a ByteBuffer in the pending queue - that's the last one.
    int USER_FLUSH = 6;      // User requesting to push the pending buffers/headers on the wire.
    int USER_READ = 7;       // User requesting to read the next chunk from the wire.
    int USER_CANCEL = 8;     // User requesting to cancel the stream.
    int ERROR = 9;           // A fatal error occurred. Can be an internal, or user related.
    int READY_TO_FLUSH = 10; // Internal Event indicating readiness to write the next ByteBuffer.
    int FLUSH_DATA_COMPLETED = 11;      // Internal event indicating that a write completed.
    int LAST_FLUSH_DATA_COMPLETED = 12; // Internal event indicating that the final write completed.
    int WRITE_COMPLETED = 13; // Internal event indicating to tell the user about a completed write.
    int READ_COMPLETED = 14;  // Internal event indicating to tell the user about a completed read.
    int LAST_WRITE_COMPLETED = 15;  // Internal event indicating to tell the user about final write.
    int LAST_READ_COMPLETED = 16;   // Internal event indicating to tell the user about final read.
    int READY_TO_FINISH = 17;       // Internal event indicating to tell the user about success.
    int ON_HEADERS = 18;            // EM invoked the "onHeaders" callback - response body to come.
    int ON_HEADERS_END_STREAM = 19; // EM invoked the "onHeaders" callback - no response body.
    int ON_DATA = 20;            // EM invoked the "onData" callback - not last "onData" callback.
    int ON_DATA_END_STREAM = 21; // EM invoked the "onData" callback - final "onData" callback.
    int ON_COMPLETE = 22;        // EM invoked the "onComplete" callback.
    int ON_CANCEL = 23;          // EM invoked the "onCancel" callback.
    int ON_ERROR = 24;           // EM invoked the "onError" callback.
  }

  /**
   * Enum of the Next Actions to be taken.
   */
  @IntDef({NextAction.CARRY_ON, NextAction.WRITE, NextAction.FLUSH_HEADERS, NextAction.SEND_DATA,
           NextAction.READ, NextAction.INVOKE_ON_READ_COMPLETED,
           NextAction.INVOKE_ON_ERROR_RECEIVED, NextAction.CANCEL,
           NextAction.INVOKE_ON_WRITE_COMPLETED_CALLBACK,
           NextAction.INVOKE_ON_READ_COMPLETED_CALLBACK, NextAction.FINISH_UP,
           NextAction.PROCESS_ERROR, NextAction.PROCESS_CANCEL, NextAction.TAKE_NO_MORE_ACTIONS})
  @Retention(RetentionPolicy.SOURCE)
  @interface NextAction {
    int CARRY_ON = 0;                 // Do nothing special at the moment - keep calm and carry on.
    int WRITE = 1;                    // Add one more ByteBuffer to the pending queue.
    int FLUSH_HEADERS = 2;            // Start sending request headers.
    int SEND_DATA = 3;                // Send one ByteBuffer on the wire, if any.
    int READ = 4;                     // Start reading the next chunk of the response body.
    int INVOKE_ON_READ_COMPLETED = 5; // Initiate the completion of a read operation.
    int INVOKE_ON_ERROR_RECEIVED = 6; // Initiate the completion of a network Error.
    int CANCEL = 7;                   // Tell EM to cancel. Can be an user induced, or due to error.
    int INVOKE_ON_WRITE_COMPLETED_CALLBACK = 8; // Tell the User that a write operation completed.
    int INVOKE_ON_READ_COMPLETED_CALLBACK = 9;  // Tell the User that a read operation completed.
    int FINISH_UP = 10;     // Tell the User that the stream is done and was completed successfully.
    int PROCESS_ERROR = 11; // Tell the User the stream completed in error.
    int PROCESS_CANCEL = 12;       // Tell the User the stream completed in a cancelled state.
    int TAKE_NO_MORE_ACTIONS = 13; // The stream is already in error state - don't do anything else.
  }

  /**
   * Bitmap used to express the global state of the BIDI Stream. Each bit represent one element of
   * the global state.
   */
  @IntDef(flag = true, // Not used as an Enum, and this is not used as the argument of a "switch".
          value = {State.NOT_STARTED, State.STARTED, State.WAITING_FOR_FLUSH,
                   State.WAITING_FOR_READ, State.END_STREAM_WRITTEN, State.END_STREAM_READ,
                   State.WRITING, State.READING, State.HEADERS_SENT, State.CANCELLING,
                   State.USER_CANCELLED, State.FAILED, State.ON_HEADER_RECEIVED,
                   State.ON_COMPLETE_RECEIVED, State.READ_DONE, State.WRITE_DONE, State.DONE,
                   State.TERMINATING_STATES})
  @Retention(RetentionPolicy.SOURCE)
  private @interface State {
    int NOT_STARTED = 0;                // Initial state.
    int STARTED = 1;                    // Started.
    int WAITING_FOR_FLUSH = 1 << 1;     // User is expected to invoke "flush" at one point.
    int WAITING_FOR_READ = 1 << 2;      // User is expected to invoke "read" at one point.
    int END_STREAM_WRITTEN = 1 << 3;    // User can't invoke "write" anymore. Maybe never could.
    int END_STREAM_READ = 1 << 4;       // EM will not invoke the "onData" callback anymore.
    int WRITING = 1 << 5;               // One RequestBody's Buffer is being sent on the wire.
    int READING = 1 << 6;               // One ResponseBody's Buffer is being read from the wire.
    int HEADERS_SENT = 1 << 7;          // EM's "sendHeaders" method has been invoked.
    int CANCELLING = 1 << 8;            // EM's "cancel" method has been invoked.
    int USER_CANCELLED = 1 << 9;        // The cancel operation was initiated by the User.
    int FAILED = 1 << 10;               // An fatal failure has been encountered.
    int ON_HEADER_RECEIVED = 1 << 11;   // EM's "onHeaders" callback has been invoked.
    int ON_COMPLETE_RECEIVED = 1 << 12; // EM's "onComplete" callback has been invoked.
    int READ_DONE = 1 << 13;            // User won't receive more read callbacks.
    int WRITE_DONE = 1 << 14;           // User won't receive more write callbacks. Maybe never had.
    int DONE = 1 << 15;                 // Terminal state. Can be successful or otherwise.
    int TERMINATING_STATES = CANCELLING | FAILED | DONE; // Hold your breath and count to ten.
  }

  private final AtomicInteger mState = new AtomicInteger(State.NOT_STARTED);

  /**
   * Returns true if the final state has been reached. At this point the EM Stream has been
   * destroyed.
   */
  boolean isDone() { return (mState.get() & State.DONE) != 0; }

  /**
   * Returns true if a terminating state has been reached. Terminating does not necessarily means
   * that the DONE state has been reached. When the DONE bit is not set, it means that we are not
   * ready yet to inform the user about the failure, as the EM as not yet destroyed the Stream. In
   * other words, EM has not yet invoked a terminal callback (onError, onCancel, onComplete).
   */
  boolean isTerminating() { return (mState.get() & State.TERMINATING_STATES) != 0; }

  /**
   * Returns the reason why the request finished. Can only be invoked if {@link #isDone} returns
   * true.
   *
   * @return one of {@link RequestFinishedInfo#SUCCEEDED}, {@link RequestFinishedInfo#FAILED}, or
   *     {@link RequestFinishedInfo#CANCELED}
   */
  @FinishedReason
  int getFinishedReason() {
    assert isDone();
    @State int finalState = mState.get();
    if ((finalState & State.FAILED) != 0) {
      return RequestFinishedInfo.FAILED;
    }
    if ((finalState & State.USER_CANCELLED) != 0) {
      return RequestFinishedInfo.CANCELED;
    }
    return RequestFinishedInfo.SUCCEEDED;
  }

  /**
   * Establishes what is the next action by taking in account the current global state, and the
   * provided {@link Event}. This method has one important side effect: the resulting global state
   * is saved through an Atomic operation. For few cases, this method will throw when the state is
   * not compatible with the event.
   */
  @NextAction int nextAction(@Event final int event) { // "final" just to avoid dumb mistakes.
    while (true) {
      @State final int originalState = mState.get(); // "final" just to avoid dumb mistakes.

      // Some events must fail immediately when the original state does not permit.
      // This mimics Cronet's behaviour: identical Exception types and error messages.
      switch (event) {
      case Event.USER_START:
      case Event.USER_START_WITH_HEADERS:
      case Event.USER_START_READ_ONLY:
      case Event.USER_START_WITH_HEADERS_READ_ONLY:
        if ((originalState & (State.STARTED | State.TERMINATING_STATES)) != 0) {
          throw new IllegalStateException("Stream is already started.");
        }
        break;

      case Event.USER_LAST_WRITE:
      case Event.USER_WRITE:
        if ((originalState & State.END_STREAM_WRITTEN) != 0) {
          throw new IllegalArgumentException("Write after writing end of stream.");
        }
        break;

      case Event.USER_READ:
        if ((originalState & State.WAITING_FOR_READ) == 0) {
          throw new IllegalStateException("Unexpected read attempt.");
        }
        break;

      default:
        // For all other events, a potentially incompatible state does not trigger an Exception.
      }

      // Those 3 events are the final events from the EnvoyMobile C++ layer.
      if (event == Event.ON_CANCEL || event == Event.ON_ERROR || event == Event.ON_COMPLETE) {
        // If this assert triggers it means that the C++ EnvoyMobile contract has been breached.
        assert (originalState & State.DONE) == 0; // Or there is a blatant bug.
      } else if ((originalState & State.TERMINATING_STATES) != 0) {
        return NextAction.TAKE_NO_MORE_ACTIONS; // No need to loop - this is irreversible.
      }

      @NextAction final int nextAction; // "final" guarantees that it is assigned exactly once.
      @State int nextState = originalState;
      switch (event) {
      case Event.USER_START:
      case Event.USER_START_WITH_HEADERS:
      case Event.USER_START_READ_ONLY:
      case Event.USER_START_WITH_HEADERS_READ_ONLY:
        nextState |= State.WAITING_FOR_READ | State.STARTED;
        if (event == Event.USER_START_READ_ONLY ||
            event == Event.USER_START_WITH_HEADERS_READ_ONLY) {
          nextState |= State.END_STREAM_WRITTEN | State.WRITE_DONE;
        }
        if (event != Event.USER_START_WITH_HEADERS_READ_ONLY) {
          nextState |= State.WAITING_FOR_FLUSH;
        }
        if (event == Event.USER_START_WITH_HEADERS ||
            event == Event.USER_START_WITH_HEADERS_READ_ONLY) {
          nextState |= State.HEADERS_SENT;
          nextAction = NextAction.FLUSH_HEADERS;
        } else {
          nextAction = NextAction.CARRY_ON;
        }
        break;

      case Event.USER_LAST_WRITE:
        nextState |= State.END_STREAM_WRITTEN;
        // FOLLOW THROUGH
      case Event.USER_WRITE:
        // Note: it is fine to write even before "start" - Cronet behaves the same.
        nextAction = NextAction.WRITE;
        break;

      case Event.USER_FLUSH:
        if ((originalState & State.WAITING_FOR_FLUSH) != 0 &&
            (originalState & State.HEADERS_SENT) == 0) {
          if ((originalState & State.WRITE_DONE) != 0) {
            nextState &= ~State.WAITING_FOR_FLUSH;
          }
          nextState |= State.HEADERS_SENT;
          nextAction = NextAction.FLUSH_HEADERS;
        } else {
          nextAction = NextAction.CARRY_ON;
        }
        break;

      case Event.USER_READ:
        nextState &= ~State.WAITING_FOR_READ;
        nextState |= State.READING;
        if ((originalState & State.ON_HEADER_RECEIVED) == 0) {
          nextAction = NextAction.CARRY_ON; // Read will occur later.
        } else {
          nextAction = (originalState & State.END_STREAM_READ) == 0
                           ? NextAction.READ
                           : NextAction.INVOKE_ON_READ_COMPLETED;
        }
        break;

      case Event.USER_CANCEL:
        if ((originalState & State.STARTED) == 0) {
          nextAction = NextAction.CARRY_ON; // Cancel came too soon - no effect.
        } else if ((originalState & State.ON_COMPLETE_RECEIVED) != 0) {
          nextState |= State.USER_CANCELLED | State.DONE;
          nextAction = NextAction.PROCESS_CANCEL;
        } else {
          nextState |= State.USER_CANCELLED | State.CANCELLING;
          nextAction = NextAction.CANCEL;
        }
        break;

      case Event.ERROR:
        if ((originalState & State.ON_COMPLETE_RECEIVED) != 0 ||
            (originalState & State.STARTED) == 0) {
          nextState |= State.FAILED | State.DONE;
          nextAction = NextAction.PROCESS_ERROR;
        } else {
          nextState |= State.FAILED | State.CANCELLING;
          nextAction = NextAction.CANCEL;
        }
        break;

      case Event.ON_HEADERS_END_STREAM:
        assert (originalState & State.END_STREAM_READ) == 0;
        nextState |= State.ON_HEADER_RECEIVED | State.END_STREAM_READ;
        nextAction = (originalState & State.READING) != 0 ? NextAction.INVOKE_ON_READ_COMPLETED
                                                          : NextAction.CARRY_ON;
        break;

      case Event.ON_HEADERS:
        assert (originalState & State.ON_HEADER_RECEIVED) == 0;
        nextState |= State.ON_HEADER_RECEIVED;
        nextAction = (originalState & State.READING) != 0 ? NextAction.READ : NextAction.CARRY_ON;
        break;

      case Event.ON_DATA_END_STREAM:
        assert (originalState & State.END_STREAM_READ) == 0;
        nextState |= State.END_STREAM_READ;
        // FOLLOW THROUGH
      case Event.ON_DATA:
        assert (originalState & State.WAITING_FOR_READ) == 0;
        nextAction = NextAction.INVOKE_ON_READ_COMPLETED;
        break;

      case Event.ON_COMPLETE:
        assert (originalState & State.ON_COMPLETE_RECEIVED) == 0;
        nextState |= State.ON_COMPLETE_RECEIVED;
        if ((originalState & State.CANCELLING) != 0) {
          nextState |= State.DONE;
          nextAction = (originalState & State.FAILED) != 0 ? NextAction.PROCESS_ERROR
                                                           : NextAction.PROCESS_CANCEL;
        } else if (((originalState & State.WRITE_DONE) != 0 &&
                    (originalState & State.READ_DONE) != 0)) {
          nextState |= State.DONE;
          nextAction = NextAction.FINISH_UP;
        } else {
          nextAction = NextAction.CARRY_ON;
        }
        break;

      case Event.ON_CANCEL:
        nextState |= State.DONE;
        nextAction = ((originalState & State.FAILED) != 0) ? NextAction.PROCESS_ERROR
                                                           : NextAction.PROCESS_CANCEL;
        break;

      case Event.ON_ERROR:
        nextState |= State.DONE | State.FAILED;
        nextAction = ((originalState & State.FAILED) != 0) ? NextAction.PROCESS_ERROR
                                                           : NextAction.INVOKE_ON_ERROR_RECEIVED;
        break;

      case Event.LAST_WRITE_COMPLETED:
        assert (originalState & State.WRITE_DONE) == 0;
        nextState |= State.WRITE_DONE;
        // FOLLOW THROUGH
      case Event.WRITE_COMPLETED:
        nextAction = NextAction.INVOKE_ON_WRITE_COMPLETED_CALLBACK;
        break;

      case Event.READY_TO_FLUSH:
        if ((originalState & State.WAITING_FOR_FLUSH) == 0) {
          nextAction = NextAction.CARRY_ON;
        } else {
          nextState &= ~State.WAITING_FOR_FLUSH;
          nextState |= State.WRITING;
          nextAction = NextAction.SEND_DATA;
        }
        break;

      case Event.FLUSH_DATA_COMPLETED:
        nextState |= State.WAITING_FOR_FLUSH;
        // FOLLOW THROUGH
      case Event.LAST_FLUSH_DATA_COMPLETED:
        assert (originalState & State.WRITING) != 0;
        assert (originalState & State.WAITING_FOR_FLUSH) == 0;
        nextState &= ~State.WRITING;
        nextAction = NextAction.CARRY_ON;
        break;

      case Event.READ_COMPLETED:
        assert (originalState & State.READING) != 0;
        nextState &= ~State.READING;
        nextState |= State.WAITING_FOR_READ;
        nextAction = NextAction.INVOKE_ON_READ_COMPLETED_CALLBACK;
        break;

      case Event.LAST_READ_COMPLETED:
        assert (originalState & State.READ_DONE) == 0;
        nextState &= ~State.READING;
        nextState |= State.READ_DONE;
        nextAction = NextAction.INVOKE_ON_READ_COMPLETED_CALLBACK;
        break;

      case Event.READY_TO_FINISH:
        if ((originalState & State.ON_COMPLETE_RECEIVED) != 0 &&
            (originalState & State.READ_DONE) != 0 && (originalState & State.WRITE_DONE) != 0) {
          nextState |= State.DONE;
          nextAction = NextAction.FINISH_UP;
        } else {
          nextAction = NextAction.CARRY_ON;
        }
        break;

      default:
        throw new AssertionError("switch is exhaustive");
      }

      if (mState.compareAndSet(originalState, nextState)) {
        return nextAction;
      }
    }
  }
}
