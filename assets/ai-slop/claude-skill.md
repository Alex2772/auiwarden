---
name: auiwarden
description: Read the user's personal computer activity log collected by AUIwarden (time per app/window group, per day, per hour of day, top window titles). Use when the user asks about how they spend their time, their screen time, productivity, work/gaming/browsing balance, daily routine, or asks for a summary of their activity and recommendations based on it.
---

# AUIwarden

AUIwarden is a background tracker running on the user's computer. It records the title of the active window once per
minute (nothing is recorded while the user is away from keyboard) and sorts window titles into user-defined groups
(i.e. "Work", "Gaming", "Web"). Titles matching no group fall into "Uncategorized".

All data is read through a command line interface that prints JSON. It is read-only and fast; call it as needed.

## Commands

```
{{EXE}} cli report [--days N | --from YYYY-MM-DD [--to YYYY-MM-DD]] [--top N]
{{EXE}} cli groups
```

The AUI framework prints a few startup log lines to stdout before the program runs. The JSON result is always a single
line and always the LAST line of stdout: append `| tail -n 1` to the command (errors go to stderr, exit code is non-zero
on failure).

- `report` - defaults to the last 7 days including today.
  - `period` - the range, timezone and current local time.
  - `total_minutes` - tracked (active) minutes in the range. This is NOT wall-clock time: idle time and time the computer
    was off are not counted.
  - `by_group` - minutes and percent of tracked time per group, descending.
  - `by_day` - one entry per day (including days with no data) with a per-group breakdown.
  - `by_hour_of_day` - 24 numbers; index is the local hour (0-23), value is the minutes accumulated at that hour over
    the whole range. Use it to find the user's working hours and late-night activity.
  - `top_titles` - most time-consuming window titles with their group.
- `groups` - configured groups and the substrings (case-insensitive) of window titles that put a title into the group.

## How to answer

1. Pick the range from the request ("this week" -> `--days 7`, "yesterday" -> `--from D --to D`, "this month" ->
   `--days 30`). When the user asks for trends or comparisons, make several calls (i.e. this week vs. the previous one
   using `--from`/`--to`).
2. Base every claim on the numbers returned. Quote minutes as hours and minutes. If `total_minutes` is small or days are
   missing, say that the data is incomplete instead of drawing conclusions.
3. A large "Uncategorized" share or noisy titles in `top_titles` usually mean the groups need tuning: suggest concrete
   substrings to add (the user edits groups in AUIwarden settings, you cannot).
4. Recommendations should be specific and tied to the data (peak hours, days with long uninterrupted sessions, late-night
   usage, imbalance between groups), short, and non-judgmental.
5. This is personal data. Do not write it to files or share it anywhere unless the user asks.

If a command fails with "can't load database", AUIwarden has not collected any data yet; tell the user to launch it.
