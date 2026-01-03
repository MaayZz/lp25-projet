/**
 * @file ui.h
 * @brief Module de gestion de l'affichage et de l'interface utilisateur
 * @author Abir Islam, Mellouk Mohamed-Amine, Issam Fallani
 *
 * Ce module implémente les fonctionnalités d'affichage des processus
 * et de gestion des événements clavier avec ncurses.
 */

#ifndef UI_H
#define UI_H

#include "process.h"
#include <time.h>

/* Forward declaration */
typedef struct machine_info machine_info_t;

#define REFRESH_TIMEOUT 100
#define MESSAGE_DISPLAY_DURATION                                               \
  5 // Durée d'affichage des messages d'action (peut etre modifié)

// Codes de retour pour les actions utilisateur
#define ACTION_CONTINUE 1
#define ACTION_QUIT 0
#define ACTION_HELP 2
#define ACTION_KILL 3
#define ACTION_PAUSE 4
#define ACTION_CONTINUE_SIGNAL 5
#define ACTION_FORCE_KILL 6
#define ACTION_RESTART 7
#define ACTION_SEARCH 8
#define ACTION_NEXT_TAB 9
#define ACTION_PREV_TAB 10

/**
 * @brief Structure pour stocker l'état de l'interface.
 */
typedef struct ui_state {
  int selected_index;
  int scroll_offset;
  char message_buffer[256];
  int message_type;    /* 0: info, 1: erreur */
  int show_help;       /* 1 si fenêtre d'aide affichée */
  time_t message_time; /* Timestamp du message pour durée d'affichage */

  /* Pour mode réseau */
  int nb_machines;      /* Nombre total de machines */
  int machine_courante; /* Index de la machine courante */
} ui_state_t;

/**
 * @brief Initialise l'environnement ncurses.
 */
void ui_init(void);

/**
 * @brief Termine l'environnement ncurses.
 */
void ui_cleanup(void);

/**
 * @brief Affiche l'interface complète avec la liste des processus.
 * @param head : Pointeur vers la liste des processus.
 * @param state : État de l'interface (sélection, scroll, messages).
 */
void ui_afficher_processus(processus_t *head, ui_state_t *state);

/**
 * @brief Affiche l'interface avec onglets pour mode réseau.
 * @param machines : Tableau des machines.
 * @param nb_machines : Nombre de machines.
 * @param machine_courante : Index de la machine courante.
 * @param state : État de l'interface.
 */
void ui_afficher_processus_network(machine_info_t *machines, int nb_machines,
                                   int machine_courante, ui_state_t *state);

/**
 * @brief Affiche la fenêtre d'aide.
 */
void ui_afficher_aide(void);

/**
 * @brief Affiche un message temporaire à l'utilisateur.
 * @param state : État de l'interface.
 * @param msg : Message à afficher.
 * @param type : Type de message (0 = info, 1 = erreur).
 */
void ui_afficher_message(ui_state_t *state, const char *msg, int type);

/**
 * @brief Demande une saisie à l'utilisateur en bas de l'écran.
 * @param state : État de l'interface.
 * @param prompt : Message d'invite.
 * @param buffer : Buffer pour stocker la saisie.
 * @param max_len : Taille maximale du buffer.
 * @return int : 1 si une saisie a été faite, 0 si annulé.
 */
int ui_demander_saisie(ui_state_t *state, const char *prompt, char *buffer,
                       int max_len);

/**
 * @brief Gère les événements clavier et la navigation.
 * @param state : État de l'interface.
 * @param nb_processus : Nombre total de processus.
 * @return int : Code d'action (ACTION_CONTINUE, ACTION_QUIT, etc.).
 */
int ui_gerer_evenements(ui_state_t *state, int nb_processus);

/**
 * @brief Initialise l'état de l'interface.
 * @param state : Pointeur vers la structure d'état à initialiser.
 */
void ui_init_state(ui_state_t *state);

#endif /* UI_H */