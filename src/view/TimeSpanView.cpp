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

_<AView> declarative::weekDay(std::chrono::weekday weekday, const _<State>& state) {
    auto day = [=] {
        return utils::date::getLastWeekdayUpTo(weekday, year_month_day(floor<days>(current_zone()->to_local(*state->currentTime))));
    };
    return Vertical {
        Label { "{}"_format(weekday) },
        Label { AUI_REACT("{:%d}"_format(day())) },
    } AUI_WITH_STYLE { Expanding() };
}

static _<AView> dayContent(
    const _<GridView>& gridView, std::chrono::time_point<std::chrono::system_clock, std::chrono::days> dayUtc,
    const AProperty<AVector<_<TimeSpan>>>& spans) {
    auto day = floor<days>(current_zone()->to_local(dayUtc));
    using namespace std::chrono_literals;
    auto intersectsWithDay = [=](const _<TimeSpan>& span) {
        auto begin = current_zone()->to_local(span->begin);
        auto end = current_zone()->to_local(span->end);
        return begin >= day && begin <= day + days(1) || end >= day && end <= day + days(1);
    };
    auto keyFunction = [dayUtc](const _<TimeSpan>& span) {
        return aui::for_each_ui::defaultKey(span, 0L) ^ aui::for_each_ui::defaultKey(dayUtc.time_since_epoch().count(), 0L);
    };
    return AUI_DECLARATIVE_FOR(i, spans | ranges::view::filter(intersectsWithDay), AAbsoluteLayout) {
        ALogger::debug("TimeSpanView") << "dayContent: " << i->title;
        _<AView> view = Vertical {
            Label { i->title } AUI_WITH_STYLE { BackgroundSolid { AColor::RED }, Expanding {} },
        } AUI_WITH_STYLE { AOverflow::HIDDEN };
        struct State {
            APropertyPrecomputed<glm::ivec2> position;
            APropertyPrecomputed<glm::ivec2> size;
        };
        auto state = aui::ptr::manage_shared(new State {
          .position =
              [gridView, day, i] {
                  auto timeRelativeToDay = seconds(current_zone()->to_local(i->begin) - day);
                  auto y =
                      int((timeRelativeToDay).count()) * gridView->size()->y /
                      int(seconds(days(1)).count());
                  return glm::ivec2 { 0, y };
              },
          .size =
              [gridView, i] {
                  auto timeRelativeToDay = minutes(i->end - i->begin);
                  auto y =
                      int((timeRelativeToDay).count()) * gridView->size()->y /
                      int(minutes(days(1)).count());
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
    } AUI_LET {
        it->setExpanding();
        it->setKeyFunction(keyFunction);
    };
}

_<AView> declarative::weekContent(const _<GridView>& gridView, const _<State>& state) {

    auto day = [&](std::chrono::weekday weekday) {
        return CustomLayout {} & state->currentTime.readProjected([gridView, state, weekday](TimeSpan::Timepoint now) -> _<AView> {
          const year_month_day today { floor<days>(current_zone()->to_local(now)) };
          auto targetDay = sys_days(utils::date::getLastWeekdayUpTo(weekday, today));
          return dayContent(gridView, targetDay, state->database.spans);
        }) AUI_WITH_STYLE { Expanding() };
    };

    return Horizontal::Expanding {
        _new<ASpacerExpanding>(2),  day(std::chrono::Monday), day(std::chrono::Tuesday),  day(std::chrono::Wednesday),
        day(std::chrono::Thursday), day(std::chrono::Friday), day(std::chrono::Saturday), day(std::chrono::Sunday),
    } AUI_WITH_STYLE { LayoutSpacing { 1_px } };
}
