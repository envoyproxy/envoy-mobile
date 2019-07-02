.. _dev_performance_cpu_battery:

Analysis of CPU/battery impact
==============================

Modified versions of the "hello world" example apps were used to run these experiments:

- `Android control app <https://github.com/lyft/envoy-mobile/tree/ac/envoy-battery-cpu-branch/examples/kotlin/control>`_
- `Android Envoy app <https://github.com/lyft/envoy-mobile/tree/ac/envoy-battery-cpu-branch/examples/kotlin/hello_world>`_
- `iOS control app <https://github.com/lyft/envoy-mobile/tree/ac/envoy-battery-cpu-branch/examples/objective-c/control/control>`_
- `iOS Envoy app <https://github.com/lyft/envoy-mobile/tree/ac/envoy-battery-cpu-branch/examples/objective-c/xcode_variant/EnvoyObjc/EnvoyObjc>`_

These apps made network requests every ``200ms`` with all request/response caching disabled.

Results
~~~~~~~

iOS
---

Envoy:

- Avg CPU: >= 100%
- Avg memory: 12MB
- Battery: 12/20 Xcode Instruments score

Control:

- Avg CPU: 12%
- Avg memory: 6MB
- Battery: 1/20 Xcode Instruments score

**Based on these results, memory usage is similar. However, CPU (and consequently battery) usage is very high.**
The root cause has been identified and is being tracked in `issue 215 <https://github.com/lyft/envoy-mobile/issues/215>`_.

Android
-------

Envoy:

- Avg CPU: 33.16075949%
- Avg memory: 2.765822785%
- Battery: 0.17%/min

Control:

- Avg CPU: 28.81012658%
- Avg memory: 2.169620253%
- Battery: 0.18%/min

**Based on these results, control and Envoy are relatively similar.**

Experimentation method
~~~~~~~~~~~~~~~~~~~~~~

iOS
---

The original investigation was completed as part of `this issue <https://github.com/lyft/envoy-mobile/issues/113>`_.

For analysis, the `Energy Diagnostics tool from Xcode Instruments <https://developer.apple.com/library/archive/documentation/Performance/Conceptual/EnergyGuide-iOS/MonitorEnergyWithInstruments.html>`_
was used.

The analysis utilized two apps:

- **Control** - An app that makes a request every ``200ms`` to an endpoint without Envoy compiled in the app
- **Envoy** - App that makes the same request at the same interval, but routed through an instance of Envoy

Requests were made using ``URLSession``, with the session's cache set to ``nil`` (disabling caching).
Envoy listened to the data sent over ``URLSession``, proxying it through.

Both apps were run (one at a time) on a physical device (iPhone 6s iOS 12.2.x) while running Instruments.

Reproducing the Envoy example app:

1. Build the library using ``bazel build ios_dist --config=ios --config=fat``
2. Copy ``./dist/Envoy.framework`` to the example's `source directory <ios_envoy_example_app>`__
3. Build/run the example app

Android
-------

We're currently using ``HttpURLConnection`` to communicate and send requests to Envoy. Envoy in it's current state is run as
a process listening to traffic sent over this connection.

Getting the build:

1. Build the library using ``bazel build android_dist --config=android``
2. Control: ``bazel mobile-install //examples/kotlin/control:hello_control_kt``
3. Envoy: ``bazel mobile-install //examples/kotlin/hello_world:hello_envoy_kt --fat_apk_cpu=armeabi-v7a``

Battery usage experiment steps:

1. Set a phone's display to sleep after 30 minutes of inactivity
2. Unplug the phone from all power sources
3. Open up the demo app
4. Wait for the phone to sleep
5. Look at the battery drain the battery settings in the phone to see the battery usage and drainage

Alternative profiling methods tried:

1. `AccuBattery <https://play.google.com/store/apps/details?id=com.digibites.accubattery&hl=en_US>`_:
We were unable to get the running time of a given application on AccuBattery to more accurately identify battery usage per minute

2. `Battery Historian <https://github.com/google/battery-historian>`_:
We were unable to get reliable data using this method. Often times, the battery usage of an application appears to use no batteries

CPU usage experiment steps:

1. Run ``adb shell top -H | grep envoy`` to get the CPU usage of the application (the ``-H`` flag displays the running threads)
2. Wait 10minutes to gather a sample set of data to analyze
3. Take the average CPU% and MEM%

Analysis
~~~~~~~~

iOS
---

Envoy had a reasonable increase in memory usage of a few megabytes compared to control.

CPU/battery usage, however, was much higher. After some digging, the largest contributor to this usage
was `identified as a poller <https://github.com/lyft/envoy-mobile/issues/113#issuecomment-505676324>`_.

Upon further investigation, the `root cause was determined <https://github.com/lyft/envoy-mobile/issues/113#issuecomment-507425528>`_
to be that ``poll_dispatch`` was being used by ``libevent`` instead of the much more performant ``kqueue``.
Forcing ``libevent`` to use ``kqueue`` reduced the CPU usage **from >= 100% down to ~3%**.
This issue and the subsequent fix are being tracked `here <https://github.com/lyft/envoy-mobile/issues/215>`_.

`We used Wireshark <https://github.com/lyft/envoy-mobile/issues/113#issuecomment-505673869>`_ to validate that
network traffic was flowing through Envoy on the phone every ``200ms``, giving us confidence that there was
no additional caching happening within ``URLSession``.

Android
-------

There are minimal differences between Envoy and control. By enabling trace logging within Envoy,
we are able to observe the following:

1. Requests to S3 are being logged in Envoy
2. DNS resolution does happen every 5 seconds
3. Stats are flushed every 5 seconds

The DNS resolution and stats flush happening every 5 seconds was originally a concern,
but updating the frequency to 1 minute did not result in a significant change.

Open issues regarding battery usage
-----------------------------------

- `(215) Excessive iOS CPU usage due to libevent polling <https://github.com/lyft/envoy-mobile/issues/215>`_
