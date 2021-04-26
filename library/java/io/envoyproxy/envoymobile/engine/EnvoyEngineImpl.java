package io.envoyproxy.envoymobile.engine;

import io.envoyproxy.envoymobile.engine.types.EnvoyHTTPCallbacks;
import io.envoyproxy.envoymobile.engine.types.EnvoyHTTPFilterFactory;
import io.envoyproxy.envoymobile.engine.types.EnvoyOnEngineRunning;
import io.envoyproxy.envoymobile.engine.types.EnvoyStringAccessor;

import java.util.Map;

/* Concrete implementation of the `EnvoyEngine` interface. */
public class EnvoyEngineImpl implements EnvoyEngine {
  // TODO(goaway): enforce agreement values in /library/common/types/c_types.h.
  private static final int ENVOY_SUCCESS = 0;
  private static final int ENVOY_FAILURE = 1;

  private final long engineHandle;
  private EnvoyOnEngineRunning onEngineRunning = () -> { return null; };

  public EnvoyEngineImpl() {
    JniLibrary.load();
    this.engineHandle = JniLibrary.initEngine();
  }

  /**
   * Creates a new stream with the provided callbacks.
   *
   * @param callbacks The callbacks for the stream.
   * @return A stream that may be used for sending data.
   */
  @Override
  public EnvoyHTTPStream startStream(EnvoyHTTPCallbacks callbacks) {
    long streamHandle = JniLibrary.initStream(engineHandle);
    EnvoyHTTPStream stream = new EnvoyHTTPStream(streamHandle, callbacks);
    stream.start();
    return stream;
  }

  @Override
  public void terminate() {
    JniLibrary.terminateEngine(engineHandle);
  }

  /**
   * Run the Envoy engine with the provided yaml string and log level.
   *
   * @param configurationYAML The configuration yaml with which to start Envoy.
   * @param logLevel           The log level to use when starting Envoy.
   * @param onEngineRunning    Called when the engine finishes its async startup and begins running.
   * @return A status indicating if the action was successful.
   */
  @Override
  public int runWithConfig(String configurationYAML, String logLevel,
                           EnvoyOnEngineRunning onEngineRunning) {
    this.onEngineRunning = onEngineRunning;
    try {
      return JniLibrary.runEngine(this.engineHandle, configurationYAML, logLevel,
                                  this.onEngineRunning);
    } catch (Throwable throwable) {
      // TODO: Need to have a way to log the exception somewhere.
      return ENVOY_FAILURE;
    }
  }

  /**
   * Run the Envoy engine with the provided envoyConfiguration and log level.
   *
   * @param envoyConfiguration The EnvoyConfiguration used to start Envoy.
   * @param logLevel           The log level to use when starting Envoy.
   * @param onEngineRunning    Called when the engine finishes its async startup and begins running.
   * @return int A status indicating if the action was successful.
   */
  @Override
  public int runWithConfig(EnvoyConfiguration envoyConfiguration, String logLevel,
                           EnvoyOnEngineRunning onEngineRunning) {

    for (EnvoyHTTPFilterFactory filterFactory : envoyConfiguration.httpPlatformFilterFactories) {
      JniLibrary.registerFilterFactory(filterFactory.getFilterName(),
                                       new JvmFilterFactoryContext(filterFactory));
    }

    for (Map.Entry<String, EnvoyStringAccessor> entry :
         envoyConfiguration.stringAccessors.entrySet()) {
      JniLibrary.registerStringAccessor(entry.getKey(),
                                        new JvmStringAccessorContext(entry.getValue()));
    }

    return runWithConfig(envoyConfiguration.resolveTemplate(
                             JniLibrary.templateString(), JniLibrary.platformFilterTemplateString(),
                             JniLibrary.nativeFilterTemplateString(),
                             JniLibrary.defaultCertificates()),
                         logLevel, onEngineRunning);
  }

  /**
   * Increment a counter with the given count.
   *
   * @param elements Elements of the counter stat.
   * @param tags Tags of the counter stat.
   * @param count Amount to add to the counter.
   * @return A status indicating if the action was successful.
   */
  @Override
  public int recordCounterInc(String elements, Map<String, String> tags, int count) {
    return JniLibrary.recordCounterInc(engineHandle, elements, JniBridgeUtility.toJniTags(tags),
                                       count);
  }

  /**
   * Set a gauge of a given string of elements with the given value.
   *
   * @param elements Elements of the gauge stat.
   * @param tags Tags of the gauge stat.
   * @param value Value to set to the gauge.
   * @return A status indicating if the action was successful.
   */
  @Override
  public int recordGaugeSet(String elements, Map<String, String> tags, int value) {
    return JniLibrary.recordGaugeSet(engineHandle, elements, JniBridgeUtility.toJniTags(tags),
                                     value);
  }

  /**
   * Add the gauge with the given string of elements and by the given amount.
   *
   * @param elements Elements of the gauge stat.
   * @param tags Tags of the gauge stat.
   * @param amount Amount to add to the gauge.
   * @return A status indicating if the action was successful.
   */
  @Override
  public int recordGaugeAdd(String elements, Map<String, String> tags, int amount) {
    return JniLibrary.recordGaugeAdd(engineHandle, elements, JniBridgeUtility.toJniTags(tags),
                                     amount);
  }

  /**
   * Subtract from the gauge with the given string of elements and by the given amount.
   *
   * @param elements Elements of the gauge stat.
   * @param tags Tags of the gauge stat.
   * @param amount Amount to subtract from the gauge.
   * @return A status indicating if the action was successful.
   */
  @Override
  public int recordGaugeSub(String elements, Map<String, String> tags, int amount) {
    return JniLibrary.recordGaugeSub(engineHandle, elements, JniBridgeUtility.toJniTags(tags),
                                     amount);
  }

  /**
   * Add another recorded duration in ms to the timer histogram with the given string of elements.
   *
   * @param elements Elements of the histogram stat.
   * @param tags Tags of the histogram stat.
   * @param durationMs Duration value to record in the histogram timer distribution.
   * @return A status indicating if the action was successful.
   */
  public int recordHistogramDuration(String elements, Map<String, String> tags, int durationMs) {
    return JniLibrary.recordHistogramDuration(engineHandle, elements,
                                              JniBridgeUtility.toJniTags(tags), durationMs);
  }

  /**
   * Add another recorded value to the generic histogram with the given string of elements.
   *
   * @param elements Elements of the histogram stat.
   * @param tags Tags of the histogram stat.
   * @param value Amount to record as a new value for the histogram distribution.
   * @return A status indicating if the action was successful.
   */
  public int recordHistogramValue(String elements, Map<String, String> tags, int value) {
    return JniLibrary.recordHistogramValue(engineHandle, elements, JniBridgeUtility.toJniTags(tags),
                                           value);
  }

  @Override
  public int registerStringAccessor(String accessor_name, EnvoyStringAccessor accessor) {
    return JniLibrary.registerStringAccessor(accessor_name, new JvmStringAccessorContext(accessor));
  }
}
