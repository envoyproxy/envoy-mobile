#import "EnvoyTypes.h"

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface EnvoyHttpStream : NSObject
/**
 Open an underlying HTTP stream.

 @param observer the observer that will run the stream callbacks.
 */
- (instancetype)initWithObserver:(EnvoyObserver *)observer;

/**
 Send headers over the provided stream.

 @param metadata Headers to send over the stream.
 @param close True if the stream should be closed after sending.
 */
- (void)sendHeaders:(EnvoyHeaders *)headers close:(BOOL)close;

/**
 Send data over the provided stream.

 @param metadata Data to send over the stream.
 @param close True if the stream should be closed after sending.
 */
- (void)sendData:(NSData *)data close:(BOOL)close;

/**
 Send metadata over the provided stream.

 @param metadata Metadata to send over the stream.
 */
- (void)sendMetadata:(EnvoyHeaders *)metadata;

/**
 Send trailers over the provided stream.

 @param trailers Trailers to send over the stream.
 */
- (void)sendTrailers:(EnvoyHeaders *)trailers;

/**
 Cancel the stream. This functions as an interrupt, and aborts further callbacks and handling of the
 stream.
 @return Success, unless the stream has already been canceled.
 */
- (EnvoyStatus)cancel;

@end

NS_ASSUME_NONNULL_END
