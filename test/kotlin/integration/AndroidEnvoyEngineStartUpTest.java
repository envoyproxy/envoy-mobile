package test.kotlin.integration;

import android.content.Context;
import androidx.test.core.app.ApplicationProvider;
import io.envoyproxy.envoymobile.AndroidEngineBuilder;
import io.envoyproxy.envoymobile.Engine;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.robolectric.RobolectricTestRunner;

@RunWith(RobolectricTestRunner.class)
public class AndroidEnvoyEngineStartUpTest {
    static  {
        System.out.println("~~~~~~~~~~~~~~~~");
        System.out.println(System.getProperty("xjnilibname"));
        System.out.println(System.getProperty("user.dir"));
        System.out.println("~~~~~~~~~~~~~~~~");
    }
    private final Context appContext = ApplicationProvider.getApplicationContext();

    @Test
    public void ensure_engine_starts_and_terminates() throws InterruptedException {
        Engine engine = new AndroidEngineBuilder(appContext).build();
        Thread.sleep(1000);
        engine.terminate();
    }
}
