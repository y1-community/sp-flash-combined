#ifndef __FLASH_TOOL_VERSION
#define __FLASH_TOOL_VERSION

#include <iostream>
class ToolInfo{
public:
    static std::string ToolName();
    static std::string BuildInfo();
    static std::string Comments();
    static std::string LoadPath();
    static std::string ToString();
    static bool IsCustomerVer();
    static bool IsTempVer();
    static std::string VersionNum();
    // Single source for the user-visible identity. Every title bar,
    // menu, dialog title, and message box names the app through these.
    static std::string DisplayName();
    static std::string AboutName();
    static std::string DonateUrl();
    // True on builds where libflash cannot run (macOS, or any ARM64
    // build): the engine is mtkclient + Penumbra instead. Decided by
    // compile-time OS/arch macros so it works back to Qt 4.
    static bool UsesReplacementEngine();
    // Rich-text About credit naming both replacement projects, their
    // authors, and their URLs.
    static std::string EngineCredit();
private:
    ToolInfo();
};
#endif
