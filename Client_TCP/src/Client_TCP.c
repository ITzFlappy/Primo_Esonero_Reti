/*
 ============================================================================

 Name        : Client_TCP.c
 Author      : Barbaro Gerardo
 Version     :
 Copyright   : Your copyright notice
 Description : Client TCP in C, Ansi-style
 ============================================================================
 */
#include "headers.h"

int main(){ //trying commit

	#if defined WIN32
		// Initialize Winsock for Windows
		WSADATA wsa_data;
		int result = WSAStartup(MAKEWORD(2, 2), &wsa_data);
		if (result != NO_ERROR)
		{
			printf("%s", "Start up failed");
			return -1;
		}
		printf("%s", "Start up done correctly\n");
	#endif

	// Socket creation
	int client_socket;
	client_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (client_socket < NO_ERROR)
	{
		errorhandler("socket creation failed\n");
		closesocket(client_socket);
		clearwinsock();
		return -1;
	}
	printf("%s", "Socket created successfully\n");

	// Setting up the server address structure
	char * address_to_connect = "127.0.0.1";
	int port = PROTOPORT;
	struct sockaddr_in server_ad;
	memset(&server_ad, 0, sizeof(server_ad)); // Clear structure memory
	server_ad.sin_family = AF_INET;
	server_ad.sin_addr.s_addr = inet_addr(address_to_connect);
	server_ad.sin_port = htons(port);
	printf("%s %s %d\n", "Trying to establish connection with", address_to_connect, port);

	// Establish connection with the server
	int server_connect;
	server_connect = connect(client_socket, (struct sockaddr *)&server_ad, sizeof(server_ad));
	if (server_connect < NO_ERROR)
	{
		errorhandler("Failed to connect to the server\n");
		closesocket(client_socket);
		clearwinsock();
		return -1;
	}
	printf("%s", "Connected with the server\n");

	// Allocate buffer for input data
	char * input_string = malloc(BUFFERSIZE);
	memset(input_string, 0, BUFFERSIZE);

		while (1) {
			memset(input_string, 0, BUFFERSIZE);
			// Display options for generating different types of passwords
			printf("%s", "The server can generate a random password.\n"
						 "The options are:\n"
						 "1. 'n' for numeric password (numbers only)\n"
						 "2. 'a' for alphabetic password (lowercase letters only)\n"
						 "3. 'm' for mixed password (lowercase letters and numbers)\n"
						 "4. 's' for strong password (uppercase, lowercase letters, numbers and symbols)\n");
			printf("%s\n", "If you want to close the connection, please enter only 'q' character");
			printf("%s ", "Enter the character associated with the type of password you want to generate\n"
						  "followed by a space and the desired length (from 6 to 32)\nThe server will send back the result -> ");
			fgets(input_string, BUFFERSIZE - 1, stdin); // Get user input with a size limit

			// Remove newline character at the end of input if it exists
			int string_len = strlen(input_string);
			if ((string_len > 0) && (input_string[string_len - 1] == '\n')) {
				input_string[string_len - 1] = '\0';
			}

			char *string_tokens[3]; // Array to hold tokenized parts of input
			char input_string_cpy[BUFFERSIZE];
			strcpy(input_string_cpy, input_string);
			tokenizer(string_tokens, input_string_cpy); // Tokenize the input

			// Check if the input is valid
			if (strcmp(string_tokens[0], "e") == 0) {
				printf("Invalid input. Please enter a valid character and length.\n");
				Sleep(1500);
				system("CLS");
				continue; // Ask for input again
			}

			// Check if client requested to quit
			if (strcmp(string_tokens[0], "q") == 0) {
				send(client_socket, input_string, string_len, 0);
				printf("%s\n", "Closing connection with server\n");
				break;
			}

			// Check for correct input format and values
			int tokens_number = atoi(string_tokens[2]);
			if (tokens_number != 2 || !checkChar(string_tokens[0]) || isValidNumber(string_tokens[1])) {
				printf("%s", "You must send to server a valid character\n");
				Sleep(1500);
				system("CLS");
				continue; // Ask for input again
			}

			// Send data to the server
			int data_sent;
			data_sent = send(client_socket, input_string, string_len, 0);
			if (data_sent != string_len) {
				errorhandler("Different number of bytes has been sent to the server\n");
				closesocket(client_socket);
				clearwinsock();
				return -1;
			}
			printf("%s", "Data sent successfully\n");

			// Receive response from server
			int bytes_received = 0;
			char server_string[BUFFERSIZE];
			memset(server_string, 0, BUFFERSIZE);
			printf("Data retrieved from server: ");
			bytes_received = recv(client_socket, server_string, BUFFERSIZE - 1, 0);
			if (bytes_received <= 0) {
				errorhandler("Retrieve failed or connection closed prematurely\n");
				closesocket(client_socket);
				clearwinsock();
				return -1;
			}
			printf("%s\n", server_string);
			Sleep(3000);
			system("CLS");
			printf("%s%s\n%s%s\n", "Previous password -> ", input_string, "Result -> ", server_string);
		}

	// Close socket and clean up Winsock
	closesocket(client_socket);
	clearwinsock();
	printf("%s", "\n");
	system("PAUSE");

	return 0;
}


// Clear the Winsock environment for Windows
void clearwinsock(){
	#if defined WIN32
		WSACleanup();
	#endif
}

// Error handler function
void errorhandler(char * string){
	printf("%s", string);
}

// Tokenizer function to split input into tokens
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
            printf("Error: Invalid or empty input\n");
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


// Checks if the first character of input is a valid option for password type
int checkChar(char * string){
	if(string[0] == 'n' || string[0] == 'a' || string[0] == 'm' || string[0] == 's'){
		return 1;
	}
	return 0;
}

// Validates if input is a valid number and within the range for password length
int isValidNumber(char *string) {
    int length = strlen(string);

    for (int i = 0; i < length; i++) {
        if (!isdigit(string[i])) {
            printf("Entered input is not a number\n");
            return 1;
        }
    }

    int number = atoi(string);

    if (number < 6 || number > 32) {
        printf("Number is out of the allowed range (6-32)\n");
        return 1;
    }

    return 0;
}
