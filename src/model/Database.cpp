//
// Created by alex2772 on 8/31/25.
//

#include "Database.h"
#include <AUI/Json/Conversion.h>
#include <AUI/IO/AFileInputStream.h>
#include <AUI/IO/AFileOutputStream.h>

AJSON_FIELDS(Database,
             AJSON_FIELDS_ENTRY(days)
)

AJSON_FIELDS(Database::Day,
             AJSON_FIELDS_ENTRY(spans)
             AJSON_FIELDS_ENTRY(timepoint)
)

AJSON_FIELDS(TimeSpan,
             AJSON_FIELDS_ENTRY(duration)
             AJSON_FIELDS_ENTRY(title)
)

template<typename T1, typename T2>
struct AJsonConv<std::chrono::duration<T1, T2>> {
    static AJson toJson(std::chrono::duration<T1, T2> v) {
        return v.count();
    }
    static void fromJson(const AJson& json, std::chrono::duration<T1, T2>& dst) {
        dst = std::chrono::duration<T1, T2>(json.asLongInt());
    }
};

template<typename T1, typename T2>
struct AJsonConv<std::chrono::time_point<T1, T2>> {
    static AJson toJson(std::chrono::time_point<T1, T2> v) {
        return v.time_since_epoch().count();
    }
    static void fromJson(const AJson& json, std::chrono::time_point<T1, T2>& dst) {
        dst = std::chrono::time_point<T1, T2>(T2(json.asLongInt()));
    }
};
Database Database::load() {
    return aui::from_json<Database>(AJson::fromStream(AFileInputStream("database.json")));
}

void Database::save() {
    AFileOutputStream("database.json") << aui::to_json(*this);
}
