#define _GNU_SOURCE
#include <assert.h>
#include <crypt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dictionary_words.h"
#include "thread_pool.h"

const char HASH_START[] = "$6$";
const size_t SALT_LENGTH = 20;
const size_t HASH_LENGTH = 106;
const size_t NUM_THREADS = 16;

static size_t hash_count = 0;
static char **hashes = NULL;

static inline bool hashes_match(const char *password, const char *hash) {
    char salt[SALT_LENGTH + 1];
    memcpy(salt, hash, sizeof(char[SALT_LENGTH]));
    salt[SALT_LENGTH] = '\0';
    struct crypt_data data;
    memset(&data, 0, sizeof(data));
    char *hashed = crypt_r(password, salt, &data);
    char *hashed_hash = &hashed[SALT_LENGTH];
    const char *hash_hash = &hash[SALT_LENGTH];
    return memcmp(hashed_hash, hash_hash, sizeof(char[HASH_LENGTH - SALT_LENGTH])) == 0;
}

// Function to test all digit-augmented variants of a dictionary word against a set of
// password hashes
void test_dictionary_variants_against_hashes(void *dictionary_word) {
    // Cast the void pointer to a char pointer to get the dictionary word (already
    // null-terminated)
    char *word = (char *) dictionary_word;
    // Get the length of the dictionary word
    size_t length = strlen(word);

    // Array to hold the modified word (original word length + 1 digit + null terminator)
    char augmented_word[length + 2];

    // Iterate over each position in the word (including the position before the first
    // character)
    for (size_t i = 0; i <= length; i++) {
        // Copy the first part of the dictionary word up to the current position
        memcpy(augmented_word, word, i * sizeof(char));

        // Copy the rest of the dictionary word after the current position
        memcpy(&augmented_word[i + 1], &word[i], (length - i + 1) * sizeof(char));

        // Try each digit (0-9) at the current position
        for (char j = '0'; j <= '9'; j++) {
            augmented_word[i] = j; // Insert the digit at the current position

            // Compare the modified word against each stored hash
            for (size_t k = 0; k < hash_count; k++) {
                // If the hash of the modified word matches any of the stored hashes
                if (hashes_match(augmented_word, hashes[k])) {
                    printf("%s\n", augmented_word); // Print the word if it matches a hash
                }
            }
        }
    }
}

int main(void) {
    // Read in the hashes from the standard input
    char *line = NULL;
    size_t line_capacity = 0;
    // Stop when the end of the input or an empty line is reached
    while (getline(&line, &line_capacity, stdin) > 0 && line[0] != '\n') {
        // Check that the line looks like a hash
        size_t line_length = strlen(line);
        assert(line_length == HASH_LENGTH ||
               (line_length == HASH_LENGTH + 1 && line[HASH_LENGTH] == '\n'));
        assert(memcmp(line, HASH_START, sizeof(HASH_START) - sizeof(char)) == 0);

        // Extend the hashes array and add the hash to it
        hashes = realloc(hashes, sizeof(char * [hash_count + 1]));
        assert(hashes != NULL);
        char *hash = malloc(sizeof(char[HASH_LENGTH + 1]));
        assert(hash != NULL);
        memcpy(hash, line, sizeof(char[HASH_LENGTH]));
        hash[HASH_LENGTH] = '\0';
        hashes[hash_count++] = hash;
    }
    free(line);

    thread_pool_t *thread_pool = thread_pool_init(NUM_THREADS);
    for (size_t i = 0; i < NUM_DICTIONARY_WORDS; i++) {
        thread_pool_add_work(thread_pool, test_dictionary_variants_against_hashes,
                             (void *) DICTIONARY[i]);
    }
    thread_pool_finish(thread_pool);
    return 0;
}
