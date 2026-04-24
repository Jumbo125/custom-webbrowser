#include "CliOverrideParser.h"

#include <QHash>

static QString normalizeKey(QString key)
{
    key = key.trimmed();
    while (key.startsWith('-')) key.remove(0, 1);
    return key.replace('-', '_').toLower();
}

static bool toBool(const QString& value, bool fallback)
{
    const QString s = value.trimmed().toLower();
    if (s == "true" || s == "1" || s == "yes" || s == "on") return true;
    if (s == "false" || s == "0" || s == "no" || s == "off") return false;
    return fallback;
}

static double toDouble(const QString& value, double fallback)
{
    bool ok = false;
    const double d = value.toDouble(&ok);
    return ok ? d : fallback;
}

static int toInt(const QString& value, int fallback)
{
    bool ok = false;
    const int i = value.toInt(&ok);
    return ok ? i : fallback;
}

void CliOverrideParser::apply(BrowserSettings& s, const QStringList& arguments)
{
    QHash<QString, QString> map;

    for (int i = 1; i < arguments.size(); ++i) {
        QString arg = arguments.at(i);
        if (!arg.startsWith("--")) continue;

        arg.remove(0, 2);
        QString key;
        QString value;

        const int eq = arg.indexOf('=');
        if (eq >= 0) {
            key = arg.left(eq);
            value = arg.mid(eq + 1);
        } else {
            key = arg;
            if (i + 1 < arguments.size() && !arguments.at(i + 1).startsWith("--")) {
                value = arguments.at(++i);
            } else {
                value = "true";
            }
        }

        map.insert(normalizeKey(key), value);
    }

    auto has = [&](const char* key) { return map.contains(QString::fromLatin1(key)); };
    auto val = [&](const char* key) { return map.value(QString::fromLatin1(key)); };

    if (has("url")) s.url = val("url");
    if (has("port")) s.port = toInt(val("port"), s.port);
    if (has("title")) s.title = val("title");
    if (has("icon_path")) s.iconPath = val("icon_path");

    if (has("enable_minimize")) s.enableMinimize = toBool(val("enable_minimize"), s.enableMinimize);
    if (has("enable_maximize")) s.enableMaximize = toBool(val("enable_maximize"), s.enableMaximize);
    if (has("enable_close")) s.enableClose = toBool(val("enable_close"), s.enableClose);
    if (has("enable_move_window")) s.enableMoveWindow = toBool(val("enable_move_window"), s.enableMoveWindow);
    if (has("enable_kiosk_mode_toggle")) s.enableKioskModeToggle = toBool(val("enable_kiosk_mode_toggle"), s.enableKioskModeToggle);

    if (has("start_window_size")) s.startWindowSize = val("start_window_size");
    if (has("start_in_kiosk")) s.startInKiosk = toBool(val("start_in_kiosk"), s.startInKiosk);
    if (has("password")) s.password = val("password");

    if (has("dev_tools")) s.devTools = toBool(val("dev_tools"), s.devTools);
    if (has("extensions_enabled")) s.extensionsEnabled = toBool(val("extensions_enabled"), s.extensionsEnabled);

    if (has("show_addressbar")) s.showAddressBar = toBool(val("show_addressbar"), s.showAddressBar);
    if (has("show_adressbar")) s.showAddressBar = toBool(val("show_adressbar"), s.showAddressBar);
    if (has("addressbar")) s.showAddressBar = toBool(val("addressbar"), s.showAddressBar);
    if (has("allow_tabs")) s.allowTabs = toBool(val("allow_tabs"), s.allowTabs);
    if (has("tabs")) s.allowTabs = toBool(val("tabs"), s.allowTabs);

    if (has("title_bar_height_px")) s.titleBarHeightPx = toDouble(val("title_bar_height_px"), s.titleBarHeightPx);
    if (has("title_bar_button_height_px")) s.titleBarButtonHeightPx = toDouble(val("title_bar_button_height_px"), s.titleBarButtonHeightPx);
    if (has("title_bar_button_width_ratio")) s.titleBarButtonWidthRatio = toDouble(val("title_bar_button_width_ratio"), s.titleBarButtonWidthRatio);
    if (has("title_bar_button_gap_px")) s.titleBarButtonGapPx = toDouble(val("title_bar_button_gap_px"), s.titleBarButtonGapPx);
}
