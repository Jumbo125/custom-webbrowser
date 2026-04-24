# custom-web-shell

<p align="center">
  <img alt="Project status" src="https://img.shields.io/badge/status-active-success">
  <img alt="License: AGPL-3.0-or-later" src="https://img.shields.io/badge/license-AGPL--3.0--or--later-blue">
  <img alt="Windows" src="https://img.shields.io/badge/Windows-WebView2-0078D4?logo=windows">
  <img alt="Linux" src="https://img.shields.io/badge/Linux-Qt%20WebEngine-FCC624?logo=linux&logoColor=black">
  <img alt="CSharp" src="https://img.shields.io/badge/C%23-.NET-512BD4?logo=dotnet">
  <img alt="C++" src="https://img.shields.io/badge/C%2B%2B-Qt-00599C?logo=cplusplus">
</p>

<p align="center">
  Configurable WebView shell for Windows and Linux with kiosk mode, JSON defaults and CLI flags.
</p>

<p align="center">
  <a href="#deutsch">Deutsch</a> |
  <a href="#english">English</a>
</p>

---

## Deutsch

`custom-web-shell` ist eine kleine, flexibel konfigurierbare Browser-Shell für Windows und Linux.

Das Projekt lädt eine URL oder eine lokale HTML-Datei in einer WebView-Umgebung und kann vollständig über eine `ini.json` neben der ausführbaren Datei konfiguriert werden. Jede Einstellung kann zusätzlich per Kommandozeilen-Flag überschrieben werden.

Das Projekt eignet sich für Kiosk-Systeme, lokale Web-Apps, Dashboards, eingebettete Browser-Oberflächen und kontrollierte WebView-Umgebungen.

### Versionen

Dieses Repository enthält zwei Implementierungen:

- Windows-Version mit C# und Microsoft Edge WebView2
- Linux-Version mit C++ und Qt WebEngine

Beide Versionen folgen demselben Konfigurationsprinzip.

### Funktionen

- Konfigurierbare Start-URL oder lokale HTML-Datei
- Relative Pfade basierend auf dem Verzeichnis der ausführbaren Datei beziehungsweise der Konfigurationsdatei
- Benutzerdefinierter Fenstertitel
- Benutzerdefiniertes Fenster-Icon
- Optionaler Minimieren-Button
- Optionaler Maximieren-Button
- Optionaler Schließen-Button
- Optional verschiebbares Fenster
- Optionaler Kioskmodus-Schalter
- Optionaler Start direkt im Kioskmodus
- Optionaler Passwortschutz für Fensteraktionen
- Optional aktivierbare Entwicklerwerkzeuge
- Optional aktivierbare Erweiterungen, sofern vom Zielsystem unterstützt
- Optionale Adressleiste
- Optionale Tabs
- JavaScript-Bridge zur Fenstersteuerung
- JSON-Standardwerte über `ini.json`
- CLI-Overrides für alle unterstützten Einstellungen

### Konfiguration

Beispiel für `ini.json`:

```json
{
  "url": "WebRoot/index.html",
  "port": 8080,
  "title": "Custom Web Shell",
  "icon_path": "Assets/default.svg",

  "enable_minimize": true,
  "enable_maximize": true,
  "enable_close": true,
  "enable_move_window": true,
  "enable_kiosk_mode_toggle": true,

  "start_window_size": "1280x720",
  "start_in_kiosk": false,
  "password": "",

  "dev_tools": false,
  "extensions_enabled": false,

  "title_bar_height_px": 32,
  "title_bar_button_height_px": 24,
  "title_bar_button_width_ratio": 1.25,
  "title_bar_button_gap_px": 0,

  "show_addressbar": false,
  "allow_tabs": false
}
```

### Konfigurationswerte

| Schlüssel | Typ | Beschreibung |
|---|---:|---|
| `url` | String | Startadresse. Kann eine URL oder eine lokale HTML-Datei sein, zum Beispiel `WebRoot/index.html`. Relative Pfade werden relativ zur `ini.json` beziehungsweise zur ausführbaren Datei aufgelöst. |
| `port` | Zahl | Port für lokale Nutzung oder spätere lokale Serverfunktionen. |
| `title` | String | Fenstertitel, der oben links beziehungsweise in der Titelleiste angezeigt wird. |
| `icon_path` | String | Pfad zum Fenster-Icon. Relative Pfade werden relativ zur `ini.json` aufgelöst. |
| `enable_minimize` | Boolean | Aktiviert oder deaktiviert die Minimieren-Funktion. |
| `enable_maximize` | Boolean | Aktiviert oder deaktiviert die Maximieren-Funktion. |
| `enable_close` | Boolean | Aktiviert oder deaktiviert die Schließen-Funktion. |
| `enable_move_window` | Boolean | Legt fest, ob das Fenster verschoben werden darf. |
| `enable_kiosk_mode_toggle` | Boolean | Aktiviert oder deaktiviert den Button beziehungsweise die API zum Umschalten des Kioskmodus. |
| `start_window_size` | String | Startgröße des Fensters. Unterstützt Werte wie `1280x720`, `auto`, `maximize` oder `minimize`. |
| `start_in_kiosk` | Boolean | Startet die Anwendung direkt im Kioskmodus. |
| `password` | String | Optionales Passwort für geschützte Aktionen. Leer bedeutet: keine Passwortabfrage. |
| `dev_tools` | Boolean | Aktiviert oder deaktiviert Entwicklerwerkzeuge. |
| `extensions_enabled` | Boolean | Aktiviert oder deaktiviert Erweiterungen, sofern die jeweilige Plattform dies unterstützt. |
| `title_bar_height_px` | Zahl | Höhe der eigenen Titelleiste in Pixeln. |
| `title_bar_button_height_px` | Zahl | Höhe der Buttons in der Titelleiste in Pixeln. |
| `title_bar_button_width_ratio` | Zahl | Breitenverhältnis der Titelleisten-Buttons. |
| `title_bar_button_gap_px` | Zahl | Abstand zwischen den Buttons in Pixeln. |
| `show_addressbar` | Boolean | Zeigt oder versteckt die Adressleiste. |
| `allow_tabs` | Boolean | Aktiviert oder deaktiviert Tabs. |

### Kommandozeilen-Beispiele

```bash
./CustomBrowser --url=WebRoot/index.html --title="Dashboard"
```

```bash
./CustomBrowser --start_in_kiosk=true --enable_close=false
```

```bash
./CustomBrowser --show_addressbar=true --allow_tabs=true
```

Die Adressleisten-Option unterstützt zusätzlich diese Aliase:

```bash
--show_adressbar=true
--addressbar=true
```

Die Tab-Option unterstützt zusätzlich:

```bash
--tabs=true
```

### JavaScript-Bridge

Die Anwendung stellt der geladenen Seite eine kleine JavaScript-API bereit.

```javascript
await window.customBrowser.minimize();
await window.customBrowser.restore();
await window.customBrowser.maximize();
await window.customBrowser.toggleKiosk();
await window.customBrowser.close();
```

Wenn ein Passwort konfiguriert ist, wird es als Argument übergeben:

```javascript
await window.customBrowser.toggleKiosk("password");
```

Der aktuelle Status kann so abgefragt werden:

```javascript
const state = await window.customBrowser.getState();
console.log(state);
```

Mögliches Beispiel für eine eigene HTML-Steuerung:

```html
<button onclick="window.customBrowser.minimize()">Minimize</button>
<button onclick="window.customBrowser.restore()">Restore</button>
<button onclick="window.customBrowser.maximize()">Maximize</button>
<button onclick="window.customBrowser.toggleKiosk()">Toggle kiosk</button>
<button onclick="window.customBrowser.close()">Close</button>
```

Mit Passwort:

```html
<button onclick="window.customBrowser.toggleKiosk('password')">
  Toggle kiosk
</button>
```

### Linux-Build

AMD64:

```bash
make linux-amd64
```

ARM64 / aarch64:

```bash
make linux-arm64
```

oder:

```bash
make linux-aarch64
```

Manueller AMD64-Build:

```bash
rm -rf build/linux-amd64
cmake -S . -B build/linux-amd64 \
  -DCMAKE_BUILD_TYPE=Release \
  -DQt6_DIR=/usr/lib/x86_64-linux-gnu/cmake/Qt6 \
  -DCMAKE_PREFIX_PATH=/usr/lib/x86_64-linux-gnu/cmake

cmake --build build/linux-amd64 -j$(nproc)
```

Manueller ARM64-Cross-Build:

```bash
rm -rf build/linux-arm64
cmake -S . -B build/linux-arm64 \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/aarch64-linux-gnu.cmake \
  -DQt6_DIR=/usr/lib/aarch64-linux-gnu/cmake/Qt6 \
  -DCMAKE_PREFIX_PATH=/usr/lib/aarch64-linux-gnu/cmake

cmake --build build/linux-arm64 -j$(nproc)
```

### Linux-Paketierung

Die Linux-Version kann als kleines `.tar.gz`-Archiv für AMD64 und ARM64 paketiert werden.

Das Paket bündelt Qt oder Systembibliotheken nicht direkt. Stattdessen enthält es eine Abhängigkeitsliste und ein Installationsskript für Debian- und Ubuntu-basierte Systeme.

Pakete erstellen:

```bash
chmod +x package-linux.sh
./package-linux.sh
```

Ausgabe:

```text
dist/tgz/custom-web-shell-YYYY.MM.DD-linux-amd64.tar.gz
dist/tgz/custom-web-shell-YYYY.MM.DD-linux-arm64.tar.gz
```

Jedes Archiv enthält:

```text
CustomBrowser
ini.json
Assets/
WebRoot/
run.sh
install-deps-debian.sh
dependencies/debian-packages.txt
INSTALL.md
```

Installation auf dem Zielsystem:

```bash
tar -xzf custom-web-shell-YYYY.MM.DD-linux-amd64.tar.gz
cd custom-web-shell-YYYY.MM.DD-linux-amd64
./install-deps-debian.sh
./run.sh
```

Für ARM64 wird das ARM64-Archiv verwendet:

```bash
tar -xzf custom-web-shell-YYYY.MM.DD-linux-arm64.tar.gz
cd custom-web-shell-YYYY.MM.DD-linux-arm64
./install-deps-debian.sh
./run.sh
```

Das Zielsystem muss zur jeweiligen CPU-Architektur passen.

### Windows-Build

Als Single-File-Anwendung veröffentlichen:

```powershell
dotnet publish .\CustomBrowser.csproj -c Release -r win-x64 --self-contained true /p:PublishSingleFile=true /p:IncludeNativeLibrariesForSelfExtract=true /p:EnableCompressionInSingleFile=true /p:PublishTrimmed=false
```

### Laufzeitdateien

Die ausführbare Datei erwartet diese Dateien und Ordner neben sich:

```text
CustomBrowser
ini.json
Assets/
WebRoot/
```

Unter Windows ist die ausführbare Datei normalerweise:

```text
CustomBrowser.exe
```

Paketierte Linux-Archive enthalten zusätzlich:

```text
run.sh
install-deps-debian.sh
dependencies/debian-packages.txt
INSTALL.md
```

### Repository-Name und Beschreibung

Empfohlener Repository-Name:

```text
custom-web-shell
```

Empfohlene Kurzbeschreibung:

```text
Configurable WebView shell for Windows and Linux with kiosk mode, JSON defaults and CLI flags.
```

### Drittanbieter-Software

Danke an alle Projekte, Frameworks, Laufzeiten, Bibliotheken und Werkzeuge, die in diesem Projekt verwendet werden.

Dazu gehören je nach Zielplattform:

- Microsoft Edge WebView2 Runtime
- .NET
- WPF
- Qt
- Qt WebEngine
- Qt WebChannel
- Chromium
- CMake
- GCC-, MinGW- und MSVC-Toolchains
- Debian- und Ubuntu-Paketbetreuer
- alle Open-Source-Mitwirkenden, deren Arbeit Projekte wie dieses möglich macht

Drittanbieter-Komponenten bleiben unter ihren jeweiligen Lizenzen.

### Lizenz

Dieses Projekt ist Open Source und unter der GNU Affero General Public License v3.0 oder später lizenziert.

Copyright (C) 2026 Andreas Rottmann

---

## English

`custom-web-shell` is a small and flexible browser shell for Windows and Linux.

The project loads a URL or a local HTML file into a WebView environment and can be fully configured through an `ini.json` file next to the executable. Every setting can also be overridden with command line flags.

It is intended for kiosk setups, local web apps, dashboards, embedded browser interfaces and controlled WebView environments.

### Versions

This repository contains two implementations:

- Windows version using C# and Microsoft Edge WebView2
- Linux version using C++ and Qt WebEngine

Both versions follow the same configuration concept.

### Features

- Configurable start URL or local HTML file
- Relative paths based on the executable or configuration directory
- Custom window title
- Custom window icon
- Optional minimize button
- Optional maximize button
- Optional close button
- Optional movable window
- Optional kiosk mode toggle
- Optional start in kiosk mode
- Optional password protection for window actions
- Optional developer tools
- Optional extensions support where available
- Optional address bar
- Optional tab support
- JavaScript bridge for window control
- JSON defaults through `ini.json`
- CLI overrides for all supported settings

### Configuration

Example `ini.json`:

```json
{
  "url": "WebRoot/index.html",
  "port": 8080,
  "title": "Custom Web Shell",
  "icon_path": "Assets/default.svg",

  "enable_minimize": true,
  "enable_maximize": true,
  "enable_close": true,
  "enable_move_window": true,
  "enable_kiosk_mode_toggle": true,

  "start_window_size": "1280x720",
  "start_in_kiosk": false,
  "password": "",

  "dev_tools": false,
  "extensions_enabled": false,

  "title_bar_height_px": 32,
  "title_bar_button_height_px": 24,
  "title_bar_button_width_ratio": 1.25,
  "title_bar_button_gap_px": 0,

  "show_addressbar": false,
  "allow_tabs": false
}
```

### Configuration values

| Key | Type | Description |
|---|---:|---|
| `url` | String | Start address. Can be a URL or a local HTML file, for example `WebRoot/index.html`. Relative paths are resolved relative to `ini.json` or the executable. |
| `port` | Number | Port for local use or future local server features. |
| `title` | String | Window title shown in the title bar. |
| `icon_path` | String | Path to the window icon. Relative paths are resolved relative to `ini.json`. |
| `enable_minimize` | Boolean | Enables or disables the minimize action. |
| `enable_maximize` | Boolean | Enables or disables the maximize action. |
| `enable_close` | Boolean | Enables or disables the close action. |
| `enable_move_window` | Boolean | Defines whether the window can be moved. |
| `enable_kiosk_mode_toggle` | Boolean | Enables or disables the button and API for toggling kiosk mode. |
| `start_window_size` | String | Initial window size. Supports values like `1280x720`, `auto`, `maximize` or `minimize`. |
| `start_in_kiosk` | Boolean | Starts the application directly in kiosk mode. |
| `password` | String | Optional password for protected actions. Empty means no password check. |
| `dev_tools` | Boolean | Enables or disables developer tools. |
| `extensions_enabled` | Boolean | Enables or disables extensions where supported by the target platform. |
| `title_bar_height_px` | Number | Height of the custom title bar in pixels. |
| `title_bar_button_height_px` | Number | Height of the title bar buttons in pixels. |
| `title_bar_button_width_ratio` | Number | Width ratio of the title bar buttons. |
| `title_bar_button_gap_px` | Number | Gap between buttons in pixels. |
| `show_addressbar` | Boolean | Shows or hides the address bar. |
| `allow_tabs` | Boolean | Enables or disables tabs. |

### Command line examples

```bash
./CustomBrowser --url=WebRoot/index.html --title="Dashboard"
```

```bash
./CustomBrowser --start_in_kiosk=true --enable_close=false
```

```bash
./CustomBrowser --show_addressbar=true --allow_tabs=true
```

The address bar option also supports these aliases:

```bash
--show_adressbar=true
--addressbar=true
```

The tabs option also supports:

```bash
--tabs=true
```

### JavaScript bridge

The application exposes a small JavaScript API to the loaded page.

```javascript
await window.customBrowser.minimize();
await window.customBrowser.restore();
await window.customBrowser.maximize();
await window.customBrowser.toggleKiosk();
await window.customBrowser.close();
```

If a password is configured, pass it as an argument:

```javascript
await window.customBrowser.toggleKiosk("password");
```

The current state can be requested with:

```javascript
const state = await window.customBrowser.getState();
console.log(state);
```

Example for a custom HTML control panel:

```html
<button onclick="window.customBrowser.minimize()">Minimize</button>
<button onclick="window.customBrowser.restore()">Restore</button>
<button onclick="window.customBrowser.maximize()">Maximize</button>
<button onclick="window.customBrowser.toggleKiosk()">Toggle kiosk</button>
<button onclick="window.customBrowser.close()">Close</button>
```

With password:

```html
<button onclick="window.customBrowser.toggleKiosk('password')">
  Toggle kiosk
</button>
```

### Linux build

AMD64:

```bash
make linux-amd64
```

ARM64 / aarch64:

```bash
make linux-arm64
```

or:

```bash
make linux-aarch64
```

Manual AMD64 build:

```bash
rm -rf build/linux-amd64
cmake -S . -B build/linux-amd64 \
  -DCMAKE_BUILD_TYPE=Release \
  -DQt6_DIR=/usr/lib/x86_64-linux-gnu/cmake/Qt6 \
  -DCMAKE_PREFIX_PATH=/usr/lib/x86_64-linux-gnu/cmake

cmake --build build/linux-amd64 -j$(nproc)
```

Manual ARM64 cross build:

```bash
rm -rf build/linux-arm64
cmake -S . -B build/linux-arm64 \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/aarch64-linux-gnu.cmake \
  -DQt6_DIR=/usr/lib/aarch64-linux-gnu/cmake/Qt6 \
  -DCMAKE_PREFIX_PATH=/usr/lib/aarch64-linux-gnu/cmake

cmake --build build/linux-arm64 -j$(nproc)
```

### Linux packaging

The Linux version can be packaged as small `.tar.gz` archives for AMD64 and ARM64.

The package does not bundle Qt or system libraries. Instead, it includes a dependency list and an install script for Debian/Ubuntu based systems.

Create packages:

```bash
chmod +x package-linux.sh
./package-linux.sh
```

Output:

```text
dist/tgz/custom-web-shell-YYYY.MM.DD-linux-amd64.tar.gz
dist/tgz/custom-web-shell-YYYY.MM.DD-linux-arm64.tar.gz
```

Each archive contains:

```text
CustomBrowser
ini.json
Assets/
WebRoot/
run.sh
install-deps-debian.sh
dependencies/debian-packages.txt
INSTALL.md
```

Install on the target system:

```bash
tar -xzf custom-web-shell-YYYY.MM.DD-linux-amd64.tar.gz
cd custom-web-shell-YYYY.MM.DD-linux-amd64
./install-deps-debian.sh
./run.sh
```

For ARM64 use the ARM64 archive:

```bash
tar -xzf custom-web-shell-YYYY.MM.DD-linux-arm64.tar.gz
cd custom-web-shell-YYYY.MM.DD-linux-arm64
./install-deps-debian.sh
./run.sh
```

The target system must use the matching CPU architecture.

### Windows build

Publish as a single-file application:

```powershell
dotnet publish .\CustomBrowser.csproj -c Release -r win-x64 --self-contained true /p:PublishSingleFile=true /p:IncludeNativeLibrariesForSelfExtract=true /p:EnableCompressionInSingleFile=true /p:PublishTrimmed=false
```

### Runtime files

The executable expects these files and folders next to it:

```text
CustomBrowser
ini.json
Assets/
WebRoot/
```

On Windows the executable is usually:

```text
CustomBrowser.exe
```

Packaged Linux archives also include:

```text
run.sh
install-deps-debian.sh
dependencies/debian-packages.txt
INSTALL.md
```

### Repository name and description

Suggested repository name:

```text
custom-web-shell
```

Suggested short description:

```text
Configurable WebView shell for Windows and Linux with kiosk mode, JSON defaults and CLI flags.
```

### Third-party software

Thanks to all projects, frameworks, runtimes, libraries and tools used by this project.

This includes, depending on the build target:

- Microsoft Edge WebView2 Runtime
- .NET
- WPF
- Qt
- Qt WebEngine
- Qt WebChannel
- Chromium
- CMake
- GCC, MinGW and MSVC toolchains
- Debian and Ubuntu package maintainers
- all open source contributors whose work makes projects like this possible

Third-party components remain under their own licenses.

### License

This project is open source and licensed under the GNU Affero General Public License v3.0 or later.

Copyright (C) 2026 Andreas Rottmann
