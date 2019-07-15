package io.envoyproxy.envoymobile

import java.util.concurrent.Executor

interface Client {
  fun request(request: Request, inboundExecutor: Executor, callback: (Response) -> Unit)
}