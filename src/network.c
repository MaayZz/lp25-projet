#include "network.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int test_remote_connection(const host_info_t *host)
{
    if (host == NULL) {
        return -1;
    }


    log_info("Test de connexion vers '%s' (%s:%d) via %s (non implémenté)",
             host->name ? host->name : "(sans nom)",
             host->address ? host->address : "(sans adresse)",
             host->port,
             host->type == HOST_REMOTE_SSH ? "ssh" : "telnet");

    return 0;
}

process_list_t *get_remote_process_list(const host_info_t *host)
{
    (void)host;

    log_error("get_remote_process_list: non implémenté pour le moment");
    return NULL;
}
