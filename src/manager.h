/**
 * @file manager.h
 * @brief Module d'orchestration de tous les modules
 * @author Abir Islam, Mellouk Mohamed-Amine, Issam Fallani
 * 
 * Ce module gère la coordination entre les modules process, ui et network.
 * Il implémente la boucle principale et la logique métier de l'application.
 */

#ifndef MANAGER_H
#define MANAGER_H

#include "process.h"
#include "ui.h"


#define REFRESH_INTERVAL 2  // Rafraîchir toutes les 2 secondes

/**
 * @brief Structure d'état du gestionnaire.
 */
typedef struct manager_state {
    processus_t *liste_processus;
    ui_state_t ui_state;
    int running;
    int cycles;
} manager_state_t;


/**
 * @brief Initialise le gestionnaire.
 * @param state : Pointeur vers l'état du gestionnaire.
 */
void manager_init(manager_state_t *state);

/**
 * @brief Lance la boucle principale du gestionnaire (mode local).
 * @param state : Pointeur vers l'état du gestionnaire.
 * @return int : Code de retour (EXIT_SUCCESS ou EXIT_FAILURE).
 */
int manager_run_local(manager_state_t *state);

/**
 * @brief Nettoie et libère les ressources du gestionnaire.
 * @param state : Pointeur vers l'état du gestionnaire.
 */
void manager_cleanup(manager_state_t *state);

/**
 * @brief Gère une action sur un processus (kill, pause, continue).
 * @param state : Pointeur vers l'état du gestionnaire.
 * @param action : Code d'action à effectuer.
 */
void manager_gerer_action_processus(manager_state_t *state, int action);

#endif /* MANAGER_H */