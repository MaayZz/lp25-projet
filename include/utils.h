#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>

/**
 * @brief Affiche un message d'erreur formaté sur stderr.
 */
void log_error(const char *fmt, ...);

/**
 * @brief Affiche un message d'information (debug, etc.).
 */
void log_info(const char *fmt, ...);

#endif /* UTILS_H */
