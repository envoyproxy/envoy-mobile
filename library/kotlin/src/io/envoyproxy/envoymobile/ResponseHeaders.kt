/*
 * Headers representing an inbound response.
 */
class ResponseHeaders: Headers {

  /**
   * HTTP status code received with the response.
   */
  val httpStatus = value(":status")?.first()?.toInt()
}
