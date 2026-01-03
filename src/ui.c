/**
 * @file ui.c
 * @brief Implémentation du module d'interface utilisateur
 * @author Abir Islam, Mellouk Mohamed-Amine, Issam Fallani
 */

#include "ui.h"
#include "manager.h"
#include <ncurses.h>
#include <string.h>
#include <sys/sysinfo.h>
#include <time.h>
#include <unistd.h>

/* Codes de couleurs */
#define COLOR_HEADER 1
#define COLOR_TABLE_HEADER 2
#define COLOR_SELECTED 3
#define COLOR_INFO_MSG 4
#define COLOR_ERROR_MSG 5
#define COLOR_HELP_BAR 6

void ui_init(void) {
  initscr();
  cbreak();
  noecho();
  curs_set(0);
  keypad(stdscr, TRUE);
  timeout(REFRESH_TIMEOUT);

  if (has_colors()) {
    start_color();
    init_pair(COLOR_HEADER, COLOR_BLACK, COLOR_GREEN);
    init_pair(COLOR_TABLE_HEADER, COLOR_BLACK, COLOR_CYAN);
    init_pair(COLOR_SELECTED, COLOR_GREEN, COLOR_BLACK);
    init_pair(COLOR_INFO_MSG, COLOR_YELLOW, COLOR_BLACK);
    init_pair(COLOR_ERROR_MSG, COLOR_RED, COLOR_BLACK);
    init_pair(COLOR_HELP_BAR, COLOR_WHITE, COLOR_BLUE);
  }
}

void ui_cleanup(void) { endwin(); }

void ui_init_state(ui_state_t *state) {
  state->selected_index = 0;
  state->scroll_offset = 0;
  state->message_buffer[0] = '\0';
  state->message_type = 0;
  state->show_help = 0;
  state->message_time = 0;
  state->nb_machines = 0;
  state->machine_courante = 0;
}

void ui_afficher_message(ui_state_t *state, const char *msg, int type) {
  strncpy(state->message_buffer, msg, sizeof(state->message_buffer) - 1);
  state->message_buffer[sizeof(state->message_buffer) - 1] = '\0';
  state->message_type = type;
  state->message_time = time(NULL);
}

void ui_afficher_aide(void) {
  int ligne = 3;

  clear();

  attron(COLOR_PAIR(COLOR_HEADER) | A_BOLD);
  mvprintw(1, (COLS - 30) / 2, "MY_HTOP - AIDE");
  attroff(COLOR_PAIR(COLOR_HEADER) | A_BOLD);

  // Contenu de l'aide
  mvprintw(ligne++, 5, "Raccourcis clavier disponibles :");
  ligne++;

  attron(A_BOLD);
  mvprintw(ligne++, 5, "Navigation :");
  attroff(A_BOLD);
  mvprintw(ligne++, 8, "Fleches Haut/Bas    - Deplacer la selection");
  mvprintw(ligne++, 8, "Page Up/Down        - Navigation rapide");
  ligne++;

  attron(A_BOLD);
  mvprintw(ligne++, 5, "Actions sur les processus :");
  attroff(A_BOLD);
  mvprintw(ligne++, 8, "F5 ou p             - Mettre en pause (SIGSTOP)");
  mvprintw(ligne++, 8, "F6 ou k             - Arreter (SIGTERM)");
  mvprintw(ligne++, 8, "F7 ou 9             - Tuer (SIGKILL)");
  mvprintw(ligne++, 8, "F8 ou c             - Reprendre/Redemarrer (SIGCONT)");
  ligne++;

  attron(A_BOLD);
  mvprintw(ligne++, 5, "Autres :");
  attroff(A_BOLD);
  mvprintw(ligne++, 8, "F1 ou h             - Afficher cette aide");
  mvprintw(ligne++, 8, "F4 ou /             - Rechercher (non implemente)");
  mvprintw(ligne++, 8, "q ou Q              - Quitter");
  ligne += 2;

  attron(COLOR_PAIR(COLOR_ERROR_MSG) | A_BOLD);
  mvprintw(ligne++, 5, "Note: Les actions necessitent des droits root (sudo)");
  attroff(COLOR_PAIR(COLOR_ERROR_MSG) | A_BOLD);
  ligne += 2;

  attron(COLOR_PAIR(COLOR_HELP_BAR) | A_BOLD);
  mvprintw(LINES - 2, 0, "%*s", COLS, "");
  mvprintw(LINES - 2, (COLS - 40) / 2, "Appuyez sur une touche pour revenir");
  attroff(COLOR_PAIR(COLOR_HELP_BAR) | A_BOLD);

  refresh();

  // Attends une touche
  timeout(-1); // Bloquant
  getch();
  timeout(REFRESH_TIMEOUT); // Remettre en non-bloquant
}

void ui_afficher_processus(processus_t *head, ui_state_t *state) {
  processus_t *courant = head;
  int ligne = 0;
  int nb_processus = compter_processus(head);

  // Récupération des infos système
  struct sysinfo si;
  sysinfo(&si);

  // Ajout d'une foncionnalité affichant l'heure
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  char time_str[32];
  strftime(time_str, sizeof(time_str), "%H:%M:%S", t);

  /* 1. Barre de titre */
  attron(COLOR_PAIR(COLOR_HEADER) | A_BOLD);
  mvprintw(ligne, 0, "%*s", COLS, "");
  mvprintw(ligne, 2, "MY_HTOP - Moniteur de Processus Local");
  mvprintw(ligne, COLS - 12, "%s", time_str);
  attroff(COLOR_PAIR(COLOR_HEADER) | A_BOLD);
  ligne++;

  /* 2. Statistiques système */
  mvprintw(ligne++, 2,
           "Processus actifs: %d | Uptime: %ld min | Memoire libre: %.1f MB",
           nb_processus, si.uptime / 60, (float)si.freeram / (1024 * 1024));
  ligne++;

  /* 3. En-tête du tableau */
  attron(COLOR_PAIR(COLOR_TABLE_HEADER) | A_BOLD);
  mvprintw(ligne, 0, "%-8s %-12s %-6s %-10s %-10s %-10s %s", "PID", "USER",
           "STATE", "CPU%", "MEM(RSS)", "TIME", "COMMAND");
  attroff(COLOR_PAIR(COLOR_TABLE_HEADER) | A_BOLD);
  ligne++;

  /* 4. Ligne de séparation */
  int i;
  for (i = 0; i < COLS; i++) {
    mvaddch(ligne, i, '-');
  }
  ligne++;

  /* 5. Affichage des processus */
  int max_lignes_affichage = LINES - ligne - 3;
  int index = 0;
  int lignes_affichees = 0;

  for (i = 0; i < state->scroll_offset && courant != NULL; i++) {
    courant = courant->suivant;
    index++;
  }

  // Afficher les processus visibles
  while (courant != NULL && lignes_affichees < max_lignes_affichage) {
    // Mise en surbrillance du processus sélectionné
    if (index == state->selected_index) {
      attron(COLOR_PAIR(COLOR_SELECTED) | A_BOLD);
      mvprintw(ligne, 0, ">");
    } else {
      mvprintw(ligne, 0, " ");
    }

    // Conversion de la mémoire RSS en MB
    float mem_mb = (float)(courant->rss_size * 4096) / (1024 * 1024);

    // Calcul du temps total
    long long total_time =
        (courant->utime + courant->stime) / sysconf(_SC_CLK_TCK);

    mvprintw(ligne, 1, "%-8d %-12s %-6c %-10.1f %-10.1f %-10lld %s",
             courant->pid, courant->utilisateur, courant->etat,
             courant->cpu_percent, mem_mb, total_time, courant->nom_commande);

    if (index == state->selected_index) {
      attroff(COLOR_PAIR(COLOR_SELECTED) | A_BOLD);
    }

    courant = courant->suivant;
    ligne++;
    index++;
    lignes_affichees++;
  }

  /* 6. Barre d'aide en bas */
  attron(COLOR_PAIR(COLOR_HELP_BAR) | A_BOLD);
  mvprintw(LINES - 2, 0, "%*s", COLS, "");
  mvprintw(LINES - 2, 2,
           "F1:Aide F5:Pause F6:Kill F7:ForceKill F8:Continue Q:Quit");
  attroff(COLOR_PAIR(COLOR_HELP_BAR) | A_BOLD);

  /* 7. Ligne d'information et messages */
  mvprintw(LINES - 1, 2, "Processus %d/%d", state->selected_index + 1,
           nb_processus);

  // Affichage du message si présent et pas expiré
  if (state->message_buffer[0] != '\0') {
    time_t current = time(NULL);
    double elapsed = difftime(current, state->message_time);

    // Afficher le message seulement s'il n'a pas expiré
    if (elapsed < MESSAGE_DISPLAY_DURATION) {
      if (state->message_type == 0) {
        attron(COLOR_PAIR(COLOR_INFO_MSG) | A_BOLD);
      } else {
        attron(COLOR_PAIR(COLOR_ERROR_MSG) | A_BOLD);
      }
      mvprintw(LINES - 1, 25, "%s", state->message_buffer);
      if (state->message_type == 0) {
        attroff(COLOR_PAIR(COLOR_INFO_MSG) | A_BOLD);
      } else {
        attroff(COLOR_PAIR(COLOR_ERROR_MSG) | A_BOLD);
      }
    } else {
      // Le message a expiré, l'effacer
      state->message_buffer[0] = '\0';
    }
  }
}

int ui_demander_saisie(ui_state_t *state, const char *prompt, char *buffer,
                       int max_len) {
  int ret = 0;

  /* Effacer la ligne de message */
  move(LINES - 1, 0);
  clrtoeol();

  /* Afficher le prompt */
  attron(COLOR_PAIR(COLOR_INFO_MSG) | A_BOLD);
  mvprintw(LINES - 1, 2, "%s", prompt);
  attroff(COLOR_PAIR(COLOR_INFO_MSG) | A_BOLD);
  refresh();

  /* Activer l'écho et le curseur temporairement */
  echo();
  curs_set(1);

  /* Saisie */
  if (getnstr(buffer, max_len - 1) == OK) {
    if (strlen(buffer) > 0) {
      ret = 1;
    }
  }

  /* Restaurer l'état */
  noecho();
  curs_set(0);

  return ret;
}

int ui_gerer_evenements(ui_state_t *state, int nb_processus) {
  int key_input = getch();
  int max_visible = LINES - 8;

  if (key_input == ERR) {
    return ACTION_CONTINUE;
  }

  switch (key_input) {
  case 'q':
  case 'Q':
    return ACTION_QUIT;

  case KEY_F(1):
  case 'h':
  case 'H':
    return ACTION_HELP;

  case KEY_F(4):
  case '/':
    return ACTION_SEARCH;

  case KEY_F(5):
  case 'p':
  case 'P':
    return ACTION_PAUSE;

  case KEY_F(6):
  case 'k':
  case 'K':
    return ACTION_KILL;

  case KEY_F(7):
  case '9':
    return ACTION_FORCE_KILL;

  case KEY_F(8):
  case 'c':
  case 'C':
    return ACTION_CONTINUE_SIGNAL;

  case KEY_UP:
    if (state->selected_index > 0) {
      state->selected_index--;
      if (state->selected_index < state->scroll_offset) {
        state->scroll_offset--;
      }
    }
    return ACTION_CONTINUE;

  case KEY_DOWN:
    if (state->selected_index < nb_processus - 1) {
      state->selected_index++;
      if (state->selected_index >= state->scroll_offset + max_visible) {
        state->scroll_offset++;
      }
    }
    return ACTION_CONTINUE;

  case KEY_PPAGE:
    state->selected_index -= max_visible;
    if (state->selected_index < 0) {
      state->selected_index = 0;
    }
    state->scroll_offset = state->selected_index;
    return ACTION_CONTINUE;

  case KEY_NPAGE:
    state->selected_index += max_visible;
    if (state->selected_index >= nb_processus) {
      state->selected_index = nb_processus - 1;
    }
    if (state->selected_index >= state->scroll_offset + max_visible) {
      state->scroll_offset = state->selected_index - max_visible + 1;
    }
    return ACTION_CONTINUE;

  case KEY_F(2):
    return ACTION_NEXT_TAB;

  case KEY_F(3):
    return ACTION_PREV_TAB;

  default:
    return ACTION_CONTINUE;
  }
}

void ui_afficher_processus_network(machine_info_t *machines, int nb_machines,
                                   int machine_courante, ui_state_t *state) {
  int ligne = 0;
  processus_t *head = machines[machine_courante].liste_processus;
  int nb_processus = compter_processus(head);

  /* 1. Afficher les onglets des machines */
  attron(COLOR_PAIR(COLOR_HEADER) | A_BOLD);
  mvprintw(ligne, 0, "%*s", COLS, "");
  int tab_x = 2;
  for (int i = 0; i < nb_machines; i++) {
    if (i == machine_courante) {
      attron(A_REVERSE);
    }
    mvprintw(ligne, tab_x, " %s ", machines[i].nom);
    if (i == machine_courante) {
      attroff(A_REVERSE);
    }
    tab_x += strlen(machines[i].nom) + 3;
  }
  attroff(COLOR_PAIR(COLOR_HEADER) | A_BOLD);
  ligne++;

  /* 2. Informations de la machine courante */
  mvprintw(ligne++, 2, "Machine: %s | Processus actifs: %d",
           machines[machine_courante].nom, nb_processus);
  ligne++;

  /* 3. En-tête du tableau */
  attron(COLOR_PAIR(COLOR_TABLE_HEADER) | A_BOLD);
  mvprintw(ligne, 0, "%-8s %-12s %-6s %-10s %-10s %-10s %s", "PID", "USER",
           "STATE", "CPU%", "MEM(RSS)", "TIME", "COMMAND");
  attroff(COLOR_PAIR(COLOR_TABLE_HEADER) | A_BOLD);
  ligne++;

  /* 4. Ligne de séparation */
  for (int i = 0; i < COLS; i++) {
    mvaddch(ligne, i, '-');
  }
  ligne++;

  /* 5. Affichage des processus */
  int max_lignes_affichage = LINES - ligne - 3;
  int index = 0;
  int lignes_affichees = 0;
  processus_t *courant = head;

  /* Sauter au scroll offset */
  for (int i = 0; i < state->scroll_offset && courant != NULL; i++) {
    courant = courant->suivant;
    index++;
  }

  /* Afficher les processus visibles */
  while (courant != NULL && lignes_affichees < max_lignes_affichage) {
    /* Mise en surbrillance du processus sélectionné */
    if (index == state->selected_index) {
      attron(COLOR_PAIR(COLOR_SELECTED) | A_BOLD);
      mvprintw(ligne, 0, ">");
    } else {
      mvprintw(ligne, 0, " ");
    }

    /* Conversion de la mémoire RSS en MB */
    float mem_mb = (float)(courant->rss_size * 4096) / (1024 * 1024);

    /* Calcul du temps total */
    long long total_time =
        (courant->utime + courant->stime) / sysconf(_SC_CLK_TCK);

    mvprintw(ligne, 1, "%-8d %-12s %-6c %-10.1f %-10.1f %-10lld %s",
             courant->pid, courant->utilisateur, courant->etat,
             courant->cpu_percent, mem_mb, total_time, courant->nom_commande);

    if (index == state->selected_index) {
      attroff(COLOR_PAIR(COLOR_SELECTED) | A_BOLD);
    }

    courant = courant->suivant;
    ligne++;
    index++;
    lignes_affichees++;
  }

  /* 6. Barre d'aide en bas */
  attron(COLOR_PAIR(COLOR_HELP_BAR) | A_BOLD);
  mvprintw(LINES - 2, 0, "%*s", COLS, "");
  mvprintw(
      LINES - 2, 2,
      "F1:Aide F2/F3:Onglets F5:Pause F6:Kill F7:ForceKill F8:Continue Q:Quit");
  attroff(COLOR_PAIR(COLOR_HELP_BAR) | A_BOLD);

  /* 7. Ligne d'information et messages */
  mvprintw(LINES - 1, 2, "Processus %d/%d", state->selected_index + 1,
           nb_processus);

  /* Affichage du message si présent et pas expiré */
  if (state->message_buffer[0] != '\0') {
    time_t current = time(NULL);
    double elapsed = difftime(current, state->message_time);

    if (elapsed < MESSAGE_DISPLAY_DURATION) {
      if (state->message_type == 0) {
        attron(COLOR_PAIR(COLOR_INFO_MSG) | A_BOLD);
      } else {
        attron(COLOR_PAIR(COLOR_ERROR_MSG) | A_BOLD);
      }
      mvprintw(LINES - 1, 25, "%s", state->message_buffer);
      if (state->message_type == 0) {
        attroff(COLOR_PAIR(COLOR_INFO_MSG) | A_BOLD);
      } else {
        attroff(COLOR_PAIR(COLOR_ERROR_MSG) | A_BOLD);
      }
    } else {
      state->message_buffer[0] = '\0';
    }
  }
}