/**
 * @file network.c
 * @brief Implémentation du module de gestion réseau
 * @author Abir Islam, Mellouk Mohamed-Amine, Issam Fallani
 */

#define _DEFAULT_SOURCE

#include "network.h"
#include <errno.h>
#include <libssh/libssh.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

/* Fonctions privées */

/**
 * @brief Parse une ligne du fichier de configuration.
 * Format: nom:adresse:port:username:password:type
 */
static int parse_config_line(const char *line, remote_host_t *host) {
  char type_str[16];
  char line_copy[512];

  strncpy(line_copy, line, sizeof(line_copy) - 1);
  line_copy[sizeof(line_copy) - 1] = '\0';

  /* Parser la ligne */
  int nb_fields = sscanf(
      line_copy, "%255[^:]:%255[^:]:%d:%63[^:]:%127[^:]:%15s", host->nom,
      host->adresse, &host->port, host->username, host->password, type_str);

  if (nb_fields != 6) {
    return -1;
  }

  /* Déterminer le type de connexion */
  if (strcmp(type_str, "ssh") == 0) {
    host->type = CONN_SSH;
  } else if (strcmp(type_str, "telnet") == 0) {
    host->type = CONN_TELNET;
  } else {
    return -1;
  }

  host->session = NULL;
  return 0;
}

/**
 * @brief Exécute une commande SSH et retourne la sortie.
 */
static char *execute_ssh_command(ssh_session session, const char *command) {
  ssh_channel channel;
  int rc;
  char buffer[4096];
  int nbytes;
  char *output = NULL;
  int output_size = 0;

  channel = ssh_channel_new(session);
  if (channel == NULL) {
    return NULL;
  }

  rc = ssh_channel_open_session(channel);
  if (rc != SSH_OK) {
    ssh_channel_free(channel);
    return NULL;
  }

  rc = ssh_channel_request_exec(channel, command);
  if (rc != SSH_OK) {
    ssh_channel_close(channel);
    ssh_channel_free(channel);
    return NULL;
  }

  /* Lire la sortie */
  while ((nbytes = ssh_channel_read(channel, buffer, sizeof(buffer) - 1, 0)) >
         0) {
    buffer[nbytes] = '\0';

    char *new_output = realloc(output, output_size + nbytes + 1);
    if (new_output == NULL) {
      free(output);
      ssh_channel_close(channel);
      ssh_channel_free(channel);
      return NULL;
    }

    output = new_output;
    if (output_size == 0) {
      output[0] = '\0';
    }
    strcat(output, buffer);
    output_size += nbytes;
  }

  ssh_channel_send_eof(channel);
  ssh_channel_close(channel);
  ssh_channel_free(channel);

  return output;
}

/**
 * @brief Parse la sortie de 'ps aux' et construit une liste de processus.
 */
static processus_t *parse_ps_output(const char *output) {
  processus_t *head = NULL;
  processus_t *current = NULL;
  char *output_copy = strdup(output);
  char *line = strtok(output_copy, "\n");
  int first_line = 1;

  while (line != NULL) {
    /* Ignorer la ligne d'en-tête */
    if (first_line) {
      first_line = 0;
      line = strtok(NULL, "\n");
      continue;
    }

    processus_t *proc = malloc(sizeof(processus_t));
    if (proc == NULL) {
      free(output_copy);
      liberer_liste_processus(head);
      return NULL;
    }

    /* Parser la ligne: USER PID %CPU %MEM VSZ RSS TTY STAT START TIME COMMAND
     */
    float cpu, mem;
    long vsz, rss;
    char stat[16];

    int nb = sscanf(line, "%31s %d %f %f %ld %ld %*s %15s %*s %*s %255[^\n]",
                    proc->utilisateur, &proc->pid, &cpu, &mem, &vsz, &rss, stat,
                    proc->nom_commande);

    if (nb >= 7) {
      proc->cpu_percent = cpu;
      proc->vmem_size = vsz;
      proc->rss_size = rss;
      proc->etat = stat[0];
      proc->utime = 0;
      proc->stime = 0;
      proc->suivant = NULL;

      /* Ajouter à la liste */
      if (head == NULL) {
        head = proc;
        current = proc;
      } else {
        current->suivant = proc;
        current = proc;
      }
    } else {
      free(proc);
    }

    line = strtok(NULL, "\n");
  }

  free(output_copy);
  return head;
}

/* Fonctions publiques */

void init_network_config(network_config_t *config) {
  config->nb_hosts = 0;
  for (int i = 0; i < MAX_HOSTS; i++) {
    config->hosts[i].session = NULL;
  }
}

int check_config_file_permissions(const char *filename) {
  struct stat st;

  if (stat(filename, &st) != 0) {
    fprintf(stderr, "ERREUR: Impossible d'accéder au fichier %s\n", filename);
    return -1;
  }

  /* Vérifier permissions (doit être 600 = rw-------) */
  mode_t perms = st.st_mode & 0777;
  if (perms != 0600) {
    fprintf(stderr, "\n");
    fprintf(stderr, "=================================================\n");
    fprintf(stderr, "  AVERTISSEMENT SÉCURITÉ\n");
    fprintf(stderr, "=================================================\n");
    fprintf(stderr, "Le fichier de configuration contient des mots de passe\n");
    fprintf(stderr, "et n'a pas les permissions correctes.\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "Permissions actuelles: %03o\n", perms);
    fprintf(stderr, "Permissions attendues: 600 (rw-------)\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "Corrigez avec: chmod 600 %s\n", filename);
    fprintf(stderr, "=================================================\n");
    fprintf(stderr, "\n");
    return -1;
  }

  return 0;
}

int parse_config_file(const char *filename, network_config_t *config) {
  FILE *file;
  char line[512];

  file = fopen(filename, "r");
  if (file == NULL) {
    fprintf(stderr, "ERREUR: Impossible d'ouvrir le fichier %s: %s\n", filename,
            strerror(errno));
    return -1;
  }

  init_network_config(config);

  while (fgets(line, sizeof(line), file) != NULL &&
         config->nb_hosts < MAX_HOSTS) {
    /* Ignorer lignes vides et commentaires */
    if (line[0] == '\n' || line[0] == '#') {
      continue;
    }

    /* Retirer le saut de ligne */
    line[strcspn(line, "\n")] = '\0';

    if (parse_config_line(line, &config->hosts[config->nb_hosts]) == 0) {
      config->nb_hosts++;
    } else {
      fprintf(stderr, "AVERTISSEMENT: Ligne invalide ignorée: %s\n", line);
    }
  }

  fclose(file);

  if (config->nb_hosts == 0) {
    fprintf(stderr, "ERREUR: Aucun hôte valide trouvé dans %s\n", filename);
    return -1;
  }

  return 0;
}

int connect_ssh(remote_host_t *host) {
  int rc;

  /* Créer la session SSH */
  host->session = ssh_new();
  if (host->session == NULL) {
    fprintf(stderr, "ERREUR: Impossible de créer la session SSH\n");
    return -1;
  }

  /* Configurer la session */
  ssh_options_set(host->session, SSH_OPTIONS_HOST, host->adresse);
  ssh_options_set(host->session, SSH_OPTIONS_PORT, &host->port);
  ssh_options_set(host->session, SSH_OPTIONS_USER, host->username);

  /* Connecter */
  rc = ssh_connect(host->session);
  if (rc != SSH_OK) {
    fprintf(stderr, "ERREUR: Connexion SSH échouée vers %s: %s\n",
            host->adresse, ssh_get_error(host->session));
    ssh_free(host->session);
    host->session = NULL;
    return -1;
  }

  /* Authentification par mot de passe */
  rc = ssh_userauth_password(host->session, NULL, host->password);
  if (rc != SSH_AUTH_SUCCESS) {
    fprintf(stderr, "ERREUR: Authentification échouée pour %s@%s\n",
            host->username, host->adresse);
    ssh_disconnect(host->session);
    ssh_free(host->session);
    host->session = NULL;
    return -1;
  }

  return 0;
}

void disconnect_ssh(remote_host_t *host) {
  if (host->session != NULL) {
    ssh_disconnect(host->session);
    ssh_free(host->session);
    host->session = NULL;
  }
}

processus_t *get_remote_processes(remote_host_t *host) {
  char *output;
  processus_t *liste;

  if (host->session == NULL) {
    fprintf(stderr, "ERREUR: Pas de session SSH active pour %s\n", host->nom);
    return NULL;
  }

  /* Exécuter 'ps aux' sur la machine distante */
  output = execute_ssh_command(host->session, "ps aux");
  if (output == NULL) {
    fprintf(stderr, "ERREUR: Impossible d'exécuter 'ps aux' sur %s\n",
            host->nom);
    return NULL;
  }

  /* Parser la sortie */
  liste = parse_ps_output(output);
  free(output);

  return liste;
}

int send_remote_signal(remote_host_t *host, pid_t pid, int signal) {
  char command[128];
  char *output;

  if (host->session == NULL) {
    return -1;
  }

  /* Construire la commande kill */
  snprintf(command, sizeof(command), "kill -%d %d 2>&1", signal, pid);

  /* Exécuter la commande */
  output = execute_ssh_command(host->session, command);
  if (output == NULL) {
    return -1;
  }

  /* Vérifier si erreur dans la sortie */
  int retour = 0;
  if (strstr(output, "No such process") != NULL) {
    errno = ESRCH;
    retour = -1;
  } else if (strstr(output, "Operation not permitted") != NULL) {
    errno = EPERM;
    retour = -1;
  }

  free(output);
  return retour;
}

void cleanup_network_config(network_config_t *config) {
  for (int i = 0; i < config->nb_hosts; i++) {
    disconnect_ssh(&config->hosts[i]);
  }
  config->nb_hosts = 0;
}
