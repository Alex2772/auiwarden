#pragma once

#include <AUI/Common/AOptional.h>
#include <AUI/Common/AStringVector.h>

namespace cli {

/**
 * @brief Command line interface of AUIwarden, designed to be consumed by AI agents (i.e. Claude).
 * @details
 * Invoked as `auiwarden cli <command> [options]`. Prints machine-readable (JSON) output to stdout; diagnostics go to
 * stderr.
 *
 * @return exit code if args contain a CLI invocation; nullopt otherwise (the program should start as usual).
 */
AOptional<int> tryRun(const AStringVector& args);

}   // namespace cli
