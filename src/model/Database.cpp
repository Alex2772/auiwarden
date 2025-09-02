//
// Created by alex2772 on 8/31/25.
//

#include "Database.h"
#include <AUI/Json/Conversion.h>
#include <AUI/IO/AFileInputStream.h>
#include <AUI/IO/AFileOutputStream.h>
#include <range/v3/view/reverse.hpp>
#include <range/v3/numeric/accumulate.hpp>

AJSON_FIELDS(Database, AJSON_FIELDS_ENTRY(spans))

// AJSON_FIELDS(Database::Day,
//              AJSON_FIELDS_ENTRY(spans)
//              AJSON_FIELDS_ENTRY(timepoint)
//)

AJSON_FIELDS(TimeSpan, AJSON_FIELDS_ENTRY(title) AJSON_FIELDS_ENTRY(begin) AJSON_FIELDS_ENTRY(end))

template <typename T1, typename T2>
struct AJsonConv<std::chrono::duration<T1, T2>> {
    static AJson toJson(std::chrono::duration<T1, T2> v) { return v.count(); }
    static void fromJson(const AJson& json, std::chrono::duration<T1, T2>& dst) {
        dst = std::chrono::duration<T1, T2>(json.asLongInt());
    }
};

template <typename T1, typename T2>
struct AJsonConv<std::chrono::time_point<T1, T2>> {
    static AJson toJson(std::chrono::time_point<T1, T2> v) { return v.time_since_epoch().count(); }
    static void fromJson(const AJson& json, std::chrono::time_point<T1, T2>& dst) {
        dst = std::chrono::time_point<T1, T2>(T2(json.asLongInt()));
    }
};
Database Database::load() { return aui::from_json<Database>(AJson::fromStream(AFileInputStream("database.json"))); }

void Database::save() { AFileOutputStream("database.json") << aui::to_json(*this); }

void Database::handleEvent(TimeSpan::Timepoint timepoint, AString activity) {
    using namespace std::chrono_literals;

    if (!spans->empty()) {
        if (timepoint - spans->last()->end <= 3min) {
            if (spans->last()->title == activity) {
                spans->last()->end = timepoint;
                return;
            }
        }
    }
    spans << aui::ptr::manage_shared(new TimeSpan {
      .begin = timepoint,
      .end = timepoint,
      .title = std::move(activity),
    });
}
