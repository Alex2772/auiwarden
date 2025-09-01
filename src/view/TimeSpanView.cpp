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
#include <range/v3/view/filter.hpp>

using namespace declarative;
using namespace std::chrono;

_<AView> declarative::weekDay(std::chrono::weekday weekday) {
    auto display = utils::date::getLastWeekdayUpTo(weekday, floor<days>(system_clock::now()));
    return Vertical {
        Label { "{}"_format(weekday) },
        Label { "{:%d}"_format(display) },
    } AUI_WITH_STYLE { Expanding() };
}

static _<AView> dayContent(
    const _<GridView>& gridView, std::chrono::time_point<std::chrono::system_clock, std::chrono::days> day,
    const AVector<_<TimeSpan>>& spans) {
    using namespace std::chrono_literals;
    auto intersectsWithDay = [=](const _<TimeSpan>& span) {
        return span->begin >= day && span->begin <= day + days(1) || span->end >= day && span->end <= day + days(1);
    };
    return AUI_DECLARATIVE_FOR(i, spans | ranges::view::filter(intersectsWithDay), AAbsoluteLayout) {
        _<AView> view = Vertical {
            Label { i->title } AUI_WITH_STYLE { BackgroundSolid { AColor::RED }, Expanding {} },
        };
        struct State {
            APropertyPrecomputed<glm::ivec2> position;
            APropertyPrecomputed<glm::ivec2> size;
        };
        auto state = aui::ptr::manage_shared(new State {
          .position =
              [gridView, day, i] {
                  auto timeRelativeToDay = i->begin - day;
                  auto y =
                      int((timeRelativeToDay).count()) * gridView->size()->y /
                      int(std::chrono::minutes(std::chrono::days(1)).count());
                  return glm::ivec2 { 0, y };
              },
          .size =
              [gridView, i] {
                  auto timeRelativeToDay = i->end - i->begin;
                  auto y =
                      int((timeRelativeToDay).count()) * gridView->size()->y /
                      int(std::chrono::minutes(std::chrono::days(1)).count());
                  return glm::ivec2 { gridView->size()->x / 8, y };
              },
        });
        AObject::connect(state->position, view, [view = view.get(), state] {
            view->setGeometry(state->position, state->size);
        });
        AObject::connect(state->size, view, [view = view.get(), state] {
            view->setGeometry(state->position, state->size);
        });
        return view;
    } AUI_WITH_STYLE { Expanding() };
}

_<AView> declarative::weekContent(const _<GridView>& gridView, const Database& db) {
    const auto now = system_clock::now();
    const year_month_day today { floor<days>(now) };

    auto day = [&](std::chrono::weekday weekday) {
        auto targetDay = sys_days(utils::date::getLastWeekdayUpTo(weekday, today));
        return dayContent(gridView, targetDay, db.spans);
    };

    return Horizontal::Expanding {
        _new<ASpacerExpanding>(2),  day(std::chrono::Monday), day(std::chrono::Tuesday),  day(std::chrono::Wednesday),
        day(std::chrono::Thursday), day(std::chrono::Friday), day(std::chrono::Saturday), day(std::chrono::Sunday),
    } AUI_WITH_STYLE { LayoutSpacing { 1_px } };
}
