/*
 * generator.h
 *
 *  Created on: 04 nov 2024
 *      Author: gerardo
 */

#ifndef GENERATOR_H_
#define GENERATOR_H_

#include <time.h>

#define QLEN 5               // Queue length for incoming connections

char * type_switcher(char *, char *, char *);
void ShowOnline(int current_clients);
void handle_client(void *arg);

void initialize_random();
char* generate_custom(int length, const char* charset, int charset_size);
char* generate_numeric(int length);
char* generate_alpha(int length);
char* generate_mixed(int length);
char* generate_secure(int length);

int current_clients = 0;
int justonce = 1;

#endif /* GENERATOR_H_ */
