#pragma once

#include <AUI/Platform/AWindow.h>
#include "MyUpdater.h"
#include "model/Database.h"

class MainWindow: public AWindow {
public:
    MainWindow(_<MyUpdater> updater);
    void onCloseButtonClicked() override;

private:
    _<MyUpdater> mUpdater;
    Database mDatabase;

    void save();
};
