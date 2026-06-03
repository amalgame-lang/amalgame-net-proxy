# amalgame-net-proxy

HTTP/1.1 **reverse proxy** for the [Mosaic](https://github.com/amalgame-lang/amalgame-web)
stack — the "front door" half of nginx/Apache. Terminate the client
connection, pick an upstream by longest matching path prefix, forward
the request, stream the response back.

Built entirely on [`amalgame-net-http`](https://github.com/amalgame-lang/amalgame-net-http):
the listener is `Http1.Serve`, the upstream leg is `HttpClient`.

## Install

```bash
amc package add net-proxy
```

## 10-line example

```amalgame
import Amalgame.Net.Proxy

ReverseProxy.New()
    .Forward("/api", "http://localhost:8081")   // path-prefix route
    .Forward("/",    "http://localhost:8080")    // catch-all
    .ServeMt(80)
```

That's it. Requests to `/api/*` go to `:8081`, everything else to
`:8080`. Order doesn't matter — the **longest matching prefix wins**,
so the catch-all `/` can sit anywhere.

## What it does

- **Longest-prefix routing.** `/api/v2` beats `/api` beats `/`. Prefix
  boundaries are respected: `/api` matches `/api` and `/api/x` but not
  `/apixyz`.
- **Header pass-through with hop-by-hop stripping** (RFC 7230 §6.1):
  `Connection` / `Keep-Alive` / `TE` / `Trailer` / `Transfer-Encoding`
  / `Upgrade` / `Proxy-*` are dropped, plus `Host` + `Content-Length`
  (the upstream client recomputes them).
- **`X-Forwarded-For` injection**, append-aware — extends an inbound
  chain rather than overwriting it (so a proxy-in-front is preserved).
  On by default; `WithXForwardedFor(false)` to disable.
- **Faithful response relay** — status + body + every non-hop-by-hop
  header (Content-Type, Set-Cookie, Location, cache headers, …).
- **HTTPS upstreams** work out of the box — a `https://…` backend URL
  negotiates TLS via amalgame-tls (SNI + ALPN http/1.1).

## API

```amalgame
let p = ReverseProxy.New()
    .Forward(prefix, backendBaseUrl)   // register an upstream (chainable)
    .WithXForwardedFor(true)           // inject XFF (default)
    .Timeout(30)                       // see "Limitations"

p.MatchBackend("/api/x")               // → "http://…" (longest prefix) or ""
p.Handle(req)                          // proxy one HttpRequest → HttpResponse
p.Serve(port)                          // single-thread accept loop
p.ServeMt(port)                        // thread-per-connection (production)
```

`Handle(req)` is the unit the listener calls — exposed so you can mount
the proxy inside a larger dispatch (e.g. proxy some paths, serve others
locally) or unit-test routing without a socket.

## End-to-end (manual)

Terminal 1 — a backend:

```amalgame
// backend.am
import Amalgame.Web
WebApp.New()
    .Get("/", ctx => HttpResponse.New().Text("hello from :8080"))
    .Serve(8080)
```

Terminal 2 — the proxy in front of it:

```amalgame
// front.am
import Amalgame.Net.Proxy
ReverseProxy.New().Forward("/", "http://localhost:8080").Serve(8000)
```

```bash
curl -s http://localhost:8000/        # → "hello from :8080"
curl -sI http://localhost:8000/       # upstream status + headers relayed
```

## Limitations (v0.1)

- **Load balancing** across N upstreams (round-robin / least-conn /
  ip-hash, health checks) is **v0.2** — `UpstreamPool`.
- **WebSocket** transparent forwarding needs connection hijack — not yet.
- **Bodies > 1 MiB**: `amalgame-net-http`'s `HttpClient` reads up to
  1 MiB per upstream response today, so very large proxied bodies are
  truncated. Fine for APIs/pages; lifts when the client streams.
- **`Timeout(sec)`** is stored but not yet enforced — net-http's client
  has no timeout knob in v0.11. Kept so the surface stays stable.
- **No path rewriting** — the full original target (path + query) is
  forwarded verbatim (nginx `proxy_pass` without a trailing slash).
  Prefix-strip is a v0.2 option.

## Roadmap

Part of the [beyond-HTTP](https://github.com/amalgame-lang/Amalgame/blob/main/docs/proposals/beyond-http.md)
nginx/Apache-parity track. Next: load balancing (`UpstreamPool`),
then TCP/UDP stream proxy (`amalgame-net-stream`).

## License

Apache-2.0.
