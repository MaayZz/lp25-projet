#include "manager.h"
#include "utils.h"

#include <stdlib.h>

int main(int argc, char **argv)
{
    app_config_t config;
    int ret;

    init_app_config(&config);

    ret = parse_arguments(argc, argv, &config);
    if (ret != 0) {
        log_error("Erreur lors de l'analyse des arguments");
        free_app_config(&config);
        return EXIT_FAILURE;
    }

    if (config.show_help) {
        print_help(argv[0]);
        free_app_config(&config);
        return EXIT_SUCCESS;
    }

    if (config.dry_run) {
        ret = run_dry_run(&config);
        free_app_config(&config);
        return (ret == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
    }

    ret = manager_run(&config);

    free_app_config(&config);
    return (ret == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
