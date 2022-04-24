package org.chromium.net.impl;

import static org.assertj.core.api.Assertions.assertThat;

import androidx.test.ext.junit.runners.AndroidJUnit4;
import java.util.concurrent.atomic.AtomicInteger;
import org.chromium.net.testing.ConditionVariable;
import org.junit.Test;
import org.junit.runner.RunWith;

@RunWith(AndroidJUnit4.class)
public class AtomicCombinatoryStateTest {

  @Test
  public void trivialCase_false() {
    assertThat(new AtomicCombinatoryState(1).hasReachedFinalState(0)).isFalse();
  }

  @Test
  public void trivialCase_true() {
    assertThat(new AtomicCombinatoryState(1).hasReachedFinalState(1)).isTrue();
  }

  @Test
  public void partialState() {
    assertThat(new AtomicCombinatoryState(3).hasReachedFinalState(1)).isFalse();
  }

  @Test
  public void finalState() {
    AtomicCombinatoryState atomicCombinatoryState = new AtomicCombinatoryState(3);
    atomicCombinatoryState.hasReachedFinalState(2);
    assertThat(atomicCombinatoryState.hasReachedFinalState(1)).isTrue();
  }

  @Test
  public void finalState_twice() {
    AtomicCombinatoryState atomicCombinatoryState = new AtomicCombinatoryState(3);
    atomicCombinatoryState.hasReachedFinalState(2);
    atomicCombinatoryState.hasReachedFinalState(1);
    assertThat(atomicCombinatoryState.hasReachedFinalState(1)).isFalse();
  }

  @Test
  public void finalState_multiThread() throws Exception {
    ConditionVariable startBlock = new ConditionVariable();
    AtomicCombinatoryState atomicCombinatoryState = new AtomicCombinatoryState(3);
    AtomicInteger trueCount = new AtomicInteger(0);
    AtomicInteger eventStatePurveyor = new AtomicInteger(0);
    Thread[] threads = new Thread[10];
    for (int i = 0; i < threads.length; i++) {
      threads[i] = new Thread() {
        @Override
        public void run() {
          int eventState = (eventStatePurveyor.incrementAndGet() & 1) + 1; // 1 and 2 only
          startBlock.block();
          if (atomicCombinatoryState.hasReachedFinalState(eventState)) {
            trueCount.incrementAndGet(); // Should be executed only once.
          }
        }
      };
      threads[i].start();
    }
    Thread.sleep(100); // Should be good enough so all 10 Threads are currently blocking.
    startBlock.open(); // Most threads will unblock simultaneously on a "multi-threading" CPU.
    for (Thread thread : threads) {
      thread.join(); // Wait for each Thread to die.
    }
    assertThat(trueCount.get()).isEqualTo(1);
  }
}
