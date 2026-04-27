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

    bool showAddressBar = false;
    bool allowTabs = false;

    double titleBarHeightPx = 32.0;
    double titleBarButtonHeightPx = 24.0;
    double titleBarButtonWidthRatio = 1.25;
    double titleBarButtonGapPx = 0.0;
};
