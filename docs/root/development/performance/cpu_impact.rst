.. _dev_performance_cpu:

.. _ios_envoy_example_app: https://github.com/lyft/envoy-mobile/tree/ac/envoy-battery-cpu-branch/examples/swift/hello_world
.. _android_envoy_example_app: https://github.com/lyft/envoy-mobile/tree/ac/envoy-battery-cpu-branch/examples/kotlin/hello_world
.. _android_envoy_example_control_app: https://github.com/lyft/envoy-mobile/tree/ac/envoy-battery-cpu-branch/examples/kotlin/control

Analysis of CPU impact
======================

Experimentation method
~~~~~~~~~~~~~~~~~~~~~~

iOS
---

* TODO

Android
-------

We're currently using HttpURLConnection to communicate and send requests to Envoy. Envoy in it's current state is run as
a process

Getting the build:

1. Build the library using ``bazel build android_dist --config=android``
2. Control: ``bazel mobile-install //examples/kotlin/control:hello_control_kt``
3. Envoy: ``bazel mobile-install //examples/kotlin/hello_world:hello_envoy_kt --fat_apk_cpu=armeabi-v7a``

Experiment steps:

1. Run `adb shell top -H | grep envoy` to get the CPU usage of the application (the `-H` flag displays the running threads)
2. Wait 10minutes to gather a sample set of data to analyze
3. Take the average CPU% and MEM%

Results
~~~~~~~

iOS
---

* TODO

Android
-------

Envoy:
15.67560976	Avg CPU%
2.696341463 Avg MEM%

Control:
11.27439024 Avg CPU%
2.152439024 Avg MEM%

Analysis
~~~~~~~~

iOS
---

* TODO

Android
-------

The results of this experiment is surprising since we should expect Envoy to use significantly more CPU than the control.

One explanation is that Envoy is running on its own native thread and the Android application is unable
to associate the Envoy process to the Android application. However, when looking at the treads and CPU usage (via `adb shell top`),
we are unable to observe another thread taking up CPU other than the running Envoy application.

Open issues regarding battery usage
-----------------------------------

Current status
~~~~~~~~~~~~~~
