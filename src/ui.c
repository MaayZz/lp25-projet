#include "ui.h"
#include "utils.h"

#include <ncurses.h>

int ui_init(void)
{
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    nodelay(stdscr, FALSE);
    return 0;
}

void ui_shutdown(void)
{
    endwin();
}

static void draw_simple_view(host_process_view_t *view)
{
    size_t i;

    clear();
    mvprintw(0, 0, "LP25 Process Manager - Vue simple (local seulement pour l'instant)");

    if (view == NULL || view->plist == NULL) {
        mvprintw(2, 0, "Aucun processus à afficher.");
        refresh();
        return;
    }

    mvprintw(2, 0, "PID      USER       %%CPU   %%MEM   ELAPSED   COMMAND");
    for (i = 0; i < view->plist->size && i < 20; i++) {
        process_info_t *p;

        p = &view->plist->items[i];
        mvprintw(3 + (int)i, 0, "%-8d %-10s %6.2f %6.2f %8ld   %.32s",
                 p->pid,
                 p->user ? p->user : "?",
                 p->cpu_usage,
                 p->mem_usage,
                 p->elapsed_time,
                 p->command ? p->command : "?");
    }

    mvprintw(25, 0, "Appuyez sur 'q' pour quitter.");
    refresh();
}

int ui_main_loop(host_process_view_t *views, size_t nb_hosts)
{
    int ch;

    (void)nb_hosts;

    // mode local pour l'instant
    draw_simple_view(nb_hosts > 0 ? &views[0] : NULL);

    while (1) {
        ch = getch();
        if (ch == 'q' || ch == 'Q') {
            break;
        }
        /* À faire: plus tard: F1..F8, rafraichissement, onglets, etc. */
    }

    return 0;
}
