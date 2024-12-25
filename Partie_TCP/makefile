# make
# make clean (nettoyer tout)
# make clean_objs (nettoyer que les fichiers objets)

CC = gcc
CFLAGS = -Wall -Wextra -pedantic
SRC_SERVEUR = serveur.c
SRC_CLIENT = client.c
OBJ_SERVEUR = serveur.o
OBJ_CLIENT = client.o
EXEC_SERVEUR = serveur
EXEC_CLIENT = client

# Règle par défaut (c'est-à-dire "make")
all: $(EXEC_SERVEUR) $(EXEC_CLIENT)

# Règle pour compiler le serveur
$(EXEC_SERVEUR): $(OBJ_SERVEUR)
	$(CC) $(OBJ_SERVEUR) -o $(EXEC_SERVEUR)

# Règle pour compiler le client
$(EXEC_CLIENT): $(OBJ_CLIENT)
	$(CC) $(OBJ_CLIENT) -o $(EXEC_CLIENT)

# Règle pour générer les fichiers objets à partir des fichiers sources
$(OBJ_SERVEUR): serveur.c
	$(CC) $(CFLAGS) -c serveur.c

$(OBJ_CLIENT): client.c
	$(CC) $(CFLAGS) -c client.c

# Règle pour nettoyer les fichiers objets et exécutables
clean:
	rm -f $(OBJ_SERVEUR) $(OBJ_CLIENT) $(EXEC_SERVEUR) $(EXEC_CLIENT)

# Règle pour nettoyer uniquement les fichiers objets
clean_objs:
	rm -f $(OBJ_SERVEUR) $(OBJ_CLIENT)