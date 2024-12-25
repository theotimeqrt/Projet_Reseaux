#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8080  // port utilisé pour la connexion
#define BUF_SIZE 1024  // taille max du buffer pour envoyer/recevoir des messages
#define MAX_CLIENTS 10  // nombre max clients

// ================ strcutures =======================

typedef struct {
    int id_compte; 
    double solde;  
    char operations[10][BUF_SIZE]; // les 10 dernières opérations
} CompteBancaire;


typedef struct {
    int id_client;  
    char password[BUF_SIZE];  
    CompteBancaire compte;  
} Client;

// tableau pour stocker les clients
Client clients[MAX_CLIENTS];


// ================== Fonctions ===================


void init_clients() {
    // init un client
    init_compte(&clients[0], 1, "mdp");
    clients[0].compte.solde = 1000.0;  
    snprintf(clients[0].compte.operations[0], BUF_SIZE, "INITIALISATION: 1000.00");
    
    // autres clients ici si nécessaire
    printf("[DEBUG] Client initialisé: id_client=%d, id_compte=%d, password=%s, solde=%.2f\n",
           clients[0].id_client, clients[0].compte.id_compte, clients[0].password, clients[0].compte.solde);
}

// fonction pour initialiser un compte bancaire pour un client
void init_compte(Client *client, int id_client, const char *password) {
    client->id_client = id_client;  
    strcpy(client->password, password);  
    client->compte.id_compte = id_client;  // même
    client->compte.solde = 0.0;
    memset(client->compte.operations, 0, sizeof(client->compte.operations));  // effacer les anciennes opérations
}

// fonction pour trouver un client par son id
Client* find_client(int id_client) {
    for (int i = 0; i < MAX_CLIENTS; i++) { 
        if (clients[i].id_client == id_client) {  
            return &clients[i];  // on retourne le pointeur vers ce client
        }
    }
    return NULL; 
}

// faire liste opérations qui se refresh
void ajouter_operation(Client *client, const char *operation) {
    for (int i = 0; i < 9; i++) {
        strcpy(client->compte.operations[i], client->compte.operations[i + 1]);
    }
    snprintf(client->compte.operations[9], BUF_SIZE, "%s", operation);
}

// fonction pour ajouter de l'argent sur le compte du client
void ajouter_solde(Client *client, double somme) {
    client->compte.solde += somme;
    char operation[BUF_SIZE];
    snprintf(operation, BUF_SIZE, "AJOUT: %.2f", somme);
    ajouter_operation(client, operation);
    printf("Solde après ajout: %.2f\n", client->compte.solde); 
}

// fonction pour retirer de l'argent du compte du client
void retirer_solde(Client *client, double somme) {
    if (client->compte.solde >= somme) {  // vérifier que le solde est suffisant
        client->compte.solde -= somme; 
        char operation[BUF_SIZE];
        snprintf(operation, BUF_SIZE, "RETRAIT: %.2f", somme);
        ajouter_operation(client, operation);
        printf("Solde après retrait: %.2f\n", client->compte.solde); 
    } else {
        printf("solde insuffisant\n");  // message si le solde est insuffisant
    }
}

// fonction pour afficher le solde du compte d'un client
void afficher_solde(Client *client) {
    printf("solde du compte %d : %.2f\n", client->compte.id_compte, client->compte.solde);  
}

// fonction pour afficher les dernières opérations du client
void afficher_operations(Client *client) {
    for (int i = 9; i >= 0; i--) {  // parcourir les 10 dernières opérations
        if (strlen(client->compte.operations[i]) > 0) { 
            printf("Opération %d: %s\n", i, client->compte.operations[i]); 
            printf("%s\n", client->compte.operations[i]);  // afficher l'opération 
        } 
    }
}

void handle_client(int client_sock) {
    char buffer[BUF_SIZE];  // buffer pour recevoir messages client
    int read_size;

    while ((read_size = recv(client_sock, buffer, BUF_SIZE, 0)) > 0) {  // recevoir des données du client
        buffer[read_size] = '\0';  // ajouter un caractère de fin de chaîne
        printf("reçu: %s\n", buffer);  // afficher le message reçu

        // analyser la commande reçue
        int id_client, id_compte;
        char password[BUF_SIZE];
        double somme;

        // AJOUT ======== gros debug ======================

        if (sscanf(buffer, "AJOUT %d %d %s %lf", &id_client, &id_compte, password, &somme) == 4) {  
            printf("[DEBUG] Commande reçue: ajout\n");
            printf("[DEBUG] Paramètres extraits : id_client=%d, id_compte=%d, password=%s, somme=%.2f\n", 
                id_client, id_compte, password, somme);
                
            Client *client = find_client(id_client); 
            
            if (client == NULL) {
                printf("[DEBUG] Client avec id_client=%d non trouvé\n", id_client);
            } else {
                printf("[DEBUG] Client trouvé: id_client=%d, id_compte=%d, password=%s\n", 
                    client->id_client, client->compte.id_compte, client->password);
            }
            
            if (client && client->compte.id_compte == id_compte) {
                printf("[DEBUG] id_compte correspondant trouvé: %d\n", id_compte);
            } else {
                printf("[DEBUG] id_compte ne correspond pas: attendu=%d, reçu=%d\n", 
                    client ? client->compte.id_compte : -1, id_compte);
            }

            if (client && strcmp(client->password, password) == 0) {
                printf("[DEBUG] Mot de passe correspondant.\n");
            } else {
                printf("[DEBUG] Mot de passe incorrect: attendu=%s, reçu=%s\n", 
                    client ? client->password : "N/A", password);
            }
            
            if (client && client->compte.id_compte == id_compte && strcmp(client->password, password) == 0) {  // check parametres
                ajouter_solde(client, somme);  // ajouter argent
                send(client_sock, "OK\n", 3, 0);  
            } else {
                send(client_sock, "KO\n", 3, 0);  // si erreur
            }
        }


        // =============================

        // RETRAIT
        
        else if (sscanf(buffer, "RETRAIT %d %d %s %lf", &id_client, &id_compte, password, &somme) == 4) {  
            printf("[DEBUG] Commande reçue: retrait\n");
            printf("[DEBUG] Paramètres extraits : id_client=%d, id_compte=%d, password=%s, somme=%.2f\n", 
                id_client, id_compte, password, somme);
                
            Client *client = find_client(id_client); 
            
            if (client == NULL) {
                printf("[DEBUG] Client avec id_client=%d non trouvé\n", id_client);
            } else {
                printf("[DEBUG] Client trouvé: id_client=%d, id_compte=%d, password=%s\n", 
                    client->id_client, client->compte.id_compte, client->password);
            }
            
            if (client && client->compte.id_compte == id_compte) {
                printf("[DEBUG] id_compte correspondant trouvé: %d\n", id_compte);
            } else {
                printf("[DEBUG] id_compte ne correspond pas: attendu=%d, reçu=%d\n", 
                    client ? client->compte.id_compte : -1, id_compte);
            }

            if (client && strcmp(client->password, password) == 0) {
                printf("[DEBUG] Mot de passe correspondant.\n");
            } else {
                printf("[DEBUG] Mot de passe incorrect: attendu=%s, reçu=%s\n", 
                    client ? client->password : "N/A", password);
            }
            
            if (client && client->compte.id_compte == id_compte && strcmp(client->password, password) == 0) { 
                retirer_solde(client, somme);  // retirer
                send(client_sock, "OK\n", 3, 0);  
            } else {
                send(client_sock, "KO\n", 3, 0);  
            }
        } 

        // SOLDE

        else if (sscanf(buffer, "SOLDE %d %d %s", &id_client, &id_compte, password) == 3) {  
  
            Client *client = find_client(id_client);  

            if (client && client->compte.id_compte == id_compte && strcmp(client->password, password) == 0) {  
                char response[BUF_SIZE];
                snprintf(response, BUF_SIZE, "Solde: %.2f\n", client->compte.solde);  
                printf("Réponse SOLDE: %s", response); 
                send(client_sock, response, strlen(response), 0);  
            } else {
                send(client_sock, "KO\n", 3, 0);  
            }
        } 
        
        // OPERATIONS

        else if (sscanf(buffer, "OPERATIONS %d %d %s", &id_client, &id_compte, password) == 3) {  

            Client *client = find_client(id_client); 

            
            if (client && client->compte.id_compte == id_compte && strcmp(client->password, password) == 0) {  
                afficher_operations(client);  // 10 dernières opérations

                char operation_message[BUF_SIZE];
                for (int i = 9; i >= 0; i--) {  // donner en sens inverse pour afficher les plus récentes en premier
                    if (strlen(client->compte.operations[i]) > 0) {
                        snprintf(operation_message, BUF_SIZE, "Opération : %s\n", client->compte.operations[i]);
                        send(client_sock, operation_message, strlen(operation_message), 0);  // send opé
                    }
                }

                send(client_sock, "FIN\n", 3, 0);  
            } else {
                send(client_sock, "KO\n", 3, 0);  
            }
        } 
        else {
            send(client_sock, "COMMANDE INVALIDE\n", 18, 0);  // commande pas reconnue
        }
    }
}


// fonction pour initialiser la connexion du serveur
int init_server() {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);  // créer un socket TCP
    if (sockfd < 0) {
        perror("erreur de création du socket");
        exit(1);
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));  
    server_addr.sin_family = AF_INET;  
    server_addr.sin_addr.s_addr = INADDR_ANY;  
    server_addr.sin_port = htons(PORT); 

    // lier le socket à l'adresse et au port
    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("erreur de bind");
        exit(1);
    }

    // écouter les connexions entrantes
    if (listen(sockfd, MAX_CLIENTS) < 0) {
        perror("erreur de listen");
        exit(1);
    }

    return sockfd;
}

// fonction principale
int main() {
    int server_sock = init_server();  // init serveur
    printf("Serveur en attente de connexions...\n");

    init_clients();

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &client_len);  // accepter une connexion client
        if (client_sock < 0) {
            perror("erreur d'acceptation");
            continue;
        }
        printf("Nouveau client connecté\n");
        handle_client(client_sock);  // gérer la communication avec le client
        close(client_sock);  // fermer le socket du client après la communication
    }

    close(server_sock);  // fermer le socket du serveur
    return 0;
}
