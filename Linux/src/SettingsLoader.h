#pragma once

#include "BrowserSettings.h"
#include <QString>

class SettingsLoader
{
public:
    static BrowserSettings load(const QString& iniPath);
};
