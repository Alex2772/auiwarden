//
// Created by alex2772 on 8/31/25.
//

#include <range/v3/numeric/accumulate.hpp>
#include "TimeSpanView.h"
#include "utils/date.h"
#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/View/ALabel.h>
#include <AUI/View/AForEachUI.h>
#include <range/v3/algorithm/find.hpp>
#include <range/v3/view/reverse.hpp>
#include <range/v3/view/concat.hpp>

using namespace declarative;
using namespace std::chrono;

_<AView> declarative::weekDay(std::chrono::weekday weekday) {
    auto display = utils::date::getLastWeekdayUpTo(weekday, floor<days>(system_clock::now()));
    return Vertical {
        Label { "{}"_format(weekday) },
        Label { "{:%d}"_format(display) },
    } AUI_WITH_STYLE { Expanding()};
}
_<AView> declarative::activityRectangle(std::chrono::minutes duration, _<AView> view) {
    return Vertical { std::move(view) } AUI_WITH_STYLE {
        Expanding { duration.count() },
        Margin { 1_px },
    };
}

_<AView> declarative::dayContent(AVector<_<TimeSpan>> spans) {
    using namespace std::chrono_literals;
    auto remainingHours = hours(days(1)) - ranges::accumulate(spans, 0min, std::plus<> {}, [](const _<TimeSpan>& s) { return s->duration; });
    auto lastElement = std::array{_new<TimeSpan>(remainingHours)};

    return AUI_DECLARATIVE_FOR(i, ranges::view::concat(spans, lastElement), AVerticalLayout) {
        _<AView> view;
        if (i->title) {
            view = Label { i->title } AUI_WITH_STYLE { BackgroundSolid { AColor::RED }, Expanding {} };
        }
        return activityRectangle(i->duration, std::move(view));
    } AUI_WITH_STYLE { Expanding() };
}

_<AView> declarative::weekContent(const Database& db) {
    const auto now = system_clock::now();
    const year_month_day today{floor<days>(now)};

    auto day = [&](std::chrono::weekday weekday) {
        auto targetDay = sys_days(utils::date::getLastWeekdayUpTo(weekday, today));
        for (const auto& i : db.days | ranges::view::reverse) {
            if (i.timepoint != targetDay) {
                continue;
            }
            return dayContent(i.spans);
        }
        return dayContent({});
    };

    return Horizontal::Expanding {
        _new<ASpacerExpanding>(2),
        day(std::chrono::Monday),
        day(std::chrono::Tuesday),
        day(std::chrono::Wednesday),
        day(std::chrono::Thursday),
        day(std::chrono::Friday),
        day(std::chrono::Saturday),
        day(std::chrono::Sunday),
    } AUI_WITH_STYLE { LayoutSpacing { 1_px } };
}
