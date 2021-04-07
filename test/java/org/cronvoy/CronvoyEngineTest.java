package org.cronvoy;

import android.content.Context;
import androidx.test.core.app.ApplicationProvider;
import org.robolectric.RobolectricTestRunner;
import org.junit.Test;
import org.junit.runner.RunWith;

@RunWith(RobolectricTestRunner.class)
public class CronvoyEngineTest {

  private final Context appContext = ApplicationProvider.getApplicationContext();

  @Test
  public void test() {
    System.out.println("Creating CronvoyEngine...");
    new CronvoyEngine(new CronvoyEngineBuilderImpl(appContext));
    System.out.println("CronvoyEngine created.");
  }
}
