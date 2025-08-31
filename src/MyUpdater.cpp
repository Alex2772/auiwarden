//
// Created by alex2772 on 1/25/25.
//

#include "MyUpdater.h"

#include <range/v3/all.hpp>

#include <AUI/Util/kAUI.h>
#include <AUI/Updater/GitHub.h>
#include <AUI/Platform/AMessageBox.h>
#include <AUI/Updater/Semver.h>
#include <AUI/Updater/AppropriatePortablePackagePredicate.h>

static constexpr auto LOG_TAG = "MyUpdater";

AFuture<void> MyUpdater::checkForUpdatesImpl() {
    return AUI_THREADPOOL {
        try {
            auto githubLatestRelease = aui::updater::github::latestRelease("aui-framework", "example_app");
            ALogger::info(LOG_TAG) << "Found latest release: " << githubLatestRelease.tag_name;
            auto ourVersion = aui::updater::Semver::fromString(AUI_PP_STRINGIZE(AUI_CMAKE_PROJECT_VERSION));
            auto theirVersion = aui::updater::Semver::fromString(githubLatestRelease.tag_name);

            if (theirVersion <= ourVersion) {
                getThread()->enqueue([] {
                    AMessageBox::show(
                        nullptr, "No updates found", "You are running the latest version.", AMessageBox::Icon::INFO);
                });
                return;
            }
            aui::updater::AppropriatePortablePackagePredicate predicate {};
            auto it = ranges::find_if(
                githubLatestRelease.assets, predicate, &aui::updater::github::LatestReleaseResponse::Asset::name);
            if (it == ranges::end(githubLatestRelease.assets)) {
                ALogger::warn(LOG_TAG)
                    << "Newer version was found but a package appropriate for your platform is not available. "
                       "Expected: "
                    << predicate.getQualifierDebug() << ", got: "
                    << (githubLatestRelease.assets |
                        ranges::view::transform(&aui::updater::github::LatestReleaseResponse::Asset::name));
                getThread()->enqueue([] {
                    AMessageBox::show(
                        nullptr, "No updates found",
                        "Newer version was found but a package appropriate for your platform is not available.",
                        AMessageBox::Icon::INFO);
                });
                return;
            }
            ALogger::info(LOG_TAG) << "To download: " << (mDownloadUrl = it->browser_download_url);

            getThread()->enqueue([this, self = shared_from_this(), version = githubLatestRelease.tag_name] {
                if (AMessageBox::show(
                        nullptr, "New version found!", "Found version: {}\n\nWould you like to update?"_format(version),
                        AMessageBox::Icon::INFO, AMessageBox::Button::YES_NO) != AMessageBox::ResultButton::YES) {
                    return;
                }

                downloadUpdate();
            });

        } catch (const AException& e) {
            ALogger::err(LOG_TAG) << "Can't check for updates: " << e;
            getThread()->enqueue([] {
                AMessageBox::show(
                    nullptr, "Oops!", "There is an error occurred while checking for updates. Please try again later.",
                    AMessageBox::Icon::CRITICAL);
            });
        }
    };
}

AFuture<void> MyUpdater::downloadUpdateImpl(const APath& unpackedUpdateDir) {
    return AUI_THREADPOOL {
        try {
            AUI_ASSERTX(!mDownloadUrl.empty(), "make a successful call to checkForUpdates first");
            downloadAndUnpack(mDownloadUrl, unpackedUpdateDir);
            reportReadyToApplyAndRestart(makeDefaultInstallationCmdline());
        } catch (const AException& e) {
            ALogger::err(LOG_TAG) << "Can't check for updates: " << e;
            getThread()->enqueue([] {
                AMessageBox::show(
                    nullptr, "Oops!", "There is an error occurred while downloading update. Please try again later.",
                    AMessageBox::Icon::CRITICAL);
            });
        }
    };
}
