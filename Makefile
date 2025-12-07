# Makefile pour le projet LP25 - Moniteur de processus
# Auteur: Groupe LP25
# Description: Compilation modulaire du projet

CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g -O2
LIBS = -lncurses
TARGET = my_htop_local

# Fichiers sources et objets
SRCS = main.c manager.c process.c ui.c
OBJS = $(SRCS:.c=.o)
HEADERS = manager.h process.h ui.h

# Règle par défaut
all: $(TARGET)

# Règle pour lier l'exécutable
$(TARGET): $(OBJS)
	@echo "Linking $(TARGET)..."
	$(CC) $(OBJS) $(LIBS) -o $(TARGET)
	@echo "Compilation reussie!"
	@echo "Executez avec: ./$(TARGET) ou sudo ./$(TARGET)"

# Règle pour compiler les fichiers sources
%.o: %.c $(HEADERS)
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) -c $< -o $@

# Nettoyage des fichiers objets
clean:
	@echo "Nettoyage des fichiers objets..."
	rm -f $(OBJS)

# Nettoyage complet
fclean: clean
	@echo "Suppression de l'executable..."
	rm -f $(TARGET)

# Recompilation complète
re: fclean all

# Test avec dry-run
test-dry-run: $(TARGET)
	@echo "Test en mode dry-run..."
	./$(TARGET) --dry-run

# Lancement normal
run: $(TARGET)
	@echo "Lancement de $(TARGET)..."
	@echo "Note: Utilisez sudo pour les actions kill/pause/continue"
	./$(TARGET)

# Lancement avec sudo
run-sudo: $(TARGET)
	@echo "Lancement de $(TARGET) avec droits root..."
	sudo ./$(TARGET)

# Vérification des fuites mémoire avec valgrind
valgrind: $(TARGET)
	@echo "Verification des fuites memoire avec valgrind..."
	@echo "Appuyez sur 'q' rapidement pour quitter"
	valgrind --leak-check=full --show-leak-kinds=all ./$(TARGET)

# Aide
help:
	@echo "=========================================="
	@echo "Makefile du projet LP25"
	@echo "=========================================="
	@echo ""
	@echo "Cibles disponibles:"
	@echo "  make              - Compile le projet"
	@echo "  make all          - Identique a make"
	@echo "  make clean        - Supprime les fichiers objets"
	@echo "  make fclean       - Supprime tout (objets + executable)"
	@echo "  make re           - Recompile depuis zero"
	@echo "  make run          - Compile et lance le programme"
	@echo "  make run-sudo     - Compile et lance avec sudo"
	@echo "  make test-dry-run - Test l'acces aux processus"
	@echo "  make valgrind     - Verifie les fuites memoire"
	@echo "  make help         - Affiche cette aide"
	@echo ""
	@echo "Structure du projet:"
	@echo "  main.c     - Point d'entree et gestion des arguments"
	@echo "  manager.c  - Orchestration et logique metier"
	@echo "  process.c  - Gestion des processus Linux"
	@echo "  ui.c       - Interface utilisateur avec ncurses"
	@echo ""

.PHONY: all clean fclean re test-dry-run run run-sudo valgrind help