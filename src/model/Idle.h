#pragma once

enum class Idle {
    AWAY_FROM_KEYBOARD,
    USER_PRESENT,
};

AUI_ENUM_VALUES(Idle, Idle::AWAY_FROM_KEYBOARD, Idle::USER_PRESENT)
