#pragma once

#include <AUI/Render/ARenderContext.h>
#include <AUI/View/AView.h>

class GridView : public AView {
public:
    void render(ARenderContext ctx) override;
};

