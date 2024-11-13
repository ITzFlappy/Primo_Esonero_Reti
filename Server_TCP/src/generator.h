/*
 * generator.h
 *
 *  Created on: 04 nov 2024
 *      Author: gerardo
 */

#ifndef GENERATOR_H_
#define GENERATOR_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Windows.h>
#include <ctype.h>
#include <time.h>


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

#endif /* GENERATOR_H_ */
