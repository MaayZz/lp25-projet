/**
 * @file main.c
 * @brief Point d'entrée principal du programme
 * @author Groupe LP25
 * 
 * Ce fichier contient le main() et la gestion des arguments en ligne de commande.
 * Pour le moment, seul le mode local est implémenté.
 */

#include "manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Affiche l'aide du programme.
 */
void afficher_aide(void) {
    printf("\n");
    printf("========================================\n");
    printf("  MY_HTOP - Moniteur de Processus\n");
    printf("========================================\n");
    printf("\n");
    printf("Usage: my_htop_local [OPTIONS]\n");
    printf("\n");
    printf("Options:\n");
    printf("  -h, --help              Affiche cette aide\n");
    printf("  --dry-run               Test l'acces aux processus sans affichage\n");
    printf("\n");
    printf("Mode local (par defaut):\n");
    printf("  Sans options, affiche les processus de la machine locale\n");
    printf("\n");
    printf("Raccourcis clavier:\n");
    printf("  F1 ou h                 Afficher l'aide\n");
    printf("  F4 ou /                 Rechercher un processus\n");
    printf("  F5 ou p                 Mettre en pause (SIGSTOP)\n");
    printf("  F6 ou k                 Arreter un processus (SIGTERM)\n");
    printf("  F7 ou 9                 Tuer un processus (SIGKILL)\n");
    printf("  F8 ou c                 Redemarrer/Reprendre (SIGCONT)\n");
    printf("  Fleches haut/bas        Navigation\n");
    printf("  Page Up/Down            Navigation rapide\n");
    printf("  q ou Q                  Quitter\n");
    printf("\n");
    printf("Note: Certaines actions necessitent des droits root (sudo)\n");
    printf("\n");
}

/**
 * @brief Mode dry-run : teste l'accès aux processus.
 */
int mode_dry_run(void) {
    processus_t *liste;
    int nb_processus;
    
    printf("Mode dry-run: Test d'acces aux processus...\n");
    
    liste = recuperer_processus_locaux();
    if (liste == NULL) {
        fprintf(stderr, "ERREUR: Impossible d'acceder a /proc\n");
        return EXIT_FAILURE;
    }
    
    nb_processus = compter_processus(liste);
    printf("Succes: %d processus detectes\n", nb_processus);
    
    liberer_liste_processus(liste);
    
    return EXIT_SUCCESS;
}

/**
 * @brief Point d'entrée principal.
 */
int main(int argc, char *argv[]) {
    manager_state_t manager_state;
    int retour;
    
    /* Gestion des arguments */
    if (argc > 1) {
        if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
            afficher_aide();
            return EXIT_SUCCESS;
        } else if (strcmp(argv[1], "--dry-run") == 0) {
            return mode_dry_run();
        } else {
            fprintf(stderr, "Option inconnue: %s\n", argv[1]);
            fprintf(stderr, "Utilisez -h ou --help pour l'aide\n");
            return EXIT_FAILURE;
        }
    }
    
    /* Mode local par défaut */
    manager_init(&manager_state);
    retour = manager_run_local(&manager_state);
    manager_cleanup(&manager_state);
    
    /* Message de fin */
    printf("\n========================================\n");
    printf("  MY_HTOP termine proprement\n");
    printf("  Cycles d'actualisation: %d\n", manager_state.cycles);
    printf("========================================\n\n");
    
    return retour;
}