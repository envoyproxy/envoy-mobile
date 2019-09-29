package io.envoyproxy.envoymobile;

import static io.grpc.MethodDescriptor.generateFullMethodName;
import static io.grpc.stub.ClientCalls.asyncBidiStreamingCall;
import static io.grpc.stub.ClientCalls.asyncClientStreamingCall;
import static io.grpc.stub.ClientCalls.asyncServerStreamingCall;
import static io.grpc.stub.ClientCalls.asyncUnaryCall;
import static io.grpc.stub.ClientCalls.blockingServerStreamingCall;
import static io.grpc.stub.ClientCalls.blockingUnaryCall;
import static io.grpc.stub.ClientCalls.futureUnaryCall;
import static io.grpc.stub.ServerCalls.asyncBidiStreamingCall;
import static io.grpc.stub.ServerCalls.asyncClientStreamingCall;
import static io.grpc.stub.ServerCalls.asyncServerStreamingCall;
import static io.grpc.stub.ServerCalls.asyncUnaryCall;
import static io.grpc.stub.ServerCalls.asyncUnimplementedStreamingCall;
import static io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall;

/**
 */
@javax.annotation.Generated(value = "by gRPC proto compiler (version 1.16.1)",
                            comments = "Source: protos/tests/envoy_mobile/envoy_mobile_test.proto")
public final class EnvoyMobileTestGrpc {

  private EnvoyMobileTestGrpc() {}

  public static final String SERVICE_NAME = "io.envoyproxy.envoymobile.EnvoyMobileTest";

  // Static method descriptors that strictly reflect the proto.
  private static volatile io.grpc.MethodDescriptor<
      io.envoyproxy.envoymobile.EnvoyMobileTestOuterClass.Request,
      io.envoyproxy.envoymobile.EnvoyMobileTestOuterClass.Response> getStreamMethod;

  @io.grpc.stub.annotations
      .RpcMethod(fullMethodName = SERVICE_NAME + '/' + "stream",
                 requestType = io.envoyproxy.envoymobile.EnvoyMobileTestOuterClass.Request.class,
                 responseType = io.envoyproxy.envoymobile.EnvoyMobileTestOuterClass.Response.class,
                 methodType = io.grpc.MethodDescriptor.MethodType.BIDI_STREAMING)
      public static io.grpc
      .MethodDescriptor<io.envoyproxy.envoymobile.EnvoyMobileTestOuterClass.Request,
                        io.envoyproxy.envoymobile.EnvoyMobileTestOuterClass.Response>
      getStreamMethod() {
    io.grpc.MethodDescriptor<io.envoyproxy.envoymobile.EnvoyMobileTestOuterClass.Request,
                             io.envoyproxy.envoymobile.EnvoyMobileTestOuterClass.Response>
        getStreamMethod;
    if ((getStreamMethod = EnvoyMobileTestGrpc.getStreamMethod) == null) {
      synchronized (EnvoyMobileTestGrpc.class) {
        if ((getStreamMethod = EnvoyMobileTestGrpc.getStreamMethod) == null) {
          EnvoyMobileTestGrpc.getStreamMethod = getStreamMethod =
              io.grpc.MethodDescriptor
                  .<io.envoyproxy.envoymobile.EnvoyMobileTestOuterClass.Request,
                    io.envoyproxy.envoymobile.EnvoyMobileTestOuterClass.Response>newBuilder()
                  .setType(io.grpc.MethodDescriptor.MethodType.BIDI_STREAMING)
                  .setFullMethodName(
                      generateFullMethodName("io.envoyproxy.envoymobile.EnvoyMobileTest", "stream"))
                  .setSampledToLocalTracing(true)
                  .setRequestMarshaller(io.grpc.protobuf.ProtoUtils.marshaller(
                      io.envoyproxy.envoymobile.EnvoyMobileTestOuterClass.Request
                          .getDefaultInstance()))
                  .setResponseMarshaller(io.grpc.protobuf.ProtoUtils.marshaller(
                      io.envoyproxy.envoymobile.EnvoyMobileTestOuterClass.Response
                          .getDefaultInstance()))
                  .setSchemaDescriptor(new EnvoyMobileTestMethodDescriptorSupplier("stream"))
                  .build();
        }
      }
    }
    return getStreamMethod;
  }

  private static volatile io.grpc.MethodDescriptor<
      io.envoyproxy.envoymobile.EnvoyMobileTestOuterClass.Request,
      io.envoyproxy.envoymobile.EnvoyMobileTestOuterClass.Response> getUnaryMethod;

  @io.grpc.stub.annotations
      .RpcMethod(fullMethodName = SERVICE_NAME + '/' + "unary",
                 requestType = io.envoyproxy.envoymobile.EnvoyMobileTestOuterClass.Request.class,
                 responseType = io.envoyproxy.envoymobile.EnvoyMobileTestOuterClass.Response.class,
                 methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
      public static io.grpc
      .MethodDescriptor<io.envoyproxy.envoymobile.EnvoyMobileTestOuterClass.Request,
                        io.envoyproxy.envoymobile.EnvoyMobileTestOuterClass.Response>
      getUnaryMethod() {
    io.grpc.MethodDescriptor<io.envoyproxy.envoymobile.EnvoyMobileTestOuterClass.Request,
                             io.envoyproxy.envoymobile.EnvoyMobileTestOuterClass.Response>
        getUnaryMethod;
    if ((getUnaryMethod = EnvoyMobileTestGrpc.getUnaryMethod) == null) {
      synchronized (EnvoyMobileTestGrpc.class) {
        if ((getUnaryMethod = EnvoyMobileTestGrpc.getUnaryMethod) == null) {
          EnvoyMobileTestGrpc.getUnaryMethod = getUnaryMethod =
              io.grpc.MethodDescriptor
                  .<io.envoyproxy.envoymobile.EnvoyMobileTestOuterClass.Request,
                    io.envoyproxy.envoymobile.EnvoyMobileTestOuterClass.Response>newBuilder()
                  .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
                  .setFullMethodName(
                      generateFullMethodName("io.envoyproxy.envoymobile.EnvoyMobileTest", "unary"))
                  .setSampledToLocalTracing(true)
                  .setRequestMarshaller(io.grpc.protobuf.ProtoUtils.marshaller(
                      io.envoyproxy.envoymobile.EnvoyMobileTestOuterClass.Request
                          .getDefaultInstance()))
                  .setResponseMarshaller(io.grpc.protobuf.ProtoUtils.marshaller(
                      io.envoyproxy.envoymobile.EnvoyMobileTestOuterClass.Response
                          .getDefaultInstance()))
                  .setSchemaDescriptor(new EnvoyMobileTestMethodDescriptorSupplier("unary"))
                  .build();
        }
      }
    }
    return getUnaryMethod;
  }

  /**
   * Creates a new async stub that supports all call types for the service
   */
  public static EnvoyMobileTestStub newStub(io.grpc.Channel channel) {
    return new EnvoyMobileTestStub(channel);
  }

  /**
   * Creates a new blocking-style stub that supports unary and streaming output calls on the service
   */
  public static EnvoyMobileTestBlockingStub newBlockingStub(io.grpc.Channel channel) {
    return new EnvoyMobileTestBlockingStub(channel);
  }

  /**
   * Creates a new ListenableFuture-style stub that supports unary calls on the service
   */
  public static EnvoyMobileTestFutureStub newFutureStub(io.grpc.Channel channel) {
    return new EnvoyMobileTestFutureStub(channel);
  }

  /**
   */
  public static abstract class EnvoyMobileTestImplBase implements io.grpc.BindableService {

    /**
     */
    public io.grpc.stub.StreamObserver<io.envoyproxy.envoymobile.EnvoyMobileTestOuterClass.Request>
    stream(io.grpc.stub.StreamObserver<io.envoyproxy.envoymobile.EnvoyMobileTestOuterClass.Response>
               responseObserver) {
      return asyncUnimplementedStreamingCall(getStreamMethod(), responseObserver);
    }

    /**
     */
    public void
    unary(io.envoyproxy.envoymobile.EnvoyMobileTestOuterClass.Request request,
          io.grpc.stub.StreamObserver<io.envoyproxy.envoymobile.EnvoyMobileTestOuterClass.Response>
              responseObserver) {
      asyncUnimplementedUnaryCall(getUnaryMethod(), responseObserver);
    }

    @java.
    lang.Override
    public final io.grpc.ServerServiceDefinition bindService() {
      return io.grpc.ServerServiceDefinition.builder(getServiceDescriptor())
          .addMethod(
              getStreamMethod(),
              asyncBidiStreamingCall(
                  new MethodHandlers<io.envoyproxy.envoymobile.EnvoyMobileTestOuterClass.Request,
                                     io.envoyproxy.envoymobile.EnvoyMobileTestOuterClass.Response>(
                      this, METHODID_STREAM)))
          .addMethod(
              getUnaryMethod(),
              asyncUnaryCall(
                  new MethodHandlers<io.envoyproxy.envoymobile.EnvoyMobileTestOuterClass.Request,
                                     io.envoyproxy.envoymobile.EnvoyMobileTestOuterClass.Response>(
                      this, METHODID_UNARY)))
          .build();
    }
  }

  /**
   */
  public static final class EnvoyMobileTestStub
      extends io.grpc.stub.AbstractStub<EnvoyMobileTestStub> {
    private EnvoyMobileTestStub(io.grpc.Channel channel) { super(channel); }

    private EnvoyMobileTestStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected EnvoyMobileTestStub build(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new EnvoyMobileTestStub(channel, callOptions);
    }

    /**
     */
    public io.grpc.stub.StreamObserver<io.envoyproxy.envoymobile.EnvoyMobileTestOuterClass.Request>
    stream(io.grpc.stub.StreamObserver<io.envoyproxy.envoymobile.EnvoyMobileTestOuterClass.Response>
               responseObserver) {
      return asyncBidiStreamingCall(getChannel().newCall(getStreamMethod(), getCallOptions()),
                                    responseObserver);
    }

    /**
     */
    public void
    unary(io.envoyproxy.envoymobile.EnvoyMobileTestOuterClass.Request request,
          io.grpc.stub.StreamObserver<io.envoyproxy.envoymobile.EnvoyMobileTestOuterClass.Response>
              responseObserver) {
      asyncUnaryCall(getChannel().newCall(getUnaryMethod(), getCallOptions()), request,
                     responseObserver);
    }
  }

  /**
   */
  public static final class EnvoyMobileTestBlockingStub
      extends io.grpc.stub.AbstractStub<EnvoyMobileTestBlockingStub> {
    private EnvoyMobileTestBlockingStub(io.grpc.Channel channel) { super(channel); }

    private EnvoyMobileTestBlockingStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected EnvoyMobileTestBlockingStub build(io.grpc.Channel channel,
                                                io.grpc.CallOptions callOptions) {
      return new EnvoyMobileTestBlockingStub(channel, callOptions);
    }

    /**
     */
    public io.envoyproxy.envoymobile.EnvoyMobileTestOuterClass.Response
    unary(io.envoyproxy.envoymobile.EnvoyMobileTestOuterClass.Request request) {
      return blockingUnaryCall(getChannel(), getUnaryMethod(), getCallOptions(), request);
    }
  }

  /**
   */
  public static final class EnvoyMobileTestFutureStub
      extends io.grpc.stub.AbstractStub<EnvoyMobileTestFutureStub> {
    private EnvoyMobileTestFutureStub(io.grpc.Channel channel) { super(channel); }

    private EnvoyMobileTestFutureStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected EnvoyMobileTestFutureStub build(io.grpc.Channel channel,
                                              io.grpc.CallOptions callOptions) {
      return new EnvoyMobileTestFutureStub(channel, callOptions);
    }

    /**
     */
    public com.google.common.util.concurrent
        .ListenableFuture<io.envoyproxy.envoymobile.EnvoyMobileTestOuterClass.Response>
        unary(io.envoyproxy.envoymobile.EnvoyMobileTestOuterClass.Request request) {
      return futureUnaryCall(getChannel().newCall(getUnaryMethod(), getCallOptions()), request);
    }
  }

  private static final int METHODID_UNARY = 0;
  private static final int METHODID_STREAM = 1;

  private static final class MethodHandlers<Req, Resp>
      implements io.grpc.stub.ServerCalls.UnaryMethod<Req, Resp>,
                 io.grpc.stub.ServerCalls.ServerStreamingMethod<Req, Resp>,
                 io.grpc.stub.ServerCalls.ClientStreamingMethod<Req, Resp>,
                 io.grpc.stub.ServerCalls.BidiStreamingMethod<Req, Resp> {
    private final EnvoyMobileTestImplBase serviceImpl;
    private final int methodId;

    MethodHandlers(EnvoyMobileTestImplBase serviceImpl, int methodId) {
      this.serviceImpl = serviceImpl;
      this.methodId = methodId;
    }

    @java.
    lang.Override
    @java.lang.SuppressWarnings("unchecked")
    public void invoke(Req request, io.grpc.stub.StreamObserver<Resp> responseObserver) {
      switch (methodId) {
      case METHODID_UNARY:
        serviceImpl.unary(
            (io.envoyproxy.envoymobile.EnvoyMobileTestOuterClass.Request)request,
            (io.grpc.stub
                 .StreamObserver<io.envoyproxy.envoymobile.EnvoyMobileTestOuterClass.Response>)
                responseObserver);
        break;
      default:
        throw new AssertionError();
      }
    }

    @java.
    lang.Override
    @java.
    lang.SuppressWarnings("unchecked")
    public io.grpc.stub.StreamObserver<Req>
    invoke(io.grpc.stub.StreamObserver<Resp> responseObserver) {
      switch (methodId) {
      case METHODID_STREAM:
        return (io.grpc.stub.StreamObserver<Req>)serviceImpl.stream(
            (io.grpc.stub
                 .StreamObserver<io.envoyproxy.envoymobile.EnvoyMobileTestOuterClass.Response>)
                responseObserver);
      default:
        throw new AssertionError();
      }
    }
  }

  private static abstract class EnvoyMobileTestBaseDescriptorSupplier
      implements io.grpc.protobuf.ProtoFileDescriptorSupplier,
                 io.grpc.protobuf.ProtoServiceDescriptorSupplier {
    EnvoyMobileTestBaseDescriptorSupplier() {}

    @java.
    lang.Override
    public com.google.protobuf.Descriptors.FileDescriptor getFileDescriptor() {
      return io.envoyproxy.envoymobile.EnvoyMobileTestOuterClass.getDescriptor();
    }

    @java.
    lang.Override
    public com.google.protobuf.Descriptors.ServiceDescriptor getServiceDescriptor() {
      return getFileDescriptor().findServiceByName("EnvoyMobileTest");
    }
  }

  private static final class EnvoyMobileTestFileDescriptorSupplier
      extends EnvoyMobileTestBaseDescriptorSupplier {
    EnvoyMobileTestFileDescriptorSupplier() {}
  }

  private static final class EnvoyMobileTestMethodDescriptorSupplier
      extends EnvoyMobileTestBaseDescriptorSupplier
      implements io.grpc.protobuf.ProtoMethodDescriptorSupplier {
    private final String methodName;

    EnvoyMobileTestMethodDescriptorSupplier(String methodName) { this.methodName = methodName; }

    @java.
    lang.Override
    public com.google.protobuf.Descriptors.MethodDescriptor getMethodDescriptor() {
      return getServiceDescriptor().findMethodByName(methodName);
    }
  }

  private static volatile io.grpc.ServiceDescriptor serviceDescriptor;

  public static io.grpc.ServiceDescriptor getServiceDescriptor() {
    io.grpc.ServiceDescriptor result = serviceDescriptor;
    if (result == null) {
      synchronized (EnvoyMobileTestGrpc.class) {
        result = serviceDescriptor;
        if (result == null) {
          serviceDescriptor = result =
              io.grpc.ServiceDescriptor.newBuilder(SERVICE_NAME)
                  .setSchemaDescriptor(new EnvoyMobileTestFileDescriptorSupplier())
                  .addMethod(getStreamMethod())
                  .addMethod(getUnaryMethod())
                  .build();
        }
      }
    }
    return result;
  }
}
