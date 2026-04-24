using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Media;

namespace CustomBrowser.Services;

public static class PasswordPrompt
{
    public static bool Ask(Window owner, string actionName, string expectedPassword)
    {
        var dialog = new Window
        {
            Title = "Passwort erforderlich",
            Width = 360,
            Height = 190,
            WindowStartupLocation = WindowStartupLocation.CenterOwner,
            ResizeMode = ResizeMode.NoResize,
            Owner = owner,
            ShowInTaskbar = false,
            WindowStyle = WindowStyle.ToolWindow,
            Background = Brushes.White
        };

        var root = new Grid { Margin = new Thickness(18) };
        root.RowDefinitions.Add(new RowDefinition { Height = GridLength.Auto });
        root.RowDefinitions.Add(new RowDefinition { Height = GridLength.Auto });
        root.RowDefinitions.Add(new RowDefinition { Height = new GridLength(1, GridUnitType.Star) });
        root.RowDefinitions.Add(new RowDefinition { Height = GridLength.Auto });

        var text = new TextBlock
        {
            Text = $"Bitte Passwort eingeben, um '{actionName}' auszuführen.",
            TextWrapping = TextWrapping.Wrap,
            Margin = new Thickness(0, 0, 0, 12)
        };
        Grid.SetRow(text, 0);
        root.Children.Add(text);

        var passwordBox = new PasswordBox
        {
            MinHeight = 28,
            Margin = new Thickness(0, 0, 0, 8)
        };
        Grid.SetRow(passwordBox, 1);
        root.Children.Add(passwordBox);

        var error = new TextBlock
        {
            Text = "",
            Foreground = Brushes.DarkRed,
            Margin = new Thickness(0, 0, 0, 8)
        };
        Grid.SetRow(error, 2);
        root.Children.Add(error);

        var buttons = new StackPanel
        {
            Orientation = Orientation.Horizontal,
            HorizontalAlignment = HorizontalAlignment.Right
        };

        var ok = new Button { Content = "OK", Width = 86, MinHeight = 28, Margin = new Thickness(0, 0, 8, 0), IsDefault = true };
        var cancel = new Button { Content = "Abbrechen", Width = 92, MinHeight = 28, IsCancel = true };
        buttons.Children.Add(ok);
        buttons.Children.Add(cancel);
        Grid.SetRow(buttons, 3);
        root.Children.Add(buttons);

        var accepted = false;
        void TryAccept()
        {
            if (passwordBox.Password == expectedPassword)
            {
                accepted = true;
                dialog.DialogResult = true;
                dialog.Close();
            }
            else
            {
                error.Text = "Passwort ist falsch.";
                passwordBox.SelectAll();
                passwordBox.Focus();
            }
        }

        ok.Click += (_, _) => TryAccept();
        passwordBox.KeyDown += (_, e) =>
        {
            if (e.Key == Key.Enter)
            {
                TryAccept();
            }
        };

        dialog.Content = root;
        dialog.Loaded += (_, _) => passwordBox.Focus();
        dialog.ShowDialog();
        return accepted;
    }
}
