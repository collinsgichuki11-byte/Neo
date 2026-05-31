#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

void serve_sema(int client, char *site, char *page) {
    char path[256];
    snprintf(path, sizeof(path), "%s/pages/%s.sema", site, page);

    FILE *f = fopen(path, "r");
    char body[8192] = "";

    if (!f) {
        snprintf(body, sizeof(body), "<html><body style='background:#0a0a0f;color:#e8e4d9;font-family:sans-serif;text-align:center;padding:50px'><h1 style='color:#e63946'>Page not found</h1><p>This page does not exist on NEO</p><a href='/Africa/home' style='color:#e63946'>Go to Africa.one</a></body></html>");
    } else {
        strcat(body, "<html><head>");
        strcat(body, "<meta name='viewport' content='width=device-width, initial-scale=1'>");
        strcat(body, "<style>");
        strcat(body, "body{font-family:sans-serif;max-width:600px;margin:auto;padding:20px;background:#0a0a0f;color:#e8e4d9;}");
        strcat(body, "h1{color:#e63946;}");
        strcat(body, "h2{color:#e63946;font-size:16px;}");
        strcat(body, "button{background:#e63946;color:white;border:none;padding:10px 20px;margin:5px;cursor:pointer;font-size:16px;border-radius:4px;}");
        strcat(body, "button:hover{background:#c62836;}");
        strcat(body, "p{color:#a8b2c1;}");
        strcat(body, "input{background:#1a2332;color:#e8e4d9;border:1px solid #e63946;padding:8px;margin:5px;width:90%;border-radius:4px;display:block;}");
        strcat(body, ".header{text-align:center;color:#e63946;font-size:11px;letter-spacing:3px;margin-bottom:20px;border-bottom:1px solid #1e2a3a;padding-bottom:10px;}");
        strcat(body, ".footer{text-align:center;color:#2d3748;font-size:10px;letter-spacing:2px;margin-top:40px;border-top:1px solid #1e2a3a;padding-top:10px;}");
        strcat(body, "form{margin:10px 0;}");
        strcat(body, "</style></head><body>");
        strcat(body, "<div class='header'>BUILT ON NEO · POWERED BY SEMA</div>");

        int has_inputs = 0;
        char nav_dest[128] = "";
        char nav_dest_site[128] = "";

        FILE *scan = fopen(path, "r");
        char scanline[256];
        while (fgets(scanline, sizeof(scanline), scan)) {
            scanline[strcspn(scanline, "\n")] = 0;
            if (strncmp(scanline, "input:", 6) == 0) has_inputs = 1;
            if (strncmp(scanline, "nav:", 4) == 0 && strstr(scanline, ":check:")) {
                char *to_pos = strstr(scanline + 4, ":to:");
                char *in_pos = to_pos ? strstr(to_pos + 4, ":in:") : NULL;
                if (to_pos && in_pos) {
                    int dlen = in_pos - (to_pos + 4);
                    strncpy(nav_dest, to_pos + 4, dlen);
                    nav_dest[dlen] = 0;
                    strcpy(nav_dest_site, in_pos + 4);
                    char *check = strstr(nav_dest_site, ":check:");
                    if (check) check[0] = 0;
                }
            }
        }
        fclose(scan);

        if (has_inputs) {
            strcat(body, "<form method='POST' action='/login'>");
        }

        char line[256];
        while (fgets(line, sizeof(line), f)) {
            line[strcspn(line, "\n")] = 0;
            if (strncmp(line, "title:", 6) == 0) {
                char tmp[256];
                snprintf(tmp, sizeof(tmp), "<h1>%s</h1>", line + 6);
                strcat(body, tmp);
            } else if (strncmp(line, "paragraph:", 10) == 0) {
                char tmp[256];
                snprintf(tmp, sizeof(tmp), "<p>%s</p>", line + 10);
                strcat(body, tmp);
            } else if (strncmp(line, "input:", 6) == 0) {
                char tmp[256];
                snprintf(tmp, sizeof(tmp),
                    "<input type='%s' name='%s' placeholder='%s'>",
                    strcmp(line+6, "password") == 0 ? "password" : "text",
                    line + 6, line + 6);
                strcat(body, tmp);
            } else if (strncmp(line, "nav:", 4) == 0) {
                char nav[256];
                strcpy(nav, line + 4);
                char btn[128] = "";
                char dest[128] = "";
                char dest_site[128] = "";
                char *to_pos = strstr(nav, ":to:");
                if (to_pos) {
                    int blen = to_pos - nav;
                    strncpy(btn, nav, blen);
                    btn[blen] = 0;
                    char *in_pos = strstr(to_pos + 4, ":in:");
                    if (in_pos) {
                        int dlen = in_pos - (to_pos + 4);
                        strncpy(dest, to_pos + 4, dlen);
                        dest[dlen] = 0;
                        strcpy(dest_site, in_pos + 4);
                        char *check = strstr(dest_site, ":check:");
                        if (check) check[0] = 0;
                    } else {
                        strcpy(dest, to_pos + 4);
                        strcpy(dest_site, site);
                    }
                    if (strstr(nav, ":check:")) {
                        char tmp[256];
                        snprintf(tmp, sizeof(tmp), "<button type='submit'>%s</button>", btn);
                        strcat(body, tmp);
                    } else {
                        char tmp[256];
                        snprintf(tmp, sizeof(tmp), "<a href='/%s/%s' style='text-decoration:none'><button type='button'>%s</button></a>", dest_site, dest, btn);
                        strcat(body, tmp);
                    }
                }
            }
        }

        if (has_inputs) {
            char hidden[256];
            snprintf(hidden, sizeof(hidden),
                "<input type='hidden' name='dest' value='%s'>"
                "<input type='hidden' name='dest_site' value='%s'>",
                nav_dest, nav_dest_site);
            strcat(body, hidden);
            strcat(body, "</form>");
        }

        strcat(body, "<div class='footer'>Africa.one · NEO INTERNET</div>");
        strcat(body, "</body></html>");
        fclose(f);
    }

    char response[16384];
    snprintf(response, sizeof(response),
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n\r\n"
        "%s", body);

    write(client, response, strlen(response));
}

void handle_login(int client, char *body_data) {
    char username[64] = "";
    char password[64] = "";
    char dest[128] = "dashboard";
    char dest_site[128] = "Africa";

    char *u = strstr(body_data, "username=");
    char *p = strstr(body_data, "password=");
    char *d = strstr(body_data, "dest=");
    char *ds = strstr(body_data, "dest_site=");

    if (u) sscanf(u + 9, "%63[^&]", username);
    if (p) sscanf(p + 9, "%63[^&]", password);
    if (d) sscanf(d + 5, "%127[^&]", dest);
    if (ds) sscanf(ds + 10, "%127[^& ]", dest_site);

    int verified = 0;
    FILE *f = fopen("users.neo", "r");
    if (f) {
        char line[128];
        while (fgets(line, sizeof(line), f)) {
            line[strcspn(line, "\n")] = 0;
            char *colon = strchr(line, ':');
            if (colon) {
                *colon = 0;
                if (strcmp(line, username) == 0 && strcmp(colon + 1, password) == 0) {
                    verified = 1;
                    break;
                }
            }
        }
        fclose(f);
    }

    char response[1024];
    if (verified) {
        snprintf(response, sizeof(response),
            "HTTP/1.1 302 Found\r\n"
            "Location: /%s/%s\r\n\r\n",
            dest_site, dest);
    } else {
        snprintf(response, sizeof(response),
            "HTTP/1.1 302 Found\r\n"
            "Location: /Africa/login\r\n\r\n");
    }
    write(client, response, strlen(response));
}

void lookup_domain(char *domain, char *site, char *page) {
    strcpy(site, "Africa");
    strcpy(page, "home");
    FILE *f = fopen("domains.neo", "r");
    if (!f) return;
    char line[256];
    while (fgets(line, sizeof(line), f)) {
        line[strcspn(line, "\n")] = 0;
        char *eq = strchr(line, '=');
        if (!eq) continue;
        *eq = 0;
        if (strcmp(line, domain) == 0) {
            char *slash = strchr(eq + 1, '/');
            if (slash) {
                int slen = slash - (eq + 1);
                strncpy(site, eq + 1, slen);
                site[slen] = 0;
                strcpy(page, slash + 1);
            }
            break;
        }
    }
    fclose(f);
}

int main() {
    int server = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(1234);

    bind(server, (struct sockaddr*)&addr, sizeof(addr));
    listen(server, 10);

    printf("1 NEO server live\n");
    printf("1 Local:  192.168.8.17:1234/Africa/home\n");
    printf("1 Domain: Africa.one\n");

    while (1) {
        int client = accept(server, NULL, NULL);
        char request[4096] = "";
        read(client, request, sizeof(request));

        char site[64] = "Africa";
        char page[64] = "home";
        char host[128] = "";
        char method[8] = "";

        sscanf(request, "%7s", method);

        char *host_pos = strstr(request, "Host: ");
        if (host_pos) {
            sscanf(host_pos + 6, "%127s", host);
            char *port = strchr(host, ':');
            if (port) port[0] = 0;
            if (strstr(host, ".one")) {
                lookup_domain(host, site, page);
            }
        }

        if (strcmp(method, "POST") == 0) {
            char *body = strstr(request, "\r\n\r\n");
            if (body) handle_login(client, body + 4);
        } else {
            char path[128] = "";
            sscanf(request, "GET /%127s", path);
            if (strlen(path) > 0 && strcmp(path, "HTTP/1.1") != 0) {
                char *slash = strchr(path, '/');
                if (slash) {
                    int slen = slash - path;
                    strncpy(site, path, slen);
                    site[slen] = 0;
                    strcpy(page, slash + 1);
                    char *space = strchr(page, ' ');
                    if (space) space[0] = 0;
                }
            }
            serve_sema(client, site, page);
        }

        close(client);
    }

    return 0;
}
