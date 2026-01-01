/**
 * @file main.c
 * @brief Point d'entrée principal du programme
 * @author Abir Islam, Mellouk Mohamed-Amine, Issam Fallani
 *
 * Ce fichier contient le main() et la gestion des arguments en ligne de
 * commande. Supporte les modes local et réseau.
 */

#include "manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

/**
 * @brief Affiche l'aide du programme.
 */
void afficher_aide(void) {
  printf("\n");
  printf("========================================\n");
  printf("  MY_HTOP - Moniteur de Processus\n");
  printf("========================================\n");
  printf("\n");
  printf("Usage: my_htop [OPTIONS]\n");
  printf("\n");
  printf("Options:\n");
  printf("  -h, --help                     Affiche cette aide\n");
  printf("  --dry-run                      Test l'acces aux processus sans "
         "affichage\n");
  printf("\n");
  printf("Mode local (par defaut):\n");
  printf("  Sans options, affiche les processus de la machine locale\n");
  printf("\n");
  printf("Mode reseau:\n");
  printf("  -c, --remote-config <fichier>  Fichier de configuration distant\n");
  printf("  -s, --remote-server <host>     Adresse IP ou DNS du serveur "
         "distant\n");
  printf("  -l, --login <user@host>        Format: utilisateur@serveur\n");
  printf("  -u, --username <user>          Nom d'utilisateur\n");
  printf("  -p, --password <password>      Mot de passe (deconseille CLI)\n");
  printf(
      "  -t, --connexion-type <type>    Type: ssh ou telnet (defaut: ssh)\n");
  printf("  -P, --port <port>              Port de connexion\n");
  printf("  -a, --all                      Affiche local + distant\n");
  printf("\n");
  printf("Raccourcis clavier:\n");
  printf("  F1 ou h                        Afficher l'aide\n");
  printf("  F2 / F3                        Onglet suivant/precedent\n");
  printf("  F4 ou /                        Rechercher un processus\n");
  printf("  F5 ou p                        Mettre en pause (SIGSTOP)\n");
  printf("  F6 ou k                        Arreter un processus (SIGTERM)\n");
  printf("  F7 ou 9                        Tuer un processus (SIGKILL)\n");
  printf("  F8 ou c                        Redemarrer/Reprendre (SIGCONT)\n");
  printf("  Fleches haut/bas               Navigation\n");
  printf("  Page Up/Down                   Navigation rapide\n");
  printf("  q ou Q                         Quitter\n");
  printf("\n");
  printf("Note: Certaines actions necessitent des droits root (sudo)\n");
  printf("\n");
}

/**
 * @brief Lit un mot de passe depuis stdin sans écho.
 */
char *lire_mot_de_passe(const char *prompt) {
  static char password[MAX_PASSWORD_LEN];
  struct termios old_term, new_term;

  printf("%s", prompt);
  fflush(stdout);

  /* Désactiver l'écho */
  tcgetattr(STDIN_FILENO, &old_term);
  new_term = old_term;
  new_term.c_lflag &= ~ECHO;
  tcsetattr(STDIN_FILENO, TCSANOW, &new_term);

  /* Lire le mot de passe */
  if (fgets(password, sizeof(password), stdin) != NULL) {
    password[strcspn(password, "\n")] = '\0';
  }

  /* Réactiver l'écho */
  tcsetattr(STDIN_FILENO, TCSANOW, &old_term);
  printf("\n");

  return password;
}

/**
 * @brief Mode dry-run : teste l'accès aux processus locaux et distants.
 */
int mode_dry_run(int has_network, network_config_t *config) {
  processus_t *liste;
  int nb_processus;

  /* Test local */
  printf("Mode dry-run: Test d'acces aux processus locaux...\n");
  liste = recuperer_processus_locaux();
  if (liste == NULL) {
    fprintf(stderr, "ERREUR: Impossible d'acceder a /proc\n");
    return EXIT_FAILURE;
  }
  nb_processus = compter_processus(liste);
  printf("Succes: %d processus locaux detectes\n", nb_processus);
  liberer_liste_processus(liste);

  /* Test distant */
  if (has_network) {
    for (int i = 0; i < config->nb_hosts; i++) {
      printf("\nTest connexion a %s (%s)...\n", config->hosts[i].nom,
             config->hosts[i].adresse);
      if (connect_ssh(&config->hosts[i]) != 0) {
        fprintf(stderr, "ERREUR: Impossible de se connecter a %s\n",
                config->hosts[i].nom);
        continue;
      }

      liste = get_remote_processes(&config->hosts[i]);
      if (liste == NULL) {
        fprintf(stderr, "ERREUR: Impossible de recuperer les processus de %s\n",
                config->hosts[i].nom);
        disconnect_ssh(&config->hosts[i]);
        continue;
      }

      nb_processus = compter_processus(liste);
      printf("Succes: %d processus detectes sur %s\n", nb_processus,
             config->hosts[i].nom);
      liberer_liste_processus(liste);
      disconnect_ssh(&config->hosts[i]);
    }
  }

  return EXIT_SUCCESS;
}

/**
 * @brief Point d'entrée principal.
 */
int main(int argc, char *argv[]) {
  manager_state_t manager_state;
  network_config_t network_config;
  remote_host_t single_host;
  int retour;

  /* Variables pour le parsing */
  char *config_file = NULL;
  char *remote_server = NULL;
  char *username = NULL;
  char *password = NULL;
  int port = 0;
  connection_type_t conn_type = CONN_SSH;
  int all_mode = 0;
  int is_dry_run = 0;
  int has_network = 0;

  /* Parsing des arguments */
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
      afficher_aide();
      return EXIT_SUCCESS;
    } else if (strcmp(argv[i], "--dry-run") == 0) {
      is_dry_run = 1;
    } else if (strcmp(argv[i], "-c") == 0 ||
               strcmp(argv[i], "--remote-config") == 0) {
      if (i + 1 < argc) {
        config_file = argv[++i];
      } else {
        fprintf(stderr, "ERREUR: %s requiert un argument\n", argv[i]);
        return EXIT_FAILURE;
      }
    } else if (strcmp(argv[i], "-s") == 0 ||
               strcmp(argv[i], "--remote-server") == 0) {
      if (i + 1 < argc) {
        remote_server = argv[++i];
      } else {
        fprintf(stderr, "ERREUR: %s requiert un argument\n", argv[i]);
        return EXIT_FAILURE;
      }
    } else if (strcmp(argv[i], "-l") == 0 || strcmp(argv[i], "--login") == 0) {
      if (i + 1 < argc) {
        char *login_str = argv[++i];
        char *at_sign = strchr(login_str, '@');
        if (at_sign == NULL) {
          fprintf(stderr,
                  "ERREUR: Format invalide pour -l (attendu: user@host)\n");
          return EXIT_FAILURE;
        }
        *at_sign = '\0';
        username = login_str;
        remote_server = at_sign + 1;
      } else {
        fprintf(stderr, "ERREUR: %s requiert un argument\n", argv[i]);
        return EXIT_FAILURE;
      }
    } else if (strcmp(argv[i], "-u") == 0 ||
               strcmp(argv[i], "--username") == 0) {
      if (i + 1 < argc) {
        username = argv[++i];
      } else {
        fprintf(stderr, "ERREUR: %s requiert un argument\n", argv[i]);
        return EXIT_FAILURE;
      }
    } else if (strcmp(argv[i], "-p") == 0 ||
               strcmp(argv[i], "--password") == 0) {
      if (i + 1 < argc) {
        password = argv[++i];
      } else {
        fprintf(stderr, "ERREUR: %s requiert un argument\n", argv[i]);
        return EXIT_FAILURE;
      }
    } else if (strcmp(argv[i], "-t") == 0 ||
               strcmp(argv[i], "--connexion-type") == 0) {
      if (i + 1 < argc) {
        char *type_str = argv[++i];
        if (strcmp(type_str, "ssh") == 0) {
          conn_type = CONN_SSH;
        } else if (strcmp(type_str, "telnet") == 0) {
          conn_type = CONN_TELNET;
          fprintf(stderr, "AVERTISSEMENT: Telnet n'est pas encore implemente, "
                          "SSH sera utilise\n");
        } else {
          fprintf(stderr, "ERREUR: Type de connexion invalide: %s\n", type_str);
          return EXIT_FAILURE;
        }
      } else {
        fprintf(stderr, "ERREUR: %s requiert un argument\n", argv[i]);
        return EXIT_FAILURE;
      }
    } else if (strcmp(argv[i], "-P") == 0 || strcmp(argv[i], "--port") == 0) {
      if (i + 1 < argc) {
        port = atoi(argv[++i]);
        if (port <= 0 || port > 65535) {
          fprintf(stderr, "ERREUR: Port invalide: %d\n", port);
          return EXIT_FAILURE;
        }
      } else {
        fprintf(stderr, "ERREUR: %s requiert un argument\n", argv[i]);
        return EXIT_FAILURE;
      }
    } else if (strcmp(argv[i], "-a") == 0 || strcmp(argv[i], "--all") == 0) {
      all_mode = 1;
    } else {
      fprintf(stderr, "ERREUR: Option inconnue: %s\n", argv[i]);
      fprintf(stderr, "Utilisez -h ou --help pour l'aide\n");
      return EXIT_FAILURE;
    }
  }

  /* Configuration réseau */
  init_network_config(&network_config);

  if (config_file != NULL) {
    /* Mode fichier de configuration */
    check_config_file_permissions(config_file);
    if (parse_config_file(config_file, &network_config) != 0) {
      return EXIT_FAILURE;
    }
    has_network = 1;
  } else if (remote_server != NULL) {
    /* Mode serveur unique */
    strncpy(single_host.nom, remote_server, MAX_HOSTNAME_LEN - 1);
    single_host.nom[MAX_HOSTNAME_LEN - 1] = '\0';
    strncpy(single_host.adresse, remote_server, MAX_HOSTNAME_LEN - 1);
    single_host.adresse[MAX_HOSTNAME_LEN - 1] = '\0';
    single_host.port = (port > 0) ? port : DEFAULT_SSH_PORT;
    single_host.type = conn_type;
    single_host.session = NULL;

    /* Demander username si manquant */
    if (username == NULL) {
      static char user_buffer[MAX_USERNAME_LEN];
      printf("Nom d'utilisateur: ");
      fgets(user_buffer, sizeof(user_buffer), stdin);
      user_buffer[strcspn(user_buffer, "\n")] = '\0';
      username = user_buffer;
    }
    strncpy(single_host.username, username, MAX_USERNAME_LEN - 1);
    single_host.username[MAX_USERNAME_LEN - 1] = '\0';

    /* Demander password si manquant */
    if (password == NULL) {
      password = lire_mot_de_passe("Mot de passe: ");
    }
    strncpy(single_host.password, password, MAX_PASSWORD_LEN - 1);
    single_host.password[MAX_PASSWORD_LEN - 1] = '\0';

    network_config.hosts[0] = single_host;
    network_config.nb_hosts = 1;
    has_network = 1;
  }

  /* Mode dry-run */
  if (is_dry_run) {
    return mode_dry_run(has_network, &network_config);
  }

  /* Lancement du programme */
  manager_init(&manager_state);

  if (has_network) {
    retour = manager_run_network(&manager_state, &network_config,
                                 all_mode || config_file == NULL);
  } else {
    retour = manager_run_local(&manager_state);
  }

  manager_cleanup(&manager_state);

  /* Message de fin */
  printf("\n========================================\n");
  printf("  MY_HTOP termine proprement\n");
  printf("  Cycles d'actualisation: %d\n", manager_state.cycles);
  printf("========================================\n\n");

  return retour;
}