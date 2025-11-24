#ifndef NETWORK_H
#define NETWORK_H

#include "config.h"
#include "process.h"

/**
 * @brief Teste la connexion à une machine distante.
 *
 * @param host informations sur l'hôte distant.
 * @return 0 si la connexion est considérée comme OK, -1 sinon.
 */
int test_remote_connection(const host_info_t *host);

/**
 * @brief Récupère la liste des processus d'une machine distante.
 *
 * Implémentation possible via ssh/telnet et commande "ps".
 */
process_list_t *get_remote_process_list(const host_info_t *host);

#endif /* NETWORK_H */
