using System;
using System.Windows;
using CustomBrowser.Services;

namespace CustomBrowser;

public partial class App : Application
{
    protected override void OnStartup(StartupEventArgs e)
    {
        base.OnStartup(e);

        try
        {
            var loaded = SettingsLoader.Load(AppContext.BaseDirectory, e.Args);
            var window = new MainWindow(loaded.Settings, loaded.ConfigDirectory);
            MainWindow = window;
            window.Show();
        }
        catch (Exception ex)
        {
            MessageBox.Show(
                "Custom Browser konnte nicht gestartet werden:\n\n" + ex.Message,
                "Custom Browser",
                MessageBoxButton.OK,
                MessageBoxImage.Error);
            Shutdown(1);
        }
    }
}
