using CustomBrowser.Models;

namespace CustomBrowser.Services;

public sealed record LoadedSettings(BrowserSettings Settings, string ConfigPath, string ConfigDirectory);
