Projet LP25 – Moniteur de processus
====================================

## Description

Moniteur de processus inspiré de `htop` pour systèmes Linux.
Modes local et réseau avec support SSH.

## Compilation

```bash
make
```

**Dépendances** : `build-essential libncurses5-dev libssh-dev`

## Utilisation

### Mode local
```bash
./my_htop                    # Affichage normal
sudo ./my_htop               # Avec droits root (pour kill/pause)
./my_htop --help             # Aide
./my_htop --dry-run          # Test d'accès aux processus
```

### Mode réseau

Fichier de configuration `.config` (format: `nom:ip:port:user:pass:ssh`) :
```bash
chmod 600 .config            # OBLIGATOIRE - permissions 600
./my_htop -c .config         # Avec fichier de config
./my_htop -c .config -a      # Local + distant
```

Connexion unique :
```bash
./my_htop -s 192.168.1.100   # Demande user/pass interactivement
./my_htop -l user@host       # Format login
```

## Raccourcis clavier

- **F1/h** : Aide
- **F2/F3** : Onglet suivant/précédent (mode réseau)
- **F4/** : Rechercher
- **F5/p** : Pause (SIGSTOP)
- **F6/k** : Arrêter (SIGTERM)
- **F7/9** : Tuer (SIGKILL)
- **F8/c** : Reprendre (SIGCONT)
- **↑↓** : Navigation
- **PgUp/PgDn** : Navigation rapide
- **q/Q** : Quitter

## Options

```
-h, --help                     Affiche l'aide
--dry-run                      Test l'accès aux processus
-c, --remote-config <file>     Fichier de configuration
-s, --remote-server <host>     Serveur distant
-l, --login <user@host>        Format login
-u, --username <user>          Nom d'utilisateur
-p, --password <pass>          Mot de passe
-t, --connexion-type <type>    Type: ssh (défaut)
-P, --port <port>              Port de connexion
-a, --all                      Local + distant
```

## Structure

```
src/
├── main.c       - Point d'entrée et parsing arguments
├── manager.c/h  - Orchestration multi-machines
├── process.c/h  - Gestion processus Linux (/proc)
├── network.c/h  - Connexions SSH et hôtes distants
└── ui.c/h       - Interface ncurses avec onglets
```

## Auteurs

- ISLAM Abir
- MELLOUK Mohamed-Amine
- FALLANI Issam
