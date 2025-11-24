#ifndef CONFIG_H
#define CONFIG_H

#include <stddef.h>

typedef enum host_type_s {
    HOST_LOCAL = 0,
    HOST_REMOTE_SSH,
    HOST_REMOTE_TELNET
} host_type_t;

typedef struct host_info_s {
    char       *name;     
    char       *address;   
    int         port;
    char       *username;
    char       *password;
    host_type_t type;
} host_info_t;

typedef struct host_array_s {
    host_info_t *items;
    size_t       size;
} host_array_t;

/**
 * @brief Charge la configuration des machines distantes depuis un fichier.
 *
 * @param path chemin vers le fichier de configuration (ex: ".config").
 * @param hosts pointeur de sortie vers une structure host_array_t.
 * @return 0 en cas de succès, -1 en cas d'erreur.
 */
int load_hosts_from_config(const char *path, host_array_t *hosts);

/**
 * @brief Libère la mémoire d'un host_array_t.
 */
void free_host_array(host_array_t *hosts);

#endif /* CONFIG_H */
