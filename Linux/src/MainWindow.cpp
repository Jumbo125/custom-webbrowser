#include "MainWindow.h"
#include "BrowserBridge.h"
#include "BrowserPage.h"

#include <QApplication>
#include <QCloseEvent>
#include <QDir>
#include <QFileInfo>
#include <QFont>
#include <QHBoxLayout>
#include <QIcon>
#include <QInputDialog>
#include <QKeySequence>
#include <QLabel>
#include <QLineEdit>
#include <QMouseEvent>
#include <QMoveEvent>
#include <QPushButton>
#include <QShortcut>
#include <QShowEvent>
#include <QSize>
#include <QStyle>
#include <QTabBar>
#include <QTabWidget>
#include <QToolButton>
#include <QUrl>
#include <QVBoxLayout>
#include <QWebChannel>
#include <QWebEngineHistory>
#include <QWebEnginePage>
#include <QWebEngineScript>
#include <QWebEngineScriptCollection>
#include <QWebEngineSettings>
#include <QWebEngineView>

MainWindow::MainWindow(const BrowserSettings& settings, const QString& iniDir, QWidget* parent)
    : QMainWindow(parent), m_settings(settings), m_iniDir(iniDir)
{
    applyWindowMode();
    setWindowTitle(m_settings.title);

    const QString iconFile = resolvePath(m_settings.iconPath);
    if (QFileInfo::exists(iconFile)) {
        setWindowIcon(QIcon(iconFile));
    }

    setupUi();
    setupWebView();
    setupDevTools();
    loadUrl();
}

void MainWindow::setupUi()
{
    m_root = new QWidget(this);
    auto* rootLayout = new QVBoxLayout(m_root);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    m_titleBar = new QWidget(m_root);
    m_titleBar->setObjectName("TitleBar");
    m_titleBar->installEventFilter(this);

    auto* titleLayout = new QHBoxLayout(m_titleBar);
    titleLayout->setContentsMargins(8, 0, 4, 0);
    titleLayout->setSpacing(6);

    m_iconLabel = new QLabel(m_titleBar);
    m_iconLabel->setObjectName("TitleIcon");
    m_iconLabel->installEventFilter(this);
    const QIcon icon = windowIcon().isNull()
        ? style()->standardIcon(QStyle::SP_ComputerIcon)
        : windowIcon();
    m_iconLabel->setPixmap(icon.pixmap(18, 18));

    m_titleLabel = new QLabel(m_settings.title, m_titleBar);
    m_titleLabel->setObjectName("TitleLabel");
    m_titleLabel->installEventFilter(this);
    m_titleLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    m_buttonContainer = new QWidget(m_titleBar);
    m_buttonLayout = new QHBoxLayout(m_buttonContainer);
    m_buttonLayout->setContentsMargins(0, 0, 0, 0);

    auto makeButton = [&](const QString& text, const QString& objectName) {
        auto* button = new QPushButton(text, m_buttonContainer);
        button->setObjectName(objectName);
        button->setFocusPolicy(Qt::NoFocus);
        button->setFlat(true);
        m_buttonLayout->addWidget(button);
        return button;
    };

    m_kioskButton = makeButton(kioskButtonDisplayText(), "KioskButton");
    configureKioskButton(m_kioskButton);
    m_minimizeButton = makeButton(QStringLiteral("−"), "MinimizeButton");
    m_maximizeButton = makeButton(QStringLiteral("□"), "MaximizeButton");
    m_closeButton = makeButton(QStringLiteral("×"), "CloseButton");

    m_kioskButton->setVisible(m_settings.enableKioskModeToggle);
    m_minimizeButton->setVisible(m_settings.enableMinimize);
    m_maximizeButton->setVisible(m_settings.enableMaximize);
    m_closeButton->setVisible(m_settings.enableClose);

    connect(m_kioskButton, &QPushButton::clicked, this, [this]() {
        if (checkPasswordInteractive()) toggleKioskAction();
    });
    connect(m_minimizeButton, &QPushButton::clicked, this, [this]() {
        if (checkPasswordInteractive()) minimizeAction();
    });
    connect(m_maximizeButton, &QPushButton::clicked, this, [this]() {
        if (!checkPasswordInteractive()) return;
        if (isMaximized() || isFullScreen() || m_isKiosk) restoreAction();
        else maximizeAction();
    });
    connect(m_closeButton, &QPushButton::clicked, this, [this]() {
        if (checkPasswordInteractive()) closeAction();
    });

    titleLayout->addWidget(m_iconLabel);
    titleLayout->addWidget(m_titleLabel, 1);
    titleLayout->addWidget(m_buttonContainer, 0);

    m_tabs = new QTabWidget(m_root);
    m_tabs->setObjectName("BrowserTabs");
    m_tabs->setDocumentMode(true);
    m_tabs->setMovable(m_settings.allowTabs);
    m_tabs->setTabsClosable(m_settings.allowTabs);
    m_tabs->tabBar()->setVisible(m_settings.allowTabs);

    connect(m_tabs, &QTabWidget::currentChanged, this, [this]() {
        m_view = currentWebView();
        updateAddressBarFromCurrentView();
        updateNavigationButtons();
        if (m_settings.devTools && m_devToolsView && m_view) {
            m_view->page()->setDevToolsPage(m_devToolsView->page());
        }
    });

    connect(m_tabs, &QTabWidget::tabCloseRequested, this, [this](int index) {
        if (!m_settings.allowTabs || m_tabs->count() <= 1) {
            return;
        }

        QWidget* widget = m_tabs->widget(index);
        m_tabs->removeTab(index);
        if (widget) {
            widget->deleteLater();
        }

        m_view = currentWebView();
        updateAddressBarFromCurrentView();
        updateNavigationButtons();
    });

    m_titleBar->setVisible(!usesNativeTitleBar());

    rootLayout->addWidget(m_titleBar, 0);
    setupContentKioskBar(rootLayout);
    setupAddressBar(rootLayout);
    rootLayout->addWidget(m_tabs, 1);
    setCentralWidget(m_root);

    setStyleSheet(QStringLiteral(R"CSS(
        QWidget#TitleBar {
            background: %1;
            color: %2;
        }
        QLabel#TitleLabel {
            color: %2;
            font-size: 12px;
        }
        QWidget#ContentKioskBar {
            background: %5;
            border-bottom: 1px solid rgba(0, 0, 0, 0.12);
        }
        QPushButton#ContentKioskButton {
            color: %6;
            background: transparent;
            border: none;
            border-radius: 6px;
            padding: 2px 10px;
            font-weight: 600;
        }
        QPushButton#ContentKioskButton:hover {
            background: rgba(0, 0, 0, 0.08);
        }
        QWidget#AddressBar {
            background: #111827;
            border-bottom: 1px solid #334155;
        }
        QLineEdit#AddressEdit {
            background: #020617;
            color: #e5e7eb;
            border: 1px solid #334155;
            border-radius: 6px;
            padding: 4px 8px;
        }
        QToolButton#AddressButton {
            background: transparent;
            color: #e5e7eb;
            border: none;
            border-radius: 6px;
            min-width: 28px;
            min-height: 24px;
        }
        QToolButton#AddressButton:hover {
            background: rgba(255, 255, 255, 0.14);
        }
        QTabWidget::pane {
            border: none;
        }
        QTabBar::tab {
            background: #1f2937;
            color: #e5e7eb;
            padding: 7px 12px;
            border-right: 1px solid #334155;
        }
        QTabBar::tab:selected {
            background: #111827;
        }
        QPushButton {
            border: none;
            color: %2;
            background: transparent;
            font-weight: 600;
        }
        QPushButton:hover {
            background: %3;
        }
        QPushButton#CloseButton:hover {
            background: %4;
        }
    )CSS")
        .arg(m_settings.customTitlebarBackground,
             m_settings.customTitlebarForeground,
             m_settings.customTitlebarButtonHover,
             m_settings.customTitlebarCloseHover,
             m_settings.contentKioskBarBackground,
             m_settings.contentKioskBarForeground));

    applyTitleBarSizing();
    updateKioskButtonState();
}

void MainWindow::setupContentKioskBar(QVBoxLayout* rootLayout)
{
    m_contentKioskBar = new QWidget(m_root);
    m_contentKioskBar->setObjectName("ContentKioskBar");

    auto* layout = new QHBoxLayout(m_contentKioskBar);
    layout->setContentsMargins(6, 3, 6, 3);
    layout->setSpacing(4);

    layout->addStretch(1);

    m_contentKioskButton = new QPushButton(kioskButtonDisplayText(), m_contentKioskBar);
    m_contentKioskButton->setObjectName("ContentKioskButton");
    m_contentKioskButton->setFocusPolicy(Qt::NoFocus);
    m_contentKioskButton->setFlat(true);
    configureKioskButton(m_contentKioskButton);
    layout->addWidget(m_contentKioskButton, 0);

    connect(m_contentKioskButton, &QPushButton::clicked, this, [this]() {
        if (checkPasswordInteractive()) toggleKioskAction();
    });

    const bool visible = usesNativeTitleBar()
        && m_settings.showKioskButtonInContent
        && m_settings.enableKioskModeToggle;
    m_contentKioskBar->setVisible(visible);

    rootLayout->addWidget(m_contentKioskBar, 0);
}

void MainWindow::setupAddressBar(QVBoxLayout* rootLayout)
{
    m_addressBarContainer = new QWidget(m_root);
    m_addressBarContainer->setObjectName("AddressBar");

    auto* layout = new QHBoxLayout(m_addressBarContainer);
    layout->setContentsMargins(6, 4, 6, 4);
    layout->setSpacing(4);

    auto makeToolButton = [&](const QString& text) {
        auto* button = new QToolButton(m_addressBarContainer);
        button->setObjectName("AddressButton");
        button->setText(text);
        button->setFocusPolicy(Qt::NoFocus);
        return button;
    };

    m_backButton = makeToolButton(QStringLiteral("‹"));
    m_forwardButton = makeToolButton(QStringLiteral("›"));
    m_reloadButton = makeToolButton(QStringLiteral("⟳"));

    m_addressEdit = new QLineEdit(m_addressBarContainer);
    m_addressEdit->setObjectName("AddressEdit");
    m_addressEdit->setClearButtonEnabled(true);
    m_addressEdit->setPlaceholderText(QStringLiteral("URL eingeben…"));

    layout->addWidget(m_backButton);
    layout->addWidget(m_forwardButton);
    layout->addWidget(m_reloadButton);
    layout->addWidget(m_addressEdit, 1);

    connect(m_backButton, &QToolButton::clicked, this, [this]() {
        if (auto* view = currentWebView()) view->back();
    });
    connect(m_forwardButton, &QToolButton::clicked, this, [this]() {
        if (auto* view = currentWebView()) view->forward();
    });
    connect(m_reloadButton, &QToolButton::clicked, this, [this]() {
        if (auto* view = currentWebView()) view->reload();
    });
    connect(m_addressEdit, &QLineEdit::returnPressed, this, [this]() {
        if (auto* view = currentWebView()) {
            const QString text = m_addressEdit->text().trimmed();
            if (!text.isEmpty()) {
                view->setUrl(QUrl::fromUserInput(text));
            }
        }
    });

    m_addressBarContainer->setVisible(m_settings.showAddressBar);
    rootLayout->addWidget(m_addressBarContainer, 0);
}

void MainWindow::setupWebView()
{
    m_bridge = new BrowserBridge(this, this);
    m_view = createBrowserTab();
}

void MainWindow::configureWebView(QWebEngineView* view)
{
    view->settings()->setAttribute(QWebEngineSettings::FullScreenSupportEnabled, true);
    view->settings()->setAttribute(QWebEngineSettings::LocalContentCanAccessFileUrls, true);
    view->settings()->setAttribute(QWebEngineSettings::LocalContentCanAccessRemoteUrls, true);

    auto* channel = new QWebChannel(view);
    channel->registerObject(QStringLiteral("customBrowserBridge"), m_bridge);
    view->page()->setWebChannel(channel);

    if (!m_settings.enableAutofill || !m_settings.enablePasswordSaving) {
        QWebEngineScript browserControlScript;
        browserControlScript.setName(QStringLiteral("customBrowserFormControl"));
        browserControlScript.setInjectionPoint(QWebEngineScript::DocumentCreation);
        browserControlScript.setWorldId(QWebEngineScript::MainWorld);
        browserControlScript.setRunsOnSubFrames(true);
        browserControlScript.setSourceCode(browserControlScriptSource());
        view->page()->scripts().insert(browserControlScript);
    }

    QWebEngineScript script;
    script.setName(QStringLiteral("customBrowserBridgeInstaller"));
    script.setInjectionPoint(QWebEngineScript::DocumentReady);
    script.setWorldId(QWebEngineScript::MainWorld);
    script.setRunsOnSubFrames(false);
    script.setSourceCode(bridgeScriptSource());
    view->page()->scripts().insert(script);
}

QWebEngineView* MainWindow::createBrowserTab(const QUrl& url)
{
    auto* view = new QWebEngineView(m_tabs);
    auto* page = new BrowserPage(view);

    page->createNewWindowPage = [this](QWebEnginePage::WebWindowType type) -> QWebEnginePage* {
        Q_UNUSED(type)

        if (!m_settings.allowTabs) {
            return nullptr;
        }

        QWebEngineView* newView = createBrowserTab();
        return newView ? newView->page() : nullptr;
    };

    view->setPage(page);
    configureWebView(view);

    const int index = m_tabs->addTab(view, QStringLiteral("Tab"));
    m_tabs->setCurrentIndex(index);
    m_view = view;

    connect(view, &QWebEngineView::titleChanged, this, [this, view](const QString& title) {
        const int index = m_tabs->indexOf(view);
        if (index >= 0) {
            m_tabs->setTabText(index, title.isEmpty() ? QStringLiteral("Tab") : title);
        }
    });

    connect(view, &QWebEngineView::urlChanged, this, [this, view](const QUrl&) {
        if (view == currentWebView()) {
            updateAddressBarFromCurrentView();
        }
    });

    connect(view, &QWebEngineView::loadFinished, this, [this, view](bool) {
        if (view == currentWebView()) {
            updateNavigationButtons();
        }
    });

    if (url.isValid()) {
        view->setUrl(url);
    }

    return view;
}

QWebEngineView* MainWindow::currentWebView() const
{
    if (!m_tabs) {
        return m_view;
    }

    return qobject_cast<QWebEngineView*>(m_tabs->currentWidget());
}

void MainWindow::updateAddressBarFromCurrentView()
{
    if (!m_addressEdit) return;

    if (auto* view = currentWebView()) {
        m_addressEdit->setText(view->url().toString());
    } else {
        m_addressEdit->clear();
    }
}

void MainWindow::updateNavigationButtons()
{
    auto* view = currentWebView();
    const bool hasView = view != nullptr;

    if (m_backButton) {
        m_backButton->setEnabled(hasView && view->history()->canGoBack());
    }
    if (m_forwardButton) {
        m_forwardButton->setEnabled(hasView && view->history()->canGoForward());
    }
    if (m_reloadButton) {
        m_reloadButton->setEnabled(hasView);
    }
}

void MainWindow::setupDevTools()
{
    if (!m_settings.devTools) return;

    m_devToolsView = new QWebEngineView();
    m_devToolsView->resize(1000, 700);
    m_devToolsView->setWindowTitle(m_settings.title + QStringLiteral(" DevTools"));

    if (auto* view = currentWebView()) {
        view->page()->setDevToolsPage(m_devToolsView->page());
    }

    auto* shortcut = new QShortcut(QKeySequence(QStringLiteral("Ctrl+Shift+I")), this);
    connect(shortcut, &QShortcut::activated, this, [this]() {
        if (m_devToolsView) {
            if (auto* view = currentWebView()) {
                view->page()->setDevToolsPage(m_devToolsView->page());
            }
            m_devToolsView->show();
            m_devToolsView->raise();
            m_devToolsView->activateWindow();
        }
    });
}

void MainWindow::applyWindowMode()
{
    if (!usesNativeTitleBar()) {
        setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
        return;
    }

    Qt::WindowFlags flags = Qt::Window | Qt::WindowTitleHint | Qt::WindowSystemMenuHint;

    if (m_settings.enableMinimize) {
        flags |= Qt::WindowMinimizeButtonHint;
    }
    if (m_settings.enableMaximize) {
        flags |= Qt::WindowMaximizeButtonHint;
    }
    if (m_settings.enableClose) {
        flags |= Qt::WindowCloseButtonHint;
    }

    setWindowFlags(flags);
}

void MainWindow::applyTitleBarSizing()
{
    if (!m_titleBar || !m_buttonLayout) return;

    const double titleBarHeight = qBound(22.0, m_settings.titleBarHeightPx, 80.0);
    const double buttonHeight = qBound(16.0, m_settings.titleBarButtonHeightPx, titleBarHeight);
    const double ratio = qBound(0.8, m_settings.titleBarButtonWidthRatio, 3.0);
    const double gap = qBound(0.0, m_settings.titleBarButtonGapPx, 20.0);
    const int buttonWidth = qRound(buttonHeight * ratio);

    m_titleBar->setFixedHeight(qRound(titleBarHeight));
    m_iconLabel->setFixedSize(qMax(12, qRound(titleBarHeight - 10)), qMax(12, qRound(titleBarHeight - 10)));
    m_buttonLayout->setSpacing(qRound(gap));

    const QList<QPushButton*> buttons { m_kioskButton, m_minimizeButton, m_maximizeButton, m_closeButton };
    for (QPushButton* button : buttons) {
        if (!button) continue;
        button->setFixedSize(buttonWidth, qRound(buttonHeight));
        button->setMinimumWidth(buttonWidth);
        button->setMaximumWidth(buttonWidth);
        button->setFont(QFont(button->font().family(), qMax(9, qRound(buttonHeight * 0.48))));
    }

    if (m_contentKioskButton) {
        m_contentKioskButton->setMinimumHeight(qMax(22, qRound(buttonHeight)));
        m_contentKioskButton->setFont(QFont(m_contentKioskButton->font().family(), qMax(9, qRound(buttonHeight * 0.45))));
    }
}

void MainWindow::loadUrl()
{
    if (auto* view = currentWebView()) {
        view->setUrl(resolveUrl());
    }
}

QUrl MainWindow::resolveUrl() const
{
    QString value = m_settings.url.trimmed();
    value.replace(QStringLiteral("{PORT}"), QString::number(m_settings.port));

    const QUrl direct(value);
    if (direct.isValid() && !direct.scheme().isEmpty()) {
        return direct;
    }

    return QUrl::fromLocalFile(resolvePath(value));
}

QString MainWindow::resolvePath(const QString& relativeOrAbsolute) const
{
    QFileInfo info(relativeOrAbsolute);
    if (info.isAbsolute()) {
        return info.absoluteFilePath();
    }
    return QFileInfo(QDir(m_iniDir), relativeOrAbsolute).absoluteFilePath();
}

QSize MainWindow::parseStartSize(const QSize& fallback) const
{
    QString value = m_settings.startWindowSize.trimmed().toLower();
    value.replace(QStringLiteral("\u00D7"), QStringLiteral("x"));

    if (value == "auto" || value == "maximize" || value == "maximaze" || value == "minimize") {
        return fallback;
    }

    if (value.contains('x')) {
        const QStringList parts = value.split('x', Qt::SkipEmptyParts);
        if (parts.size() == 2) {
            bool okW = false;
            bool okH = false;
            const int w = parts.at(0).trimmed().toInt(&okW);
            const int h = parts.at(1).trimmed().toInt(&okH);
            if (okW && okH && w > 100 && h > 100) return QSize(w, h);
        }
    }

    if (value.contains(',')) {
        const QStringList parts = value.split(',', Qt::SkipEmptyParts);
        if (parts.size() == 2) {
            bool okH = false;
            bool okW = false;
            const int h = parts.at(0).trimmed().toInt(&okH);
            const int w = parts.at(1).trimmed().toInt(&okW);
            if (okW && okH && w > 100 && h > 100) return QSize(w, h);
        }
    }

    return fallback;
}

void MainWindow::applyInitialWindowState()
{
    const QSize fallback(1280, 720);
    resize(parseStartSize(fallback));

    const QString mode = m_settings.startWindowSize.trimmed().toLower();
    if (m_settings.startInKiosk) {
        show();
        enterKiosk();
    } else if (mode == "maximize" || mode == "maximaze") {
        showMaximized();
    } else if (mode == "minimize") {
        showMinimized();
    } else {
        show();
    }

    updateMaximizeButtonText();
}

bool MainWindow::checkPasswordInteractive()
{
    if (m_settings.password.isEmpty()) return true;

    bool ok = false;
    const QString entered = QInputDialog::getText(
        this,
        QStringLiteral("Password"),
        QStringLiteral("Password:"),
        QLineEdit::Password,
        QString(),
        &ok
    );

    return ok && entered == m_settings.password;
}

bool MainWindow::checkPasswordValue(const QString& password) const
{
    return m_settings.password.isEmpty() || password == m_settings.password;
}

void MainWindow::minimizeAction()
{
    if (!m_settings.enableMinimize) return;
    showMinimized();
}

void MainWindow::restoreAction()
{
    if (m_isKiosk) {
        leaveKiosk();
    } else {
        showNormal();
    }
    updateMaximizeButtonText();
}

void MainWindow::maximizeAction()
{
    if (!m_settings.enableMaximize) return;
    if (m_isKiosk) leaveKiosk();
    showMaximized();
    updateMaximizeButtonText();
}

void MainWindow::toggleKioskAction()
{
    if (!m_settings.enableKioskModeToggle) return;
    if (m_isKiosk) leaveKiosk();
    else enterKiosk();
    updateMaximizeButtonText();
    updateKioskButtonState();
}

void MainWindow::closeAction()
{
    if (!m_settings.enableClose) return;
    m_forceClose = true;
    close();
}

void MainWindow::enterKiosk()
{
    if (m_isKiosk) return;
    m_wasMaximizedBeforeKiosk = isMaximized();
    m_isKiosk = true;
    showFullScreen();
    updateKioskButtonState();
}

void MainWindow::leaveKiosk()
{
    if (!m_isKiosk) return;
    m_isKiosk = false;
    if (m_wasMaximizedBeforeKiosk) showMaximized();
    else showNormal();
    updateKioskButtonState();
}

void MainWindow::updateMaximizeButtonText()
{
    if (!m_maximizeButton) return;
    m_maximizeButton->setText((isMaximized() || isFullScreen() || m_isKiosk) ? QStringLiteral("❐") : QStringLiteral("□"));
}

bool MainWindow::usesNativeTitleBar() const
{
    // A native Linux/Windows title bar is controlled by the window manager.
    // Qt does not provide a portable way to disable dragging of that native title bar.
    // Therefore, when movement is disabled, fall back to the frameless/custom bar
    // and additionally lock the window position in moveEvent().
    if (!m_settings.enableMoveWindow) {
        return false;
    }

    const QString style = m_settings.titlebarStyle.trimmed().toLower();
    return style == QStringLiteral("native") || style == QStringLiteral("system") || style == QStringLiteral("os");
}

QString MainWindow::kioskButtonDisplayText() const
{
    if (!m_settings.kioskButtonText.trimmed().isEmpty()) {
        return m_settings.kioskButtonText;
    }

    if (!m_settings.kioskButtonIcon.trimmed().isEmpty()) {
        return m_settings.kioskButtonIcon;
    }

    return QStringLiteral("📌");
}

void MainWindow::configureKioskButton(QPushButton* button)
{
    if (!button) return;

    button->setToolTip(m_settings.kioskButtonTooltip);
    button->setAccessibleName(m_settings.kioskButtonTooltip);
    button->setCheckable(true);
    button->setIconSize(QSize(16, 16));

    const QString type = m_settings.kioskButtonIconType.trimmed().toLower();
    const QString value = m_settings.kioskButtonIcon.trimmed();

    if (type == QStringLiteral("theme") && !value.isEmpty()) {
        const QIcon themeIcon = QIcon::fromTheme(value);
        if (!themeIcon.isNull()) {
            button->setIcon(themeIcon);
            button->setText(QString());
            return;
        }
    }

    if ((type == QStringLiteral("svg")
        || type == QStringLiteral("image")
        || type == QStringLiteral("png")
        || type == QStringLiteral("ico")) && !value.isEmpty()) {
        const QIcon fileIcon(resolvePath(value));
        if (!fileIcon.isNull()) {
            button->setIcon(fileIcon);
            button->setText(QString());
            return;
        }
    }

    button->setIcon(QIcon());
    button->setText(kioskButtonDisplayText());
}

void MainWindow::updateKioskButtonState()
{
    const QString suffix = m_isKiosk ? QStringLiteral(" aktiv") : QString();

    if (m_kioskButton) {
        m_kioskButton->setChecked(m_isKiosk);
        m_kioskButton->setToolTip(m_settings.kioskButtonTooltip + suffix);
    }

    if (m_contentKioskButton) {
        m_contentKioskButton->setChecked(m_isKiosk);
        m_contentKioskButton->setToolTip(m_settings.kioskButtonTooltip + suffix);
    }
}

QVariantMap MainWindow::ok(const QString& action) const
{
    QVariantMap result;
    result.insert(QStringLiteral("ok"), true);
    result.insert(QStringLiteral("action"), action);
    result.insert(QStringLiteral("isKiosk"), m_isKiosk);
    result.insert(QStringLiteral("isMaximized"), isMaximized());
    result.insert(QStringLiteral("isMinimized"), isMinimized());
    result.insert(QStringLiteral("isFullScreen"), isFullScreen());
    return result;
}

QVariantMap MainWindow::error(const QString& action, const QString& message) const
{
    QVariantMap result;
    result.insert(QStringLiteral("ok"), false);
    result.insert(QStringLiteral("action"), action);
    result.insert(QStringLiteral("error"), message);
    return result;
}

QVariantMap MainWindow::jsMinimize(const QString& password)
{
    if (!m_settings.enableMinimize) return error(QStringLiteral("minimize"), QStringLiteral("minimize disabled"));
    if (!checkPasswordValue(password)) return error(QStringLiteral("minimize"), QStringLiteral("invalid password"));
    minimizeAction();
    return ok(QStringLiteral("minimize"));
}

QVariantMap MainWindow::jsRestore(const QString& password)
{
    if (!checkPasswordValue(password)) return error(QStringLiteral("restore"), QStringLiteral("invalid password"));
    restoreAction();
    return ok(QStringLiteral("restore"));
}

QVariantMap MainWindow::jsMaximize(const QString& password)
{
    if (!m_settings.enableMaximize) return error(QStringLiteral("maximize"), QStringLiteral("maximize disabled"));
    if (!checkPasswordValue(password)) return error(QStringLiteral("maximize"), QStringLiteral("invalid password"));
    maximizeAction();
    return ok(QStringLiteral("maximize"));
}

QVariantMap MainWindow::jsToggleKiosk(const QString& password)
{
    if (!m_settings.enableKioskModeToggle) return error(QStringLiteral("toggleKiosk"), QStringLiteral("kiosk toggle disabled"));
    if (!checkPasswordValue(password)) return error(QStringLiteral("toggleKiosk"), QStringLiteral("invalid password"));
    toggleKioskAction();
    return ok(QStringLiteral("toggleKiosk"));
}

QVariantMap MainWindow::jsClose(const QString& password)
{
    if (!m_settings.enableClose) return error(QStringLiteral("close"), QStringLiteral("close disabled"));
    if (!checkPasswordValue(password)) return error(QStringLiteral("close"), QStringLiteral("invalid password"));
    closeAction();
    return ok(QStringLiteral("close"));
}

QVariantMap MainWindow::jsGetState() const
{
    const QWebEngineView* view = currentWebView();

    QVariantMap result = ok(QStringLiteral("getState"));
    result.insert(QStringLiteral("title"), m_settings.title);
    result.insert(QStringLiteral("url"), view ? view->url().toString() : QString());
    result.insert(QStringLiteral("enableMinimize"), m_settings.enableMinimize);
    result.insert(QStringLiteral("enableMaximize"), m_settings.enableMaximize);
    result.insert(QStringLiteral("enableClose"), m_settings.enableClose);
    result.insert(QStringLiteral("enableMoveWindow"), m_settings.enableMoveWindow);
    result.insert(QStringLiteral("enableKioskModeToggle"), m_settings.enableKioskModeToggle);
    result.insert(QStringLiteral("titlebarStyle"), m_settings.titlebarStyle);
    result.insert(QStringLiteral("usesNativeTitleBar"), usesNativeTitleBar());
    result.insert(QStringLiteral("showKioskButtonInContent"), m_settings.showKioskButtonInContent);
    result.insert(QStringLiteral("showAddressBar"), m_settings.showAddressBar);
    result.insert(QStringLiteral("allowTabs"), m_settings.allowTabs);
    result.insert(QStringLiteral("tabCount"), m_tabs ? m_tabs->count() : 1);
    return result;
}

bool MainWindow::eventFilter(QObject* watched, QEvent* event)
{
    const bool isTitleObject = watched == m_titleBar || watched == m_titleLabel || watched == m_iconLabel;
    if (usesNativeTitleBar() || !isTitleObject || !m_settings.enableMoveWindow || m_isKiosk || isFullScreen()) {
        return QMainWindow::eventFilter(watched, event);
    }

    if (event->type() == QEvent::MouseButtonPress) {
        auto* mouse = static_cast<QMouseEvent*>(event);
        if (mouse->button() == Qt::LeftButton) {
            m_dragging = true;
            m_dragOffset = mouse->globalPosition().toPoint() - frameGeometry().topLeft();
            return true;
        }
    }

    if (event->type() == QEvent::MouseMove && m_dragging) {
        auto* mouse = static_cast<QMouseEvent*>(event);
        move(mouse->globalPosition().toPoint() - m_dragOffset);
        return true;
    }

    if (event->type() == QEvent::MouseButtonRelease) {
        m_dragging = false;
        return true;
    }

    return QMainWindow::eventFilter(watched, event);
}

void MainWindow::showEvent(QShowEvent* event)
{
    QMainWindow::showEvent(event);

    if (!m_settings.enableMoveWindow && !m_windowMoveLockReady) {
        m_lockedWindowPos = pos();
        m_windowMoveLockReady = true;
    }
}

void MainWindow::moveEvent(QMoveEvent* event)
{
    if (m_settings.enableMoveWindow || m_restoringLockedPosition || !m_windowMoveLockReady || m_isKiosk || isFullScreen()) {
        QMainWindow::moveEvent(event);
        return;
    }

    if (event->pos() != m_lockedWindowPos) {
        m_restoringLockedPosition = true;
        move(m_lockedWindowPos);
        m_restoringLockedPosition = false;
        event->accept();
        return;
    }

    QMainWindow::moveEvent(event);
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    if (m_forceClose) {
        event->accept();
        return;
    }

    if (!m_settings.enableClose) {
        event->ignore();
        return;
    }

    if (!checkPasswordInteractive()) {
        event->ignore();
        return;
    }

    event->accept();
}

QString MainWindow::browserControlScriptSource() const
{
    const QString disableAutofill = m_settings.enableAutofill ? QStringLiteral("false") : QStringLiteral("true");
    const QString disablePasswordSaving = m_settings.enablePasswordSaving ? QStringLiteral("false") : QStringLiteral("true");

    return QStringLiteral(R"JS(
(function () {
  var disableAutofill = %1;
  var disablePasswordSaving = %2;

  function isEditableField(element) {
    if (!element || !element.tagName) return false;
    var tag = element.tagName.toLowerCase();
    if (tag === 'textarea' || tag === 'select') return true;
    if (tag !== 'input') return false;

    var type = (element.getAttribute('type') || 'text').toLowerCase();
    return [
      'text', 'search', 'email', 'tel', 'url', 'number',
      'date', 'datetime-local', 'month', 'week', 'time'
    ].indexOf(type) !== -1;
  }

  function isPasswordField(element) {
    if (!element || !element.tagName || element.tagName.toLowerCase() !== 'input') return false;
    var type = (element.getAttribute('type') || '').toLowerCase();
    var hint = [
      element.getAttribute('name') || '',
      element.getAttribute('id') || '',
      element.getAttribute('autocomplete') || ''
    ].join(' ');
    return type === 'password' || /pass(word)?|pwd|pin|otp|token/i.test(hint);
  }

  function lockAttribute(element, name, value) {
    if (!element || !element.setAttribute) return;
    if (element.getAttribute(name) !== value) {
      element.setAttribute(name, value);
    }
  }

  function applyToElement(element) {
    if (!element || !element.setAttribute) return;

    if (disableAutofill && isEditableField(element) && !isPasswordField(element)) {
      lockAttribute(element, 'autocomplete', 'off');
      lockAttribute(element, 'autocorrect', 'off');
      lockAttribute(element, 'autocapitalize', 'off');
      lockAttribute(element, 'spellcheck', 'false');
      lockAttribute(element, 'data-custom-browser-autofill', 'disabled');
    }

    if (disablePasswordSaving && isPasswordField(element)) {
      lockAttribute(element, 'autocomplete', 'new-password');
      lockAttribute(element, 'data-custom-browser-password-saving', 'disabled');

      if (element.form) {
        lockAttribute(element.form, 'autocomplete', 'off');
      }
    }
  }

  function applyToRoot(root) {
    if (!root || !root.querySelectorAll) return;

    if (root.nodeType === 1) {
      applyToElement(root);
    }

    root.querySelectorAll('input, textarea, select, form').forEach(function (element) {
      applyToElement(element);
    });
  }

  function installObserver() {
    applyToRoot(document);

    var observer = new MutationObserver(function (mutations) {
      mutations.forEach(function (mutation) {
        mutation.addedNodes.forEach(function (node) {
          if (node.nodeType === 1) {
            applyToRoot(node);
          }
        });
      });
    });

    observer.observe(document.documentElement || document, {
      childList: true,
      subtree: true
    });
  }

  if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', installObserver, { once: true });
  } else {
    installObserver();
  }
})();
)JS").arg(disableAutofill, disablePasswordSaving);
}

QString MainWindow::bridgeScriptSource() const
{
    return QStringLiteral(R"JS(
(function () {
  if (window.__customBrowserBridgeInstalling || window.customBrowser) return;
  window.__customBrowserBridgeInstalling = true;

  function installBridge() {
    if (window.customBrowser) return;
    if (!window.qt || !qt.webChannelTransport || typeof QWebChannel === 'undefined') return;

    new QWebChannel(qt.webChannelTransport, function (channel) {
      var native = channel.objects.customBrowserBridge;

      function call(method, password) {
        return new Promise(function (resolve, reject) {
          try {
            if (!native || typeof native[method] !== 'function') {
              reject(new Error('customBrowserBridge method not available: ' + method));
              return;
            }

            if (method === 'getState') {
              native.getState(function (result) { resolve(result); });
            } else {
              native[method](password || '', function (result) { resolve(result); });
            }
          } catch (err) {
            reject(err);
          }
        });
      }

      window.customBrowser = {
        minimize: function (password) { return call('minimize', password); },
        restore: function (password) { return call('restore', password); },
        maximize: function (password) { return call('maximize', password); },
        toggleKiosk: function (password) {
          if (window.__customBrowserToggleKioskLock) {
            return window.__customBrowserToggleKioskLock;
          }

          window.__customBrowserToggleKioskLock = call('toggleKiosk', password)
            .finally(function () {
              setTimeout(function () {
                window.__customBrowserToggleKioskLock = null;
              }, 500);
            });

          return window.__customBrowserToggleKioskLock;
        },
        close: function (password) { return call('closeWindow', password); },
        getState: function () { return call('getState'); }
      };

      window.dispatchEvent(new CustomEvent('customBrowserReady'));
    });
  }

  function loadQWebChannel() {
    if (typeof QWebChannel !== 'undefined') {
      installBridge();
      return;
    }

    var script = document.createElement('script');
    script.src = 'qrc:///qtwebchannel/qwebchannel.js';
    script.onload = installBridge;
    (document.head || document.documentElement).appendChild(script);
  }

  if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', loadQWebChannel, { once: true });
  } else {
    loadQWebChannel();
  }
})();
)JS");
}
