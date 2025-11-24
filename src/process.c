#define _GNU_SOURCE  /* Active popen, pclose, etc. */

#include "process.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>

/* --------------------------------------------------------------------------
   Duplique une chaîne (équivalent de strdup mais compatible C11 strict)
   -------------------------------------------------------------------------- */
static char *str_dup(const char *s)
{
    size_t len;
    char *copy;

    if (s == NULL) {
        return NULL;
    }

    len = strlen(s);
    copy = malloc(len + 1);
    if (copy == NULL) {
        return NULL;
    }

    memcpy(copy, s, len + 1);
    return copy;
}

/* --------------------------------------------------------------------------
   get_local_process_list :
   Récupère la liste des processus locaux via `ps`
   -------------------------------------------------------------------------- */
process_list_t *get_local_process_list(void)
{
    FILE *fp;
    char line[1024];
    process_list_t *plist;
    size_t capacity = 64;
    size_t count = 0;

    fp = popen("ps -eo pid,user,pcpu,pmem,etimes,comm --no-headers", "r");
    if (fp == NULL) {
        log_error("Impossible d'exécuter la commande ps");
        return NULL;
    }

    plist = malloc(sizeof(process_list_t));
    if (plist == NULL) {
        pclose(fp);
        return NULL;
    }

    plist->items = calloc(capacity, sizeof(process_info_t));
    if (plist->items == NULL) {
        free(plist);
        pclose(fp);
        return NULL;
    }

    while (fgets(line, sizeof(line), fp) != NULL) {
        process_info_t *p;
        int pid;
        char user[64];
        double cpu;
        double mem;
        long etimes;
        char command[256];

        if (sscanf(line, "%d %63s %lf %lf %ld %255[^\n]",
                   &pid, user, &cpu, &mem, &etimes, command) != 6) {
            continue;
        }

        if (count == capacity) {
            size_t new_capacity = capacity * 2;
            process_info_t *new_items =
                realloc(plist->items, new_capacity * sizeof(process_info_t));

            if (new_items == NULL) {
                log_error("Allocation mémoire échouée lors de la lecture des processus");
                break;
            }

            plist->items = new_items;
            capacity = new_capacity;
        }

        p = &plist->items[count];
        p->pid = pid;
        p->user = str_dup(user);
        p->cpu_usage = cpu;
        p->mem_usage = mem;
        p->elapsed_time = etimes;
        p->command = str_dup(command);
        p->state = NULL;
        p->host = NULL;

        count++;
    }

    pclose(fp);
    plist->size = count;
    return plist;
}

/* --------------------------------------------------------------------------
   Libération de la liste
   -------------------------------------------------------------------------- */
void free_process_list(process_list_t *plist)
{
    if (plist == NULL) {
        return;
    }

    if (plist->items != NULL) {
        for (size_t i = 0; i < plist->size; i++) {
            free(plist->items[i].user);
            free(plist->items[i].command);
            free(plist->items[i].state);
        }
        free(plist->items);
    }

    free(plist);
}

/* --------------------------------------------------------------------------
   Actions sur les processus (signaux)
   -------------------------------------------------------------------------- */

int pause_process(const process_info_t *proc)
{
    if (proc == NULL) {
        return -1;
    }
    return kill(proc->pid, SIGSTOP);
}

int resume_process(const process_info_t *proc)
{
    if (proc == NULL) {
        return -1;
    }
    return kill(proc->pid, SIGCONT);
}

int stop_process(const process_info_t *proc)
{
    if (proc == NULL) {
        return -1;
    }
    return kill(proc->pid, SIGTERM);
}

int kill_process_force(const process_info_t *proc)
{
    if (proc == NULL) {
        return -1;
    }
    return kill(proc->pid, SIGKILL);
}
