#pragma once

#include "BrowserSettings.h"
#include <QStringList>

class CliOverrideParser
{
public:
    static void apply(BrowserSettings& settings, const QStringList& arguments);
};
