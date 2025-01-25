#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    // Retrieve the QUERY_STRING environment variable
    char *query_string = getenv("QUERY_STRING");

    // Print HTTP response headers
    printf("Content-Type: text/plain\r\n\r\n");

    // Check if QUERY_STRING exists
    if (query_string && strlen(query_string) > 0) {
        printf("GET Request Received:\n");
        printf("Query Parameters: %s\n", query_string);
    } else {
        printf("GET Request Received:\n");
        printf("No query parameters provided.\n");
    }

    return 0;
}
