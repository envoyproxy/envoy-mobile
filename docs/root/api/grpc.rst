.. _api_grpc:

gRPC
====

Starting an instance of Envoy for making requests is done by creating an ``EnvoyClient``.
To do so, create an ``EnvoyClientBuilder`` and call ``build()``:

**Kotlin**::

  val envoy = AndroidEnvoyClientBuilder(baseContext, Domain("api.envoyproxy.io"))
    .addLogLevel(LogLevel.WARN)
    .addStatsFlushSeconds(60)
    ...
    .build()

**Swift**::

  let envoy = try EnvoyClientBuilder(domain: "api.envoyproxy.io")
    .addLogLevel(.warn)
    .addStatsFlushSeconds(60)
    ...
    .build()
