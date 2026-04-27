#pragma once

#include <QString>

struct BrowserSettings
{
    QString url = "WebRoot/index.html";
    int port = 8080;
    QString title = "Custom Browser Qt";
    QString iconPath = "Assets/default.svg";

    bool enableMinimize = true;
    bool enableMaximize = true;
    bool enableClose = true;
    bool enableMoveWindow = true;
    bool enableKioskModeToggle = true;

    QString startWindowSize = "1280x720";
    bool startInKiosk = false;
    QString password;

    bool devTools = false;
    bool extensionsEnabled = false;

    bool enableAutofill = true;
    bool enablePasswordSaving = false;
    bool enableTranslate = true;

    QString titlebarStyle = "native";
    bool showKioskButtonInContent = true;
    QString kioskButtonIconType = "unicode";
    QString kioskButtonIcon = "📌";
    QString kioskButtonText = "📌";
    QString kioskButtonTooltip = "Kiosk-Modus umschalten";

    QString customTitlebarBackground = "#202124";
    QString customTitlebarForeground = "#ffffff";
    QString customTitlebarButtonHover = "rgba(255, 255, 255, 0.16)";
    QString customTitlebarCloseHover = "#d93025";
    QString contentKioskBarBackground = "#f3f4f6";
    QString contentKioskBarForeground = "#111827";

    bool showAddressBar = false;
    bool allowTabs = false;

    double titleBarHeightPx = 32.0;
    double titleBarButtonHeightPx = 24.0;
    double titleBarButtonWidthRatio = 1.25;
    double titleBarButtonGapPx = 0.0;
};
