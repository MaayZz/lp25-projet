#ifndef PROCESS_H
#define PROCESS_H

#include "config.h"
#include <stddef.h>

typedef struct process_info_s {
    int          pid;
    char        *user;
    double       cpu_usage;
    double       mem_usage;
    long         elapsed_time;
    char        *command;
    char        *state;
    host_info_t *host;
} process_info_t;

typedef struct process_list_s {
    process_info_t *items;
    size_t          size;
} process_list_t;

/**
 * @brief Récupère la liste des processus sur la machine locale.
 *
 * Implémentation initiale possible via "ps" + popen().
 */
process_list_t *get_local_process_list(void);

/**
 * @brief Libère la mémoire associée à une liste de processus.
 */
void free_process_list(process_list_t *plist);


int pause_process(const process_info_t *proc);
int resume_process(const process_info_t *proc);
int stop_process(const process_info_t *proc);
int kill_process_force(const process_info_t *proc);

#endif /* PROCESS_H */
