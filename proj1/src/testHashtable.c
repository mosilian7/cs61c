#include "hashtable.h"
#include "philphix.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

char* convert(char* word, HashTable *dictionary);

char *convert(char* word, HashTable *dictionary) {
  char* inputs = malloc(sizeof(char) * (strlen(word) + 1));
  char* find;
  strcpy(inputs, word);
  find = findData(dictionary, word);
  if (find != NULL) {
    free(inputs);
    return find;
  } else {
    for (int i = 1; i < strlen(word); i += 1) {
      if (word[i] >= 'A' && word[i] <= 'Z') {
        word[i] += 32;
      }
    }
  }
  find = findData(dictionary, word);
  if (find != NULL) {
    free(inputs);
    return find;
  } else {
    if (word[0] >= 'A' && word[0] <= 'Z') {
      word[0] += 32;
    }
  }
  find = findData(dictionary, word);
  if (find != NULL) {
    free(inputs);
    return find;
  } else {
    return inputs;
  }
}

int main(int argc, char **argv) {
    printf("Running tests...\n\n");
    char* k1 = "aoi";
    char* v1 = "blue";
    char* k2 = "midori";
    char* v2 = "green";
    char* k3 = "akai";
    char* v3 = "red";
    char* k4 = "kuroi";
    char* v4 = "black";
    char* k5 = "shiroi";
    char* v5 = "white";
    char* k6 = "haiiro";
    char* v6 = "grey";
    char* k7 = "murasaki";
    char* v7 = "purple";

    HashTable *h = createHashTable(64, &stringHash, &stringEquals);
    insertData(h, k1, v1);
    insertData(h, k2, v2);
    insertData(h, k3, v3);
    insertData(h, k4, v4);
    insertData(h, k5, v5);
    insertData(h, k6, v6);
    //insertData(h, k5, v4);
    //insertData(h, k2, v3);
    //insertData(h, k1, v5);
    //insertData(h, k5, v3);
    insertData(h, k7, v7);
    insertData(h, "kiiro", "yellow");
    insertData(h, "sakurairo", "pink");
    insertData(h, "sekishishoku", "magenta");
    insertData(h, "pinku", "pink");
    insertData(h, "kuriimu", "milky");
    insertData(h, "orenji", "orange");
    insertData(h, "Haiiro", "Grey");
    insertData(h, "hAIIRO", "gREY");
    insertData(h, "HAIIRO", "GREY");
   

    //char* t = "murasaki";
    /*
    char* t;
    printf("input test case:");
    scanf("%s", t);
    printf("------------------------------\n");
    printf("target: %s\n", `);
    void* result = findData(h, t);

    if (result == NULL) {
        printf("find nothing\n");
    } else {
        printf("find result: %s\n", (char*) result);
    }*/


    char* c;
    
    printf("convert:");
    scanf("%s", c);

    char* result2 = convert(c, h);
    printf("convert result: %s\n", result2);

    free(h);
    
    

    return 0;
}