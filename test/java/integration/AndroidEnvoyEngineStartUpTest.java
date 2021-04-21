package test.kotlin.integration;

import android.content.Context;
import androidx.test.core.app.ApplicationProvider;
import io.envoyproxy.envoymobile.AndroidEngineBuilder;
import io.envoyproxy.envoymobile.Engine;
import io.envoyproxy.envoymobile.engine.AndroidJniLibrary;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.robolectric.RobolectricTestRunner;

import static org.assertj.core.api.Assertions.assertThat;

// NOLINT(namespace-envoy)

@RunWith(RobolectricTestRunner.class)
public class AndroidEnvoyEngineStartUpTest {
  static {
    System.out.println("~~~~~~~~~~~~~~~~~");
    System.out.println(System.getProperty("user.dir"));
    System.out.println("~~~~~~~~~~~~~~~~~");
    AndroidJniLibrary.loadTestLibrary();
  }

  private final Context appContext = ApplicationProvider.getApplicationContext();

  @Test
  public void ensure_engine_starts_and_terminates() throws InterruptedException {
//    Engine engine = new AndroidEngineBuilder(appContext).build();
//    Thread.sleep(1000);
//    engine.terminate();
    assertThat(true).isFalse();
  }
}
