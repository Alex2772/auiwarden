#pragma once

#include <AUI/Render/ARenderContext.h>
#include <AUI/View/AView.h>
#include "model/State.h"

class GridView : public AView {
public:
    GridView(_<State> state);
    void render(ARenderContext ctx) override;
private:
    _<State> mState;
};

