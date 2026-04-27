#include "SettingsLoader.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>

static bool getBool(const QJsonObject& obj, const char* key, bool fallback)
{
    if (!obj.contains(key)) return fallback;
    const auto v = obj.value(key);
    if (v.isBool()) return v.toBool();
    if (v.isString()) {
        const QString s = v.toString().trimmed().toLower();
        return s == "true" || s == "1" || s == "yes" || s == "on";
    }
    return fallback;
}

static double getDouble(const QJsonObject& obj, const char* key, double fallback)
{
    if (!obj.contains(key)) return fallback;
    const auto v = obj.value(key);
    if (v.isDouble()) return v.toDouble();
    if (v.isString()) {
        bool ok = false;
        const double d = v.toString().toDouble(&ok);
        return ok ? d : fallback;
    }
    return fallback;
}

static int getInt(const QJsonObject& obj, const char* key, int fallback)
{
    if (!obj.contains(key)) return fallback;
    const auto v = obj.value(key);
    if (v.isDouble()) return v.toInt();
    if (v.isString()) {
        bool ok = false;
        const int i = v.toString().toInt(&ok);
        return ok ? i : fallback;
    }
    return fallback;
}

static QString getString(const QJsonObject& obj, const char* key, const QString& fallback)
{
    if (!obj.contains(key)) return fallback;
    const auto v = obj.value(key);
    if (v.isString()) return v.toString();
    if (v.isDouble()) return QString::number(v.toDouble());
    if (v.isBool()) return v.toBool() ? "true" : "false";
    return fallback;
}

BrowserSettings SettingsLoader::load(const QString& iniPath)
{
    BrowserSettings s;

    QFile file(iniPath);
    if (!file.open(QIODevice::ReadOnly)) {
        return s;
    }

    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (!doc.isObject()) {
        return s;
    }

    const QJsonObject obj = doc.object();

    s.url = getString(obj, "url", s.url);
    s.port = getInt(obj, "port", s.port);
    s.title = getString(obj, "title", s.title);
    s.iconPath = getString(obj, "icon_path", s.iconPath);

    s.enableMinimize = getBool(obj, "enable_minimize", s.enableMinimize);
    s.enableMaximize = getBool(obj, "enable_maximize", s.enableMaximize);
    s.enableClose = getBool(obj, "enable_close", s.enableClose);
    s.enableMoveWindow = getBool(obj, "enable_move_window", s.enableMoveWindow);
    s.enableKioskModeToggle = getBool(obj, "enable_kiosk_mode_toggle", s.enableKioskModeToggle);

    s.startWindowSize = getString(obj, "start_window_size", s.startWindowSize);
    s.startInKiosk = getBool(obj, "start_in_kiosk", s.startInKiosk);
    s.password = getString(obj, "password", s.password);

    s.devTools = getBool(obj, "dev_tools", s.devTools);
    s.extensionsEnabled = getBool(obj, "extensions_enabled", s.extensionsEnabled);

    s.titlebarStyle = getString(obj, "titlebar_style", s.titlebarStyle).trimmed().toLower();
    if (obj.contains("use_native_titlebar")) {
        s.titlebarStyle = getBool(obj, "use_native_titlebar", s.titlebarStyle == "native") ? QStringLiteral("native") : QStringLiteral("custom");
    }
    if (s.titlebarStyle.isEmpty()) {
        s.titlebarStyle = QStringLiteral("native");
    }
    s.showKioskButtonInContent = getBool(obj, "show_kiosk_button_in_content", s.showKioskButtonInContent);
    s.kioskButtonIconType = getString(obj, "kiosk_button_icon_type", s.kioskButtonIconType).trimmed().toLower();
    s.kioskButtonIcon = getString(obj, "kiosk_button_icon", s.kioskButtonIcon);
    s.kioskButtonText = getString(obj, "kiosk_button_text", s.kioskButtonText);
    s.kioskButtonTooltip = getString(obj, "kiosk_button_tooltip", s.kioskButtonTooltip);

    s.customTitlebarBackground = getString(obj, "custom_titlebar_background", s.customTitlebarBackground);
    s.customTitlebarForeground = getString(obj, "custom_titlebar_foreground", s.customTitlebarForeground);
    s.customTitlebarButtonHover = getString(obj, "custom_titlebar_button_hover", s.customTitlebarButtonHover);
    s.customTitlebarCloseHover = getString(obj, "custom_titlebar_close_hover", s.customTitlebarCloseHover);
    s.contentKioskBarBackground = getString(obj, "content_kiosk_bar_background", s.contentKioskBarBackground);
    s.contentKioskBarForeground = getString(obj, "content_kiosk_bar_foreground", s.contentKioskBarForeground);

    s.showAddressBar = getBool(obj, "show_addressbar", s.showAddressBar);
    s.showAddressBar = getBool(obj, "show_adressbar", s.showAddressBar);
    s.showAddressBar = getBool(obj, "addressbar", s.showAddressBar);
    s.allowTabs = getBool(obj, "allow_tabs", s.allowTabs);
    s.allowTabs = getBool(obj, "tabs", s.allowTabs);

    s.titleBarHeightPx = getDouble(obj, "title_bar_height_px", s.titleBarHeightPx);
    s.titleBarButtonHeightPx = getDouble(obj, "title_bar_button_height_px", s.titleBarButtonHeightPx);
    s.titleBarButtonWidthRatio = getDouble(obj, "title_bar_button_width_ratio", s.titleBarButtonWidthRatio);
    s.titleBarButtonGapPx = getDouble(obj, "title_bar_button_gap_px", s.titleBarButtonGapPx);

    return s;
}
