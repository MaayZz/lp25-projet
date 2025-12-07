#ifndef PROCESS_H
#define PROCESS_H

#include <sys/types.h>

#define MAX_CMD_LEN   256
#define MAX_USER_LEN  32
#define PROC_DIR      "/proc"

/**
 * Structure représentant les informations d'un processus.
 * On ne stocke que ce qui est utile pour l'affichage et les actions.
 */
typedef struct Processus {
    pid_t pid;
    char nom_commande[MAX_CMD_LEN];
    char utilisateur[MAX_USER_LEN];
    char etat;
    long long utime;
    long long stime;
    long vmem_size;
    long rss_size;
    float cpu_percent;
    struct Processus *suivant;
} Processus_t;

/**
 * Parcourt /proc et construit la liste chaînée des processus actifs.
 * Retourne NULL en cas d'erreur (ex: pas /proc, pas Linux).
 */
Processus_t *creer_liste_processus_locaux(void);

/**
 * Libère toute la mémoire associée à la liste de processus.
 */
void liberer_liste_processus(Processus_t *head);

/**
 * Compte le nombre de processus dans la liste.
 */
int compter_processus(Processus_t *head);

/**
 * Récupère le processus à l'index donné dans la liste.
 * Retourne NULL si l'index est hors de la liste.
 */
Processus_t *get_processus_at_index(Processus_t *head, int index);

/**
 * Envoie un signal système à un processus donné.
 * Retourne 0 en cas de succès, -1 sinon (errno rempli).
 */
int envoyer_signal_a_processus(pid_t pid, int signal);

#endif /* PROCESS_H */
