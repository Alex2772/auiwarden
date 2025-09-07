//
// Created by alex2772 on 8/31/25.
//

#include <range/v3/numeric/accumulate.hpp>
#include "TimeSpanView.h"
#include "utils/date.h"
#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/View/ALabel.h>
#include <AUI/View/AForEachUI.h>
#include <range/v3/all.hpp>

using namespace declarative;
using namespace std::chrono;

_<AView> declarative::weekDay(std::chrono::weekday weekday, const _<State>& state) {
    auto day = [=] {
        return utils::date::getLastWeekdayUpTo(
            weekday, year_month_day(floor<days>(current_zone()->to_local(*state->currentTime))));
    };
    return Vertical {
        Label { "{}"_format(weekday) },
        Label { AUI_REACT("{:%d}"_format(day())) },
    } AUI_WITH_STYLE { Expanding() };
}

struct TimeSpanViewModel {
    TimeSpan timespan;
    AColor color;
};

static TimeSpanViewModel groupify(const Database& database, const TimeSpan& span) {
    const auto& groups = *database.groups;
    for (const auto& group : groups) {
        aui::react::DependencyObserverRegistrar::addDependency(group->windowTitleContains.changed);
    }
    auto group = ranges::find_if(groups, [&](const _<Group>& group) {
        auto s = group->windowTitleContains->split('\n');
        return ranges::any_of(s, [&](const AString& probe) {
            if (probe.empty()) {
                return false;
            }
            return span.title.lowercase().contains(probe.lowercase());
        });
    });
    if (group == groups.end()) {
        return {
            .timespan = span,
            .color = AColor::RED,
        };
    }
    return {
        .timespan = {
            .begin = span.begin,
            .end = span.end,
          .title = *(*group)->name,
        },
        .color = (*group)->color,
    };
}

static _<AView> dayContent(
    const _<State>& state, const _<GridView>& gridView,
    std::chrono::time_point<std::chrono::system_clock, std::chrono::days> dayUtc) {
    auto day = floor<days>(current_zone()->to_local(dayUtc));
    using namespace std::chrono_literals;
    auto intersectsWithDay = [=](const _<TimeSpan>& span) {
        auto begin = current_zone()->to_local(span->begin);
        auto end = current_zone()->to_local(span->end);
        return begin >= day && begin <= day + days(1) || end >= day && end <= day + days(1);
    };
    auto keyFunction = [dayUtc](const TimeSpanViewModel& span) {
        uint64_t hash = 0;
        for (auto i :
             { uint64_t(dayUtc.time_since_epoch().count()),
               uint64_t(span.timespan.begin.time_since_epoch().count()),
               uint64_t(span.timespan.end.time_since_epoch().count()),
               uint64_t(std::hash<AByteBufferView> {}(AByteBufferView::fromRaw(span.color))),
               uint64_t(std::hash<AString> {}(span.timespan.title)) }) {
            hash ^= i;
            hash *= 0x9e3779b97f4a7c15;
        }
        return hash;
    };
    auto toGroups =
        ranges::view::filter(intersectsWithDay) |
        ranges::view::transform([=](const _<TimeSpan>& span) { return groupify(state->database, *span); }) |
        ranges::view::chunk_by([](const TimeSpanViewModel& a, const TimeSpanViewModel& b) {
                return std::tie(a.color, a.timespan.title) == std::tie(b.color, b.timespan.title) && b.timespan.begin - a.timespan.end <= 5min;
            }) |
        ranges::view::transform([](const auto& chunk) {
            auto copy = ranges::front(chunk);
            copy.timespan.end = ranges::back(chunk).timespan.end;
            return copy;
        });
    return AUI_DECLARATIVE_FOR(i, state->database.spans | toGroups, AAbsoluteLayout) {
        _<AView> view = Vertical {
            Label { i.timespan.title } AUI_WITH_STYLE { BackgroundSolid { i.color }, Expanding {} },
        } AUI_WITH_STYLE {
            AOverflow::HIDDEN,
            on_state::Hovered {
                AOverflow::VISIBLE,
            },
        };
        struct State {
            APropertyPrecomputed<glm::ivec2> position;
            APropertyPrecomputed<glm::ivec2> size;
        };
        auto state = aui::ptr::manage_shared(new State {
          .position =
              [gridView, day, i] {
                  auto timeRelativeToDay = seconds(current_zone()->to_local(i.timespan.begin) - day);
                  auto y = int((timeRelativeToDay).count()) * gridView->size()->y / int(seconds(days(1)).count());
                  return glm::ivec2 { 0, y };
              },
          .size =
              [gridView, i] {
                  auto timeRelativeToDay = minutes(i.timespan.end - i.timespan.begin) + 1min;
                  auto y = int((timeRelativeToDay).count()) * gridView->size()->y / int(minutes(days(1)).count());
                  return glm::ivec2 { gridView->size()->x / 8, glm::max(y, 1) };
              },
        });
        AObject::connect(state->position, view, [view = view.get(), state] {
            view->setGeometry(state->position, state->size);
        });
        AObject::connect(state->size, view, [view = view.get(), state] {
            view->setGeometry(state->position, state->size);
        });
        return view;
    } AUI_LET {
        it->setExpanding();
        it->setKeyFunction(keyFunction);
    };
}

_<AView> declarative::weekContent(const _<GridView>& gridView, const _<State>& state) {
    auto day = [&](std::chrono::weekday weekday) {
        return CustomLayout {} &
               state->currentTime.readProjected([gridView, state, weekday](TimeSpan::Timepoint now) -> _<AView> {
                   const year_month_day today { floor<days>(current_zone()->to_local(now)) };
                   auto targetDay = sys_days(utils::date::getLastWeekdayUpTo(weekday, today));
                   return dayContent(state, gridView, targetDay);
               }) AUI_WITH_STYLE { Expanding() };
    };

    return Horizontal::Expanding {
        _new<ASpacerExpanding>(2),  day(std::chrono::Monday), day(std::chrono::Tuesday),  day(std::chrono::Wednesday),
        day(std::chrono::Thursday), day(std::chrono::Friday), day(std::chrono::Saturday), day(std::chrono::Sunday),
    } AUI_WITH_STYLE { LayoutSpacing { 1_px } };
}
