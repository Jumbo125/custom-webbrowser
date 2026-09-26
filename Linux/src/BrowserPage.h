#pragma once

#include <QWebEnginePage>
#include <functional>

class QWebEngineProfile;

class BrowserPage : public QWebEnginePage
{
public:
    explicit BrowserPage(QWebEngineProfile* profile, QObject* parent = nullptr);

    std::function<QWebEnginePage*(QWebEnginePage::WebWindowType)> createNewWindowPage;

protected:
    QWebEnginePage* createWindow(QWebEnginePage::WebWindowType type) override;
};
