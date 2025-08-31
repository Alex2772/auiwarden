#pragma once

#include <AUI/Updater/AUpdater.h>

class MyUpdater: public AUpdater {
public:
    ~MyUpdater() override = default;

protected:
    AFuture<void> checkForUpdatesImpl() override;
    AFuture<void> downloadUpdateImpl(const APath& unpackedUpdateDir) override;

private:
    AString mDownloadUrl;
};
