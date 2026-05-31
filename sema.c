#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

char username[64] = "";
char memory_file[] = "users.neo";
char dictionary_file[] = "dictionary.neo";
char domains_file[] = "domains.neo";
char identities_dir[] = "identities";

void save_user(char *name, char *pass) {
    FILE *f = fopen(memory_file, "a");
    if (f) {
        fprintf(f, "%s:%s\n", name, pass);
        fclose(f);
    }
    char path[256];
    snprintf(path, sizeof(path), "mkdir -p users/%s/sites", name);
    system(path);
}

int verify_user(char *name, char *pass) {
    FILE *f = fopen(memory_file, "r");
    if (!f) return 0;
    char line[128];
    while (fgets(line, sizeof(line), f)) {
        line[strcspn(line, "\n")] = 0;
        char *colon = strchr(line, ':');
        if (colon) {
            *colon = 0;
            if (strcmp(line, name) == 0 && strcmp(colon + 1, pass) == 0) {
                fclose(f);
                return 1;
            }
        }
    }
    fclose(f);
    return 0;
}

int user_exists(char *name) {
    FILE *f = fopen(memory_file, "r");
    if (!f) return 0;
    char line[128];
    while (fgets(line, sizeof(line), f)) {
        line[strcspn(line, "\n")] = 0;
        char *colon = strchr(line, ':');
        if (colon) {
            *colon = 0;
            if (strcmp(line, name) == 0) {
                fclose(f);
                return 1;
            }
        }
    }
    fclose(f);
    return 0;
}

void trim(char *str) {
    int len = strlen(str);
    while (len > 0 && str[len-1] == ' ') {
        str[len-1] = 0;
        len--;
    }
    while (*str == ' ') {
        memmove(str, str+1, strlen(str));
    }
}

void display_sema_file(char *path);
void handle_navigation(char *page_path, char *site);

void display_sema_file(char *path) {
    FILE *pf = fopen(path, "r");
    if (!pf) {
        printf("1 Page not found\n");
        return;
    }
    printf("\n================================\n");
    char pline[256];
    while (fgets(pline, sizeof(pline), pf)) {
        pline[strcspn(pline, "\n")] = 0;
        if (strncmp(pline, "title:", 6) == 0)
            printf("     %s\n", pline + 6);
        else if (strncmp(pline, "button:", 7) == 0)
            printf("[ %s ]\n", pline + 7);
        else if (strncmp(pline, "paragraph:", 10) == 0)
            printf("%s\n", pline + 10);
        else if (strncmp(pline, "input:", 6) == 0)
            printf("%s: ?\n", pline + 6);
        else if (strncmp(pline, "nav:", 4) == 0) {
            char nav_info[256];
            strcpy(nav_info, pline + 4);
            char btn[128] = "";
            char dest[128] = "";
            char *to_pos = strstr(nav_info, ":to:");
            if (to_pos) {
                int blen = to_pos - nav_info;
                strncpy(btn, nav_info, blen);
                btn[blen] = 0;
                strcpy(dest, to_pos + 4);
                char *in_pos = strstr(dest, ":in:");
                if (in_pos) in_pos[0] = 0;
                trim(btn); trim(dest);
                printf("[ %s ] -> %s\n", btn, dest);
            }
        }
    }
    printf("================================\n\n");
    fclose(pf);
}

void handle_navigation(char *page_path, char *site) {
    FILE *pf = fopen(page_path, "r");
    if (!pf) return;

    char inputs[8][64];
    char input_values[8][64];
    int input_count = 0;

    char pline[256];
    while (fgets(pline, sizeof(pline), pf)) {
        pline[strcspn(pline, "\n")] = 0;
        if (strncmp(pline, "input:", 6) == 0) {
            strcpy(inputs[input_count], pline + 6);
            trim(inputs[input_count]);
            printf("1 Enter %s:\n", inputs[input_count]);
            fgets(input_values[input_count], 64, stdin);
            input_values[input_count][strcspn(input_values[input_count], "\n")] = 0;
            input_count++;
        }
    }
    fclose(pf);

    pf = fopen(page_path, "r");
    if (!pf) return;
    while (fgets(pline, sizeof(pline), pf)) {
        pline[strcspn(pline, "\n")] = 0;
        if (strncmp(pline, "nav:", 4) == 0) {
            char nav_info[256];
            strcpy(nav_info, pline + 4);
            char btn[128] = "";
            char dest_page[128] = "";
            char dest_site[128] = "";
            char *to_pos = strstr(nav_info, ":to:");
            if (to_pos) {
                int blen = to_pos - nav_info;
                strncpy(btn, nav_info, blen);
                btn[blen] = 0;
                char *in_pos = strstr(to_pos + 4, ":in:");
                if (in_pos) {
                    int dlen = in_pos - (to_pos + 4);
                    strncpy(dest_page, to_pos + 4, dlen);
                    dest_page[dlen] = 0;
                    strcpy(dest_site, in_pos + 4);
                    char *check_trim = strstr(dest_site, ":check:");
                    if (check_trim) check_trim[0] = 0;
                } else {
                    strcpy(dest_page, to_pos + 4);
                    strcpy(dest_site, site);
                }
                trim(btn); trim(dest_page); trim(dest_site);

                char *check_pos = strstr(nav_info, ":check:");
                if (check_pos) {
                    char check_type[64] = "";
                    strcpy(check_type, check_pos + 7);
                    trim(check_type);
                    if (strcmp(check_type, "user") == 0 && input_count >= 2) {
                        if (verify_user(input_values[0], input_values[1])) {
                            strcpy(username, input_values[0]);
                            printf("1 Welcome back %s\n", input_values[0]);
                            char new_path[256];
                            snprintf(new_path, sizeof(new_path), "%s/pages/%s.sema", dest_site, dest_page);
                            display_sema_file(new_path);
                            handle_navigation(new_path, dest_site);
                        } else {
                            printf("1 Uongo. Wrong username or password.\n");
                        }
                        fclose(pf);
                        return;
                    }
                }

                printf("1 Click %s to go to %s? y/n\n", btn, dest_page);
                char choice[8];
                fgets(choice, sizeof(choice), stdin);
                choice[strcspn(choice, "\n")] = 0;
                if (strcmp(choice, "y") == 0) {
                    char new_path[256];
                    snprintf(new_path, sizeof(new_path), "%s/pages/%s.sema", dest_site, dest_page);
                    display_sema_file(new_path);
                    handle_navigation(new_path, dest_site);
                }
            }
        }
    }
    fclose(pf);
}

void run_primitive(char *action) {
    if (strncmp(action, "create:folder:", 14) == 0) {
        char *fname = action + 14;
        trim(fname);
        char path[256];
        snprintf(path, sizeof(path), "mkdir -p %s/pages %s/users %s/data", fname, fname, fname);
        system(path);
        printf("1 %s is ready\n", fname);
    } else if (strncmp(action, "create:page:", 12) == 0) {
        char page[128] = "";
        char site[128] = "";
        char *rest = action + 12;
        char *in_pos = strstr(rest, ":in:");
        if (in_pos) {
            int plen = in_pos - rest;
            strncpy(page, rest, plen);
            page[plen] = 0;
            strcpy(site, in_pos + 4);
            trim(page); trim(site);
            char path[256];
            snprintf(path, sizeof(path), "%s/pages/%s.sema", site, page);
            FILE *pf = fopen(path, "w");
            if (pf) { fprintf(pf, "title:%s\n", page); fclose(pf); }
            printf("1 Page %s added to %s\n", page, site);
        }
    } else if (strncmp(action, "create:identity:", 16) == 0) {
        char *name = action + 16;
        trim(name);
        char path[256];
        snprintf(path, sizeof(path), "mkdir -p %s", identities_dir);
        system(path);
        snprintf(path, sizeof(path), "%s/%s.identity", identities_dir, name);
        FILE *f = fopen(path, "w");
        if (f) {
            time_t t = time(NULL);
            struct tm *tm = localtime(&t);
            char id[32];
            snprintf(id, sizeof(id), "NEO%04d%02d%02d%02d%02d%02d",
                tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday,
                tm->tm_hour, tm->tm_min, tm->tm_sec);
            fprintf(f, "name:%s\n", name);
            fprintf(f, "location:unknown\n");
            fprintf(f, "status:unverified\n");
            fprintf(f, "id:%s\n", id);
            fprintf(f, "created:%04d\n", tm->tm_year+1900);
            fclose(f);
        }
        printf("1 Identity created for %s\n", name);
    } else if (strncmp(action, "set:name:", 9) == 0) {
        char name[128] = "";
        char person[128] = "";
        char *rest = action + 9;
        char *for_pos = strstr(rest, ":for:");
        if (for_pos) {
            int nlen = for_pos - rest;
            strncpy(name, rest, nlen);
            name[nlen] = 0;
            strcpy(person, for_pos + 5);
            trim(name); trim(person);
            char path[256];
            snprintf(path, sizeof(path), "%s/%s.identity", identities_dir, person);
            FILE *f = fopen(path, "r");
            char content[1024] = "";
            if (f) {
                char line[256];
                while (fgets(line, sizeof(line), f)) {
                    if (strncmp(line, "name:", 5) != 0)
                        strcat(content, line);
                }
                fclose(f);
            }
            f = fopen(path, "w");
            if (f) {
                fprintf(f, "name:%s\n", name);
                fputs(content, f);
                fclose(f);
            }
            printf("1 Name set to %s for %s\n", name, person);
        }
    } else if (strncmp(action, "set:location:", 13) == 0) {
        char location[128] = "";
        char person[128] = "";
        char *rest = action + 13;
        char *for_pos = strstr(rest, ":for:");
        if (for_pos) {
            int llen = for_pos - rest;
            strncpy(location, rest, llen);
            location[llen] = 0;
            strcpy(person, for_pos + 5);
            trim(location); trim(person);
            char path[256];
            snprintf(path, sizeof(path), "%s/%s.identity", identities_dir, person);
            FILE *f = fopen(path, "r");
            char content[1024] = "";
            if (f) {
                char line[256];
                while (fgets(line, sizeof(line), f)) {
                    if (strncmp(line, "location:", 9) != 0)
                        strcat(content, line);
                }
                fclose(f);
            }
            f = fopen(path, "w");
            if (f) {
                fputs(content, f);
                fprintf(f, "location:%s\n", location);
                fclose(f);
            }
            printf("1 Location set to %s for %s\n", location, person);
        }
    } else if (strncmp(action, "set:status:", 11) == 0) {
        char status[128] = "";
        char person[128] = "";
        char *rest = action + 11;
        char *for_pos = strstr(rest, ":for:");
        if (for_pos) {
            int slen = for_pos - rest;
            strncpy(status, rest, slen);
            status[slen] = 0;
            strcpy(person, for_pos + 5);
            trim(status); trim(person);
            char path[256];
            snprintf(path, sizeof(path), "%s/%s.identity", identities_dir, person);
            FILE *f = fopen(path, "r");
            char content[1024] = "";
            if (f) {
                char line[256];
                while (fgets(line, sizeof(line), f)) {
                    if (strncmp(line, "status:", 7) != 0)
                        strcat(content, line);
                }
                fclose(f);
            }
            f = fopen(path, "w");
            if (f) {
                fputs(content, f);
                fprintf(f, "status:%s\n", status);
                fclose(f);
            }
            printf("1 Status set to %s for %s\n", status, person);
        }
    } else if (strncmp(action, "verify:identity:", 16) == 0) {
        char *name = action + 16;
        trim(name);
        char path[256];
        snprintf(path, sizeof(path), "%s/%s.identity", identities_dir, name);
        FILE *f = fopen(path, "r");
        if (!f) {
            printf("1 Identity not found for %s\n", name);
        } else {
            printf("\n================================\n");
            char line[256];
            while (fgets(line, sizeof(line), f)) {
                line[strcspn(line, "\n")] = 0;
                if (strncmp(line, "name:", 5) == 0)
                    printf("  Name:     %s\n", line + 5);
                else if (strncmp(line, "location:", 9) == 0)
                    printf("  Location: %s\n", line + 9);
                else if (strncmp(line, "status:", 7) == 0)
                    printf("  Status:   %s\n", line + 7);
                else if (strncmp(line, "id:", 3) == 0)
                    printf("  NEO ID:   %s\n", line + 3);
                else if (strncmp(line, "created:", 8) == 0)
                    printf("  Created:  %s\n", line + 8);
            }
            printf("================================\n\n");
            fclose(f);
        }
    } else if (strncmp(action, "append:title:", 13) == 0) {
        char title[128] = "";
        char page[128] = "";
        char site[128] = "";
        char *rest = action + 13;
        char *to_pos = strstr(rest, ":to:");
        if (to_pos) {
            int tlen = to_pos - rest;
            strncpy(title, rest, tlen);
            title[tlen] = 0;
            char *in_pos = strstr(to_pos + 4, ":in:");
            if (in_pos) {
                int plen = in_pos - (to_pos + 4);
                strncpy(page, to_pos + 4, plen);
                page[plen] = 0;
                strcpy(site, in_pos + 4);
                trim(title); trim(page); trim(site);
                char path[256];
                snprintf(path, sizeof(path), "%s/pages/%s.sema", site, page);
                FILE *pf = fopen(path, "a");
                if (pf) { fprintf(pf, "title:%s\n", title); fclose(pf); }
                printf("1 Title added to %s in %s\n", page, site);
            }
        }
    } else if (strncmp(action, "append:button:", 14) == 0) {
        char button[128] = "";
        char page[128] = "";
        char site[128] = "";
        char *rest = action + 14;
        char *to_pos = strstr(rest, ":to:");
        if (to_pos) {
            int tlen = to_pos - rest;
            strncpy(button, rest, tlen);
            button[tlen] = 0;
            char *in_pos = strstr(to_pos + 4, ":in:");
            if (in_pos) {
                int plen = in_pos - (to_pos + 4);
                strncpy(page, to_pos + 4, plen);
                page[plen] = 0;
                strcpy(site, in_pos + 4);
                trim(button); trim(page); trim(site);
                char path[256];
                snprintf(path, sizeof(path), "%s/pages/%s.sema", site, page);
                FILE *pf = fopen(path, "a");
                if (pf) { fprintf(pf, "button:%s\n", button); fclose(pf); }
                printf("1 Button added to %s in %s\n", page, site);
            }
        }
    } else if (strncmp(action, "append:paragraph:", 17) == 0) {
        char para[128] = "";
        char page[128] = "";
        char site[128] = "";
        char *rest = action + 17;
        char *to_pos = strstr(rest, ":to:");
        if (to_pos) {
            int tlen = to_pos - rest;
            strncpy(para, rest, tlen);
            para[tlen] = 0;
            char *in_pos = strstr(to_pos + 4, ":in:");
            if (in_pos) {
                int plen = in_pos - (to_pos + 4);
                strncpy(page, to_pos + 4, plen);
                page[plen] = 0;
                strcpy(site, in_pos + 4);
                trim(para); trim(page); trim(site);
                char path[256];
                snprintf(path, sizeof(path), "%s/pages/%s.sema", site, page);
                FILE *pf = fopen(path, "a");
                if (pf) { fprintf(pf, "paragraph:%s\n", para); fclose(pf); }
                printf("1 Paragraph added to %s in %s\n", page, site);
            }
        }
    } else if (strncmp(action, "append:input:", 13) == 0) {
        char input[128] = "";
        char page[128] = "";
        char site[128] = "";
        char *rest = action + 13;
        char *to_pos = strstr(rest, ":to:");
        if (to_pos) {
            int tlen = to_pos - rest;
            strncpy(input, rest, tlen);
            input[tlen] = 0;
            char *in_pos = strstr(to_pos + 4, ":in:");
            if (in_pos) {
                int plen = in_pos - (to_pos + 4);
                strncpy(page, to_pos + 4, plen);
                page[plen] = 0;
                strcpy(site, in_pos + 4);
                trim(input); trim(page); trim(site);
                char path[256];
                snprintf(path, sizeof(path), "%s/pages/%s.sema", site, page);
                FILE *pf = fopen(path, "a");
                if (pf) { fprintf(pf, "input:%s\n", input); fclose(pf); }
                printf("1 Input %s added to %s in %s\n", input, page, site);
            }
        }
    } else if (strncmp(action, "navigate:button:", 16) == 0) {
        char button[128] = "";
        char page[128] = "";
        char site[128] = "";
        char dest[128] = "";
        char *rest = action + 16;
        char *in_pos = strstr(rest, ":in:");
        if (in_pos) {
            int blen = in_pos - rest;
            strncpy(button, rest, blen);
            button[blen] = 0;
            char *on_pos = strstr(in_pos + 4, ":on:");
            if (on_pos) {
                int plen = on_pos - (in_pos + 4);
                strncpy(page, in_pos + 4, plen);
                page[plen] = 0;
                char *to_pos = strstr(on_pos + 4, ":to:");
                if (to_pos) {
                    int slen = to_pos - (on_pos + 4);
                    strncpy(site, on_pos + 4, slen);
                    site[slen] = 0;
                    strcpy(dest, to_pos + 4);
                    trim(button); trim(page); trim(site); trim(dest);
                    char path[256];
                    snprintf(path, sizeof(path), "%s/pages/%s.sema", site, page);
                    FILE *pf = fopen(path, "a");
                    if (pf) {
                        fprintf(pf, "nav:%s:to:%s:in:%s\n", button, dest, site);
                        fclose(pf);
                    }
                    printf("1 %s in %s now goes to %s\n", button, page, dest);
                }
            }
        }
    } else if (strncmp(action, "navigate:check:", 15) == 0) {
        char button[128] = "";
        char page[128] = "";
        char site[128] = "";
        char dest[128] = "";
        char *rest = action + 15;
        char *in_pos = strstr(rest, ":in:");
        if (in_pos) {
            int blen = in_pos - rest;
            strncpy(button, rest, blen);
            button[blen] = 0;
            char *on_pos = strstr(in_pos + 4, ":on:");
            if (on_pos) {
                int plen = on_pos - (in_pos + 4);
                strncpy(page, in_pos + 4, plen);
                page[plen] = 0;
                char *to_pos = strstr(on_pos + 4, ":to:");
                if (to_pos) {
                    int slen = to_pos - (on_pos + 4);
                    strncpy(site, on_pos + 4, slen);
                    site[slen] = 0;
                    strcpy(dest, to_pos + 4);
                    trim(button); trim(page); trim(site); trim(dest);
                    char path[256];
                    snprintf(path, sizeof(path), "%s/pages/%s.sema", site, page);
                    FILE *pf = fopen(path, "a");
                    if (pf) {
                        fprintf(pf, "nav:%s:to:%s:in:%s:check:user\n", button, dest, site);
                        fclose(pf);
                    }
                    printf("1 %s in %s will verify user then go to %s\n", button, page, dest);
                }
            }
        }
    } else if (strncmp(action, "register:domain:", 16) == 0) {
        char domain[128] = "";
        char site[128] = "";
        char *rest = action + 16;
        char *as_pos = strstr(rest, ":as:");
        if (as_pos) {
            int dlen = as_pos - rest;
            strncpy(domain, rest, dlen);
            domain[dlen] = 0;
            strcpy(site, as_pos + 4);
            trim(domain); trim(site);
            FILE *f = fopen(domains_file, "a");
            if (f) {
                fprintf(f, "%s.one=%s/home\n", domain, site);
                fclose(f);
            }
            printf("1 %s.one now points to %s\n", domain, site);
        }
    } else if (strncmp(action, "load:page:", 10) == 0) {
        char page[128] = "";
        char site[128] = "";
        char *rest = action + 10;
        char *in_pos = strstr(rest, ":in:");
        if (in_pos) {
            int plen = in_pos - rest;
            strncpy(page, rest, plen);
            page[plen] = 0;
            strcpy(site, in_pos + 4);
            trim(page); trim(site);
            char path[256];
            snprintf(path, sizeof(path), "%s/pages/%s.sema", site, page);
            display_sema_file(path);
            handle_navigation(path, site);
        }
    } else if (strncmp(action, "load:pages:", 11) == 0) {
        char *fname = action + 11;
        trim(fname);
        char path[256];
        snprintf(path, sizeof(path), "%s/pages/index.sema", fname);
        display_sema_file(path);
        handle_navigation(path, fname);
    } else if (strncmp(action, "show:", 5) == 0) {
        printf("1 %s\n", action + 5);
    } else {
        printf("1 Done\n");
    }
}

int match_pattern(char *pattern, char *command, char *x_val, char *y_val, char *z_val, char *w_val) {
    x_val[0] = 0; y_val[0] = 0; z_val[0] = 0; w_val[0] = 0;

    char *vars[4];
    char *vals[4];
    int var_count = 0;

    char *xp = strstr(pattern, "X");
    char *yp = strstr(pattern, "Y");
    char *zp = strstr(pattern, "Z");
    char *wp = strstr(pattern, "W");

    if (xp) { vars[var_count] = xp; vals[var_count] = x_val; var_count++; }
    if (yp) { vars[var_count] = yp; vals[var_count] = y_val; var_count++; }
    if (zp) { vars[var_count] = zp; vals[var_count] = z_val; var_count++; }
    if (wp) { vars[var_count] = wp; vals[var_count] = w_val; var_count++; }

    if (var_count == 0)
        return strcmp(pattern, command) == 0;

    int first_prefix = vars[0] - pattern;
    if (strncmp(pattern, command, first_prefix) != 0) return 0;

    char *cmd_pos = command + first_prefix;

    for (int i = 0; i < var_count - 1; i++) {
        char between[64] = "";
        int blen = vars[i+1] - vars[i] - 1;
        strncpy(between, vars[i] + 1, blen);
        between[blen] = 0;

        char *found = strstr(cmd_pos, between);
        if (!found) return 0;

        int vlen = found - cmd_pos;
        strncpy(vals[i], cmd_pos, vlen);
        vals[i][vlen] = 0;
        trim(vals[i]);

        cmd_pos = found + blen;
    }

    char *pattern_end = vars[var_count-1] + 1;
    if (strlen(pattern_end) > 0) {
        char *suffix = strstr(cmd_pos, pattern_end);
        if (!suffix) return 0;
        int vlen = suffix - cmd_pos;
        strncpy(vals[var_count-1], cmd_pos, vlen);
        vals[var_count-1][vlen] = 0;
    } else {
        strcpy(vals[var_count-1], cmd_pos);
    }
    trim(vals[var_count-1]);
    return 1;
}

void fill_action(char *action, char *result, char *x_val, char *y_val, char *z_val, char *w_val) {
    char temp[256] = "";
    char *p = action;
    while (*p) {
        if (*p == 'X') { strcat(temp, x_val); p++; }
        else if (*p == 'Y') { strcat(temp, y_val); p++; }
        else if (*p == 'Z') { strcat(temp, z_val); p++; }
        else if (*p == 'W') { strcat(temp, w_val); p++; }
        else {
            int len = strlen(temp);
            temp[len] = *p;
            temp[len+1] = 0;
            p++;
        }
    }
    strcpy(result, temp);
}

int check_dictionary(char *command) {
    FILE *f = fopen(dictionary_file, "r");
    if (!f) return 0;
    char line[256];
    char stored_command[256];
    char action[256];
    while (fgets(line, sizeof(line), f)) {
        line[strcspn(line, "\n")] = 0;
        if (strncmp(line, "command:", 8) == 0) {
            strcpy(stored_command, line + 8);
            char x_val[128] = "";
            char y_val[128] = "";
            char z_val[128] = "";
            char w_val[128] = "";
            if (match_pattern(stored_command, command, x_val, y_val, z_val, w_val)) {
                if (fgets(action, sizeof(action), f)) {
                    action[strcspn(action, "\n")] = 0;
                    char final_action[256];
                    fill_action(action, final_action, x_val, y_val, z_val, w_val);
                    run_primitive(final_action);
                }
                fclose(f);
                return 1;
            }
        }
    }
    fclose(f);
    return 0;
}

void execute(char *command) {
    if (strcmp(command, "listen") == 0) {
        printf("1 I am ready\n");
    } else if (strncmp(command, "learn ", 6) == 0) {
        char *new_cmd = command + 6;
        if (new_cmd[0] == '"') new_cmd++;
        int len = strlen(new_cmd);
        if (new_cmd[len-1] == '"') new_cmd[len-1] = 0;
        printf("1 What should I do when you say this?\n");
        char action[256];
        fgets(action, sizeof(action), stdin);
        action[strcspn(action, "\n")] = 0;
        FILE *f = fopen(dictionary_file, "a");
        if (f) {
            fprintf(f, "command:%s\n", new_cmd);
            fprintf(f, "%s\n", action);
            fprintf(f, "---\n");
            fclose(f);
        }
        printf("1 Understood. Saved.\n");
    } else if (check_dictionary(command)) {
        return;
    } else {
        printf("1 Uongo\n");
    }
}

void run_sema_file(char *filepath) {
    FILE *f = fopen(filepath, "r");
    if (!f) {
        printf("1 Uongo %s not found\n", filepath);
        return;
    }
    char line[256];
    while (fgets(line, sizeof(line), f)) {
        line[strcspn(line, "\n")] = 0;
        if (strlen(line) == 0) continue;
        execute(line);
    }
    fclose(f);
}

int main(int argc, char *argv[]) {
    if (argc > 1) {
        run_sema_file(argv[1]);
        return 0;
    }

    printf("1 Hello\n");
    char input[256];
    fgets(input, sizeof(input), stdin);
    run_sema_file("neo.sema");

    while (1) {
        printf("1 What would you like to do?\n");
        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = 0;

        if (strcmp(input, "create a new user") == 0) {
            printf("1 Choose a username\n");
            fgets(username, sizeof(username), stdin);
            username[strcspn(username, "\n")] = 0;
            if (user_exists(username)) {
                printf("1 Username taken. Choose another.\n");
                continue;
            }
            char pass[64];
            printf("1 Choose a password\n");
            fgets(pass, sizeof(pass), stdin);
            pass[strcspn(pass, "\n")] = 0;
            save_user(username, pass);
            printf("1 Welcome %s\n", username);
            break;
        } else if (strcmp(input, "login") == 0) {
            printf("1 Enter username\n");
            fgets(username, sizeof(username), stdin);
            username[strcspn(username, "\n")] = 0;
            char pass[64];
            printf("1 Enter password\n");
            fgets(pass, sizeof(pass), stdin);
            pass[strcspn(pass, "\n")] = 0;
            if (verify_user(username, pass)) {
                printf("1 Welcome back %s\n", username);
                break;
            } else {
                printf("1 Uongo. Wrong username or password.\n");
            }
        } else {
            printf("1 Uongo\n");
        }
    }

    while (1) {
        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = 0;
        if (strcmp(input, "stop") == 0) {
            printf("1 Going back\n");
            strcpy(username, "");
            break;
        } else if (strcmp(input, "listen") == 0) {
            printf("1 I am ready\n");
        } else if (strncmp(input, "run ", 4) == 0) {
            run_sema_file(input + 4);
        } else {
            execute(input);
        }
    }
    return 0;
}
