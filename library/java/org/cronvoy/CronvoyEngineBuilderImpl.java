// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.cronvoy;

import android.content.Context;
import org.chromium.net.ExperimentalCronetEngine;
import org.chromium.net.ICronetEngineBuilder;

/** Implementation of {@link ICronetEngineBuilder} that builds Java-based Cronet engine. */
public class CronvoyEngineBuilderImpl extends CronetEngineBuilderImpl {
  /**
   * Builder for Platform Cronet Engine.
   *
   * @param context Android {@link Context} for engine to use.
   */
  public CronvoyEngineBuilderImpl(Context context) {
    super(context);
  }

  @Override
  public ExperimentalCronetEngine build() {
    if (getUserAgent() == null) {
      setUserAgent(getDefaultUserAgent());
    }
    return new CronvoyEngine(this);
  }
}
