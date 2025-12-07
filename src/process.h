/**
 * @file process.h
 * @brief Module de gestion des processus sur une machine Linux
 * @author Groupe LP25
 * 
 * Ce module implémente les fonctionnalités de lecture, manipulation
 * et gestion des processus via le système de fichiers /proc.
 */

#ifndef PROCESS_H
#define PROCESS_H

#include <sys/types.h>

/* Constantes */
#define MAX_CMD_LEN 256
#define MAX_USER_LEN 32
#define PROC_DIR "/proc"

/**
 * @brief Structure représentant un processus
 */
typedef struct processus {
    pid_t pid;
    char nom_commande[MAX_CMD_LEN];
    char utilisateur[MAX_USER_LEN];
    char etat;
    long long utime;
    long long stime;
    long vmem_size;
    long rss_size;
    float cpu_percent;
    struct processus *suivant;
} processus_t;

/* Prototypes des fonctions publiques */

/**
 * @brief Parcourt /proc et construit la liste chaînée des processus actifs.
 * @return processus_t* : Pointeur vers le début de la liste, ou NULL si erreur.
 */
processus_t *recuperer_processus_locaux(void);

/**
 * @brief Libère la mémoire allouée pour la liste de processus.
 * @param head : Pointeur vers le premier élément de la liste.
 */
void liberer_liste_processus(processus_t *head);

/**
 * @brief Compte le nombre de processus dans la liste.
 * @param head : Pointeur vers le début de la liste.
 * @return int : Nombre de processus.
 */
int compter_processus(processus_t *head);

/**
 * @brief Récupère le processus à l'index donné.
 * @param head : Pointeur vers le début de la liste.
 * @param index : Index du processus recherché.
 * @return processus_t* : Pointeur vers le processus, ou NULL si non trouvé.
 */
processus_t *get_processus_at_index(processus_t *head, int index);

/**
 * @brief Envoie un signal à un processus.
 * @param pid : PID du processus cible.
 * @param signal : Signal à envoyer (SIGTERM, SIGSTOP, etc.).
 * @return int : 0 en cas de succès, -1 en cas d'erreur.
 */
int envoyer_signal(pid_t pid, int signal);

#endif /* PROCESS_H */