#ifndef MANAGER_H
#define MANAGER_H

/**
 * Point d'entrée "métier" du programme.
 * Pour l'instant, on ne gère que le mode local.
 * Plus tard, c'est ici qu'on pourra brancher les options -c, -s, --all, etc.
 */
int manager_run(int argc, char *argv[]);

#endif /* MANAGER_H */
