//
// Created by alex2772 on 8/31/25.
//

#include "GridView.h"
#include <AUI/View/ADrawableView.h>

GridView::GridView(_<State> state) : mState(std::move(state)) { connect(mState->currentTime.changed, me::redraw); }

void GridView::render(ARenderContext ctx) {
    AView::render(ctx);
    static const auto GRID_COLOR = AColor::GRAY;
    static const auto FONT_SIZE = 12_pt;
    static const auto OFFSET = 10_pt;

    using namespace std::chrono;
    using namespace std::chrono_literals;

    auto mapX = [this](int columnIndex) { return columnIndex * getSize().x / 7; };
    auto mapY = [this](std::chrono::minutes h) { return h.count() * getSize().y / minutes(days(1)).count(); };

    for (auto h = hours(0); h <= days(1); ++h) {
        const int y = mapY(h);
        ctx.render.line(ASolidBrush { GRID_COLOR }, { 0, y }, { getSize().x + OFFSET, y });
    }
    for (int d = 0; d <= 7; ++d) {
        const int x = mapX(d);
        ctx.render.line(ASolidBrush { GRID_COLOR }, { x, -OFFSET }, { x, getSize().y + OFFSET });
    }

    {
        // now
        const auto localTime = floor<minutes>(std::chrono::current_zone()->to_local(*mState->currentTime));
        auto todaysWeekDay = weekday(floor<days>(localTime));
        auto columnIndex = [&] {
            // ya russki
            if (todaysWeekDay == Monday) {
                return 0;
            }
            if (todaysWeekDay == Tuesday) {
                return 1;
            }
            if (todaysWeekDay == Wednesday) {
                return 2;
            }
            if (todaysWeekDay == Thursday) {
                return 3;
            }
            if (todaysWeekDay == Friday) {
                return 4;
            }
            if (todaysWeekDay == Saturday) {
                return 5;
            }
            if (todaysWeekDay == Sunday) {
                return 6;
            }
            return 0;
        }();
        auto x = mapX(columnIndex);
        auto y = mapY(localTime - floor<days>(localTime));

        ctx.render.line(
            ASolidBrush { 0xdc143c_rgb },
            { x, y },
            { x + getSize().x / 8, y });
    }
}
