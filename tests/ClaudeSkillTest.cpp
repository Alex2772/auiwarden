#include "ai/ClaudeSkill.h"
#include <gtest/gtest.h>
#include <cstdlib>
#include <AUI/IO/APath.h>

TEST(ClaudeSkill, InstallAndRemove) {
    auto dir = APath::getDefaultPath(APath::TEMP) / "auiwarden_claude_skill_test";
    dir.removeFileRecursive();
    dir.makeDirs();
#if AUI_PLATFORM_WIN
    _putenv_s("CLAUDE_CONFIG_DIR", dir.toStdString().c_str());
#else
    setenv("CLAUDE_CONFIG_DIR", dir.toStdString().c_str(), 1);
#endif

    aislop::isClaudeSkillInstalled.invalidate();
    EXPECT_FALSE(*aislop::isClaudeSkillInstalled);

    aislop::setClaudeSkillInstalled(true);
    EXPECT_TRUE(*aislop::isClaudeSkillInstalled);
    EXPECT_TRUE((dir / "skills" / "auiwarden" / "SKILL.md").isRegularFileExists());

    aislop::setClaudeSkillInstalled(false);
    EXPECT_FALSE(*aislop::isClaudeSkillInstalled);
    EXPECT_FALSE((dir / "skills" / "auiwarden").exists());

    dir.removeFileRecursive();
}
