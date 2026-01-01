/**
 * @file network.h
 * @brief Module de gestion des connexions réseau et machines distantes
 * @author Abir Islam, Mellouk Mohamed-Amine, Issam Fallani
 * 
 * Ce module implémente les fonctionnalités de communication réseau
 * pour la connexion SSH aux machines distantes et la récupération
 * des processus distants.
 */

#ifndef NETWORK_H
#define NETWORK_H

#include "process.h"
#include <libssh/libssh.h>

/* Constantes */
#define MAX_HOSTNAME_LEN 256
#define MAX_PASSWORD_LEN 128
#define MAX_USERNAME_LEN 64
#define MAX_HOSTS 32
#define DEFAULT_SSH_PORT 22
#define DEFAULT_TELNET_PORT 23
#define CONFIG_FILE_DEFAULT ".config"

/* Types de connexion */
typedef enum {
    CONN_SSH,
    CONN_TELNET
} connection_type_t;

/**
 * @brief Structure représentant un hôte distant
 */
typedef struct remote_host {
    char nom[MAX_HOSTNAME_LEN];           /* Nom d'affichage */
    char adresse[MAX_HOSTNAME_LEN];       /* IP ou DNS */
    int port;                             /* Port de connexion */
    char username[MAX_USERNAME_LEN];      /* Nom d'utilisateur */
    char password[MAX_PASSWORD_LEN];      /* Mot de passe */
    connection_type_t type;               /* Type de connexion */
    ssh_session session;                  /* Session SSH (NULL si non connecté) */
} remote_host_t;

/**
 * @brief Structure de configuration réseau
 */
typedef struct network_config {
    remote_host_t hosts[MAX_HOSTS];       /* Liste des hôtes distants */
    int nb_hosts;                         /* Nombre d'hôtes configurés */
} network_config_t;

/* Prototypes des fonctions publiques */

/**
 * @brief Parse le fichier de configuration et remplit la structure config.
 * @param filename : Chemin vers le fichier de configuration
 * @param config : Pointeur vers la structure à remplir
 * @return int : 0 en cas de succès, -1 en cas d'erreur
 */
int parse_config_file(const char *filename, network_config_t *config);

/**
 * @brief Vérifie les permissions du fichier de configuration (doit être 600).
 * @param filename : Chemin vers le fichier
 * @return int : 0 si permissions correctes, -1 sinon (affiche avertissement)
 */
int check_config_file_permissions(const char *filename);

/**
 * @brief Établit une connexion SSH vers un hôte distant.
 * @param host : Pointeur vers la structure de l'hôte
 * @return int : 0 en cas de succès, -1 en cas d'erreur
 */
int connect_ssh(remote_host_t *host);

/**
 * @brief Ferme la connexion SSH d'un hôte.
 * @param host : Pointeur vers la structure de l'hôte
 */
void disconnect_ssh(remote_host_t *host);

/**
 * @brief Récupère la liste des processus d'un hôte distant via SSH.
 * @param host : Pointeur vers l'hôte distant (déjà connecté)
 * @return processus_t* : Liste chaînée des processus, ou NULL en cas d'erreur
 */
processus_t *get_remote_processes(remote_host_t *host);

/**
 * @brief Envoie un signal à un processus distant via SSH.
 * @param host : Pointeur vers l'hôte distant
 * @param pid : PID du processus cible
 * @param signal : Signal à envoyer
 * @return int : 0 en cas de succès, -1 en cas d'erreur
 */
int send_remote_signal(remote_host_t *host, pid_t pid, int signal);

/**
 * @brief Initialise une structure network_config_t.
 * @param config : Pointeur vers la structure à initialiser
 */
void init_network_config(network_config_t *config);

/**
 * @brief Libère les ressources d'une configuration réseau.
 * @param config : Pointeur vers la configuration
 */
void cleanup_network_config(network_config_t *config);

#endif /* NETWORK_H */
