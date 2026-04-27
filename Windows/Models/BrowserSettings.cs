using System.Text.Json.Serialization;

namespace CustomBrowser.Models;

public sealed class BrowserSettings
{
    [JsonPropertyName("url")]
    public string Url { get; set; } = "WebRoot/index.html";

    [JsonPropertyName("port")]
    public int Port { get; set; } = 8765;

    [JsonPropertyName("title_name")]
    public string TitleName { get; set; } = "Custom Browser";

    [JsonPropertyName("icon_path")]
    public string IconPath { get; set; } = "Assets/default.ico";

    [JsonPropertyName("enable_minimize")]
    public bool EnableMinimize { get; set; } = true;

    [JsonPropertyName("enable_maximize")]
    public bool EnableMaximize { get; set; } = true;

    [JsonPropertyName("enable_close")]
    public bool EnableClose { get; set; } = true;

    [JsonPropertyName("enable_move_window")]
    public bool EnableMoveWindow { get; set; } = true;

    [JsonPropertyName("enable_kiosk_mode_toggle")]
    public bool EnableKioskModeToggle { get; set; } = true;

    [JsonPropertyName("start_window_size")]
    public string StartWindowSize { get; set; } = "1100,720";

    [JsonPropertyName("start_in_kiosk")]
    public bool StartInKiosk { get; set; } = false;

    [JsonPropertyName("control_password")]
    public string ControlPassword { get; set; } = string.Empty;

    [JsonPropertyName("dev_tools")]
    public bool DevTools { get; set; } = true;

    [JsonPropertyName("extensions_enabled")]
    public bool ExtensionsEnabled { get; set; } = false;

    [JsonPropertyName("enable_autofill")]
    public bool EnableAutofill { get; set; } = true;

    [JsonPropertyName("enable_password_saving")]
    public bool EnablePasswordSaving { get; set; } = false;

    [JsonPropertyName("enable_translate")]
    public bool EnableTranslate { get; set; } = true;

    [JsonPropertyName("title_bar_height_px")]
    public double TitleBarHeightPx { get; set; } = 32;

    [JsonPropertyName("title_bar_button_height_px")]
    public double TitleBarButtonHeightPx { get; set; } = 24;

    [JsonPropertyName("title_bar_button_width_ratio")]
    public double TitleBarButtonWidthRatio { get; set; } = 1.25;

    [JsonPropertyName("title_bar_button_gap_px")]
    public double TitleBarButtonGapPx { get; set; } = 0;

    [JsonPropertyName("show_addressbar")]
    public bool ShowAddressBar { get; set; } = false;

    [JsonPropertyName("allow_tabs")]
    public bool AllowTabs { get; set; } = false;
}
