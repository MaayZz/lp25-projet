#define _GNU_SOURCE
#include "manager.h"
#include "config.h"
#include "process.h"
#include "network.h"
#include "ui.h"
#include "utils.h"

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

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

void init_app_config(app_config_t *config)
{
    if (config == NULL) {
        return;
    }
    config->show_help = false;
    config->dry_run = false;
    config->collect_local = true;
    config->collect_remote = false;
    config->collect_all = false;

    config->remote_server = NULL;
    config->login = NULL;
    config->username = NULL;
    config->password = NULL;
    config->port = 0;
    config->connection_type = CONNECTION_UNSET;

    config->config_path = NULL;
    config->hosts.items = NULL;
    config->hosts.size = 0;
}

void free_app_config(app_config_t *config)
{
    if (config == NULL) {
        return;
    }
    free(config->remote_server);
    free(config->login);
    free(config->username);
    free(config->password);
    free(config->config_path);
    free_host_array(&config->hosts);
}

static connection_type_t parse_connection_type_opt(const char *s)
{
    if (strcmp(s, "ssh") == 0) {
        return CONNECTION_SSH;
    }
    if (strcmp(s, "telnet") == 0) {
        return CONNECTION_TELNET;
    }
    return CONNECTION_UNSET;
}

void print_help(const char *prog_name)
{
    printf("Usage: %s [OPTIONS]\n", prog_name);
    printf("\n");
    printf("Options:\n");
    printf("  -h, --help                 Affiche cette aide\n");
    printf("      --dry-run              Teste l'accès à la liste des processus sans les afficher\n");
    printf("  -c, --remote-config PATH   Fichier de configuration des machines distantes (.config)\n");
    printf("  -t, --connection-type TYPE Type de connexion (ssh, telnet)\n");
    printf("  -P, --port PORT            Port de connexion\n");
    printf("  -l, --login USER@HOST      Identifiant+machine distante\n");
    printf("  -s, --remote-server HOST   Adresse IP ou nom DNS de la machine distante\n");
    printf("  -u, --username USER        Nom d'utilisateur\n");
    printf("  -p, --password PASS        Mot de passe\n");
    printf("  -a, --all                  Collecter processus locaux + distants\n");
    printf("\n");
    printf("Sans aucune option, le programme affiche uniquement les processus locaux.\n");
}

int parse_arguments(int argc, char **argv, app_config_t *config)
{
    int opt;
    int option_index;

    static struct option long_options[] = {
        {"help",            no_argument,       0, 'h'},
        {"dry-run",         no_argument,       0,  0 },
        {"remote-config",   required_argument, 0, 'c'},
        {"connection-type", required_argument, 0, 't'},
        {"port",            required_argument, 0, 'P'},
        {"login",           required_argument, 0, 'l'},
        {"remote-server",   required_argument, 0, 's'},
        {"username",        required_argument, 0, 'u'},
        {"password",        required_argument, 0, 'p'},
        {"all",             no_argument,       0, 'a'},
        {0, 0, 0, 0}
    };

    if (config == NULL) {
        return -1;
    }

    while (1) {
        opt = getopt_long(argc, argv, "hc:t:P:l:s:u:p:a", long_options, &option_index);
        if (opt == -1) {
            break;
        }

        switch (opt) {
        case 0:

            if (strcmp(long_options[option_index].name, "dry-run") == 0) {
                config->dry_run = true;
            }
            break;
        case 'h':
            config->show_help = true;
            break;
        case 'c':
            free(config->config_path);
            config->config_path = str_dup(optarg);
            break;
        case 't':
            config->connection_type = parse_connection_type_opt(optarg);
            break;
        case 'P':
            config->port = atoi(optarg);
            break;
        case 'l':
            free(config->login);
            config->login = str_dup(optarg);
            break;
        case 's':
            free(config->remote_server);
            config->remote_server = str_dup(optarg);
            break;
        case 'u':
            free(config->username);
            config->username = str_dup(optarg);
            break;
        case 'p':
            free(config->password);
            config->password = str_dup(optarg);
            break;
        case 'a':
            config->collect_all = true;
            config->collect_local = true;
            config->collect_remote = true;
            break;
        default:
            return -1;
        }
    }


    if (config->config_path != NULL || config->remote_server != NULL || config->login != NULL) {
        config->collect_remote = true;
    }

    return 0;
}

int run_dry_run(app_config_t *config)
{
    process_list_t *local;

    if (config == NULL) {
        return -1;
    }

    if (config->collect_local) {
        log_info("Dry-run: test d'accès aux processus locaux");
        local = get_local_process_list();
        if (local == NULL) {
            log_error("Echec d'accès à la liste des processus locaux");
        } else {
            log_info("Accès aux processus locaux OK (%zu processus)", local->size);
            free_process_list(local);
        }
    }

    if (config->collect_remote) {
        size_t i;

        log_info("Dry-run: test d'accès aux processus distants");

        if (config->config_path != NULL && config->hosts.items == NULL) {
            if (load_hosts_from_config(config->config_path, &config->hosts) != 0) {
                log_error("Echec de chargement du fichier de configuration '%s'", config->config_path);
            }
        }

        for (i = 0; i < config->hosts.size; i++) {
            if (test_remote_connection(&config->hosts.items[i]) != 0) {
                log_error("Connexion vers '%s' échouée", config->hosts.items[i].name);
            } else {
                log_info("Connexion vers '%s' OK", config->hosts.items[i].name);
            }
        }


    }

    return 0;
}

int manager_run(app_config_t *config)
{
    process_list_t *local = NULL;
    host_process_view_t view;
    int ret;

    if (config == NULL) {
        return -1;
    }

    local = get_local_process_list();
    if (local == NULL) {
        log_error("Impossible de récupérer la liste des processus locaux");
        return -1;
    }

    view.host = NULL;
    view.plist = local;

    if (ui_init() != 0) {
        free_process_list(local);
        return -1;
    }

    ret = ui_main_loop(&view, 1);

    ui_shutdown();
    free_process_list(local);

    return ret;
}
