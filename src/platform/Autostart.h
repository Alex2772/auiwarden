#pragma once

#include "AUI/Common/APropertyPrecomputed.h"

namespace platform {

/**
 * @brief Checks whether the system has a startup entry pointing to the current executable.
 * @details Implemented in platform/linux and platform/win32.
 */
extern APropertyPrecomputed<bool> isAutostartEnabled;


/**
 * @brief Adds (true) or removes (false) the startup entry of the current executable.
 * @details Implemented in platform/linux and platform/win32.
 */
void setAutostartEnabled(bool enabled);

}   // namespace platform
