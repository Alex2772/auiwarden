#include "Cli.h"

#include <chrono>
#include <cmath>
#include <cstdio>
#include <iostream>

#include <fmt/chrono.h>
#include <range/v3/algorithm.hpp>

#include <AUI/Common/AMap.h>
#include <AUI/Json/AJson.h>
#include <AUI/Logging/ALogger.h>
#include <AUI/Traits/strings.h>

#include "model/Database.h"

using namespace std::chrono;
using namespace std::chrono_literals;

namespace {

constexpr auto UNCATEGORIZED = "Uncategorized";
constexpr auto DEFAULT_DAYS = 7;
constexpr auto DEFAULT_TOP = 15;

constexpr auto HELP = R"(auiwarden cli - activity tracker report for AI agents. Output is JSON.

usage: auiwarden cli <command> [options]

commands:
  report   time spent per group / day / hour of day / window title
  groups   configured groups (rules used to categorize window titles)
  help     this text

report options:
  --days N          last N days including today (default 7)
  --from YYYY-MM-DD first day, local time (use with --to)
  --to YYYY-MM-DD   last day inclusive, local time (default: today)
  --top N           number of window titles to list (default 15)
)";

struct UsageError : AException {
    using AException::AException;
};

struct Options {
    AMap<AString, AString> values;

    static Options parse(const AStringVector& args, size_t from) {
        Options result;
        for (size_t i = from; i < args.size(); i += 2) {
            const auto& key = args[i];
            if (!key.startsWith("--")) {
                throw UsageError("unexpected argument: " + key);
            }
            if (i + 1 >= args.size()) {
                throw UsageError("missing value for " + key);
            }
            result.values[key] = args[i + 1];
        }
        return result;
    }

    AOptional<AString> get(const AString& key) const {
        if (auto it = values.find(key); it != values.end()) {
            return it->second;
        }
        return std::nullopt;
    }

    int getPositiveInt(const AString& key, int defaultValue) const {
        auto v = get(key);
        if (!v) {
            return defaultValue;
        }
        try {
            auto n = v->toIntOrException();
            if (n > 0) {
                return n;
            }
        } catch (const AException&) {
        }
        throw UsageError(key + " expects a positive integer, got: " + *v);
    }
};

local_days parseDate(const AString& key, const AString& value) {
    int y = 0, m = 0, d = 0;
    char tail = 0;
    if (std::sscanf(value.toStdString().c_str(), "%d-%d-%d%c", &y, &m, &d, &tail) != 3) {
        throw UsageError(key + " expects YYYY-MM-DD, got: " + value);
    }
    const year_month_day ymd { year(y), month(unsigned(m)), day(unsigned(d)) };
    if (!ymd.ok()) {
        throw UsageError(key + " is not a valid date: " + value);
    }
    return local_days(ymd);
}

sys_time<minutes> toSys(local_days day) {
    return floor<minutes>(current_zone()->to_sys(day, choose::earliest));
}

// fmt can't format local_time with older standard libraries, so reinterpret it as a (naive) sys_time
template <typename Duration>
sys_time<Duration> naive(local_time<Duration> t) { return sys_time<Duration>(t.time_since_epoch()); }

AString formatDate(local_days day) { return "{:%F}"_format(naive(day)); }

AJson::Array toArray(auto&& range) {
    AJson::Array result;
    for (auto&& i : range) {
        result.push_back(std::move(i));
    }
    return result;
}

double round1(double v) { return std::round(v * 10.0) / 10.0; }

int report(const Options& options) {
    const auto now = floor<minutes>(system_clock::now());
    const auto today = floor<days>(current_zone()->to_local(now));

    local_days firstDay = today;
    local_days lastDay = today;
    if (auto from = options.get("--from")) {
        firstDay = parseDate("--from", *from);
        if (auto to = options.get("--to")) {
            lastDay = parseDate("--to", *to);
        }
        if (lastDay < firstDay) {
            throw UsageError("--to is earlier than --from");
        }
    } else {
        if (options.get("--to")) {
            throw UsageError("--to requires --from");
        }
        firstDay = today - days(options.getPositiveInt("--days", DEFAULT_DAYS) - 1);
    }
    const auto top = options.getPositiveInt("--top", DEFAULT_TOP);

    const auto rangeBegin = toSys(firstDay);
    const auto rangeEnd = std::min(toSys(lastDay + days(1)), now + 1min);   // the future has no data

    Database database;
    try {
        database = Database::load();
    } catch (const AException& e) {
        throw AException("can't load database (has AUIwarden been run yet?): " + e.getMessage());
    }

    struct Day {
        AMap<AString, int> groups;
        int total = 0;
    };
    AMap<AString, int> byGroup;
    std::map<local_days, Day> byDay;
    std::array<int, 24> byHour {};
    struct Title {
        AString group;
        int minutes = 0;
    };
    AMap<AString, Title> byTitle;
    int total = 0;

    for (auto day = firstDay; day <= lastDay; day += days(1)) {
        byDay[day];   // make sure days without data are listed too
    }

    for (const auto& span : *database.spans) {
        const auto begin = std::max(span->begin, rangeBegin);
        const auto end = std::min(span->end + 1min, rangeEnd);   // span end is inclusive
        if (begin >= end) {
            continue;
        }
        const auto group = database.findGroup(span->title);
        const AString groupName = group ? *group->name : AString(UNCATEGORIZED);
        const auto title = span->title.empty() ? AString("(no title)") : span->title;

        for (auto t = begin; t < end; t += 1min) {
            const auto local = current_zone()->to_local(t);
            const auto day = floor<days>(local);
            auto& d = byDay[day];
            d.groups[groupName] += 1;
            d.total += 1;
            byHour[size_t(duration_cast<hours>(local - day).count())] += 1;
        }
        const int minutesInSpan = int((end - begin).count());
        byGroup[groupName] += minutesInSpan;
        auto& entry = byTitle[title];
        entry.group = groupName;
        entry.minutes += minutesInSpan;
        total += minutesInSpan;
    }

    auto sortedByMinutes = [](auto&& map, auto&& getMinutes) {
        std::vector<std::pair<AString, typename std::decay_t<decltype(map)>::mapped_type>> v(map.begin(), map.end());
        ranges::sort(v, std::greater<> {}, getMinutes);
        return v;
    };

    AJson::Array groupsJson;
    for (const auto& [name, minutes] : sortedByMinutes(byGroup, [](const auto& i) { return i.second; })) {
        groupsJson.push_back(AJson {
          { "group", name },
          { "minutes", minutes },
          { "percent", round1(100.0 * minutes / std::max(total, 1)) },
        });
    }

    AJson::Array daysJson;
    for (const auto& [day, data] : byDay) {
        AJson::Object groups;
        for (const auto& [name, minutes] : sortedByMinutes(data.groups, [](const auto& i) { return i.second; })) {
            groups.push_back({ name, minutes });
        }
        daysJson.push_back(AJson {
          { "date", formatDate(day) },
          { "weekday", "{:%a}"_format(naive(day)) },
          { "total_minutes", data.total },
          { "groups", std::move(groups) },
        });
    }

    AJson::Array hoursJson;
    for (auto m : byHour) {
        hoursJson.push_back(m);
    }

    AJson::Array titlesJson;
    for (const auto& [title, data] :
         sortedByMinutes(byTitle, [](const auto& i) { return i.second.minutes; })) {
        if (int(titlesJson.size()) >= top) {
            break;
        }
        titlesJson.push_back(AJson {
          { "title", title },
          { "group", data.group },
          { "minutes", data.minutes },
        });
    }

    AJson result {
        { "period",
          AJson {
            { "from", formatDate(firstDay) },
            { "to", formatDate(lastDay) },
            { "days", int((lastDay - firstDay).count()) + 1 },
            { "timezone", AString(current_zone()->name()) },
            { "now", "{:%F %R}"_format(naive(current_zone()->to_local(now))) },
          } },
        { "total_minutes", total },
        { "by_group", std::move(groupsJson) },
        { "by_day", std::move(daysJson) },
        { "by_hour_of_day", std::move(hoursJson) },
        { "top_titles", std::move(titlesJson) },
    };
    std::cout << AJson::toString(result) << std::endl;
    return 0;
}

int groups() {
    Database database;
    try {
        database = Database::load();
    } catch (const AException& e) {
        throw AException("can't load database (has AUIwarden been run yet?): " + e.getMessage());
    }
    AJson::Array result;
    for (const auto& g : *database.groups) {
        AJson::Array rules;
        for (const auto& line : g->windowTitleContains->split('\n')) {
            if (!line.empty()) {
                rules.push_back(line);
            }
        }
        result.push_back(AJson {
          { "group", *g->name },
          { "window_title_contains", std::move(rules) },
        });
    }
    std::cout << AJson::toString(AJson { { "groups", std::move(result) } }) << std::endl;
    return 0;
}

}   // namespace

AOptional<int> cli::tryRun(const AStringVector& args) {
    // args[0] is the executable
    if (args.size() < 2 || args[1] != "cli") {
        return std::nullopt;
    }
    try {
        const AString command = args.size() > 2 ? args[2] : AString("help");
        if (command == "help" || command == "--help") {
            std::cout << HELP;
            return 0;
        }
        if (command == "report") {
            return report(Options::parse(args, 3));
        }
        if (command == "groups") {
            return groups();
        }
        throw UsageError("unknown command: " + command);
    } catch (const UsageError& e) {
        std::cerr << "error: " << e.getMessage().toStdString() << "\n\n" << HELP;
        return 2;
    } catch (const AException& e) {
        std::cerr << "error: " << e.getMessage().toStdString() << std::endl;
        return 1;
    }
}
