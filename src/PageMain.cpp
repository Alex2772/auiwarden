//
// Created by alex2772 on 10/14/25.
//

#include "PageMain.h"
#include <AUI/Util/UIBuildingHelpers.h>
#include "view/TimeSpanView.h"
#include <AUI/View/ALabel.h>
#include <AUI/View/AScrollArea.h>
#include <AUI/View/AForEachUI.h>
#include <range/v3/view/iota.hpp>
#include <AUI/View/ASpacerFixed.h>

using namespace declarative;
using namespace ass;

_<AView> views::pageMain(const _<State>& state) {
    auto gridView = _new<GridView>(state) AUI_WITH_STYLE { Expanding() };
    return Vertical::Expanding {
        Horizontal {
            SpacerFixed { 40_dp },
            weekDay(std::chrono::Monday, state),
            weekDay(std::chrono::Tuesday, state),
            weekDay(std::chrono::Wednesday, state),
            weekDay(std::chrono::Thursday, state),
            weekDay(std::chrono::Friday, state),
            weekDay(std::chrono::Saturday, state),
            weekDay(std::chrono::Sunday, state),
        },
        AScrollArea::Builder()
            .withContents(Horizontal {
                AUI_DECLARATIVE_FOR(i, ranges::view::iota(0, 25), AVerticalLayout) {
                    auto l = Label { "{}:00"_format(i % 24) } AUI_WITH_STYLE {
                        TransformOffset { 0, 11_pt / -2.f },
                        ATextAlign::RIGHT,
                        Padding { 0, 4_dp },
                        TextColor { AColor::GRAY },
                        FixedSize { 0, 1_px },
                    };
                    if (i == 24) {
                        return l;
                    }
                    return Vertical::Expanding {
                        std::move(l),
                        SpacerExpanding {},
                    };
                } AUI_WITH_STYLE { FixedSize{ 40_dp, {} } },
                Stacked::Expanding {
                    gridView,
                        weekContent(gridView, state),
                } AUI_WITH_STYLE { MinSize(600_dp, 400_dp) },
            })
            .build() AUI_WITH_STYLE { Expanding() },
    };
}
