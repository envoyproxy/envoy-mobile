package io.envoyproxy.envoymobile.engine.types;

/* Interface used to support lambdas being passed from Kotlin for engine setup completion. */
public interface EnvoyEngineOnSetupComplete {
  Object invokeOnSetupComplete();
}
