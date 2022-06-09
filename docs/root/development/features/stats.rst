.. _stats:

Stats
============

Envoy and by extension Envoy Mobile implements a comprehensive set of stats
that may be used to monitor the health of the library and for debugging purposes.

Envoy Mobile does not enable all of the Envoy stats by default. Instead, it
implements an allow list of stats that are allowed to be created on the client-side
and ingested to a remote service. The allow list can be found under `stats_config`
in the `library/common/config.cc` file.

The original idea behind not enabling all Envoy stats by default was to limit
the usage of resources on the client:

1. Lower the usage of memory and CPU resources by creating and maintaining the lower number of instances of stats
2. Reduce the cell data plan usage by ingesting less data to a remote service
