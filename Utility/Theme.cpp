#include "Theme.h"
#include "IniItem.h"

#include <QApplication>
#include <QPalette>
#include <QProcess>
#include <QSettings>
#include <QStringList>

namespace Theme
{

namespace
{
const char *THEME_FILE = "option.ini";
const char *THEME_SECTION = "Theme";
const char *THEME_KEY = "Mode";

bool s_darkActive = false;
QPalette s_lightPalette;
bool s_lightSaved = false;

// Each helper is compiled only on its own platform, matching the
// SystemUsesDark() dispatch below, so no helper is ever unused.
#ifdef Q_OS_WIN
bool DetectWindowsDark()
{
    const QSettings settings(
        "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
        QSettings::NativeFormat);
    return settings.value("AppsUseLightTheme", 1).toInt() == 0;
}
#endif

#ifdef Q_OS_MAC
bool DetectMacDark()
{
    QProcess proc;
    proc.start("defaults", QStringList() << "read" << "-g" << "AppleInterfaceStyle");
    if(!proc.waitForFinished(500))
    {
        return false;
    }
    return QString::fromLocal8Bit(proc.readAllStandardOutput()).trimmed() == "Dark";
}
#endif

#ifdef Q_OS_LINUX
bool DetectLinuxDark()
{
    QProcess proc;
    proc.start("gsettings",
               QStringList() << "get" << "org.gnome.desktop.interface" << "gtk-theme");
    if(!proc.waitForFinished(500))
    {
        return false;
    }
    return QString::fromLocal8Bit(proc.readAllStandardOutput()).contains("dark",
                                                                          Qt::CaseInsensitive);
}
#endif
} // namespace

Mode LoadChoice()
{
    IniItem item(THEME_FILE, THEME_SECTION, THEME_KEY);
    const int value = item.GetIntValue();
    if(value < FollowSystem || value > Dark)
    {
        return FollowSystem;
    }
    return static_cast<Mode>(value);
}

void SaveChoice(Mode mode)
{
    IniItem item(THEME_FILE, THEME_SECTION, THEME_KEY);
    item.SaveIntValue(static_cast<int>(mode));
}

bool SystemUsesDark()
{
#if defined(Q_OS_WIN)
    return DetectWindowsDark();
#elif defined(Q_OS_MAC)
    return DetectMacDark();
#elif defined(Q_OS_LINUX)
    return DetectLinuxDark();
#else
    return false;
#endif
}

void ApplyDark(QApplication *app)
{
    if(app == 0)
    {
        return;
    }
    if(!s_lightSaved)
    {
        s_lightPalette = app->palette();
        s_lightSaved = true;
    }
    QPalette dark;
    dark.setColor(QPalette::Window, QColor(53, 53, 53));
    dark.setColor(QPalette::WindowText, Qt::white);
    dark.setColor(QPalette::Base, QColor(43, 43, 43));
    dark.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
    dark.setColor(QPalette::Text, Qt::white);
    dark.setColor(QPalette::Button, QColor(53, 53, 53));
    dark.setColor(QPalette::ButtonText, Qt::white);
    dark.setColor(QPalette::BrightText, Qt::red);
    dark.setColor(QPalette::Highlight, QColor(42, 130, 218));
    dark.setColor(QPalette::HighlightedText, Qt::black);
    dark.setColor(QPalette::Link, QColor(42, 130, 218));
    dark.setColor(QPalette::ToolTipBase, Qt::white);
    dark.setColor(QPalette::ToolTipText, Qt::black);
    const QColor disabled(127, 127, 127);
    dark.setColor(QPalette::Disabled, QPalette::WindowText, disabled);
    dark.setColor(QPalette::Disabled, QPalette::Text, disabled);
    dark.setColor(QPalette::Disabled, QPalette::ButtonText, disabled);
    dark.setColor(QPalette::Disabled, QPalette::HighlightedText, disabled);
    app->setPalette(dark);
    s_darkActive = true;
}

void ApplyLight(QApplication *app)
{
    if(app == 0)
    {
        return;
    }
    if(s_lightSaved)
    {
        app->setPalette(s_lightPalette);
    }
    s_darkActive = false;
}

bool IsDarkActive()
{
    return s_darkActive;
}

void ApplySaved(QApplication *app)
{
    const Mode choice = LoadChoice();
    if(choice == Dark || (choice == FollowSystem && SystemUsesDark()))
    {
        ApplyDark(app);
    }
    else
    {
        ApplyLight(app);
    }
}

} // namespace Theme
