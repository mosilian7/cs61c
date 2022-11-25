#include "hashtable.h"
#include <stdlib.h>
#include <stdio.h>

/*
 * This creates a new hash table of the specified size and with
 * the given hash function and comparison function.
 */
HashTable *createHashTable(int size, unsigned int (*hashFunction)(void *),
                           int (*equalFunction)(void *, void *)) {
  int i = 0;
  HashTable *newTable = malloc(sizeof(HashTable));
  if (NULL == newTable) {
    fprintf(stderr, "malloc failed \n");
    exit(1);
  }
  newTable->size = size;
  newTable->data = malloc(sizeof(HashBucket *) * size);
  if (NULL == newTable->data) {
    fprintf(stderr, "malloc failed \n");
    exit(1);
  }
  for (i = 0; i < size; i++) {
    newTable->data[i] = NULL;
  }
  newTable->hashFunction = hashFunction;
  newTable->equalFunction = equalFunction;
  return newTable;
}

HashBucket *createBucket(void *key, void *data) {
  HashBucket* newBucket = malloc(sizeof(HashBucket));
  if (newBucket == NULL) {
    fprintf(stderr, "malloc failed \n");
    exit(1);
  }
  newBucket->key = key;
  newBucket->data = data;
  newBucket->next = NULL;
  newBucket->testStuff = -1;
  return newBucket;
}

/*
 * This inserts a key/data pair into a hash table.  To use this
 * to store strings, simply cast the char * to a void * (e.g., to store
 * the string referred to by the declaration char *string, you would
 * call insertData(someHashTable, (void *) string, (void *) string).
 */
void insertData(HashTable *table, void *key, void *data) {
  // -- TODO --
  unsigned int hashValue = table->hashFunction(key); // why not (*table->hashFunction)(key)
  int bucketIndex = hashValue % table->size;

  if (table->data[bucketIndex] == NULL) {
    table->data[bucketIndex] = createBucket(key, data);
    //printf("%s", ((HashBucket*) table->data[bucketIndex])->key);
    return;
  }

  HashBucket** curr =(HashBucket**) &(table->data[bucketIndex]);
  do {
    if (table->equalFunction((*curr)->key, key)) {
      (*curr)->data = data;
      return;
    }
    curr = &((*curr)->next);
  } while (*curr != NULL);
  *curr = createBucket(key, data);
  // HINT:
  // 1. Find the right hash bucket location with table->hashFunction.
  // 2. Allocate a new hash bucket struct.
  // 3. Append to the linked list or create it if it does not yet exist. 
}

/*
 * This returns the corresponding data for a given key.
 * It returns NULL if the key is not found. 
 */
void *findData(HashTable *table, void *key) {
  // -- TODO --
  unsigned int hashValue = table->hashFunction(key);
  int bucketIndex = hashValue % table->size;

  if (table->data[bucketIndex] == NULL) {
    return NULL;
  }

  HashBucket *curr = (HashBucket*) table->data[bucketIndex];
  do {
    //printf("findData: %s\n", (char*) curr->key);
    if (table->equalFunction(curr->key, key)) {
      return curr->data;
    }
    curr = curr->next;
  } while (curr != NULL);

  return NULL;
  // HINT:
  // 1. Find the right hash bucket with table->hashFunction.
  // 2. Walk the linked list and check for equality with table->equalFunction.
}
