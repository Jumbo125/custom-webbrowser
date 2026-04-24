#pragma once

#include <QWebEnginePage>
#include <functional>

class BrowserPage : public QWebEnginePage
{
public:
    explicit BrowserPage(QObject* parent = nullptr);

    std::function<QWebEnginePage*(QWebEnginePage::WebWindowType)> createNewWindowPage;

protected:
    QWebEnginePage* createWindow(QWebEnginePage::WebWindowType type) override;
};
