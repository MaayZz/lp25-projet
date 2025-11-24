#include "config.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

static int check_file_permissions(const char *path)
{
    struct stat st;

    if (stat(path, &st) != 0) {
        log_error("Impossible de lire les informations du fichier de configuration '%s'", path);
        return -1;
    }
    
    if ((st.st_mode & 0777) != 0600) {
        log_error("Alerte: le fichier de configuration '%s' n'a pas les droits 600 (rw-------)", path);
    }
    return 0;
}

static host_type_t parse_connection_type(const char *s)
{
    if (strcmp(s, "ssh") == 0) {
        return HOST_REMOTE_SSH;
    }
    if (strcmp(s, "telnet") == 0) {
        return HOST_REMOTE_TELNET;
    }
    return HOST_LOCAL;
}

static char *str_dup(const char *s)
{
    size_t len;
    char *copy;

    if (s == NULL) {
        return NULL;
    }
    len = strlen(s);
    copy = malloc(len + 1);
    if (copy == NULL) {
        return NULL;
    }
    memcpy(copy, s, len + 1);
    return copy;
}

int load_hosts_from_config(const char *path, host_array_t *hosts)
{
    FILE *fp;
    char line[1024];
    size_t capacity;
    size_t count;

    if (hosts == NULL) {
        return -1;
    }
    hosts->items = NULL;
    hosts->size = 0;

    if (path == NULL) {
        return 0;
    }

    if (check_file_permissions(path) != 0) {
    }

    fp = fopen(path, "r");
    if (fp == NULL) {
        log_error("Impossible d'ouvrir le fichier de configuration '%s'", path);
        return -1;
    }

    capacity = 4;
    count = 0;
    hosts->items = calloc(capacity, sizeof(host_info_t));
    if (hosts->items == NULL) {
        fclose(fp);
        return -1;
    }

    while (fgets(line, sizeof(line), fp) != NULL) {
        char *name;
        char *addr;
        char *port_str;
        char *user;
        char *pass;
        char *type_str;
        char *saveptr;
        host_info_t *h;
        int port;

        if (line[0] == '#' || line[0] == '\n') {
            continue;
        }

        line[strcspn(line, "\r\n")] = '\0';

        name = strtok_r(line, ":", &saveptr);
        addr = strtok_r(NULL, ":", &saveptr);
        port_str = strtok_r(NULL, ":", &saveptr);
        user = strtok_r(NULL, ":", &saveptr);
        pass = strtok_r(NULL, ":", &saveptr);
        type_str = strtok_r(NULL, ":", &saveptr);

        if (name == NULL || addr == NULL || port_str == NULL ||
            user == NULL || pass == NULL || type_str == NULL) {
            log_error("Ligne de configuration invalide (champs manquants)");
            continue;
        }

        port = atoi(port_str);

        if (count == capacity) {
            host_info_t *new_items;
            size_t new_capacity;

            new_capacity = capacity * 2;
            new_items = realloc(hosts->items, new_capacity * sizeof(host_info_t));
            if (new_items == NULL) {
                log_error("Allocation mémoire échouée lors du chargement des hôtes");
                break;
            }
            hosts->items = new_items;
            capacity = new_capacity;
        }

        h = &hosts->items[count];
        h->name = str_dup(name);
        h->address = str_dup(addr);
        h->port = port;
        h->username = str_dup(user);
        h->password = str_dup(pass);
        h->type = parse_connection_type(type_str);

        count++;
    }

    fclose(fp);
    hosts->size = count;
    return 0;
}

void free_host_array(host_array_t *hosts)
{
    size_t i;

    if (hosts == NULL || hosts->items == NULL) {
        return;
    }
    for (i = 0; i < hosts->size; i++) {
        free(hosts->items[i].name);
        free(hosts->items[i].address);
        free(hosts->items[i].username);
        free(hosts->items[i].password);
    }
    free(hosts->items);
    hosts->items = NULL;
    hosts->size = 0;
}
