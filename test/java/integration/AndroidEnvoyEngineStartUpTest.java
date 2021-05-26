package test.kotlin.integration;

import android.content.Context;
import java.io.IOException;
import androidx.test.core.app.ApplicationProvider;
import io.envoyproxy.envoymobile.AndroidEngineBuilder;
import io.envoyproxy.envoymobile.Engine;
import io.envoyproxy.envoymobile.engine.AndroidJniLibrary;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.robolectric.RobolectricTestRunner;
import test.kotlin.integration.TestStatsdServer;

import static org.assertj.core.api.Assertions.assertThat;

// NOLINT(namespace-envoy)

@RunWith(RobolectricTestRunner.class)
public class AndroidEnvoyEngineStartUpTest {
  static { AndroidJniLibrary.loadTestLibrary(); }

  private final Context appContext = ApplicationProvider.getApplicationContext();

  @Test
  public void ensure_engine_starts_and_terminates() throws InterruptedException, IOException {
    TestStatsdServer server = new TestStatsdServer();
    server.runAsync(5555);

    Engine engine = new AndroidEngineBuilder(appContext).addStatsdPort(5555).build();

    // Sleep until the server boots.
    // TODO(snowp): We can probably do better here, block on stat?
    Thread.sleep(2);
    engine.flushStats();

    String packet = server.awaitNextPacket();

    assertThat(packet).contains("envoy.pulse.android_permissions.network_state_denied:1|c");
    engine.terminate();
    assertThat(true).isTrue();
  }
}
