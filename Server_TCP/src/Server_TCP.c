/*
 ============================================================================
 Name        : Server_TCP.c
 Author      : Barbaro Gerardo
 Version     :
 Copyright   : Your copyright notice
 Description : TCP Server in C, ANSI-style
 ============================================================================
 */

#if defined WIN32
    #include <winsock2.h>
#else
    #define closesocket close
    #include <sys/socket.h>
    #include <arpa/inet.h>
    #include <unistd.h>
#endif

#include "generator.h"

#define BUFFERSIZE 512       // Define buffer size for data transfer
#define PROTOPORT 60001      // Default server port
#define SERVER_ADDRESS "127.0.0.1" // Default server address
#define QLEN 5               // Queue length for incoming connections

// Function prototypes
char * type_switcher(char *, char *, char *);
void tokenizer(char * [3], char *);
void errorhandler(char *);
void clearwinsock();

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

    printf("%s", "Waiting for a client to connect\n");

    // Loop to accept and handle client connections
    struct sockaddr_in client_address;
    int client_socket, client_len;
    int continue_loop = 1;

    while (1)
    {
        client_len = sizeof(client_address);
        client_socket = accept(server_socket, (struct sockaddr *)&client_address, &client_len);
        if (client_socket < NO_ERROR)
        {
            errorhandler("Client acceptance failed\n");
            closesocket(client_socket);
            clearwinsock();
            return 0;
        }
        printf("%s%s%s%d%s\n", "*New connection from ", inet_ntoa(client_address.sin_addr), ":", ntohs(client_address.sin_port), "*");

        // Buffer to hold client data
        char client_data[BUFFERSIZE];
        continue_loop = 1;

        // Loop to receive data from the client
        do{
            memset(client_data, 0, BUFFERSIZE);
            recv(client_socket, client_data, BUFFERSIZE - 1, 0);

            // Remove the newline character if present
            int string_len = strlen(client_data);
            if ((string_len > 0) && (client_data[string_len - 1] == '\n'))
            {
                client_data[string_len - 1] = '\0';
            }
            printf("\n%s ", "Data received from client: ");
            printf("%s\n", client_data);

            // Tokenize client input data
            char * string_tokens[3];
            tokenizer(string_tokens, client_data);

            // Check if client requested to quit
            if(strcmp(string_tokens[0], "q") == 0){
                continue_loop = 0;
                printf("%s%s\n", "Closed connection with client ", inet_ntoa(client_address.sin_addr));
            } else {
                // Generate password based on client's request
                printf("%s\n", "Generating password... ");
                char password_result[BUFFERSIZE];
                memset(password_result, 0, BUFFERSIZE);

                // Use the type_switcher function to determine password type
                char * switcher = type_switcher(string_tokens[0], string_tokens[1], string_tokens[2]);
                if (switcher != NULL) {
                    strcpy(password_result, switcher);
                    free(switcher);
                } else {
                    strcpy(password_result, "Error: Invalid type");
                }

                // Send generated password back to client
                string_len = strlen(password_result);
                Sleep(1000);
                printf("%s %s \n", "Generated password -> ", password_result);
                send(client_socket, password_result, string_len, 0);
            }
        } while (continue_loop);
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
