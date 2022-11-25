/*
 * Include the provided hash table library.
 */
#include "hashtable.h"

/*
 * Include the header file.
 */
#include "philphix.h"

/*
 * Standard IO and file routines.
 */
#include <stdio.h>

/*
 * General utility routines (including malloc()).
 */
#include <stdlib.h>

/*
 * Character utility routines.
 */
#include <ctype.h>

/*
 * String utility routines.
 */
#include <string.h>

char *stringInput();
char *stringProcess(char *inputs);
char *convert(char* word);

/*
 * This hash table stores the dictionary.
 */
HashTable *dictionary;

/*
 * The MAIN routine.  You can safely print debugging information
 * to standard error (stderr) as shown and it will be ignored in
 * the grading process.
 */

int main(int argc, char **argv)
{
  dictionary = createHashTable(0x61C, &stringHash, &stringEquals);
  processInput();
}

int submain(int argc, char **argv)
{
  if (argc != 2)
  {
    fprintf(stderr, "Specify a dictionary\n");
    return 1;
  }
  /*
   * Allocate a hash table to store the dictionary.
   */
  fprintf(stderr, "Creating hashtable\n");
  dictionary = createHashTable(0x61C, &stringHash, &stringEquals);

  fprintf(stderr, "Loading dictionary %s\n", argv[1]);
  readDictionary(argv[1]);
  fprintf(stderr, "Dictionary loaded\n");

  fprintf(stderr, "Processing stdin\n");
  processInput();

  /*
   * The MAIN function in C should always return 0 as a way of telling
   * whatever program invoked this that everything went OK.
   */
  return 0;
}

/*
 * This should hash a string to a bucket index.  void *s can be safely cast
 * to a char * (null terminated string)
 */
unsigned int stringHash(void *s)
{
  // -- TODO --
  char *string = (char *)s;
  int out = 0;
  while (*string != '\0')
  {
    out = out * 31;
    out += *string;
    string += 1;
  }

  return out;
}

/*
 * This should return a nonzero value if the two strings are identical
 * (case sensitive comparison) and 0 otherwise.
 */
int stringEquals(void *s1, void *s2)
{
  // -- TODO --

  char *string1 = (char *)s1;
  char *string2 = (char *)s2;
  if (strlen(string1) != strlen(string2))
  {
    return 0;
  }
  while (*string1 != '\0')
  {
    if (*string1 != *string2)
    {
      return 0;
    }
    string1 += 1;
    string2 += 1;
  }

  return 1;
}

/*
 * This function should read in every word and replacement from the dictionary
 * and store it in the hash table.  You should first open the file specified,
 * then read the words one at a time and insert them into the dictionary.
 * Once the file is read in completely, return.  You will need to allocate
 * (using malloc()) space for each word.  As described in the spec, you
 * can initially assume that no word is longer than 60 characters.  However,
 * for the final bit of your grade, you cannot assumed that words have a bounded
 * length.  You CANNOT assume that the specified file exists.  If the file does
 * NOT exist, you should print some message to standard error and call exit(61)
 * to cleanly exit the program.
 */
void readDictionary(char *dictName)
{
  // -- TODO --
  FILE *fp = NULL;
  fp = fopen(dictName, "r");
  if (fp == NULL)
  {
    fprintf(stderr, "open file failed \n");
    exit(61);
  }
  
}

/*
 * This should process standard input (stdin) and perform replacements as
 * described by the replacement set then print either the original text or
 * the replacement to standard output (stdout) as specified in the spec (e.g.,
 * if a replacement set of `taest test\n` was used and the string "this is
 * a taest of  this-proGram" was given to stdin, the output to stdout should be
 * "this is a test of  this-proGram").  All words should be checked
 * against the replacement set as they are input, then with all but the first
 * letter converted to lowercase, and finally with all letters converted
 * to lowercase.  Only if all 3 cases are not in the replacement set should
 * it report the original word.
 *
 * Since we care about preserving whitespace and pass through all non alphabet
 * characters untouched, scanf() is probably insufficent (since it only considers
 * whitespace as breaking strings), meaning you will probably have
 * to get characters from stdin one at a time.
 *
 * Do note that even under the initial assumption that no word is longer than 60
 * characters, you may still encounter strings of non-alphabetic characters (e.g.,
 * numbers and punctuation) which are longer than 60 characters. Again, for the
 * final bit of your grade, you cannot assume words have a bounded length.
 */
void processInput()
{
  int set_num;
  printf("number of replacement sets: ");
  scanf("%d", &set_num);
  char *k[set_num];
  char *v[set_num];
  
  for (int i = 0; i < set_num; i += 1){
    k[i] = malloc(sizeof(char) * 60);
    v[i] = malloc(sizeof(char) * 60);
    printf("replacement set: ");
    if (i == set_num - 1) {
      scanf("%s %s\n", k[i], v[i]);
    } else {
      scanf("%s %s", k[i], v[i]);
    }
    insertData(dictionary, k[i], v[i]);
  }

  char *inputs;
  inputs = stringInput();

  char *outputs = stringProcess(inputs);
  printf("output string: %s\n", outputs);
}

char *stringProcess(char *inputs)
{
  int input_length = strlen(inputs);

  /* may cause bug!!!!!!! */
  char *outputs = malloc(sizeof(char) * ((input_length + 5) * 10 ));

  const char s[2] = " ";
  char *token;
  char *find;
  token = strtok(inputs, s);
  if (token == NULL)
  {
    return NULL;
  }
  while (token != NULL)
  {
    find = convert(token);
    strcat(outputs, find);
    strcat(outputs, " ");
    token = strtok(NULL, s);
  }
  outputs[strlen(outputs)-1] = '\0';
  free(inputs);
  return outputs;
}

char *stringInput()
{
  char tmp = '\0';
  int strsize = 64;
  char *str = malloc(sizeof(char) * strsize);
  for (int i = 0; 1; i++)
  {
    tmp = getchar();
    if ((tmp == '\r') || (tmp == '\n'))
      break;
    if (i >= strsize)
    {
      strsize = 2 * strsize;
      str = (char *)realloc(str, strsize);
    }
    if (str == NULL)
    {
      fprintf(stderr, "realloc failed \n");
      exit(1);
    }
    str[i] = tmp;
  }
  return str;
}

char *convert(char* word) {
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