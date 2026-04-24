# custom-web-shell

Configurable WebView shell for Windows and Linux with kiosk mode, JSON defaults and CLI flags.

This project provides a small browser-like application that can be configured through an `ini.json` file next to the executable. Every setting can also be overridden with command line flags.

It is intended for kiosk setups, local web apps, dashboards, embedded browser use cases and controlled WebView environments.

## Versions

This repository contains two implementations:

- Windows version using C# and Microsoft Edge WebView2
- Linux version using C++ and Qt WebEngine

Both versions follow the same configuration idea.

## Features

- Configurable start URL or local HTML file
- Relative paths based on the executable/config directory
- Custom window title
- Custom icon
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

## Configuration

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

## Command line examples

```bash
./CustomBrowser --url=WebRoot/index.html --title="Dashboard"
```

```bash
./CustomBrowser --start_in_kiosk=true --enable_close=false
```

```bash
./CustomBrowser --show_addressbar=true --allow_tabs=true
```

The address bar flag also supports these aliases:

```bash
--show_adressbar=true
--addressbar=true
```

The tabs flag also supports:

```bash
--tabs=true
```

## JavaScript bridge

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

State can be requested with:

```javascript
const state = await window.customBrowser.getState();
console.log(state);
```

## Linux build

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

## Linux packaging

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

## Windows build

Publish as single file:

```powershell
dotnet publish .\CustomBrowser.csproj -c Release -r win-x64 --self-contained true /p:PublishSingleFile=true /p:IncludeNativeLibrariesForSelfExtract=true /p:EnableCompressionInSingleFile=true /p:PublishTrimmed=false
```

## Runtime files

The executable expects these files/folders next to it:

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

## Repository name and description

Suggested repository name:

```text
custom-web-shell
```

Suggested short description:

```text
Configurable WebView shell for Windows and Linux with kiosk mode, JSON defaults and CLI flags.
```

## Third-party software

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
- GCC / MinGW / MSVC toolchains
- Debian and Ubuntu package maintainers
- all open source contributors whose work makes projects like this possible

Third-party components remain under their own licenses.

## License

This project is open source and licensed under the GNU Affero General Public License v3.0 or later.

Copyright (C) 2026 Andreas Rottmann
