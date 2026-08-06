#pragma once

#include <QObject>
#include <QVariantMap>

class MainWindow;

class BrowserBridge : public QObject
{
    Q_OBJECT
public:
    explicit BrowserBridge(MainWindow* window, QObject* parent = nullptr);

    Q_INVOKABLE QVariantMap minimize(const QString& password);
    Q_INVOKABLE QVariantMap restore(const QString& password);
    Q_INVOKABLE QVariantMap maximize(const QString& password);
    Q_INVOKABLE QVariantMap toggleKiosk(const QString& password);
    Q_INVOKABLE QVariantMap closeWindow(const QString& password);
    Q_INVOKABLE QVariantMap getState();
    Q_INVOKABLE QVariantMap runSystemAction(const QString& actionName, const QString& parameter);

private:
    MainWindow* m_window = nullptr;
};
