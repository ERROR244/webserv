#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_POST_DATA 1024

int main() {
    // Retrieve the CONTENT_LENGTH environment variable
    char *content_length_str = getenv("CONTENT_LENGTH");
    int content_length = 0;

    // Convert CONTENT_LENGTH to an integer
    if (content_length_str) {
        content_length = atoi(content_length_str);
    }

    // Print HTTP response headers
    printf("Content-Type: text/plain\r\n\r\n");

    // If there's no content length or it's invalid
    if (content_length <= 0) {
        printf("POST Request Received:\n");
        printf("No data provided or invalid content length.\n");
        return 0;
    }

    // Read the POST data from stdin
    char post_data[MAX_POST_DATA] = {0};
    if (fread(post_data, 1, content_length, stdin) > 0) {
        printf("POST Request Received:\n");
        printf("Body: %s\n", post_data);
    } else {
        printf("POST Request Received:\n");
        printf("Failed to read POST data.\n");
    }

    return 0;
}
