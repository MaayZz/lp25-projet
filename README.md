Projet LP25 – Moniteur de processus

## Description

Implémentation d’un **moniteur de processus** inspiré de `htop` pour systèmes **Linux**.  
Cette version implémente pour l’instant le **mode local uniquement**.

---

## Architecture modulaire

Le projet est organisé en plusieurs modules :

lp25-projet/
├── src/
│   ├── main.c         # Point d'entrée et gestion des arguments
│   ├── manager.c/h    # Orchestration de tous les modules
│   ├── process.c/h    # Gestion des processus Linux (/proc)
│   ├── ui.c/h         # Interface utilisateur (ncurses)
├── Makefile           # Compilation
└── README.md          # Cette documentation

---

## Compilation 

# Compilation standard
make

# Recompilation complète
make re

# Nettoyage des fichiers objets
make clean

# Nettoyage complet (objets + exécutable)
make fclean

---

## Utilisation 

# Mode normal (affichage et interaction)
./my_htop_local

# Avec droits root (recommandé pour kill/pause/continue)
sudo ./my_htop_local

# Afficher l'aide
./my_htop_local --help

# Test d'accès aux processus (mode dry-run)
./my_htop_local --dry-run

---

## Test

# Test de fuites mémoire (selon cible Makefile)
make valgrind

# Test d'accès aux processus (dry-run)
make test-dry-run

---

## Dépendances

sudo apt-get install build-essential libncurses5-dev libncurses-dev

---

## Auteurs

MELLOUK Mohamed
ISLAM Abir
FALLANI Issam
