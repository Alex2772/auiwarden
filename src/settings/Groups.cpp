//
// Created by alex2772 on 9/2/25.
//

#include "Groups.h"
#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/View/AButton.h>
#include <AUI/View/ADrawableView.h>
#include <AUI/View/ATabView.h>
#include <AUI/View/AText.h>
#include <AUI/View/AScrollArea.h>
#include <AUI/View/AForEachUI.h>
#include <AUI/View/AGroupBox.h>
#include <AUI/View/ATextField.h>
#include <AUI/View/ATextArea.h>
#include <AUI/Platform/AMessageBox.h>

using namespace declarative;
using namespace ass;

static _<AView> colorView(const AProperty<AColor>& color) {
    auto v = _new<AView>();
    AObject::connect(color, v, [&v = *v](AColor c) {
        v.setCustomStyle({
          Expanding(),
          BackgroundSolid { c },
        });
    });

    return v;
}

static _<AView> groupEditor(const _<Group>& group) {
    return GroupBox {
        Horizontal {
          Button { colorView(group->color), [group]{
                      group->color = Group::nextRandomColor();
                  } },
          _new<ATextField>() AUI_WITH_STYLE { Expanding() } && group->name,
        },
        Vertical {
          AText::fromString("List of window captions that will belong to this group. Use new line to separate them."),
          AScrollArea::Builder().withContents(
              _new<ATextArea>() && group->windowTitleContains
                  ).build() << ".input-field",
        },
    } AUI_WITH_STYLE { Expanding() };
}


Groups::Groups(_<State> state) {
    auto tabHost = _new<ATabView>();

    auto selectFirst = [this, state] {
      if (!state->database.groups->empty()) {
          mSelectedGroup = state->database.groups->first();
      } else {
          mSelectedGroup = nullptr;
      }
    };

    setContents(Vertical::Expanding {
      AText::fromString("Grouping allows to categorize time spans into groups (e.g. work, home, gaming, etc.)"),
      Horizontal::Expanding {
        Vertical::Expanding {
          Button { "Add group", [this, state] { state->database.groups << (mSelectedGroup = _new<Group>("New group")); } },
          AScrollArea::Builder()
              .withExpanding()
              .withContents(
                  AUI_DECLARATIVE_FOR(i, *state->database.groups, AVerticalLayout) {
                  _<AView> v = Horizontal {
                      Stacked { colorView(i->color) } AUI_WITH_STYLE { FixedSize(10_pt) },
                      Label { AUI_REACT(i->name) } AUI_WITH_STYLE { Expanding() },
                      Button { "-", [selectFirst, state, i] {
                                  state->database.groups.writeScope()->removeAll(i);
                                  selectFirst();
                              } },
                  };
                  connect(v->clicked, [this, i] {
                      mSelectedGroup = i;
                  });
                  return v;
              })
              .build() AUI_WITH_STYLE { BackgroundSolid { AColor::WHITE } },
        },
        CustomLayout::Expanding {} & mSelectedGroup.readProjected([state](const _<Group>& selectedGroup) -> _<AView> {
            if (selectedGroup == nullptr) {
                return Centered { Label { "No group selected" } };
            }
            return groupEditor(selectedGroup);
        }) } });
}
