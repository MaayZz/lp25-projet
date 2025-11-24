#ifndef UI_H
#define UI_H

#include "process.h"
#include "config.h"
#include <stddef.h>

typedef struct host_process_view_s {
    host_info_t    *host;
    process_list_t *plist;
} host_process_view_t;

/**
 * @brief Initialise le système d'affichage (ncurses, etc.).
 */
int ui_init(void);

/**
 * @brief Ferme proprement le système d'affichage.
 */
void ui_shutdown(void);

/**
 * @brief Boucle principale d'affichage.
 *
 * @param views tableau des vues (host + liste de processus).
 * @param nb_hosts nombre d'hôtes dans le tableau.
 *
 * Cette fonction gérera plus tard les touches F1..F8, navigation onglets, etc.
 */
int ui_main_loop(host_process_view_t *views, size_t nb_hosts);

#endif /* UI_H */
