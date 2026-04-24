using System.IO;
using System.Net;
using System.Text;

namespace CustomBrowser.Services;

public sealed class LocalWebServer : IDisposable
{
    private readonly string _rootDirectory;
    private readonly int _port;
    private HttpListener? _listener;
    private CancellationTokenSource? _cts;

    public LocalWebServer(string rootDirectory, int port)
    {
        _rootDirectory = Path.GetFullPath(rootDirectory).TrimEnd(Path.DirectorySeparatorChar, Path.AltDirectorySeparatorChar) + Path.DirectorySeparatorChar;
        _port = port;
    }

    public string BaseUrl => $"http://127.0.0.1:{_port}/";

    public void Start()
    {
        if (_port <= 0 || _listener is not null)
        {
            return;
        }

        _cts = new CancellationTokenSource();
        _listener = new HttpListener();
        _listener.Prefixes.Add(BaseUrl);
        _listener.Start();
        _ = Task.Run(() => ListenLoopAsync(_cts.Token));
    }

    private async Task ListenLoopAsync(CancellationToken cancellationToken)
    {
        while (!cancellationToken.IsCancellationRequested && _listener is not null)
        {
            HttpListenerContext? context = null;
            try
            {
                context = await _listener.GetContextAsync().ConfigureAwait(false);
                await HandleAsync(context).ConfigureAwait(false);
            }
            catch (ObjectDisposedException)
            {
                break;
            }
            catch (HttpListenerException)
            {
                break;
            }
            catch (Exception ex)
            {
                if (context is not null)
                {
                    await WriteTextAsync(context.Response, 500, "text/plain; charset=utf-8", ex.Message).ConfigureAwait(false);
                }
            }
        }
    }

    private async Task HandleAsync(HttpListenerContext context)
    {
        var request = context.Request;
        var response = context.Response;

        if (request.HttpMethod is not "GET" and not "HEAD")
        {
            await WriteTextAsync(response, 405, "text/plain; charset=utf-8", "Method Not Allowed").ConfigureAwait(false);
            return;
        }

        var rawPath = Uri.UnescapeDataString(request.Url?.AbsolutePath ?? "/");
        if (rawPath == "/")
        {
            rawPath = "/WebRoot/index.html";
        }

        var relativePath = rawPath.TrimStart('/').Replace('/', Path.DirectorySeparatorChar);
        var fullPath = Path.GetFullPath(Path.Combine(_rootDirectory, relativePath));

        if (!fullPath.StartsWith(_rootDirectory, StringComparison.OrdinalIgnoreCase))
        {
            await WriteTextAsync(response, 403, "text/plain; charset=utf-8", "Forbidden").ConfigureAwait(false);
            return;
        }

        if (Directory.Exists(fullPath))
        {
            fullPath = Path.Combine(fullPath, "index.html");
        }

        if (!File.Exists(fullPath))
        {
            await WriteTextAsync(response, 404, "text/plain; charset=utf-8", "Not Found").ConfigureAwait(false);
            return;
        }

        response.StatusCode = 200;
        response.ContentType = GetContentType(fullPath);
        response.Headers["Cache-Control"] = "no-store";

        if (request.HttpMethod == "HEAD")
        {
            response.Close();
            return;
        }

        await using var file = File.OpenRead(fullPath);
        response.ContentLength64 = file.Length;
        await file.CopyToAsync(response.OutputStream).ConfigureAwait(false);
        response.OutputStream.Close();
    }

    private static async Task WriteTextAsync(HttpListenerResponse response, int statusCode, string contentType, string text)
    {
        var buffer = Encoding.UTF8.GetBytes(text);
        response.StatusCode = statusCode;
        response.ContentType = contentType;
        response.ContentLength64 = buffer.Length;
        await response.OutputStream.WriteAsync(buffer).ConfigureAwait(false);
        response.OutputStream.Close();
    }

    private static string GetContentType(string path)
    {
        return Path.GetExtension(path).ToLowerInvariant() switch
        {
            ".html" or ".htm" => "text/html; charset=utf-8",
            ".css" => "text/css; charset=utf-8",
            ".js" => "application/javascript; charset=utf-8",
            ".json" => "application/json; charset=utf-8",
            ".svg" => "image/svg+xml",
            ".ico" => "image/x-icon",
            ".png" => "image/png",
            ".jpg" or ".jpeg" => "image/jpeg",
            ".gif" => "image/gif",
            ".webp" => "image/webp",
            ".woff" => "font/woff",
            ".woff2" => "font/woff2",
            ".txt" => "text/plain; charset=utf-8",
            _ => "application/octet-stream"
        };
    }

    public void Dispose()
    {
        try
        {
            _cts?.Cancel();
            _listener?.Stop();
            _listener?.Close();
        }
        catch
        {
            // Server shutdown should not block application close.
        }
        finally
        {
            _listener = null;
            _cts?.Dispose();
            _cts = null;
        }
    }
}
