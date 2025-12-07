/**
 * @file process.c
 * @brief Implémentation du module de gestion des processus
 * @author Abir Islam, Mellouk Mohamed-Amine, Issam Fallani
 */

#define _POSIX_C_SOURCE 200809L

#include "process.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <ctype.h>
#include <pwd.h>
#include <sys/stat.h>
#include <signal.h>
#include <unistd.h>


/**
 * @brief Ajoute un processus en tête de liste.
 */
static void ajouter_processus_en_tete(processus_t **head, processus_t proc_data) {
    processus_t *nouveau = (processus_t *)malloc(sizeof(processus_t));
    if (nouveau == NULL) {
        perror("Erreur d'allocation memoire");
        exit(EXIT_FAILURE);
    }
    
    *nouveau = proc_data;
    nouveau->suivant = *head;
    *head = nouveau;
}

/**
 * @brief Récupère le nom d'utilisateur propriétaire d'un processus.
 */
static void get_username_from_pid(pid_t pid, char *username, size_t size) {
    char path[128];
    struct stat st;
    struct passwd *pw;
    
    snprintf(path, sizeof(path), "/proc/%d", pid);

    if (stat(path, &st) == 0) {
        pw = getpwuid(st.st_uid);
        if (pw) {
            strncpy(username, pw->pw_name, size - 1);
            username[size - 1] = '\0';
        } else {
            snprintf(username, size, "%d", (int)st.st_uid);
        }
    } else {
        strncpy(username, "N/A", size - 1);
    }
}

/**
 * @brief Lit les informations d'un processus depuis /proc/[PID]/stat.
 */
static int lire_infos_processus(pid_t pid, processus_t *proc_data) {
    char path[256];
    FILE *file;
    
    snprintf(path, sizeof(path), "/proc/%d/stat", pid);
    file = fopen(path, "r");
    if (!file) {
        return -1;
    }

    // Lecture des champs depuis /proc/[PID]/stat
    int fields_read = fscanf(file, "%d %s %c %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u %lld %lld %*d %*d %*d %*d %*d %*d %ld %ld",
               &proc_data->pid, proc_data->nom_commande, &proc_data->etat,
               &proc_data->utime, &proc_data->stime, 
               &proc_data->vmem_size, &proc_data->rss_size);
    
    fclose(file);
    
    if (fields_read != 7) {
        return -1;
    }
    
    // Nettoyage du nom de commande (enlever les parenthèses)
    size_t len = strlen(proc_data->nom_commande);
    if (proc_data->nom_commande[0] == '(' && proc_data->nom_commande[len - 1] == ')') {
        proc_data->nom_commande[len - 1] = '\0';
        memmove(proc_data->nom_commande, proc_data->nom_commande + 1, len - 1);
    }
    
    get_username_from_pid(pid, proc_data->utilisateur, MAX_USER_LEN);
    
    // Calcul du pourcentage CPU
    long long total_time = proc_data->utime + proc_data->stime;
    proc_data->cpu_percent = (float)total_time / 100.0;
    
    return 0;
}


processus_t *recuperer_processus_locaux(void) {
    DIR *dir;
    struct dirent *entree;
    processus_t *liste_head = NULL;
    
    dir = opendir(PROC_DIR);
    if (!dir) {
        return NULL;
    }

    while ((entree = readdir(dir)) != NULL) {
        // Vérifier si l'entrée est un PID
        int est_pid = 1;
        char *p;
        for (p = entree->d_name; *p; p++) {
            if (!isdigit(*p)) {
                est_pid = 0;
                break;
            }
        }

        if (est_pid) {
            pid_t pid = (pid_t)atoi(entree->d_name);
            processus_t nouveau_proc;
            
            if (lire_infos_processus(pid, &nouveau_proc) == 0) {
                ajouter_processus_en_tete(&liste_head, nouveau_proc);
            }
        }
    }

    closedir(dir);
    return liste_head;
}

void liberer_liste_processus(processus_t *head) {
    processus_t *courant = head;
    processus_t *suivant;

    while (courant != NULL) {
        suivant = courant->suivant;
        free(courant);
        courant = suivant;
    }
}

int compter_processus(processus_t *head) {
    int count = 0;
    processus_t *courant = head;
    
    while (courant != NULL) {
        count++;
        courant = courant->suivant;
    }
    
    return count;
}

processus_t *get_processus_at_index(processus_t *head, int index) {
    processus_t *courant = head;
    int i = 0;
    
    while (courant != NULL && i < index) {
        courant = courant->suivant;
        i++;
    }
    
    return courant;
}

int envoyer_signal(pid_t pid, int signal) {
    return kill(pid, signal);
}