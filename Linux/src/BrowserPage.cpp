#include "BrowserPage.h"

BrowserPage::BrowserPage(QObject* parent)
    : QWebEnginePage(parent)
{
}

QWebEnginePage* BrowserPage::createWindow(QWebEnginePage::WebWindowType type)
{
    if (!createNewWindowPage) {
        return nullptr;
    }

    return createNewWindowPage(type);
}
