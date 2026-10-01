#ifndef NOMIME_LOGGGING_H_
#define NOMIME_LOGGGING_H_

#ifdef NOMIME_ENABLE_LOGGING
#define GLOG_NO_ABBREVIATED_SEVERITIES
#pragma warning(disable : 4244)
#include <glog/logging.h>
#pragma warning(default : 4244)
#else
#include "no_logging.h"
#endif  // NOMIME_ENABLE_LOGGING

#endif  // NOMIME_LOGGGING_H_
