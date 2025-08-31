#pragma once

#include "model/Database.h"

class Tracker {
public:
    explicit Tracker(_<Database> database) : mDatabase(std::move(database)) {}

private:
    _<Database> mDatabase;
};
