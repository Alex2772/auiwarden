#pragma once

#include <range/v3/all.hpp>
#include <AUI/Common/AColor.h>
#include "Database.h"

/**
 * @brief Grouping of raw time spans by user-defined groups. The reference implementation shared by the GUI and the CLI.
 */
namespace span_grouping {

struct GroupedSpan {
    /**
     * @brief Span titled with the group name (or with the original title if no group matches).
     */
    TimeSpan timespan;
    AColor color;

    /**
     * @brief Matching group; nullptr if the span is uncategorized.
     */
    _<Group> group;
};

/**
 * @brief Maximum gap between two spans of the same group to be displayed as a single one.
 */
constexpr auto MAX_MERGE_GAP = std::chrono::minutes(5);

/**
 * @brief Finds a group for the span.
 * @details
 * Registers the reactive dependency on groups' rules, so it's safe to use in AUI_REACT/AUI_DECLARATIVE_FOR.
 */
inline GroupedSpan groupify(const Database& database, const TimeSpan& span) {
    for (const auto& group : *database.groups) {
        aui::react::DependencyObserverScope::addDependency(group->windowTitleContains.changed);
    }
    auto group = database.findGroup(span.title);

    if (group == nullptr) {
        return {
            .timespan = span,
            .color = AColor::RED,
            .group = nullptr,
        };
    }
    return {
        .timespan = {
            .begin = span.begin,
            .end = span.end,
            .title = group->name,
        },
        .color = group->color,
        .group = std::move(group),
    };
}

/**
 * @brief Merges neighbouring spans of the same group and color.
 * @details
 * Pipe-able; expects the range of GroupedSpan in REVERSED chronological order (latest first). The result is in the
 * same order.
 */
inline auto mergeAdjacent() {
    return ranges::view::chunk_by([](const GroupedSpan& a, const GroupedSpan& b) {
               // the range is reversed: a is later than b
               return std::tie(a.color, a.timespan.title) == std::tie(b.color, b.timespan.title) &&
                      a.timespan.begin - b.timespan.end <= MAX_MERGE_GAP;
           }) |
           ranges::view::transform([](const auto& chunk) {
               auto copy = ranges::front(chunk);
               const auto& last = ranges::back(chunk);
               copy.timespan.begin = std::min(copy.timespan.begin, last.timespan.begin);
               copy.timespan.end = std::max(copy.timespan.end, last.timespan.end);
               return copy;
           });
}

}   // namespace span_grouping
