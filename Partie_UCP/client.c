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
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);  // créer un socket UDP
    if (sockfd < 0) {
        perror("Erreur de création du socket");
        exit(1);
    }
    return sockfd;
}

void send_command(int sockfd, struct sockaddr_in *server_addr, const char *command) {
    char buffer[BUF_SIZE];

    // envoyer la commande au serveur
    sendto(sockfd, command, strlen(command), 0, (struct sockaddr *)server_addr, sizeof(*server_addr));

    // si la commande est "OPERATIONS", gérer plusieurs réponses
    if (strncmp(command, "OPERATIONS", 10) == 0) {
        while (1) {
            socklen_t server_len = sizeof(*server_addr);
            int read_size = recvfrom(sockfd, buffer, BUF_SIZE - 1, 0, (struct sockaddr *)server_addr, &server_len);
            if (read_size < 0) {
                perror("Erreur de réception");
                return;
            }

            buffer[read_size] = '\0';  // ajouter un '\0' à la fin

            // réponse reçue
            printf("%s", buffer);

            // arrêter la réception si "FIN" est présent
            if (strstr(buffer, "FIN") != NULL) {
                break;
            }
        }
    } else {
        // une seule réponse
        socklen_t server_len = sizeof(*server_addr);
        int read_size = recvfrom(sockfd, buffer, BUF_SIZE - 1, 0, (struct sockaddr *)server_addr, &server_len);
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
    struct sockaddr_in server_addr;
    
    // Initialiser le client
    int client_sock = init_client(server_ip);  // créer un socket UDP

    memset(&server_addr, 0, sizeof(server_addr));  // init adresse serveur à zéro
    server_addr.sin_family = AF_INET;  // utilisation IPv4
    server_addr.sin_port = htons(PORT);  // définir port serveur
    server_addr.sin_addr.s_addr = inet_addr(server_ip);  // adresse IP serveur

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
        snprintf(full_command, BUF_SIZE, "%s %d", command, id_client);  // ajoute l'id_client à la commande

        send_command(client_sock, &server_addr, full_command);  // envoyer commande au serveur
    }

    close(client_sock);  // fermer connexion
    return 0;
}
