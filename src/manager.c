/**
 * @file manager.c
 * @brief Implémentation du module d'orchestration
 * @author Abir Islam, Mellouk Mohamed-Amine, Issam Fallani
 */

#define _DEFAULT_SOURCE

#include "manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>

/* Fonctions publiques */

void manager_init(manager_state_t *state) {
    state->liste_processus = NULL;
    state->running = 1;
    state->cycles = 0;
    ui_init_state(&state->ui_state);
}

void manager_cleanup(manager_state_t *state) {
    if (state->liste_processus != NULL) {
        liberer_liste_processus(state->liste_processus);
        state->liste_processus = NULL;
    }
}

void manager_gerer_action_processus(manager_state_t *state, int action) {
    processus_t *proc_selectionne;
    int signal_to_send = -1;
    char msg[256];
    const char *action_name = "";
    
    /* Déterminer le signal à envoyer */
    switch (action) {
        case ACTION_KILL:
            signal_to_send = SIGTERM;
            action_name = "termine (SIGTERM)";
            break;
        case ACTION_PAUSE:
            signal_to_send = SIGSTOP;
            action_name = "mis en pause (SIGSTOP)";
            break;
        case ACTION_CONTINUE_SIGNAL:
            signal_to_send = SIGCONT;
            action_name = "repris (SIGCONT)";
            break;
        case ACTION_FORCE_KILL:
            signal_to_send = SIGKILL;
            action_name = "tue (SIGKILL)";
            break;
        default:
            return;
    }
    
    /* Récupérer le processus sélectionné */
    proc_selectionne = get_processus_at_index(state->liste_processus, 
                                               state->ui_state.selected_index);
    
    if (proc_selectionne == NULL) {
        ui_afficher_message(&state->ui_state, 
                           "ERREUR: Processus introuvable dans la liste", 1);
        return;
    }
    
    /* Vérifier que le processus existe encore */
    char proc_path[64];
    snprintf(proc_path, sizeof(proc_path), "/proc/%d", proc_selectionne->pid);
    
    if (access(proc_path, F_OK) != 0) {
        snprintf(msg, sizeof(msg), "ERREUR: Le processus %d n'existe plus", 
                 proc_selectionne->pid);
        ui_afficher_message(&state->ui_state, msg, 1);
        return;
    }
    
    /* Envoyer le signal */
    int resultat = envoyer_signal(proc_selectionne->pid, signal_to_send);
    
    if (resultat == 0) {
        snprintf(msg, sizeof(msg), "PID %d (%s) %s",
                 proc_selectionne->pid, proc_selectionne->nom_commande, action_name);
        ui_afficher_message(&state->ui_state, msg, 0);
    } else {
        /* Gestion des erreurs */
        if (errno == EPERM) {
            snprintf(msg, sizeof(msg), 
                     "ERREUR: Permission refusee pour PID %d. Utilisez sudo",
                     proc_selectionne->pid);
        } else if (errno == ESRCH) {
            snprintf(msg, sizeof(msg), 
                     "ERREUR: Processus %d introuvable",
                     proc_selectionne->pid);
        } else {
            snprintf(msg, sizeof(msg), 
                     "ERREUR: Echec signal vers PID %d (errno: %d)",
                     proc_selectionne->pid, errno);
        }
        ui_afficher_message(&state->ui_state, msg, 1);
    }
}

int manager_run_local(manager_state_t *state) {
    time_t last_refresh = time(NULL);
    time_t current_time;
    int action;
    
    /* Initialisation de l'interface */
    ui_init();
    
    /* Message de bienvenue */
    ui_afficher_message(&state->ui_state, 
                       "Bienvenue dans MY_HTOP - F1:Aide Q:Quitter", 0);
    
    /* Premier chargement des processus */
    state->liste_processus = recuperer_processus_locaux();
    if (state->liste_processus == NULL) {
        ui_cleanup();
        fprintf(stderr, "ERREUR FATALE: Impossible de lire /proc\n");
        return EXIT_FAILURE;
    }
    
    /* Boucle principale */
    while (state->running) {
        current_time = time(NULL);
        
        /* A. Collecte des données (si intervalle écoulé) */
        if (difftime(current_time, last_refresh) >= REFRESH_INTERVAL) {
            /* Libérer l'ancienne liste */
            liberer_liste_processus(state->liste_processus);
            
            /* Recharger les processus */
            state->liste_processus = recuperer_processus_locaux();
            
            if (state->liste_processus == NULL) {
                ui_cleanup();
                fprintf(stderr, "ERREUR FATALE: Impossible de lire /proc\n");
                return EXIT_FAILURE;
            }
            
            last_refresh = current_time;
            state->cycles++;
        }
        
        int nb_processus = compter_processus(state->liste_processus);
        
        /* Ajuster la sélection si nécessaire */
        if (state->ui_state.selected_index >= nb_processus) {
            state->ui_state.selected_index = nb_processus - 1;
        }
        if (state->ui_state.selected_index < 0) {
            state->ui_state.selected_index = 0;
        }
        
        /* B. Affichage */
        clear();
        ui_afficher_processus(state->liste_processus, &state->ui_state);
        refresh();
        
        /* C. Gestion des événements */
        action = ui_gerer_evenements(&state->ui_state, nb_processus);
        
        if (action == ACTION_QUIT) {
            state->running = 0;
        } else if (action == ACTION_HELP) {
            ui_afficher_aide();
        } else if (action == ACTION_SEARCH) {
            ui_afficher_message(&state->ui_state, 
                               "Fonction recherche non implementee", 0);
        } else if (action == ACTION_KILL || action == ACTION_PAUSE || 
                   action == ACTION_CONTINUE_SIGNAL || action == ACTION_FORCE_KILL) {
            manager_gerer_action_processus(state, action);
        }
        
        /* Petit délai pour ne pas surcharger le CPU */
        usleep(50000);
    }
    
    /* Nettoyage */
    ui_cleanup();
    
    return EXIT_SUCCESS;
}