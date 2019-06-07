package io.envoyproxy.envoymobile.helloenvoy;

import android.app.Activity;
import android.content.Context;
import android.os.Bundle;
import android.os.Handler;
import android.os.HandlerThread;
import android.support.v7.widget.DividerItemDecoration;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.util.Log;

import java.io.BufferedReader;
import java.io.BufferedInputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.net.HttpURLConnection;
import java.net.URL;
import java.util.List;
import java.util.Map;
import java.util.concurrent.TimeUnit;

import io.envoyproxy.envoymobile.Envoy;

public class MainActivity extends Activity {
  private static final String ENDPOINT =
      "http://0.0.0.0:9001/api.lyft.com/static/demo/hello_world.txt";
  private RecyclerView recyclerView;

  @Override
  protected void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    setContentView(R.layout.activity_main);

    Envoy envoy = new Envoy();
    envoy.load();
    String config = null;
    try {
      config = loadEnvoyConfig(getBaseContext(), R.raw.config);
    } catch (RuntimeException e) {
      Log.d("MainActivity", "exception getting config.", e);
      throw new RuntimeException("Can't get config to run envoy.");
    }
    envoy.run(getBaseContext(), config);

    recyclerView = (RecyclerView)findViewById(R.id.recycler_view);
    recyclerView.setLayoutManager(new LinearLayoutManager(this));

    final ResponseRecyclerViewAdapter adapter = new ResponseRecyclerViewAdapter();
    recyclerView.setAdapter(adapter);
    DividerItemDecoration dividerItemDecoration =
        new DividerItemDecoration(recyclerView.getContext(), DividerItemDecoration.VERTICAL);
    recyclerView.addItemDecoration(dividerItemDecoration);
    HandlerThread thread = new HandlerThread("");
    thread.start();
    final Handler handler = new Handler(thread.getLooper());
    handler.postDelayed(new Runnable() {
      @Override
      public void run() {
        try {
          final Response response = makeRequest();
          recyclerView.post((Runnable)() -> adapter.add(response));
        } catch (IOException e) {
          Log.d("MainActivity", "exception making request.", e);
        }
        // Make a call again
        handler.postDelayed(this, TimeUnit.SECONDS.toMillis(1));
      }
    }, 0);
  }

  private Response makeRequest() throws IOException {
    URL url = new URL(ENDPOINT);
    HttpURLConnection connection = (HttpURLConnection)url.openConnection();
    int status = connection.getResponseCode();
    if (status != 200) {
      throw new IOException("non 200 status: " + status);
    }

    List<String> serverHeaderField = connection.getHeaderFields().get("server");
    InputStream inputStream = connection.getInputStream();
    String body = deserialize(inputStream);
    inputStream.close();
    return new Response(body,
                        serverHeaderField != null ? String.join(", ", serverHeaderField) : "");
  }

  private String deserialize(InputStream inputStream) throws IOException {
    StringBuilder stringBuilder = new StringBuilder();
    BufferedReader bufferedReader = new BufferedReader(new InputStreamReader(inputStream));
    String line = bufferedReader.readLine();
    while (line != null) {
      stringBuilder.append(line);
      line = bufferedReader.readLine();
    }
    bufferedReader.close();
    return stringBuilder.toString();
  }

  private String loadEnvoyConfig(Context context, int configResourceId) throws RuntimeException {
    InputStream inputStream = context.getResources().openRawResource(configResourceId);
    InputStreamReader inputReader = new InputStreamReader(inputStream);
    BufferedReader bufReader = new BufferedReader(inputReader);
    StringBuilder text = new StringBuilder();

    try {
      String line;
      while ((line = bufReader.readLine()) != null) {
        text.append(line);
        text.append('\n');
      }
    } catch (IOException e) {
      return null;
    }
    return text.toString();
  }
}
