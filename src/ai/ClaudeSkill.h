#pragma once

#include "AUI/Common/APropertyPrecomputed.h"

namespace aislop {

/**
 * @brief Checks whether the AUIwarden skill is installed to Claude and is up to date (points to the current
 * executable).
 */
extern APropertyPrecomputed<bool> isClaudeSkillInstalled;

/**
 * @brief Installs (true) or removes (false) the AUIwarden skill of Claude.
 * @details The skill text is taken from assets (`:ai-slop/claude-skill.md`) and is placed to
 * `~/.claude/skills/auiwarden/SKILL.md` (or `$CLAUDE_CONFIG_DIR/skills/...`).
 */
void setClaudeSkillInstalled(bool enabled);

}   // namespace aislop
