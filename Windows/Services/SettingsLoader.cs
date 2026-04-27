using System.Globalization;
using System.IO;
using System.Text.Json;
using System.Text.Json.Nodes;
using CustomBrowser.Models;

namespace CustomBrowser.Services;

public static class SettingsLoader
{
    private static readonly JsonSerializerOptions JsonOptions = new()
    {
        WriteIndented = true,
        PropertyNameCaseInsensitive = true
    };

    private static readonly Dictionary<string, string> CliAliases = new(StringComparer.OrdinalIgnoreCase)
    {
        ["url"] = "url",
        ["port"] = "port",
        ["title"] = "title_name",
        ["window_title"] = "title_name",
        ["title_name"] = "title_name",
        ["name"] = "title_name",
        ["ico"] = "icon_path",
        ["icon"] = "icon_path",
        ["ico_path"] = "icon_path",
        ["icon_path"] = "icon_path",
        ["enable_minimize"] = "enable_minimize",
        ["minimize"] = "enable_minimize",
        ["enable_maximize"] = "enable_maximize",
        ["enable_maximaze"] = "enable_maximize",
        ["maximize"] = "enable_maximize",
        ["maximaze"] = "enable_maximize",
        ["enable_close"] = "enable_close",
        ["close"] = "enable_close",
        ["enable_move_window"] = "enable_move_window",
        ["enable_movement"] = "enable_move_window",
        ["movement"] = "enable_move_window",
        ["move_window"] = "enable_move_window",
        ["move"] = "enable_move_window",
        ["enable_kiosk_mode_toggle"] = "enable_kiosk_mode_toggle",
        ["kiosk_toggle"] = "enable_kiosk_mode_toggle",
        ["toggle_kiosk"] = "enable_kiosk_mode_toggle",
        ["start_window_size"] = "start_window_size",
        ["window_size"] = "start_window_size",
        ["start_in_kiosk"] = "start_in_kiosk",
        ["kiosk"] = "start_in_kiosk",
        ["control_password"] = "control_password",
        ["password"] = "control_password",
        ["use_password_to_change_maximize_minimize_close_toggle_kiosk"] = "control_password",
        ["dev_tools"] = "dev_tools",
        ["devtools"] = "dev_tools",
        ["extensions_enabled"] = "extensions_enabled",
        ["extensions"] = "extensions_enabled",
        ["enable_autofill"] = "enable_autofill",
        ["autofill"] = "enable_autofill",
        ["form_autofill"] = "enable_autofill",
        ["enable_password_saving"] = "enable_password_saving",
        ["password_saving"] = "enable_password_saving",
        ["save_passwords"] = "enable_password_saving",
        ["password_autosave"] = "enable_password_saving",
        ["enable_translate"] = "enable_translate",
        ["translate"] = "enable_translate",
        ["translation"] = "enable_translate",
        ["translations"] = "enable_translate",
        ["titlebar_style"] = "titlebar_style",
        ["title_bar_style"] = "titlebar_style",
        ["use_native_titlebar"] = "use_native_titlebar",
        ["native_titlebar"] = "use_native_titlebar",
        ["show_kiosk_button_in_content"] = "show_kiosk_button_in_content",
        ["content_kiosk_button"] = "show_kiosk_button_in_content",
        ["kiosk_button_icon_type"] = "kiosk_button_icon_type",
        ["kiosk_icon_type"] = "kiosk_button_icon_type",
        ["kiosk_button_icon"] = "kiosk_button_icon",
        ["kiosk_icon"] = "kiosk_button_icon",
        ["kiosk_button_text"] = "kiosk_button_text",
        ["kiosk_text"] = "kiosk_button_text",
        ["kiosk_button_tooltip"] = "kiosk_button_tooltip",
        ["kiosk_tooltip"] = "kiosk_button_tooltip",
        ["custom_titlebar_background"] = "custom_titlebar_background",
        ["titlebar_background"] = "custom_titlebar_background",
        ["custom_titlebar_foreground"] = "custom_titlebar_foreground",
        ["titlebar_foreground"] = "custom_titlebar_foreground",
        ["content_kiosk_bar_background"] = "content_kiosk_bar_background",
        ["content_kiosk_bar_foreground"] = "content_kiosk_bar_foreground",
        ["title_bar_height_px"] = "title_bar_height_px",
        ["titlebar_height"] = "title_bar_height_px",
        ["title_bar_button_height_px"] = "title_bar_button_height_px",
        ["titlebar_button_height"] = "title_bar_button_height_px",
        ["title_bar_button_width_ratio"] = "title_bar_button_width_ratio",
        ["titlebar_button_ratio"] = "title_bar_button_width_ratio",
        ["title_bar_button_gap_px"] = "title_bar_button_gap_px",
        ["titlebar_button_gap"] = "title_bar_button_gap_px",
        ["show_addressbar"] = "show_addressbar",
        ["show_adressbar"] = "show_addressbar",
        ["addressbar"] = "show_addressbar",
        ["address_bar"] = "show_addressbar",
        ["allow_tabs"] = "allow_tabs",
        ["tabs"] = "allow_tabs"
    };

    public static LoadedSettings Load(string executableDirectory, string[] args)
    {
        var configPath = GetConfigPath(executableDirectory, args);
        var configDirectory = Path.GetDirectoryName(configPath) ?? executableDirectory;

        if (!File.Exists(configPath))
        {
            Directory.CreateDirectory(configDirectory);
            File.WriteAllText(configPath, JsonSerializer.Serialize(new BrowserSettings(), JsonOptions));
        }

        var settings = LoadJson(configPath);
        ApplyCliOverrides(settings, args);
        Validate(settings);

        return new LoadedSettings(settings, configPath, configDirectory);
    }

    private static string GetConfigPath(string executableDirectory, string[] args)
    {
        string? explicitConfig = null;

        for (var i = 0; i < args.Length; i++)
        {
            var arg = args[i];
            if (!IsFlag(arg))
            {
                continue;
            }

            var (key, valueFromEquals) = SplitFlag(arg);
            key = NormalizeKey(key);
            if (key is not "config" and not "ini" and not "settings")
            {
                continue;
            }

            explicitConfig = valueFromEquals;
            if (string.IsNullOrWhiteSpace(explicitConfig) && i + 1 < args.Length)
            {
                explicitConfig = args[++i];
            }
        }

        if (string.IsNullOrWhiteSpace(explicitConfig))
        {
            return Path.Combine(executableDirectory, "ini.json");
        }

        return Path.GetFullPath(explicitConfig);
    }

    private static BrowserSettings LoadJson(string configPath)
    {
        var json = File.ReadAllText(configPath);
        var root = JsonNode.Parse(json) as JsonObject
            ?? throw new InvalidOperationException("ini.json muss ein JSON-Objekt enthalten.");

        var settings = new BrowserSettings();

        settings.Url = GetString(root, settings.Url, "url");
        settings.Port = GetInt(root, settings.Port, "port");
        settings.TitleName = GetString(root, settings.TitleName, "title_name", "window_title", "title");
        settings.IconPath = GetString(root, settings.IconPath, "icon_path", "ico_path", "ico", "icon");
        settings.EnableMinimize = GetBool(root, settings.EnableMinimize, "enable_minimize", "minimize");
        settings.EnableMaximize = GetBool(root, settings.EnableMaximize, "enable_maximize", "enable_maximaze", "maximize", "maximaze");
        settings.EnableClose = GetBool(root, settings.EnableClose, "enable_close", "close");
        settings.EnableMoveWindow = GetBool(root, settings.EnableMoveWindow, "enable_move_window", "enable_movement", "movement", "move_window", "move");
        settings.EnableKioskModeToggle = GetBool(root, settings.EnableKioskModeToggle, "enable_kiosk_mode_toggle", "kiosk_toggle", "toggle_kiosk");
        settings.StartWindowSize = GetString(root, settings.StartWindowSize, "start_window_size", "window_size");
        settings.StartInKiosk = GetBool(root, settings.StartInKiosk, "start_in_kiosk", "kiosk");
        settings.ControlPassword = GetString(root, settings.ControlPassword, "control_password", "password", "use_password_to_change_maximize_minimize_close_toggle_kiosk");
        settings.DevTools = GetBool(root, settings.DevTools, "dev_tools", "devtools");
        settings.ExtensionsEnabled = GetBool(root, settings.ExtensionsEnabled, "extensions_enabled", "extensions");
        settings.EnableAutofill = GetBool(root, settings.EnableAutofill, "enable_autofill", "autofill", "form_autofill");
        settings.EnablePasswordSaving = GetBool(root, settings.EnablePasswordSaving, "enable_password_saving", "password_saving", "save_passwords", "password_autosave");
        settings.EnableTranslate = GetBool(root, settings.EnableTranslate, "enable_translate", "translate", "translation", "translations");
        settings.TitlebarStyle = GetString(root, settings.TitlebarStyle, "titlebar_style", "title_bar_style");
        if (TryGetBool(root, out var useNativeTitlebar, "use_native_titlebar", "native_titlebar"))
        {
            settings.TitlebarStyle = useNativeTitlebar ? "native" : "custom";
        }
        settings.ShowKioskButtonInContent = GetBool(root, settings.ShowKioskButtonInContent, "show_kiosk_button_in_content", "content_kiosk_button");
        settings.KioskButtonIconType = GetString(root, settings.KioskButtonIconType, "kiosk_button_icon_type", "kiosk_icon_type");
        settings.KioskButtonIcon = GetString(root, settings.KioskButtonIcon, "kiosk_button_icon", "kiosk_icon");
        settings.KioskButtonText = GetString(root, settings.KioskButtonText, "kiosk_button_text", "kiosk_text");
        settings.KioskButtonTooltip = GetString(root, settings.KioskButtonTooltip, "kiosk_button_tooltip", "kiosk_tooltip");
        settings.CustomTitlebarBackground = GetString(root, settings.CustomTitlebarBackground, "custom_titlebar_background", "titlebar_background");
        settings.CustomTitlebarForeground = GetString(root, settings.CustomTitlebarForeground, "custom_titlebar_foreground", "titlebar_foreground");
        settings.ContentKioskBarBackground = GetString(root, settings.ContentKioskBarBackground, "content_kiosk_bar_background");
        settings.ContentKioskBarForeground = GetString(root, settings.ContentKioskBarForeground, "content_kiosk_bar_foreground");
        settings.TitleBarHeightPx = GetDouble(root, settings.TitleBarHeightPx, "title_bar_height_px", "titlebar_height");
        settings.TitleBarButtonHeightPx = GetDouble(root, settings.TitleBarButtonHeightPx, "title_bar_button_height_px", "titlebar_button_height");
        settings.TitleBarButtonWidthRatio = GetDouble(root, settings.TitleBarButtonWidthRatio, "title_bar_button_width_ratio", "titlebar_button_ratio");
        settings.TitleBarButtonGapPx = GetDouble(root, settings.TitleBarButtonGapPx, "title_bar_button_gap_px", "titlebar_button_gap");
        settings.ShowAddressBar = GetBool(root, settings.ShowAddressBar, "show_addressbar", "show_adressbar", "addressbar", "address_bar");
        settings.AllowTabs = GetBool(root, settings.AllowTabs, "allow_tabs", "tabs");

        return settings;
    }

    private static void ApplyCliOverrides(BrowserSettings settings, string[] args)
    {
        for (var i = 0; i < args.Length; i++)
        {
            var arg = args[i];
            if (!IsFlag(arg))
            {
                continue;
            }

            var (rawKey, value) = SplitFlag(arg);
            var normalized = NormalizeKey(rawKey);

            if (normalized is "config" or "ini" or "settings")
            {
                if (string.IsNullOrWhiteSpace(value) && i + 1 < args.Length)
                {
                    i++;
                }
                continue;
            }

            if (!CliAliases.TryGetValue(normalized, out var key))
            {
                throw new InvalidOperationException($"Unbekannter CLI-Schalter: {arg}");
            }

            if (value is null)
            {
                if (i + 1 < args.Length && !IsFlag(args[i + 1]))
                {
                    value = args[++i];
                }
                else
                {
                    value = "true";
                }
            }

            SetValue(settings, key, value);
        }
    }

    private static void SetValue(BrowserSettings settings, string key, string value)
    {
        switch (key)
        {
            case "url": settings.Url = value; break;
            case "port": settings.Port = ParseInt(value, key); break;
            case "title_name": settings.TitleName = value; break;
            case "icon_path": settings.IconPath = value; break;
            case "enable_minimize": settings.EnableMinimize = ParseBool(value, key); break;
            case "enable_maximize": settings.EnableMaximize = ParseBool(value, key); break;
            case "enable_close": settings.EnableClose = ParseBool(value, key); break;
            case "enable_move_window": settings.EnableMoveWindow = ParseBool(value, key); break;
            case "enable_kiosk_mode_toggle": settings.EnableKioskModeToggle = ParseBool(value, key); break;
            case "start_window_size": settings.StartWindowSize = value; break;
            case "start_in_kiosk": settings.StartInKiosk = ParseBool(value, key); break;
            case "control_password": settings.ControlPassword = value; break;
            case "dev_tools": settings.DevTools = ParseBool(value, key); break;
            case "extensions_enabled": settings.ExtensionsEnabled = ParseBool(value, key); break;
            case "enable_autofill": settings.EnableAutofill = ParseBool(value, key); break;
            case "enable_password_saving": settings.EnablePasswordSaving = ParseBool(value, key); break;
            case "enable_translate": settings.EnableTranslate = ParseBool(value, key); break;
            case "titlebar_style": settings.TitlebarStyle = value; break;
            case "use_native_titlebar": settings.TitlebarStyle = ParseBool(value, key) ? "native" : "custom"; break;
            case "show_kiosk_button_in_content": settings.ShowKioskButtonInContent = ParseBool(value, key); break;
            case "kiosk_button_icon_type": settings.KioskButtonIconType = value; break;
            case "kiosk_button_icon": settings.KioskButtonIcon = value; break;
            case "kiosk_button_text": settings.KioskButtonText = value; break;
            case "kiosk_button_tooltip": settings.KioskButtonTooltip = value; break;
            case "custom_titlebar_background": settings.CustomTitlebarBackground = value; break;
            case "custom_titlebar_foreground": settings.CustomTitlebarForeground = value; break;
            case "content_kiosk_bar_background": settings.ContentKioskBarBackground = value; break;
            case "content_kiosk_bar_foreground": settings.ContentKioskBarForeground = value; break;
            case "title_bar_height_px": settings.TitleBarHeightPx = ParseDouble(value, key); break;
            case "title_bar_button_height_px": settings.TitleBarButtonHeightPx = ParseDouble(value, key); break;
            case "title_bar_button_width_ratio": settings.TitleBarButtonWidthRatio = ParseDouble(value, key); break;
            case "title_bar_button_gap_px": settings.TitleBarButtonGapPx = ParseDouble(value, key); break;
            case "show_addressbar": settings.ShowAddressBar = ParseBool(value, key); break;
            case "allow_tabs": settings.AllowTabs = ParseBool(value, key); break;
            default: throw new InvalidOperationException($"Interner Fehler: unbekannter Settings-Key {key}");
        }
    }

    private static bool IsFlag(string value) => value.StartsWith("--") || (value.StartsWith('/') && value.Length > 1);

    private static (string Key, string? Value) SplitFlag(string flag)
    {
        var trimmed = flag.TrimStart('-').TrimStart('/');
        var eq = trimmed.IndexOf('=');
        if (eq < 0)
        {
            return (trimmed, null);
        }

        return (trimmed[..eq], trimmed[(eq + 1)..]);
    }

    private static string NormalizeKey(string key) => key.Trim().Replace('-', '_').Replace('.', '_').ToLowerInvariant();

    private static string GetString(JsonObject root, string fallback, params string[] names)
    {
        foreach (var name in names)
        {
            if (root.TryGetPropertyValue(name, out var node) && node is not null)
            {
                return node.GetValue<string>();
            }
        }
        return fallback;
    }

    private static int GetInt(JsonObject root, int fallback, params string[] names)
    {
        foreach (var name in names)
        {
            if (root.TryGetPropertyValue(name, out var node) && node is not null)
            {
                if (node.GetValueKind() == JsonValueKind.Number && node.GetValue<int>() is var numeric)
                {
                    return numeric;
                }
                return ParseInt(node.GetValue<string>(), name);
            }
        }
        return fallback;
    }

    private static bool GetBool(JsonObject root, bool fallback, params string[] names)
    {
        foreach (var name in names)
        {
            if (root.TryGetPropertyValue(name, out var node) && node is not null)
            {
                if (node.GetValueKind() == JsonValueKind.True)
                {
                    return true;
                }
                if (node.GetValueKind() == JsonValueKind.False)
                {
                    return false;
                }
                return ParseBool(node.GetValue<string>(), name);
            }
        }
        return fallback;
    }

    private static bool TryGetBool(JsonObject root, out bool value, params string[] names)
    {
        foreach (var name in names)
        {
            if (root.TryGetPropertyValue(name, out var node) && node is not null)
            {
                if (node.GetValueKind() == JsonValueKind.True)
                {
                    value = true;
                    return true;
                }
                if (node.GetValueKind() == JsonValueKind.False)
                {
                    value = false;
                    return true;
                }
                value = ParseBool(node.GetValue<string>(), name);
                return true;
            }
        }

        value = false;
        return false;
    }

    private static double GetDouble(JsonObject root, double fallback, params string[] names)
    {
        foreach (var name in names)
        {
            if (root.TryGetPropertyValue(name, out var node) && node is not null)
            {
                if (node.GetValueKind() == JsonValueKind.Number && node.GetValue<double>() is var numeric)
                {
                    return numeric;
                }
                return ParseDouble(node.GetValue<string>(), name);
            }
        }
        return fallback;
    }

    private static double ParseDouble(string value, string key)
    {
        if (double.TryParse(value, NumberStyles.Float, CultureInfo.InvariantCulture, out var result))
        {
            return result;
        }
        throw new InvalidOperationException($"{key} muss eine Zahl sein. Wert: {value}");
    }

    private static int ParseInt(string value, string key)
    {
        if (int.TryParse(value, NumberStyles.Integer, CultureInfo.InvariantCulture, out var result))
        {
            return result;
        }
        throw new InvalidOperationException($"{key} muss eine ganze Zahl sein. Wert: {value}");
    }

    private static bool ParseBool(string value, string key)
    {
        if (bool.TryParse(value, out var result))
        {
            return result;
        }

        return value.Trim().ToLowerInvariant() switch
        {
            "1" or "yes" or "ja" or "on" => true,
            "0" or "no" or "nein" or "off" => false,
            _ => throw new InvalidOperationException($"{key} muss true oder false sein. Wert: {value}")
        };
    }

    private static void Validate(BrowserSettings settings)
    {
        if (string.IsNullOrWhiteSpace(settings.Url))
        {
            throw new InvalidOperationException("url darf nicht leer sein.");
        }

        if (settings.Port < 0 || settings.Port > 65535)
        {
            throw new InvalidOperationException("port muss zwischen 0 und 65535 liegen. 0 deaktiviert den lokalen Server.");
        }

        var titlebarStyle = settings.TitlebarStyle.Trim().ToLowerInvariant();
        if (titlebarStyle is not ("custom" or "native" or "system" or "windows"))
        {
            throw new InvalidOperationException("titlebar_style muss custom, native, system oder windows sein.");
        }

        var kioskIconType = settings.KioskButtonIconType.Trim().ToLowerInvariant();
        if (kioskIconType is not ("text" or "ascii" or "unicode" or "image" or "png" or "ico" or "svg"))
        {
            throw new InvalidOperationException("kiosk_button_icon_type muss text, ascii, unicode, image, png, ico oder svg sein.");
        }

        if (settings.TitleBarHeightPx < 16 || settings.TitleBarHeightPx > 160)
        {
            throw new InvalidOperationException("title_bar_height_px muss zwischen 16 und 160 liegen.");
        }

        if (settings.TitleBarButtonHeightPx < 12 || settings.TitleBarButtonHeightPx > settings.TitleBarHeightPx)
        {
            throw new InvalidOperationException("title_bar_button_height_px muss zwischen 12 und title_bar_height_px liegen.");
        }

        if (settings.TitleBarButtonWidthRatio < 0.5 || settings.TitleBarButtonWidthRatio > 6)
        {
            throw new InvalidOperationException("title_bar_button_width_ratio muss zwischen 0.5 und 6 liegen.");
        }
    }
}
