/* amalgame-net-proxy — runtime header (stub).
 *
 * This package is pure Amalgame: ReverseProxy / ProxyRoute are AM
 * classes in facade.am that build on amalgame-net-http's HttpClient
 * (upstream leg) and Http1 server (listener). There are no C runtime
 * symbols of our own — the upstream/runtime work all lives in
 * amalgame-net-http (and, transitively, amalgame-tls / amalgame-async).
 *
 * The [stdlib].header key in amalgame.toml points here so amc has a
 * header to include for the namespace; it intentionally declares
 * nothing.
 */
#ifndef AMALGAME_NET_PROXY_H
#define AMALGAME_NET_PROXY_H
#endif /* AMALGAME_NET_PROXY_H */
