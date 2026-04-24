#include "BrowserBridge.h"
#include "MainWindow.h"

BrowserBridge::BrowserBridge(MainWindow* window, QObject* parent)
    : QObject(parent), m_window(window)
{
}

QVariantMap BrowserBridge::minimize(const QString& password)
{
    return m_window->jsMinimize(password);
}

QVariantMap BrowserBridge::restore(const QString& password)
{
    return m_window->jsRestore(password);
}

QVariantMap BrowserBridge::maximize(const QString& password)
{
    return m_window->jsMaximize(password);
}

QVariantMap BrowserBridge::toggleKiosk(const QString& password)
{
    return m_window->jsToggleKiosk(password);
}

QVariantMap BrowserBridge::closeWindow(const QString& password)
{
    return m_window->jsClose(password);
}

QVariantMap BrowserBridge::getState()
{
    return m_window->jsGetState();
}
