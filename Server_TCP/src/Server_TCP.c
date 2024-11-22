/*
 ============================================================================
 Name        : Server_TCP.c
 Author      : Barbaro Gerardo
 Version     :
 Copyright   : Your copyright notice
 Description : TCP Server in C, ANSI-style
 ============================================================================
 */
#include "../../headers.h"
#include "generator.h"



int main(int argc, char *argv[])
{
    int port;
    char * start_address_server = "";
    // Check command line arguments for server address and port
    if (argc > 1){
        start_address_server = argv[1];
        port = atoi(argv[2]);
    } else {
        port = PROTOPORT;
        start_address_server = SERVER_ADDRESS;
        if (port < 0){
            printf("%d is a bad port number\n", port);
            return 0;
        }
        printf("%s", "This port is available\n");
    }
    printf("%s %s:%d\n", "Trying to start server with", start_address_server, port);

    #if defined WIN32
        // Initialize Winsock on Windows
        WSADATA wsa_data;
        int result;
        result = WSAStartup(MAKEWORD(2, 2), &wsa_data);
        if (result != NO_ERROR)
        {
            errorhandler("Start up failed\n");
            return 0;
        }
        printf("Start up done correctly\n");
    #endif

    // Socket creation
    int server_socket;
    server_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_socket < 0)
    {
        errorhandler("Socket creation failed\n");
        clearwinsock();
        return -1;
    }
    printf("%s", "Socket created successfully\n");

    // Set up server address structure
    struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(start_address_server);
    server_address.sin_port = htons(port);

    // Bind socket to the specified port and address
    int server_bind;
    server_bind = bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address));
    if (server_bind < NO_ERROR)
    {
        errorhandler("Binding failed\n");
        closesocket(server_socket);
        clearwinsock();
        return -1;
    }
    printf("%s", "Binding successful\n");

    // Set socket to listen for incoming connections
    int listening;
    listening = listen(server_socket, QLEN-1);
    if (listening < NO_ERROR)
    {
        errorhandler("Listening failed\n");
        closesocket(server_socket);
        clearwinsock();
        return -1;
    }
    printf("%s", "Listening successful\n");

    Sleep(5000);
    system("CLS");

    printf("%s", "Waiting for a client to connect\n\n");

    // Loop to accept and handle client connections
    struct sockaddr_in client_address;
    int client_socket, client_len;


    ShowOnline(current_clients);

    while (1) {

    	if (current_clients == QLEN && justonce) {
    		justonce = 0;
			printf("Max number of clients connected, refusing new connections.\n");
			// Non accetta nuove connessioni finché non si libera uno slot
			Sleep(1000); // Pausa per evitare un sovraccarico della CPU
			continue;
		}

        client_len = sizeof(client_address);
        client_socket = accept(server_socket, (struct sockaddr *)&client_address, &client_len);
        if (client_socket < 0) {
            errorhandler("Client acceptance failed\n");
            continue;
        }

        // Incrementare il contatore di client connessi
		current_clients++;

		if(current_clients < QLEN + 1){
			SetColor(2);
			printf("\t*New connection from ");

			SetColor(1);
			printf("%s:%d", inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));

			SetColor(2);
			printf("*\n");
			SetColor(7);
		}


		if (current_clients > QLEN) {
			SetColor(1); // Colore blu
			printf("%s:%d", inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));
			SetColor(7);
			printf(" tried to connect, ");
			SetColor(4);
			printf("connection refused.\n");
			SetColor(7);

			// Invia un messaggio al client che si sta rifiutando
			    const char *full_message = "server is full";
			    send(client_socket, full_message, strlen(full_message), 0);

			closesocket(client_socket);  // Close the rejected connection
			current_clients--;  // Decrement the counter for rejected clients
			continue;
		} else{
			const char *full_message = "server is ready";
			send(client_socket, full_message, strlen(full_message), 0);
		}

        // Allocazione dinamica del client_socket
        int *client_socket_ptr = malloc(sizeof(int));
        if (client_socket_ptr == NULL) {
            printf("Memory allocation error\n");
            closesocket(client_socket);
            continue;
        }
        *client_socket_ptr = client_socket;

        #if defined WIN32
            HANDLE thread_handle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)handle_client, client_socket_ptr, 0, NULL);
            if (thread_handle == NULL) {
                printf("Error creating thread.\n");
                closesocket(client_socket);
                free(client_socket_ptr);
            } else {
                CloseHandle(thread_handle); // Lascia il thread lavorare
            }
        #else
            pthread_t thread_id;
            if (pthread_create(&thread_id, NULL, (void *)handle_client, client_socket_ptr) != 0) {
                printf("Error creating thread.\n");
                closesocket(client_socket);
                free(client_socket_ptr);
            } else {
                pthread_detach(thread_id); // Il thread si libera da solo
            }
        #endif
            ShowOnline(current_clients);
    }

    system("PAUSE");
    return 0;
}

// Function to select password type based on client's request
char * type_switcher(char *type, char *length, char *token_num) {
    int token_number = atoi(token_num);

    if (token_number > 2) {
        printf("%d\n", token_number);
        printf("%s", "Parsing error");
        return NULL;
    }

    int number = atoi(length);
    char *pw_result = NULL;
    printf("Type is: %c\n", type[0]);

    // Generate password based on type character
    switch (type[0]) {
        case 'n':
            pw_result = generate_numeric(number);
            break;
        case 'a':
            pw_result = generate_alpha(number);
            break;
        case 'm':
            pw_result = generate_mixed(number);
            break;
        case 's':
            pw_result = generate_secure(number);
            break;
        default:
            printf("Invalid type \n");
            return NULL;
    }

    return pw_result;
}

// Function to tokenize the client input string into an array
void tokenizer(char *tokens[3], char *string){
	if (string == NULL || strlen(string) == 0) {
	        printf("Error: Input is empty\n");
	        tokens[0] = "e";  // Indicates an error
	        return;
	    }

    char *type = string;
    short temp = 0;
    char *token_string;
    token_string = strtok(type, " ");

    // Check if the toke_string is not null
        if (token_string == NULL || strlen(token_string) != 1) {
            printf("Error: Invalid or empty token\n");
            tokens[0] = "e";  // Indicates an error
            return;
        }

    if (strcmp(token_string, "q") == 0) {
        tokens[0] = "q";
        tokens[1] = NULL;
        tokens[2] = "1";
    } else {
        while (token_string != NULL) {
            tokens[temp] = token_string;
            temp++;
            token_string = strtok(NULL, " ");
        }
        if (temp == 2) {
        	char str_temp_var[BUFFERSIZE];
			snprintf(str_temp_var, BUFFERSIZE, "%d", temp);
			tokens[temp] = str_temp_var;
        }
    }
}


// Function to clean up Winsock resources on Windows
void clearwinsock(){
    #if defined WIN32
        WSACleanup();
    #endif
}

// Error handling function for printing messages
void errorhandler(char * string){
    printf("%s", string);
}

void handle_client(void *arg) {
    int client_socket = *((int *)arg);
    free(arg);  // Libera la memoria allocata per il socket

    struct sockaddr_in client_address;
    int client_len = sizeof(client_address);
    getpeername(client_socket, (struct sockaddr *)&client_address, &client_len);

    char client_data[BUFFERSIZE];
    int continue_loop = 1;

    while (continue_loop) {
        memset(client_data, 0, BUFFERSIZE);

        int bytes_received = recv(client_socket, client_data, BUFFERSIZE - 1, 0);
        if (bytes_received == 0) {
            // Messaggio di disconnessione
        			SetColor(4);
        			printf("\t*Closed connection with ");

        			SetColor(1);
        			printf("%s:%d", inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));

        			SetColor(4);
        			printf("*\n");
        			SetColor(7);

        			ShowOnline(current_clients);

        			justonce = 1;
            break;
        } else if (bytes_received < 0) {
            printf("The client closed unexpectedly.\n");
            break;
        }

        int string_len = strlen(client_data);
        if ((string_len > 0) && (client_data[string_len - 1] == '\n')) {
            client_data[string_len - 1] = '\0';
        }

        printf("\nData received from client ");

        SetColor(1);
        printf("%s:%d", inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));
        SetColor(7);

        printf(" : %s\n", client_data);
        char *string_tokens[3];
        tokenizer(string_tokens, client_data);

        if (strcmp(string_tokens[0], "q") == 0) {
            printf("Client requested to quit.\n");
            break;
        } else {
            printf("Generating password...\n");
            char password_result[BUFFERSIZE];
            memset(password_result, 0, BUFFERSIZE);

            char *switcher = type_switcher(string_tokens[0], string_tokens[1], string_tokens[2]);
            if (switcher != NULL) {
                strcpy(password_result, switcher);
                free(switcher);
            } else {
                strcpy(password_result, "Error: Invalid type");
            }

            string_len = strlen(password_result);
            printf("Password generated -> %s\n\n", password_result);
            send(client_socket, password_result, string_len, 0);
        }
    }

    current_clients--;
    closesocket(client_socket);
    //disconnecting message
    		SetColor(4);
			printf("\t*Closed connection with ");

			SetColor(1);
			printf("%s:%d", inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));

			SetColor(4);
			printf("*\n");
			SetColor(7);

			ShowOnline(current_clients);

			justonce = 1;
}

void SetColor(unsigned short color){
	HANDLE hCon = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hCon,color);
}

void ShowOnline(int current_clients){
	printf("\t\t\t\tClient currently online: ");
	SetColor(2);
	printf("%d", current_clients);
	SetColor(7);
	printf("/5\n");
}

//Initializes the random number generator once
void initialize_random() {
    static int initialized = 0;
    if (!initialized) {
        srand((unsigned int)time(NULL));
        initialized = 1;
    }
}

//Generates a random string using a specified character set
char* generate_custom(int length, const char* charset, int charset_size) {
    initialize_random();
    char* result = (char*)malloc((length + 1) * sizeof(char));
    if (result == NULL) {
        printf("Memory allocation failed\n");
        return NULL;
    }

    for (int i = 0; i < length; i++) {
        result[i] = charset[rand() % charset_size];
    }

    result[length] = '\0';
    return result;
}

// Generates a numeric password
char* generate_numeric(int length) {
    return generate_custom(length, "0123456789", 10);
}

// Generates an alphabetic password
char* generate_alpha(int length) {
    return generate_custom(length, "abcdefghijklmnopqrstuvwxyz", 26);
}

// Generates a mixed alphanumeric password
char* generate_mixed(int length) {
    return generate_custom(length, "abcdefghijklmnopqrstuvwxyz0123456789", 36);
}

// Generates a secure password with special characters
char* generate_secure(int length) {
    return generate_custom(length, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$%^&*()-_=+[]{}|;:,.<>?", 86);
}


