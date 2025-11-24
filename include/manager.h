#ifndef MANAGER_H
#define MANAGER_H

#include <stdbool.h>
#include "config.h"


typedef enum connection_type_s {
    CONNECTION_UNSET = 0,
    CONNECTION_SSH,
    CONNECTION_TELNET
} connection_type_t;

typedef struct app_config_s {
    bool              show_help;
    bool              dry_run;
    bool              collect_local;
    bool              collect_remote;
    bool              collect_all;

    // Options de connexion directe : -s / -l / -u / -p / -t / -P
    char             *remote_server;
    char             *login;
    char             *username;
    char             *password;
    int               port;
    connection_type_t connection_type;

    // Fichier de configuration : -c / --remote-config
    char             *config_path;
    host_array_t      hosts;

} app_config_t;

/**
 * @brief Initialise une structure de configuration avec des valeurs par défaut.
 */
void init_app_config(app_config_t *config);

/**
 * @brief Libère les ressources associées à la configuration.
 */
void free_app_config(app_config_t *config);

/**
 * @brief Analyse les arguments de la ligne de commande et remplit app_config_t.
 *
 * @return 0 en cas de succès, -1 en cas d'erreur (option invalide, etc.).
 */
int parse_arguments(int argc, char **argv, app_config_t *config);

/**
 * @brief Affiche l'aide du programme.
 */
void print_help(const char *prog_name);

/**
 * @brief Exécute le mode "dry run".
 *
 * Teste l'accès aux processus locaux et/ou distants sans les afficher.
 */
int run_dry_run(app_config_t *config);

/**
 * @brief Point d'entrée principal de la logique métier.
 *
 * Après le parsing et en dehors du mode dry-run, c'est cette fonction
 * qui coordonnera process/network/ui.
 */
int manager_run(app_config_t *config);

#endif /* MANAGER_H */
