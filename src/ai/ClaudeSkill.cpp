#include "ClaudeSkill.h"

#include <cstdlib>
#include <filesystem>

#include "AUI/Common/AByteBuffer.h"
#include "AUI/Common/AException.h"
#include "AUI/IO/AFileInputStream.h"
#include "AUI/IO/AFileOutputStream.h"
#include "AUI/IO/APath.h"
#include "AUI/Platform/AProcess.h"
#include "AUI/Url/AUrl.h"
#include "AUI/Util/kAUI.h"

namespace {

APath skillDir() {
    APath configDir;
    if (const char* custom = std::getenv("CLAUDE_CONFIG_DIR"); custom && *custom) {
        configDir = APath(custom);
    } else {
        configDir = APath::getDefaultPath(APath::HOME) / ".claude";
    }
    return configDir / "skills" / "auiwarden";
}

APath skillFile() { return skillDir() / "SKILL.md"; }

APath currentExe() {
    // AppImage: the process executable points to a temporary mount.
    if (const char* appImage = std::getenv("APPIMAGE"); appImage && *appImage) {
        return APath(appImage);
    }
    return AProcess::self()->getPathToExecutable();
}

// the executable is substituted to a shell command line (inside the skill's code blocks).
AString quoteForShell(const APath& path) {
    AString out = "\"";
    for (char c : static_cast<const std::string&>(path)) {
        if (c == '\\' || c == '"' || c == '`' || c == '$') {
            out += '\\';
        }
        out += c;
    }
    out += "\"";
    return out;
}

AString skillContent() {
    auto exe = currentExe();
    if (exe.empty()) {
        throw AException("unable to determine current executable path");
    }
    auto text = AString::fromUtf8(AByteBuffer::fromStream(AUrl(":ai-slop/claude-skill.md").open()));
    return text.replacedAll("{{EXE}}", quoteForShell(exe));
}

}   // namespace

APropertyPrecomputed<bool> aislop::isClaudeSkillInstalled = [] {
    auto path = skillFile();
    if (!path.isRegularFileExists()) {
        return false;
    }
    try {
        return AString::fromUtf8(AByteBuffer::fromStream(AFileInputStream(path))) == skillContent();
    } catch (const AException&) {
        return false;
    }
};

void aislop::setClaudeSkillInstalled(bool enabled) {
    AUI_DEFER { isClaudeSkillInstalled.invalidate(); };
    auto path = skillFile();
    if (!enabled) {
        if (path.exists()) {
            path.removeFile();
        }
        std::error_code ec;
        std::filesystem::remove(static_cast<const std::string&>(skillDir()), ec);   // only if empty
        return;
    }
    const auto content = skillContent();
    path.parent().makeDirs();
    AFileOutputStream(path).write(content.toStdString().data(), content.toStdString().size());
}
