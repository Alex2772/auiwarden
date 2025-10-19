//
// Created by alex2772 on 10/14/25.
//

#include "PagePie.h"
#include <AUI/Util/UIBuildingHelpers.h>
#include "view/TimeSpanView.h"
#include <AUI/View/ALabel.h>
#include <AUI/View/AScrollArea.h>
#include <AUI/View/AForEachUI.h>
#include <range/v3/all.hpp>
#include <AUI/View/ASpacerFixed.h>
#include <AUI/View/AHDividerView.h>

using namespace declarative;
using namespace ass;

namespace {

struct PieChartEntry {
    AAngleRadians degrees;
    AColor color;
    AString title;
    std::chrono::minutes minutes;
};

class PieChartView : public AView {
public:
    PieChartView(_<APropertyPrecomputed<std::vector<PieChartEntry>>> values): mValues(std::move(values)) {
        connect(mValues->changed, me::redraw);
    }

    void render(ARenderContext ctx) override {
        AView::render(ctx);

        static constexpr auto WIDTH = 32_dp;
        ctx.render.roundedRectangleBorder(ASolidBrush { AColor::WHITE }, { 0, 0 }, getSize(), getSize().x / 2, WIDTH);

        AAngleRadians accumulator = 0_deg;
        for (const auto& i : **mValues) {
            {
                RenderHints::PushMask mask(ctx.render, [&] {
                    ctx.render.squareSector(ASolidBrush {}, { 0, 0 }, getSize(), accumulator, accumulator + i.degrees);
                });
                ctx.render.roundedRectangleBorder(ASolidBrush { i.color }, { 0, 0 }, getSize(), getSize().x / 2, WIDTH);
            }
            accumulator += i.degrees;
        }
    }

private:
    _<APropertyPrecomputed<std::vector<PieChartEntry>>> mValues;
};

_<AView> pieWithLegend(AStringView title, const _<State>& state, std::chrono::local_time<std::chrono::seconds> pointOfInterest, std::chrono::system_clock::duration durationOfInterest) {
    using namespace std::chrono;

    auto pointOfInterestBegin = pointOfInterest;
    auto pointOfInterestEnd = pointOfInterest + durationOfInterest;
    auto value = _new<APropertyPrecomputed<std::vector<PieChartEntry>>>([=] {
        auto intersectsWithDay = [=](const _<TimeSpan>& span) {
            auto begin = current_zone()->to_local(span->begin);
            auto end = current_zone()->to_local(span->end);
            return begin >= pointOfInterestBegin && begin <= pointOfInterestEnd ||
                   end >= pointOfInterestBegin && end <= pointOfInterestEnd;
        };
        AMap<_<Group>, minutes> groups;
        for (const auto& i : state->database.spans | ranges::view::filter(intersectsWithDay)) {
            auto duration = minutes(i->end - i->begin);
            groups[state->database.findGroup(i->title)] += duration + 1min; // adding 1min because duration is exclusive
        }

        std::vector<PieChartEntry> v =
            groups | ranges::view::transform([=](const auto& i) {
                auto color = i.first == nullptr ? AColor::RED : *i.first->color;
                return PieChartEntry {
                    .degrees = AAngleRadians(
                        float(i.second.count()) * AAngleRadians(360_deg).radians() / floor<minutes>(durationOfInterest).count()),
                    .color = color,
                    .title = i.first == nullptr ? "Uncategorized" : *i.first->name,
                    .minutes = i.second,
                };
            }) |
            ranges::to_vector;
        ranges::sort(v, std::greater<> {}, [](const PieChartEntry& i) { return i.degrees.radians(); });
        return v;
    });

    return Vertical {
        SpacerFixed { 16_dp },
        _new<AHDividerView>(),
        SpacerFixed { 16_dp },
        Horizontal {
          Centered {
            _new<PieChartView>(value) AUI_WITH_STYLE { FixedSize { 200_dp, 200_dp } },
            Vertical {
              Label { std::move(title) } AUI_WITH_STYLE { FontSize { 14_pt }, ATextAlign::CENTER },
              Label { "{:%D} + {}"_format(pointOfInterestBegin, floor<days>(durationOfInterest)) } AUI_WITH_STYLE { FontSize { 9_pt }, ATextAlign::CENTER },
            },
          },
          Centered {
            AUI_DECLARATIVE_FOR(i, **value, AVerticalLayout) {
                return Horizontal {
                    Centered {
                      _new<AView>() AUI_WITH_STYLE { FixedSize { 12_dp, 12_dp }, BackgroundSolid { i.color } } },
                    Label {
                      "{} - {}% - {}min"_format(i.title, int(i.degrees.degrees() / 360.f * 100), i.minutes.count()) },
                } AUI_WITH_STYLE { LayoutSpacing { 2_dp } };
            },
          },
        } AUI_WITH_STYLE { LayoutSpacing { 8_dp } },
    };
}
}   // namespace

_<AView> views::pagePie(const _<State>& state) {
    using namespace std::chrono;

    auto view = Centered::Expanding {
        Vertical {
          Label { "Summary" } AUI_WITH_STYLE { FontSize { 16_pt } },
          SpacerFixed { 8_dp },
          pieWithLegend("Today", state, floor<days>(current_zone()->to_local(*state->currentTime)), days(1)),
          pieWithLegend("Last 24H", state, current_zone()->to_local(*state->currentTime) - days(1), days(1)),
          pieWithLegend("Yesterday", state, floor<days>(current_zone()->to_local(*state->currentTime)) - days(1), days(1)),
          pieWithLegend("Last 7D", state, current_zone()->to_local(*state->currentTime) - days(7), days(7)),
          pieWithLegend("Last 30D", state, current_zone()->to_local(*state->currentTime) - days(30), days(30)),
          SpacerFixed { 8_dp },
        } AUI_WITH_STYLE { Expanding(), MaxSize { 500_dp, {} } },
    };
    return AScrollArea::Builder().withContents(view).build();
}
