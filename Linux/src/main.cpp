#include "CliOverrideParser.h"
#include "MainWindow.h"
#include "SettingsLoader.h"

#include <QApplication>
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QString>
#include <QStringList>

static QStringList argumentsFromArgv(int argc, char* argv[])
{
    QStringList args;
    for (int i = 0; i < argc; ++i) {
        args << QString::fromLocal8Bit(argv[i]);
    }
    return args;
}

static QString findIniPath(int argc, char* argv[])
{
    QString appDir = QDir::currentPath();
    if (argc > 0 && argv[0]) {
        QFileInfo exeInfo(QString::fromLocal8Bit(argv[0]));
        if (exeInfo.isRelative()) {
            exeInfo = QFileInfo(QDir::current(), QString::fromLocal8Bit(argv[0]));
        }
        appDir = exeInfo.absolutePath();
    }

    const QString appIni = QDir(appDir).filePath(QStringLiteral("ini.json"));
    if (QFileInfo::exists(appIni)) return appIni;

    const QString cwdIni = QDir(QDir::currentPath()).filePath(QStringLiteral("ini.json"));
    if (QFileInfo::exists(cwdIni)) return cwdIni;

    return appIni;
}

static void appendChromiumFlag(QString& flags, const QString& flag)
{
    if (flags.contains(flag)) {
        return;
    }

    if (!flags.trimmed().isEmpty()) {
        flags += QLatin1Char(' ');
    }

    flags += flag;
}

static void appendDisabledChromiumFeatures(QString& flags, const QStringList& features)
{
    if (features.isEmpty()) {
        return;
    }

    appendChromiumFlag(flags, QStringLiteral("--disable-features=") + features.join(QLatin1Char(',')));
}

static void appendUniqueFeature(QStringList& features, const QString& feature)
{
    if (!features.contains(feature)) {
        features.append(feature);
    }
}

static void applyChromiumFlags(const BrowserSettings& settings)
{
    QString flags = QString::fromUtf8(qgetenv("QTWEBENGINE_CHROMIUM_FLAGS"));
    QStringList disabledFeatures;

    if (settings.extensionsEnabled) {
        appendChromiumFlag(flags, QStringLiteral("--enable-extensions"));
    } else {
        appendChromiumFlag(flags, QStringLiteral("--disable-extensions"));
    }

    if (!settings.enableAutofill) {
        appendUniqueFeature(disabledFeatures, QStringLiteral("AutofillServerCommunication"));
        appendUniqueFeature(disabledFeatures, QStringLiteral("AutofillAddressSavePrompt"));
        appendUniqueFeature(disabledFeatures, QStringLiteral("AutofillEnableAccountWalletStorage"));
    }

    if (!settings.enablePasswordSaving) {
        appendChromiumFlag(flags, QStringLiteral("--disable-save-password-bubble"));
        appendUniqueFeature(disabledFeatures, QStringLiteral("PasswordManagerEnableAccountStorage"));
        appendUniqueFeature(disabledFeatures, QStringLiteral("PasswordManagerEnableAccountStorageForNonSyncingUsers"));
        appendUniqueFeature(disabledFeatures, QStringLiteral("PasswordManagerRedesign"));
    }

    if (!settings.enableTranslate) {
        appendChromiumFlag(flags, QStringLiteral("--disable-translate"));
        appendUniqueFeature(disabledFeatures, QStringLiteral("Translate"));
    }

    appendDisabledChromiumFeatures(flags, disabledFeatures);
    qputenv("QTWEBENGINE_CHROMIUM_FLAGS", flags.trimmed().toUtf8());
}

int main(int argc, char* argv[])
{
    const QString iniPath = findIniPath(argc, argv);
    BrowserSettings settings = SettingsLoader::load(iniPath);
    CliOverrideParser::apply(settings, argumentsFromArgv(argc, argv));

    applyChromiumFlags(settings);

    QApplication app(argc, argv);
    QCoreApplication::setApplicationName(settings.title);
    QCoreApplication::setApplicationVersion(QStringLiteral("1.0.0"));

    MainWindow window(settings, QFileInfo(iniPath).absolutePath());
    window.applyInitialWindowState();

    return app.exec();
}
