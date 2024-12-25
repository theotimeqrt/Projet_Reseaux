#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define PORT 8080  // le même port que le serveur
#define BUF_SIZE 1024  // taille max du buffer

// ========================== Fonctions ==========================

// fonction pour initialiser la connexion avec le serveur
int init_client(const char *ip_address) {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);  // créer un socket TCP
    if (sockfd < 0) {
        perror("erreur de création du socket");
        exit(1);
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));  // init adresse serveur à zéro
    server_addr.sin_family = AF_INET;  // utilisation IPv4
    server_addr.sin_port = htons(PORT);  // définir port serveur
    server_addr.sin_addr.s_addr = inet_addr(ip_address);  // adresse IP serveur

    // se co au serveur
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("échec de la connexion");
        exit(1);
    }

    return sockfd;
}

void send_command(int sockfd, const char *command) {
    char buffer[BUF_SIZE];

    // send la commande au serveur
    send(sockfd, command, strlen(command), 0);

    // si la commande est "OPERATIONS", gérer plusieurs réponses
    if (strncmp(command, "OPERATIONS", 10) == 0) {
        while (1) {
            int read_size = recv(sockfd, buffer, BUF_SIZE - 1, 0);
            if (read_size < 0) {
                perror("Erreur de réception");
                return;
            }

            buffer[read_size] = '\0'; 

            // réponse reçue
            printf("%s", buffer);

            // stop la réception si "FIN" est présent
            if (strstr(buffer, "FIN") != NULL) {
                break;
            }
        }
    } else {
        // une seule réponse
        int read_size = recv(sockfd, buffer, BUF_SIZE - 1, 0);
        if (read_size < 0) {
            perror("Erreur de réception");
            return;
        }

        buffer[read_size] = '\0'; 
        printf("Réponse du serveur : %s\n", buffer);
    }
}


int main() {
    const char *server_ip = "127.0.0.1";  // adresse IP du serveur (localhost pour tests)
    int client_sock = init_client(server_ip);  // initialiser la connexion avec le serveur

    char command[BUF_SIZE];
    int id_client;

    // utilisateur doit choisir ID
    printf("Entrez votre ID client : ");
    scanf("%d", &id_client);
    getchar();  // pour enlever le '\n' restant après scanf

    printf("Client connecté au serveur %s sur le port %d avec l'ID client : %d\n", server_ip, PORT, id_client);

    // boucle pour envoyer des commandes au serveur
    while (1) {
        printf("\nEntrez une commande (ou 'exit' pour quitter) : ");
        fgets(command, BUF_SIZE, stdin);  // lire une commande de l'utilisateur

        // retirer le caractère de nouvelle ligne (\n) à la fin de la commande
        command[strcspn(command, "\n")] = 0;

        if (strcmp(command, "exit") == 0) { // fin échange
            printf("Déconnexion du serveur...\n");
            break;  
        }

        // check longueur commande
        size_t max_command_length = BUF_SIZE - 20;  // réserver de l'espace pour l'ID client (10 chiffres max + espace)
        if (strlen(command) > max_command_length) {
            printf("La commande est trop longue pour être envoyée.\n");
            continue;
        }

        // faire commande en ajoutant l'id_client
        char full_command[BUF_SIZE];
        // limiter la taille pour éviter le dépassement
        snprintf(full_command, BUF_SIZE, "%s %d", command, id_client);  // Ajoute l'id_client à la commande

        send_command(client_sock, full_command);  // envoyer commande au serveur
    }

    close(client_sock);  // fermer connexion
    return 0;
}
