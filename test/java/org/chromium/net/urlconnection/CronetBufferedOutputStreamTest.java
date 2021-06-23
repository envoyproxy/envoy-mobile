// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.net.urlconnection;

import static org.junit.Assert.assertEquals;
// import static org.junit.Assert.assertTrue;
import static org.junit.Assert.fail;

// import static org.chromium.net.CronetTestRule.getContext;

import android.content.Context;
import androidx.test.core.app.ApplicationProvider;
import androidx.test.filters.SmallTest;

import io.envoyproxy.envoymobile.engine.AndroidJniLibrary;
import java.io.IOException;
import java.io.OutputStream;
import java.net.HttpURLConnection;
import java.net.URL;

import org.chromium.net.impl.CronetUrlRequestContext;
import org.chromium.net.impl.NativeCronetEngineBuilderImpl;
import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
// import org.junit.Rule;
import org.junit.Test;
import org.junit.runner.RunWith;

import org.robolectric.RobolectricTestRunner;

/**
 * Tests the CronetBufferedOutputStream implementation.
 */
@RunWith(RobolectricTestRunner.class)
public class CronetBufferedOutputStreamTest {

    private static final String TEST_URL_PATH = "get/flowers";

    private static CronetUrlRequestContext cronvoyEngine;

    private final NativeTestServer server = new NativeTestServer();

    @BeforeClass
    public static void loadJniLibrary() {
        AndroidJniLibrary.loadTestLibrary();
    }

    @AfterClass
    public static void shutdown() {
        if (cronvoyEngine != null) {
            cronvoyEngine.shutdown();
        }
    }

    @Before
    public void setUp() {
        if (cronvoyEngine == null) {
            Context appContext = ApplicationProvider.getApplicationContext();
            cronvoyEngine = new CronetUrlRequestContext(
                new NativeCronetEngineBuilderImpl(appContext).setUserAgent("Cronvoy"));
            // URL.setURLStreamHandlerFactory(cronvoyEngine.createURLStreamHandlerFactory());
        }
    }

    @After
    public void shutdownMockWebServer() throws IOException {
        server.shutdown();
    }

    @Test
    @SmallTest
    // @Feature({"Cronet"})
    // @CompareDefaultWithCronet
    public void testGetOutputStreamAfterConnectionMade() throws Exception {
        // mockWebServer.enqueue(new MockResponse().setBody("hello, world"));
        server.setDispatchAndStart();
        URL url = new URL(server.getURL(TEST_URL_PATH));
        HttpURLConnection connection =
                (HttpURLConnection) cronvoyEngine.openConnection(url);
        connection.setDoOutput(true);
        connection.setRequestMethod("POST");
        assertEquals(200, connection.getResponseCode());
        try {
            connection.getOutputStream();
            fail();
        } catch (java.net.ProtocolException e) {
            // Expected.
        }
    }

    /**
     * Tests write after connect.
     * TODO (@colibie) implement this test rule
     * Strangely, the default implementation allows
     * writing after being connected, so this test only runs against Cronet
     * implementation.
     * Use {@code OnlyRunCronetHttpURLConnection} as the default implementation
     * does not pass this test.
     */
    // @Test
    // @SmallTest
    // @Feature({"Cronet"})
    // @OnlyRunCronetHttpURLConnection
    // public void testWriteAfterConnect() throws Exception {
    //     server.setDispatchAndStart();
    //     URL url = new URL(server.getURL(TEST_URL_PATH));
    //     HttpURLConnection connection =
    //             (HttpURLConnection) url.openConnection();
    //     connection.setDoOutput(true);
    //     connection.setRequestMethod("POST");
    //     OutputStream out = connection.getOutputStream();
    //     out.write(TestUtil.UPLOAD_DATA);
    //     connection.connect();
    //     try {
    //         // Attempt to write some more.
    //         out.write(TestUtil.UPLOAD_DATA);
    //         fail("Writing to the output stream after connect should throw error");
    //     } catch (IllegalStateException e) {
    //         assertEquals("Use setFixedLengthStreamingMode() or "
    //                         + "setChunkedStreamingMode() for writing after connect",
    //                 e.getMessage());
    //     }
    // }

    // @Test
    // @SmallTest
    // @Feature({"Cronet"})
    // @CompareDefaultWithCronet
    // public void testWriteAfterReadingResponse() throws Exception {
    //     // mockWebServer.enqueue(new MockResponse().setBody("hello, world"));
    //     server.setDispatchAndStart();
    //     URL url = new URL(server.getURL(TEST_URL_PATH));
    //     HttpURLConnection connection =
    //             (HttpURLConnection) cronvoyEngine.openConnection(url);
    //     connection.setDoOutput(true);
    //     connection.setRequestMethod("POST");
    //     OutputStream out = connection.getOutputStream();
    //     assertEquals(200, connection.getResponseCode());
    //     try {
    //         out.write(TestUtil.UPLOAD_DATA);
    //         fail();
    //     } catch (Exception e) {
    //         // Default implementation gives an IOException and says that the
    //         // stream is closed. Cronet gives an IllegalStateException and
    //         // complains about write after connected.
    //     }
    // }

    @Test
    @SmallTest
    // @Feature({"Cronet"})
    // @CompareDefaultWithCronet
    public void testPostWithContentLength() throws Exception {
        // mockWebServer.enqueue(new MockResponse().setBody("hello world"));
        server.setDispatchAndStart();
        URL url = new URL(server.getURL(TEST_URL_PATH));
        HttpURLConnection connection =
                (HttpURLConnection) url.openConnection();
        connection.setDoOutput(true);
        connection.setRequestMethod("POST");
        byte[] largeData = TestUtil.getLargeData();
        connection.setRequestProperty("Content-Length",
                Integer.toString(largeData.length));
        OutputStream out = connection.getOutputStream();
        int totalBytesWritten = 0;
        // Number of bytes to write each time. It is doubled each time
        // to make sure that the buffer grows.
        int bytesToWrite = 683;
        while (totalBytesWritten < largeData.length) {
            if (bytesToWrite > largeData.length - totalBytesWritten) {
                // Do not write out of bound.
                bytesToWrite = largeData.length - totalBytesWritten;
            }
            out.write(largeData, totalBytesWritten, bytesToWrite);
            totalBytesWritten += bytesToWrite;
            bytesToWrite *= 2;
        }
        assertEquals(200, connection.getResponseCode());
        assertEquals("OK", connection.getResponseMessage());

        TestUtil.checkLargeData(TestUtil.getResponseAsString(connection));
        connection.disconnect();
    }

    //
    @Test
    @SmallTest
    // @Feature({"Cronet"})
    // @CompareDefaultWithCronet
    public void testPostWithContentLengthOneMassiveWrite() throws Exception {
        server.setDispatchAndStart();
        URL url = new URL(server.getURL(TEST_URL_PATH));
        HttpURLConnection connection =
                (HttpURLConnection) url.openConnection();
        connection.setDoOutput(true);
        connection.setRequestMethod("POST");
        byte[] largeData = TestUtil.getLargeData();
        connection.setRequestProperty("Content-Length",
                Integer.toString(largeData.length));
        OutputStream out = connection.getOutputStream();
        out.write(largeData);
        assertEquals(200, connection.getResponseCode());
        assertEquals("OK", connection.getResponseMessage());
        TestUtil.checkLargeData(TestUtil.getResponseAsString(connection));
        connection.disconnect();
    }

    @Test
    @SmallTest
    // @Feature({"Cronet"})
    // @CompareDefaultWithCronet
    public void testPostWithContentLengthWriteOneByte() throws Exception {
        server.setDispatchAndStart();
        URL url = new URL(server.getURL(TEST_URL_PATH));
        HttpURLConnection connection =
                (HttpURLConnection) url.openConnection();
        connection.setDoOutput(true);
        connection.setRequestMethod("POST");
        byte[] largeData = TestUtil.getLargeData();
        connection.setRequestProperty("Content-Length",
                Integer.toString(largeData.length));
        OutputStream out = connection.getOutputStream();
        for (byte largeDatum : largeData) {
            out.write(largeDatum);
        }
        assertEquals(200, connection.getResponseCode());
        assertEquals("OK", connection.getResponseMessage());
        TestUtil.checkLargeData(TestUtil.getResponseAsString(connection));
        connection.disconnect();
    }

    @Test
    @SmallTest
    // @Feature({"Cronet"})
    // @CompareDefaultWithCronet
    public void testPostWithZeroContentLength() throws Exception {
        server.setDispatchAndStart();
        URL url = new URL(server.getURL(TEST_URL_PATH));
        HttpURLConnection connection =
                (HttpURLConnection) url.openConnection();
        connection.setDoOutput(true);
        connection.setRequestMethod("POST");
        connection.setRequestProperty("Content-Length", "0");
        assertEquals(200, connection.getResponseCode());
        assertEquals("OK", connection.getResponseMessage());
        assertEquals("", TestUtil.getResponseAsString(connection));
        connection.disconnect();
    }
    //
    // @Test
    // @SmallTest
    // @Feature({"Cronet"})
    // @CompareDefaultWithCronet
    // public void testPostZeroByteWithoutContentLength() throws Exception {
    //     // Make sure both implementation sets the Content-Length header to 0.
    //     URL url = new URL(NativeTestServer.getEchoHeaderURL("Content-Length"));
    //     HttpURLConnection connection =
    //             (HttpURLConnection) url.openConnection();
    //     connection.setDoOutput(true);
    //     connection.setRequestMethod("POST");
    //     assertEquals(200, connection.getResponseCode());
    //     assertEquals("OK", connection.getResponseMessage());
    //     assertEquals("0", TestUtil.getResponseAsString(connection));
    //     connection.disconnect();
    //
    //     // Make sure the server echoes back empty body for both implementation.
    //     URL echoBody = new URL(NativeTestServer.getEchoBodyURL());
    //     HttpURLConnection connection2 =
    //             (HttpURLConnection) echoBody.openConnection();
    //     connection2.setDoOutput(true);
    //     connection2.setRequestMethod("POST");
    //     assertEquals(200, connection2.getResponseCode());
    //     assertEquals("OK", connection2.getResponseMessage());
    //     assertEquals("", TestUtil.getResponseAsString(connection2));
    //     connection2.disconnect();
    // }
    //
    @Test
    @SmallTest
    // @Feature({"Cronet"})
    // @CompareDefaultWithCronet
    public void testPostWithoutContentLengthSmall() throws Exception {
        server.setDispatchAndStart();
        URL url = new URL(server.getURL(TEST_URL_PATH));
        HttpURLConnection connection =
                (HttpURLConnection) url.openConnection();
        connection.setDoOutput(true);
        connection.setRequestMethod("POST");
        OutputStream out = connection.getOutputStream();
        out.write(TestUtil.UPLOAD_DATA);
        assertEquals(200, connection.getResponseCode());
        assertEquals("OK", connection.getResponseMessage());
        assertEquals(TestUtil.UPLOAD_DATA_STRING, TestUtil.getResponseAsString(connection));
        connection.disconnect();
    }

    @Test
    @SmallTest
    // @Feature({"Cronet"})
    // @CompareDefaultWithCronet
    public void testPostWithoutContentLength() throws Exception {
        server.setDispatchAndStart();
        URL url = new URL(server.getURL(TEST_URL_PATH));
        HttpURLConnection connection =
                (HttpURLConnection) url.openConnection();
        connection.setDoOutput(true);
        connection.setRequestMethod("POST");
        byte[] largeData = TestUtil.getLargeData();
        OutputStream out = connection.getOutputStream();
        int totalBytesWritten = 0;
        // Number of bytes to write each time. It is doubled each time
        // to make sure that the buffer grows.
        int bytesToWrite = 683;
        while (totalBytesWritten < largeData.length) {
            if (bytesToWrite > largeData.length - totalBytesWritten) {
                // Do not write out of bound.
                bytesToWrite = largeData.length - totalBytesWritten;
            }
            out.write(largeData, totalBytesWritten, bytesToWrite);
            totalBytesWritten += bytesToWrite;
            bytesToWrite *= 2;
        }
        assertEquals(200, connection.getResponseCode());
        assertEquals("OK", connection.getResponseMessage());
        TestUtil.checkLargeData(TestUtil.getResponseAsString(connection));
        connection.disconnect();
    }

    @Test
    @SmallTest
    // @Feature({"Cronet"})
    // @CompareDefaultWithCronet
    public void testPostWithoutContentLengthOneMassiveWrite() throws Exception {
        server.setDispatchAndStart();
        URL url = new URL(server.getURL(TEST_URL_PATH));
        HttpURLConnection connection =
                (HttpURLConnection) url.openConnection();
        connection.setDoOutput(true);
        connection.setRequestMethod("POST");
        OutputStream out = connection.getOutputStream();
        byte[] largeData = TestUtil.getLargeData();
        out.write(largeData);
        assertEquals(200, connection.getResponseCode());
        assertEquals("OK", connection.getResponseMessage());
        TestUtil.checkLargeData(TestUtil.getResponseAsString(connection));
        connection.disconnect();
    }

    @Test
    @SmallTest
    // @Feature({"Cronet"})
    // @CompareDefaultWithCronet
    public void testPostWithoutContentLengthWriteOneByte() throws Exception {
        server.setDispatchAndStart();
        URL url = new URL(server.getURL(TEST_URL_PATH));
        HttpURLConnection connection =
                (HttpURLConnection) url.openConnection();
        connection.setDoOutput(true);
        connection.setRequestMethod("POST");
        OutputStream out = connection.getOutputStream();
        byte[] largeData = TestUtil.getLargeData();
        for (byte largeDatum : largeData) {
            out.write(largeDatum);
        }
        assertEquals(200, connection.getResponseCode());
        assertEquals("OK", connection.getResponseMessage());
        TestUtil.checkLargeData(TestUtil.getResponseAsString(connection));
        connection.disconnect();
    }

    // @Test
    // @SmallTest
    // @Feature({"Cronet"})
    // @CompareDefaultWithCronet
    // public void testWriteLessThanContentLength() throws Exception {
    //     server.setDispatchAndStart();
    //     URL url = new URL(server.getURL(TEST_URL_PATH));
    //     HttpURLConnection connection =
    //             (HttpURLConnection) url.openConnection();
    //     connection.setDoOutput(true);
    //     connection.setRequestMethod("POST");
    //     // Set a content length that's 1 byte more.
    //     connection.setRequestProperty(
    //             "Content-Length", Integer.toString(TestUtil.UPLOAD_DATA.length + 1));
    //     OutputStream out = connection.getOutputStream();
    //     out.write(TestUtil.UPLOAD_DATA);
    //     try {
    //         connection.getResponseCode();
    //         fail();
    //     } catch (IOException e) {
    //         // Expected.
    //     }
    //     connection.disconnect();
    // }
    //
    // /**
    //  * Tests that if caller writes more than the content length provided,
    //  * an exception should occur.
    //  */
    // @Test
    // @SmallTest
    // @Feature({"Cronet"})
    // @CompareDefaultWithCronet
    // public void testWriteMoreThanContentLength() throws Exception {
    //     server.setDispatchAndStart();
    //     URL url = new URL(server.getURL(TEST_URL_PATH));
    //     HttpURLConnection connection =
    //             (HttpURLConnection) url.openConnection();
    //     connection.setDoOutput(true);
    //     connection.setRequestMethod("POST");
    //     // Use a content length that is 1 byte shorter than actual data.
    //     connection.setRequestProperty(
    //             "Content-Length", Integer.toString(TestUtil.UPLOAD_DATA.length - 1));
    //     OutputStream out = connection.getOutputStream();
    //     // Write a few bytes first.
    //     out.write(TestUtil.UPLOAD_DATA, 0, 3);
    //     try {
    //         // Write remaining bytes.
    //         out.write(TestUtil.UPLOAD_DATA, 3, TestUtil.UPLOAD_DATA.length - 3);
    //         // On Lollipop, default implementation only triggers the error when reading response.
    //         connection.getInputStream();
    //         fail();
    //     } catch (IOException e) {
    //         assertEquals("exceeded content-length limit of " + (TestUtil.UPLOAD_DATA.length - 1)
    //                         + " bytes",
    //                 e.getMessage());
    //     }
    //     connection.disconnect();
    // }
    //
    // /**
    //  * Same as {@code testWriteMoreThanContentLength()}, but it only writes one byte
    //  * at a time.
    //  */
    // @Test
    // @SmallTest
    // @Feature({"Cronet"})
    // @CompareDefaultWithCronet
    // public void testWriteMoreThanContentLengthWriteOneByte() throws Exception {
    //     URL url = new URL(NativeTestServer.getEchoBodyURL());
    //     HttpURLConnection connection =
    //             (HttpURLConnection) url.openConnection();
    //     connection.setDoOutput(true);
    //     connection.setRequestMethod("POST");
    //     // Use a content length that is 1 byte shorter than actual data.
    //     connection.setRequestProperty(
    //             "Content-Length", Integer.toString(TestUtil.UPLOAD_DATA.length - 1));
    //     OutputStream out = connection.getOutputStream();
    //     try {
    //         for (int i = 0; i < TestUtil.UPLOAD_DATA.length; i++) {
    //             out.write(TestUtil.UPLOAD_DATA[i]);
    //         }
    //         // On Lollipop, default implementation only triggers the error when reading response.
    //         connection.getInputStream();
    //         fail();
    //     } catch (IOException e) {
    //         assertEquals("exceeded content-length limit of " + (TestUtil.UPLOAD_DATA.length - 1)
    //                         + " bytes",
    //                 e.getMessage());
    //     }
    //     connection.disconnect();
    // }
    //
    // /**
    //  * Tests that {@link CronetBufferedOutputStream} supports rewind in a
    //  * POST preserving redirect.
    //  * Use {@code OnlyRunCronetHttpURLConnection} as the default implementation
    //  * does not pass this test.
    //  */
    // @Test
    // @SmallTest
    // @Feature({"Cronet"})
    // @OnlyRunCronetHttpURLConnection
    // public void testRewind() throws Exception {
    //     URL url = new URL(NativeTestServer.getRedirectToEchoBody());
    //     HttpURLConnection connection =
    //             (HttpURLConnection) url.openConnection();
    //     connection.setDoOutput(true);
    //     connection.setRequestMethod("POST");
    //     connection.setRequestProperty(
    //             "Content-Length", Integer.toString(TestUtil.UPLOAD_DATA.length));
    //     OutputStream out = connection.getOutputStream();
    //     out.write(TestUtil.UPLOAD_DATA);
    //     assertEquals(TestUtil.UPLOAD_DATA_STRING, TestUtil.getResponseAsString(connection));
    //     connection.disconnect();
    // }
    //
    // /**
    //  * Like {@link #testRewind} but does not set Content-Length header.
    //  */
    // @Test
    // @SmallTest
    // @Feature({"Cronet"})
    // @OnlyRunCronetHttpURLConnection
    // public void testRewindWithoutContentLength() throws Exception {
    //     URL url = new URL(NativeTestServer.getRedirectToEchoBody());
    //     HttpURLConnection connection =
    //             (HttpURLConnection) url.openConnection();
    //     connection.setDoOutput(true);
    //     connection.setRequestMethod("POST");
    //     OutputStream out = connection.getOutputStream();
    //     out.write(TestUtil.UPLOAD_DATA);
    //     assertEquals(TestUtil.UPLOAD_DATA_STRING, TestUtil.getResponseAsString(connection));
    //     connection.disconnect();
    // }
}
