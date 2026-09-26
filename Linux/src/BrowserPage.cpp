#include "BrowserPage.h"
#include <QWebEngineProfile>

BrowserPage::BrowserPage(QWebEngineProfile* profile, QObject* parent)
    : QWebEnginePage(profile, parent)
{
}

QWebEnginePage* BrowserPage::createWindow(QWebEnginePage::WebWindowType type)
{
    if (!createNewWindowPage) {
        return nullptr;
    }

    return createNewWindowPage(type);
}
