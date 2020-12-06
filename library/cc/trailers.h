#pragma once

#include "headers.h"


class Trailers : public Headers {
public:
  Trailers(const RawHeaders& headers) : Headers(headers) {}
};
