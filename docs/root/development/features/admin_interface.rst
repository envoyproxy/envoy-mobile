.. _admin_interface:

Admin Interface
============

You can read more about this Envoy's feature and how to use it [here|https://www.envoyproxy.io/docs/envoy/latest/operations/admin]. 
By default, Envoy Mobiles does not come with an admin endpoint enabled. You can enable it in the following way.

**Kotlin**::

  builder.enableAdminInterface()

**Swift**::

  builder.enableAdminInterface()

Envoy Mobile runs its administration interface on port 9901.

One of the most common use cases for the interface is to check the state of all of the Envoy stats at a given moment. You can do this by running the following command:

```
> curl localhost:9901/stats
```

Please note that to make this command work when you are running an example app using Android emulator you need to set up port forwarding first:
```
> adb forward tcp:9901 tcp:9901
```
