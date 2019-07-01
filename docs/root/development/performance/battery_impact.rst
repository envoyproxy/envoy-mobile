.. _dev_performance_battery:

Analysis of battery impact
==========================

In order to identify how Envoy impacts an application, we have created a control application with modifications to our
current hello world example applications. To see the actual applications we used, you can go to `iOS Envoy example <https://github.com/lyft/envoy-mobile/tree/ac/envoy-battery-cpu-branch/examples/swift/hello_world>`_,
`Android control app <https://github.com/lyft/envoy-mobile/tree/ac/envoy-battery-cpu-branch/examples/kotlin/control>`_, `Android Envoy example <https://github.com/lyft/envoy-mobile/tree/ac/envoy-battery-cpu-branch/examples/kotlin/hello_world>`_.

The current configurations make network requests every 200ms and disable caching.

* TODO: Fill more details about how Envoy is configured

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

1. Set a phone's display to sleep after 30minutes of inactivity
2. Unplug the phone from any power source
3. Open up the demo app
4. Wait for the phone to sleep
5. Look at the battery drain the battery settings in the phone to see the battery usage and drainage

Alternative profiling methods tried:

1. `AccuBattery <https://play.google.com/store/apps/details?id=com.digibites.accubattery&hl=en_US>`_
We were unable to get the running time of a given application on AccuBattery to more accurately identify battery usage per minute
2. `Battery Historian <https://github.com/google/battery-historian>`_
We were unable to get reliable data using this method. Often times, the battery usage of an application appears to use no batteries

Results
~~~~~~~

iOS
---

Android
-------

Through running the applications for 30minutes, the results are:
Envoy   : 0.17%/min
Control : 0.18%/min

The results of Envoy and Control are very comparable

Analysis
~~~~~~~~

iOS
---

* TODO

Android
-------

The results of this experiment is surprising since we should expect Envoy to use more battery than the control by a
large margin.

One explanation is that Envoy is running on its own native thread and the Android application is unable
to associate the Envoy process to the Android application. However, when looking at the treads and CPU usage (via `adb shell top`),
we are unable to observe another thread taking up CPU other than the running Envoy application.

Open issues regarding battery usage
-----------------------------------

Current status
~~~~~~~~~~~~~~
