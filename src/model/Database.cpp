//
// Created by alex2772 on 8/31/25.
//

#include "Database.h"
#include <AUI/Json/Conversion.h>
#include <AUI/IO/AFileInputStream.h>
#include <AUI/IO/AFileOutputStream.h>
#include <range/v3/view/reverse.hpp>
#include <range/v3/numeric/accumulate.hpp>

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

void Database::handleEvent(std::chrono::system_clock::time_point timepoint, AOptional<AString> activity) {
    auto day = [&] {
      for (auto& d : days | ranges::view::reverse) {
          if (d->timepoint == floor<std::chrono::days>(timepoint)) {
              // append to the existing day
              return d;
          }
      }
      auto day = _new<Database::Day>(floor<std::chrono::days>(timepoint));
      days << day;
      return day;
    }();

    using namespace std::chrono_literals;

    auto priorTime = ranges::accumulate(day->spans, 0min, std::plus<> {}, [](const _<TimeSpan>& s) { return s->duration; });;
    auto eventTime = ceil<std::chrono::minutes>(timepoint - floor<std::chrono::days>(timepoint));

    if (priorTime > eventTime) {
        // this event is the past, ignore it
        return;
    }
    auto diff = eventTime - priorTime;
    if (diff <= 2min) {
        if (day->spans.last()->title == activity) {
            day->spans.last()->duration += diff;
            return;
        }
    }
    day->spans << _new<TimeSpan>(diff, std::nullopt); // inject idle activity
    day->spans << _new<TimeSpan>(1min, std::move(activity));
}
