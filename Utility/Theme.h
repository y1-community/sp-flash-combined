#ifndef APP_THEME_H
#define APP_THEME_H

// Single owner of the app's color theme: a plain QPalette plus
// QSettings/QProcess OS-detection workarounds, valid back to Qt 4.6.
class QApplication;

namespace Theme
{

enum Mode
{
    FollowSystem = 0,
    Light = 1,
    Dark = 2
};

Mode LoadChoice();
void SaveChoice(Mode mode);

// Best-effort OS detection (registry/defaults/gsettings); unknown means light.
bool SystemUsesDark();

// Apply the saved choice (call once at startup).
void ApplySaved(QApplication *app);

void ApplyDark(QApplication *app);
void ApplyLight(QApplication *app);

// Dialogs with hardcoded light colors consult this to tone themselves down.
bool IsDarkActive();

} // namespace Theme

#endif // APP_THEME_H
