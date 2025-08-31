//
// Created by alex2772 on 8/31/25.
//

#include "GridView.h"
#include <AUI/View/ADrawableView.h>

void GridView::render(ARenderContext ctx) {
    AView::render(ctx);
    static const auto GRID_COLOR = AColor::GRAY;
    static const auto FONT_SIZE = 12_pt;
    static const auto OFFSET = 10_pt;

    using namespace std::chrono;
    using namespace std::chrono_literals;

    for (auto h = hours(0); h <= days(1); ++h) {
        const int y = h.count() * getSize().y / hours(days(1)).count();
        ctx.render.line(ASolidBrush { GRID_COLOR }, { getSize().x / 8 - OFFSET, y }, { getSize().x + OFFSET, y });
        RenderHints::PushState s(ctx.render);
        ctx.render.setColor(GRID_COLOR);
        ctx.render.string({ 0, y - FONT_SIZE / 2.f }, "{:%H:%M}"_format(h), { .size = (unsigned int) (FONT_SIZE) });
    }
    for (int d = 1; d <= 8; ++d) {
        const int x = d * getSize().x / 8;
        ctx.render.line(ASolidBrush { GRID_COLOR }, { x, -OFFSET }, { x, getSize().y + OFFSET });
    }
}
