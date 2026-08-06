#pragma once

#include "BrowserSettings.h"

#include <QHash>
#include <QMainWindow>
#include <QUrl>
#include <QPoint>
#include <QVariantMap>

class BrowserBridge;
class QLabel;
class QPushButton;
class QHBoxLayout;
class QVBoxLayout;
class QWebChannel;
class QWebEngineView;
class QCloseEvent;
class QShowEvent;
class QMoveEvent;
class QWebEngineScript;
class QTabWidget;
class QLineEdit;
class QToolButton;
class QWidget;
class QUrl;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(const BrowserSettings& settings, const QString& iniDir, QWidget* parent = nullptr);

    void applyInitialWindowState();

    QVariantMap jsMinimize(const QString& password);
    QVariantMap jsRestore(const QString& password);
    QVariantMap jsMaximize(const QString& password);
    QVariantMap jsToggleKiosk(const QString& password);
    QVariantMap jsClose(const QString& password);
    QVariantMap jsGetState() const;
    QVariantMap jsRunSystemAction(const QString& actionName, const QString& parameter);

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;
    void showEvent(QShowEvent* event) override;
    void moveEvent(QMoveEvent* event) override;
    void closeEvent(QCloseEvent* event) override;

private:
    void setupUi();
    void setupContentKioskBar(QVBoxLayout* rootLayout);
    void setupAddressBar(QVBoxLayout* rootLayout);
    void setupWebView();
    void configureWebView(QWebEngineView* view);
    void setupDevTools();
    void applyWindowMode();
    void applyTitleBarSizing();
    void loadUrl();

    QWebEngineView* createBrowserTab(const QUrl& url = QUrl());
    QWebEngineView* currentWebView() const;
    void updateAddressBarFromCurrentView();
    void updateNavigationButtons();

    QUrl resolveUrl() const;
    QString resolvePath(const QString& relativeOrAbsolute) const;
    QSize parseStartSize(const QSize& fallback) const;

    bool checkPasswordInteractive();
    bool checkPasswordValue(const QString& password) const;

    void minimizeAction();
    void restoreAction();
    void maximizeAction();
    void toggleKioskAction();
    void closeAction();

    void enterKiosk();
    void leaveKiosk();
    void updateMaximizeButtonText();

    bool usesNativeTitleBar() const;
    QString kioskButtonDisplayText() const;
    void configureKioskButton(QPushButton* button);
    void updateKioskButtonState();

    QVariantMap ok(const QString& action) const;
    QVariantMap error(const QString& action, const QString& message) const;
	QString browserControlScriptSource() const;
    QString bridgeScriptSource() const;
    QString systemActionsScriptSource() const;

    const SystemActionConfig* findSystemAction(const QString& actionName) const;
    bool isOriginAllowed(const QString& origin) const;
    bool checkSystemActionCooldown(const QString& actionName, int cooldownMs);

    BrowserSettings m_settings;
    QString m_iniDir;

    QWidget* m_root = nullptr;
    QWidget* m_titleBar = nullptr;
    QLabel* m_iconLabel = nullptr;
    QLabel* m_titleLabel = nullptr;
    QWidget* m_buttonContainer = nullptr;
    QHBoxLayout* m_buttonLayout = nullptr;
    QPushButton* m_kioskButton = nullptr;
    QPushButton* m_minimizeButton = nullptr;
    QPushButton* m_maximizeButton = nullptr;
    QPushButton* m_closeButton = nullptr;

    QWidget* m_contentKioskBar = nullptr;
    QPushButton* m_contentKioskButton = nullptr;

    QWidget* m_addressBarContainer = nullptr;
    QToolButton* m_backButton = nullptr;
    QToolButton* m_forwardButton = nullptr;
    QToolButton* m_reloadButton = nullptr;
    QLineEdit* m_addressEdit = nullptr;

    QTabWidget* m_tabs = nullptr;
    QWebEngineView* m_view = nullptr;
    QWebEngineView* m_devToolsView = nullptr;
    BrowserBridge* m_bridge = nullptr;

    bool m_dragging = false;
    QPoint m_dragOffset;
    QPoint m_lockedWindowPos;
    bool m_windowMoveLockReady = false;
    bool m_restoringLockedPosition = false;
    bool m_isKiosk = false;
    bool m_wasMaximizedBeforeKiosk = false;
    bool m_forceClose = false;

    QHash<QString, qint64> m_systemActionLastRunMs;
};
