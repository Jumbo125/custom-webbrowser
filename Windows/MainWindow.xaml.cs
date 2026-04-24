using System.IO;
using System.Text.Json;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using CustomBrowser.Models;
using CustomBrowser.Services;
using Microsoft.Web.WebView2.Core;
using Microsoft.Web.WebView2.Wpf;

namespace CustomBrowser;

public partial class MainWindow : Window
{
    private readonly BrowserSettings _settings;
    private readonly string _configDirectory;
    private CoreWebView2Environment? _environment;
    private LocalWebServer? _server;
    private bool _isKiosk;
    private bool _closeWasApproved;
    private double _currentTitleBarHeight = 36;
    private WindowSnapshot? _windowSnapshot;

    private WebView2? CurrentBrowser => BrowserTabs.SelectedItem is TabItem tab ? tab.Content as WebView2 : null;

    public MainWindow(BrowserSettings settings, string configDirectory)
    {
        InitializeComponent();
        _settings = settings;
        _configDirectory = configDirectory;
    }

    private async void Window_Loaded(object sender, RoutedEventArgs e)
    {
        ApplyInitialWindowSettings();
        await InitializeWebViewAsync();

        if (_settings.StartInKiosk)
        {
            await ExecuteCommandAsync("toggleKiosk", null, skipPassword: true);
        }
    }

    private void ApplyInitialWindowSettings()
    {
        Title = _settings.TitleName;
        WindowTitle.Text = _settings.TitleName;

        var iconPath = ResolvePath(_settings.IconPath);
        if (File.Exists(iconPath))
        {
            Icon = new BitmapImage(new Uri(iconPath, UriKind.Absolute));
            WindowIcon.Source = Icon;
        }

        KioskToggleButton.Visibility = _settings.EnableKioskModeToggle ? Visibility.Visible : Visibility.Collapsed;
        MinimizeButton.IsEnabled = _settings.EnableMinimize;
        MaximizeButton.IsEnabled = _settings.EnableMaximize;
        CloseButton.IsEnabled = _settings.EnableClose;
        AddressBarContainer.Visibility = _settings.ShowAddressBar ? Visibility.Visible : Visibility.Collapsed;

        if (!_settings.AllowTabs)
        {
            BrowserTabs.Template = (ControlTemplate)FindResource("BrowserOnlyTabControlTemplate");
        }

        ApplyTitleBarSizing();
        ApplyStartWindowSize(_settings.StartWindowSize);
        UpdateMaximizeButton();
        UpdateNavigationUi();
    }

    private async Task InitializeWebViewAsync()
    {
        if (_settings.Port > 0)
        {
            _server = new LocalWebServer(_configDirectory, _settings.Port);
            _server.Start();
        }

        var userDataFolder = Path.Combine(_configDirectory, "UserData");
        Directory.CreateDirectory(userDataFolder);

        var options = new CoreWebView2EnvironmentOptions
        {
            AreBrowserExtensionsEnabled = _settings.ExtensionsEnabled
        };

        _environment = await CoreWebView2Environment.CreateAsync(null, userDataFolder, options);
        await CreateBrowserTabAsync(ResolveNavigationUri(_settings.Url), "Start");
    }

    private async Task<WebView2> CreateBrowserTabAsync(Uri? source, string title = "Neuer Tab")
    {
        if (_environment is null)
        {
            throw new InvalidOperationException("WebView2-Umgebung ist noch nicht initialisiert.");
        }

        var browser = new WebView2();
        var tab = CreateTabItem(browser, title);
        BrowserTabs.Items.Add(tab);
        BrowserTabs.SelectedItem = tab;

        await browser.EnsureCoreWebView2Async(_environment);
        await ConfigureBrowserAsync(browser);

        if (source is not null)
        {
            browser.Source = source;
        }

        UpdateNavigationUi();
        return browser;
    }

    private async Task ConfigureBrowserAsync(WebView2 browser)
    {
        var core = browser.CoreWebView2;
        if (core is null)
        {
            return;
        }

        core.Settings.AreDevToolsEnabled = _settings.DevTools;
        core.WebMessageReceived += Browser_WebMessageReceived;
        core.NewWindowRequested += async (_, e) => await HandleNewWindowRequestedAsync(browser, e);
        core.SourceChanged += (_, _) => Dispatcher.Invoke(UpdateNavigationUi);
        core.NavigationCompleted += (_, _) => Dispatcher.Invoke(() =>
        {
            UpdateNavigationUi();
            UpdateTabTitle(browser);
        });
        core.DocumentTitleChanged += (_, _) => Dispatcher.Invoke(() => UpdateTabTitle(browser));

        await core.AddScriptToExecuteOnDocumentCreatedAsync(GetBridgeScript());
    }

    private async Task HandleNewWindowRequestedAsync(WebView2 currentBrowser, CoreWebView2NewWindowRequestedEventArgs e)
    {
        e.Handled = true;

        if (!_settings.AllowTabs)
        {
            if (!string.IsNullOrWhiteSpace(e.Uri) && currentBrowser.CoreWebView2 is not null)
            {
                currentBrowser.CoreWebView2.Navigate(e.Uri);
            }
            return;
        }

        var deferral = e.GetDeferral();
        try
        {
            var newBrowser = await CreateBrowserTabAsync(null);
            e.NewWindow = newBrowser.CoreWebView2;
        }
        finally
        {
            deferral.Complete();
        }
    }

    private TabItem CreateTabItem(WebView2 browser, string title)
    {
        var tab = new TabItem
        {
            Content = browser
        };

        if (!_settings.AllowTabs)
        {
            tab.Header = title;
            return tab;
        }

        var headerText = new TextBlock
        {
            Text = title,
            MaxWidth = 180,
            TextTrimming = TextTrimming.CharacterEllipsis,
            VerticalAlignment = VerticalAlignment.Center
        };

        var closeButton = new Button
        {
            Content = "×",
            Width = 18,
            Height = 18,
            Margin = new Thickness(8, 0, 0, 0),
            Padding = new Thickness(0),
            ToolTip = "Tab schließen"
        };

        closeButton.Click += (_, e) =>
        {
            e.Handled = true;
            CloseTab(tab);
        };

        var header = new StackPanel
        {
            Orientation = Orientation.Horizontal
        };
        header.Children.Add(headerText);
        header.Children.Add(closeButton);

        tab.Tag = headerText;
        tab.Header = header;
        return tab;
    }

    private void CloseTab(TabItem tab)
    {
        if (BrowserTabs.Items.Count <= 1)
        {
            return;
        }

        if (tab.Content is WebView2 browser)
        {
            browser.Dispose();
        }

        BrowserTabs.Items.Remove(tab);
        UpdateNavigationUi();
    }

    private void UpdateTabTitle(WebView2 browser)
    {
        foreach (var item in BrowserTabs.Items)
        {
            if (item is not TabItem tab || tab.Content != browser)
            {
                continue;
            }

            var title = browser.CoreWebView2?.DocumentTitle;
            if (string.IsNullOrWhiteSpace(title))
            {
                title = browser.Source?.ToString() ?? "Tab";
            }

            if (tab.Tag is TextBlock textBlock)
            {
                textBlock.Text = title;
            }
            else
            {
                tab.Header = title;
            }

            return;
        }
    }

    private Uri ResolveNavigationUri(string configuredUrl)
    {
        if (Uri.TryCreate(configuredUrl, UriKind.Absolute, out var absolute) &&
            absolute.Scheme is "http" or "https" or "file")
        {
            return absolute;
        }

        var urlLikePath = configuredUrl.Replace('\\', '/');

        if (_settings.Port > 0)
        {
            if (!urlLikePath.StartsWith('/'))
            {
                urlLikePath = "/" + urlLikePath;
            }
            return new Uri($"http://127.0.0.1:{_settings.Port}{urlLikePath}");
        }

        var fullPath = ResolvePath(configuredUrl);
        return new Uri(fullPath, UriKind.Absolute);
    }

    private Uri ResolveUserInputUri(string value)
    {
        var text = (value ?? string.Empty).Trim();
        if (string.IsNullOrWhiteSpace(text))
        {
            throw new InvalidOperationException("URL darf nicht leer sein.");
        }

        if (Uri.TryCreate(text, UriKind.Absolute, out var absolute))
        {
            return absolute;
        }

        var localPath = ResolvePath(text);
        if (File.Exists(localPath))
        {
            return new Uri(localPath, UriKind.Absolute);
        }

        return new Uri("https://" + text);
    }

    private string ResolvePath(string path)
    {
        if (Path.IsPathRooted(path))
        {
            return Path.GetFullPath(path);
        }
        return Path.GetFullPath(Path.Combine(_configDirectory, path));
    }

    private void ApplyTitleBarSizing()
    {
        var titleBarHeight = Math.Clamp(_settings.TitleBarHeightPx, 22, 96);
        var buttonHeight = Math.Clamp(_settings.TitleBarButtonHeightPx, 16, titleBarHeight);
        var ratio = Math.Clamp(_settings.TitleBarButtonWidthRatio, 0.8, 4.0);
        var gap = Math.Clamp(_settings.TitleBarButtonGapPx, 0, 24);
        var buttonWidth = Math.Round(buttonHeight * ratio);

        _currentTitleBarHeight = titleBarHeight;
        TitleBarRow.Height = new GridLength(titleBarHeight);

        ApplyTitleButtonSize(KioskToggleButton, buttonWidth, buttonHeight, 0);
        ApplyTitleButtonSize(MinimizeButton, buttonWidth, buttonHeight, gap);
        ApplyTitleButtonSize(MaximizeButton, buttonWidth, buttonHeight, gap);
        ApplyTitleButtonSize(CloseButton, buttonWidth, buttonHeight, gap);
    }

    private static void ApplyTitleButtonSize(Button button, double width, double height, double leftGap)
    {
        button.Width = width;
        button.Height = height;
        button.MinWidth = width;
        button.Padding = new Thickness(0);
        button.Margin = new Thickness(leftGap, 0, 0, 0);
        button.FontSize = Math.Max(10, height * 0.55);
    }

    private void ApplyStartWindowSize(string value)
    {
        var normalized = (value ?? string.Empty).Trim().ToLowerInvariant();
        if (string.IsNullOrWhiteSpace(normalized) || normalized == "auto")
        {
            SizeToContent = SizeToContent.Manual;
            return;
        }

        if (normalized is "maximize" or "maximaze" or "maximized")
        {
            WindowState = WindowState.Maximized;
            return;
        }

        if (normalized is "minimize" or "minimized")
        {
            WindowState = WindowState.Minimized;
            return;
        }

        var separator = normalized.Contains('x') ? 'x' : ',';
        var parts = normalized.Split(separator, StringSplitOptions.TrimEntries | StringSplitOptions.RemoveEmptyEntries);
        if (parts.Length == 2 && double.TryParse(parts[0], out var width) && double.TryParse(parts[1], out var height))
        {
            Width = Math.Max(MinWidth, width);
            Height = Math.Max(MinHeight, height);
            WindowStartupLocation = WindowStartupLocation.CenterScreen;
            return;
        }

        throw new InvalidOperationException("start_window_size muss auto, maximize, minimize oder Breite,Höhe sein, z. B. 1100,720.");
    }

    private void TitleBar_MouseLeftButtonDown(object sender, MouseButtonEventArgs e)
    {
        if (_isKiosk || !_settings.EnableMoveWindow)
        {
            return;
        }

        if (e.ClickCount == 2 && _settings.EnableMaximize)
        {
            _ = ExecuteCommandAsync(WindowState == WindowState.Maximized ? "restore" : "maximize");
            return;
        }

        if (e.ButtonState == MouseButtonState.Pressed)
        {
            try
            {
                DragMove();
            }
            catch
            {
                // DragMove may throw if the mouse state changes during the operation.
            }
        }
    }

    private async void MinimizeButton_Click(object sender, RoutedEventArgs e) => await ExecuteCommandAsync("minimize");

    private async void MaximizeButton_Click(object sender, RoutedEventArgs e)
    {
        await ExecuteCommandAsync(WindowState == WindowState.Maximized ? "restore" : "maximize");
    }

    private async void KioskToggleButton_Click(object sender, RoutedEventArgs e) => await ExecuteCommandAsync("toggleKiosk");

    private async void CloseButton_Click(object sender, RoutedEventArgs e) => await ExecuteCommandAsync("close");

    private void BackButton_Click(object sender, RoutedEventArgs e)
    {
        CurrentBrowser?.CoreWebView2?.GoBack();
    }

    private void ForwardButton_Click(object sender, RoutedEventArgs e)
    {
        CurrentBrowser?.CoreWebView2?.GoForward();
    }

    private void ReloadButton_Click(object sender, RoutedEventArgs e)
    {
        CurrentBrowser?.CoreWebView2?.Reload();
    }

    private void GoButton_Click(object sender, RoutedEventArgs e) => NavigateFromAddressBar();

    private void AddressTextBox_KeyDown(object sender, KeyEventArgs e)
    {
        if (e.Key == Key.Enter)
        {
            e.Handled = true;
            NavigateFromAddressBar();
        }
    }

    private void NavigateFromAddressBar()
    {
        var browser = CurrentBrowser;
        if (browser is null)
        {
            return;
        }

        browser.Source = ResolveUserInputUri(AddressTextBox.Text);
    }

    private void BrowserTabs_SelectionChanged(object sender, SelectionChangedEventArgs e)
    {
        if (e.Source == BrowserTabs)
        {
            UpdateNavigationUi();
        }
    }

    private void UpdateNavigationUi()
    {
        var browser = CurrentBrowser;
        if (browser?.CoreWebView2 is null)
        {
            AddressTextBox.Text = string.Empty;
            BackButton.IsEnabled = false;
            ForwardButton.IsEnabled = false;
            ReloadButton.IsEnabled = false;
            GoButton.IsEnabled = false;
            return;
        }

        var focused = AddressTextBox.IsKeyboardFocusWithin;
        if (!focused)
        {
            AddressTextBox.Text = browser.Source?.ToString() ?? browser.CoreWebView2.Source ?? string.Empty;
        }

        BackButton.IsEnabled = browser.CoreWebView2.CanGoBack;
        ForwardButton.IsEnabled = browser.CoreWebView2.CanGoForward;
        ReloadButton.IsEnabled = true;
        GoButton.IsEnabled = true;
    }

    protected override void OnStateChanged(EventArgs e)
    {
        base.OnStateChanged(e);
        UpdateMaximizeButton();
    }

    private void UpdateMaximizeButton()
    {
        MaximizeButton.Content = WindowState == WindowState.Maximized ? "❐" : "□";
    }

    protected override void OnClosing(System.ComponentModel.CancelEventArgs e)
    {
        if (!_closeWasApproved)
        {
            e.Cancel = true;
            _ = ExecuteCommandAsync("close");
            return;
        }

        _server?.Dispose();
        foreach (var item in BrowserTabs.Items)
        {
            if (item is TabItem { Content: WebView2 browser })
            {
                browser.Dispose();
            }
        }
        base.OnClosing(e);
    }

    private async void Browser_WebMessageReceived(object? sender, CoreWebView2WebMessageReceivedEventArgs e)
    {
        var target = sender as CoreWebView2;
        var requestId = "";
        var command = "unknown";

        try
        {
            using var doc = JsonDocument.Parse(e.WebMessageAsJson);
            var root = doc.RootElement;
            if (!root.TryGetProperty("type", out var type) || type.GetString() != "custom-browser-command")
            {
                return;
            }

            requestId = root.TryGetProperty("requestId", out var req) ? req.GetString() ?? "" : "";
            command = root.TryGetProperty("command", out var cmd) ? cmd.GetString() ?? "" : "";
            string? password = null;
            if (root.TryGetProperty("password", out var pwd) && pwd.ValueKind == JsonValueKind.String)
            {
                password = pwd.GetString();
            }

            var result = await ExecuteCommandAsync(command, password, invokedByJavaScript: true);
            SendBridgeResponse(target, requestId, command, true, null, result);
        }
        catch (Exception ex)
        {
            SendBridgeResponse(target, requestId, command, false, ex.Message, GetState());
        }
    }

    private static void SendBridgeResponse(CoreWebView2? target, string requestId, string command, bool ok, string? error, object state)
    {
        if (target is null)
        {
            return;
        }

        var payload = JsonSerializer.Serialize(new
        {
            type = "custom-browser-response",
            requestId,
            command,
            ok,
            error,
            state
        });
        target.PostWebMessageAsJson(payload);
    }

    private async Task<object> ExecuteCommandAsync(string command, string? suppliedPassword = null, bool invokedByJavaScript = false, bool skipPassword = false)
    {
        var normalized = command.Trim().Replace("-", "", StringComparison.Ordinal).Replace("_", "", StringComparison.Ordinal).ToLowerInvariant();

        if (normalized is "getstate" or "state")
        {
            return GetState();
        }

        async Task CheckPasswordAsync()
        {
            if (!skipPassword && !await EnsurePasswordAsync(command, suppliedPassword))
            {
                throw new InvalidOperationException("Passwortprüfung fehlgeschlagen oder abgebrochen.");
            }
        }

        switch (normalized)
        {
            case "minimize":
                Require(_settings.EnableMinimize, "Minimize ist in ini.json deaktiviert.");
                await CheckPasswordAsync();
                WindowState = WindowState.Minimized;
                break;

            case "restore":
                Require(_settings.EnableMaximize, "Restore ist deaktiviert, weil enable_maximize=false ist.");
                await CheckPasswordAsync();
                if (_isKiosk)
                {
                    ExitKioskMode();
                }
                WindowState = WindowState.Normal;
                break;

            case "maximize" or "maximaze":
                Require(_settings.EnableMaximize, "Maximize ist in ini.json deaktiviert.");
                await CheckPasswordAsync();
                if (_isKiosk)
                {
                    ExitKioskMode();
                }
                WindowState = WindowState.Maximized;
                break;

            case "togglekiosk" or "kiosktoggle":
                Require(_settings.EnableKioskModeToggle, "Kiosk-Toggle ist in ini.json deaktiviert.");
                await CheckPasswordAsync();
                if (_isKiosk)
                {
                    ExitKioskMode();
                }
                else
                {
                    EnterKioskMode();
                }
                break;

            case "close":
                Require(_settings.EnableClose, "Close ist in ini.json deaktiviert.");
                await CheckPasswordAsync();
                _closeWasApproved = true;
                Close();
                break;

            default:
                throw new InvalidOperationException($"Unbekannter Browser-Befehl: {command}");
        }

        return GetState();
    }

    private Task<bool> EnsurePasswordAsync(string actionName, string? suppliedPassword)
    {
        if (string.IsNullOrEmpty(_settings.ControlPassword))
        {
            return Task.FromResult(true);
        }

        if (suppliedPassword is not null)
        {
            return Task.FromResult(suppliedPassword == _settings.ControlPassword);
        }

        return Task.FromResult(PasswordPrompt.Ask(this, actionName, _settings.ControlPassword));
    }

    private static void Require(bool condition, string message)
    {
        if (!condition)
        {
            throw new InvalidOperationException(message);
        }
    }

    private void EnterKioskMode()
    {
        _windowSnapshot = new WindowSnapshot(Left, Top, Width, Height, WindowState, Topmost, ResizeMode);
        _isKiosk = true;
        TitleBarRow.Height = new GridLength(0);
        AddressBarContainer.Visibility = Visibility.Collapsed;
        Topmost = true;
        ResizeMode = ResizeMode.NoResize;
        WindowState = WindowState.Maximized;
    }

    private void ExitKioskMode()
    {
        _isKiosk = false;
        TitleBarRow.Height = new GridLength(_currentTitleBarHeight);
        AddressBarContainer.Visibility = _settings.ShowAddressBar ? Visibility.Visible : Visibility.Collapsed;
        Topmost = _windowSnapshot?.Topmost ?? false;
        ResizeMode = _windowSnapshot?.ResizeMode ?? ResizeMode.CanResizeWithGrip;

        var snapshot = _windowSnapshot;
        if (snapshot is not null)
        {
            WindowState = WindowState.Normal;
            Left = snapshot.Left;
            Top = snapshot.Top;
            Width = snapshot.Width;
            Height = snapshot.Height;
            WindowState = snapshot.WindowState == WindowState.Minimized ? WindowState.Normal : snapshot.WindowState;
        }
    }

    private object GetState()
    {
        return new
        {
            isKiosk = _isKiosk,
            windowState = WindowState.ToString(),
            title = _settings.TitleName,
            canMinimize = _settings.EnableMinimize,
            canMaximize = _settings.EnableMaximize,
            canClose = _settings.EnableClose,
            canMoveWindow = _settings.EnableMoveWindow,
            canToggleKiosk = _settings.EnableKioskModeToggle,
            passwordRequired = !string.IsNullOrEmpty(_settings.ControlPassword),
            showAddressBar = _settings.ShowAddressBar,
            allowTabs = _settings.AllowTabs,
            tabCount = BrowserTabs.Items.Count,
            currentUrl = CurrentBrowser?.Source?.ToString()
        };
    }

    private static string GetBridgeScript()
    {
        return """
(() => {
  if (window.customBrowser && window.CustomBrowser) return;

  const pending = new Map();
  let nextId = 0;

  function call(command, password) {
    return new Promise((resolve, reject) => {
      const requestId = `custom-browser-${Date.now()}-${++nextId}`;
      pending.set(requestId, { resolve, reject });
      window.chrome.webview.postMessage({
        type: 'custom-browser-command',
        requestId,
        command,
        password
      });
    });
  }

  window.chrome.webview.addEventListener('message', event => {
    const data = event.data;
    if (!data || data.type !== 'custom-browser-response') return;
    const handler = pending.get(data.requestId);
    if (!handler) return;
    pending.delete(data.requestId);
    if (data.ok) handler.resolve(data);
    else handler.reject(data);
  });

  const api = Object.freeze({
    minimize: password => call('minimize', password),
    restore: password => call('restore', password),
    maximize: password => call('maximize', password),
    toggleKiosk: password => {
      if (window.__customBrowserToggleKioskLock) {
        return window.__customBrowserToggleKioskLock;
      }

      window.__customBrowserToggleKioskLock = call('toggleKiosk', password)
        .finally(() => {
          setTimeout(() => {
            window.__customBrowserToggleKioskLock = null;
          }, 500);
        });

      return window.__customBrowserToggleKioskLock;
    },
    close: password => call('close', password),
    getState: () => call('getState')
  });

  window.customBrowser = api;
  window.CustomBrowser = api;
})();
""";
    }

    private sealed record WindowSnapshot(double Left, double Top, double Width, double Height, WindowState WindowState, bool Topmost, ResizeMode ResizeMode);
}
