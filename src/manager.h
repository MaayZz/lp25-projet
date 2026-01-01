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

#include "network.h"
#include "process.h"
#include "ui.h"

#define REFRESH_INTERVAL 2 // Rafraîchir toutes les 2 secondes
#define MAX_MACHINES 33    // 1 locale + 32 distantes max

/**
 * @brief Structure représentant une machine (locale ou distante).
 */
typedef struct machine_info {
  char nom[MAX_HOSTNAME_LEN]; /* Nom d'affichage */
  int is_local;               /* 1 si machine locale, 0 si distante */
  remote_host_t
      *remote_host; /* Pointeur vers config distante (NULL si local) */
  processus_t *liste_processus; /* Liste des processus de cette machine */
} machine_info_t;

/**
 * @brief Structure d'état du gestionnaire.
 */
typedef struct manager_state {
  /* Mode local */
  processus_t *liste_processus;

  /* Mode réseau */
  machine_info_t machines[MAX_MACHINES];
  int nb_machines;
  int machine_courante;

  /* Commun */
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

/**
 * @brief Lance la boucle principale du gestionnaire (mode réseau).
 * @param state : Pointeur vers l'état du gestionnaire.
 * @param config : Configuration réseau avec les hôtes distants.
 * @param include_local : 1 pour inclure la machine locale, 0 sinon.
 * @return int : Code de retour (EXIT_SUCCESS ou EXIT_FAILURE).
 */
int manager_run_network(manager_state_t *state, network_config_t *config,
                        int include_local);

/**
 * @brief Ajoute une machine à la liste des machines gérées.
 * @param state : Pointeur vers l'état du gestionnaire.
 * @param nom : Nom d'affichage de la machine.
 * @param is_local : 1 si machine locale, 0 si distante.
 * @param host : Pointeur vers l'hôte distant (NULL si local).
 * @return int : Index de la machine ajoutée, ou -1 en cas d'erreur.
 */
int manager_add_machine(manager_state_t *state, const char *nom, int is_local,
                        remote_host_t *host);

#endif /* MANAGER_H */