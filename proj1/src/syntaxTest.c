#include "syntaxTest.h"
#include "philphix.h"
#include "hashtable.h"
#include <stdlib.h>
#include <stdio.h>

char *stringInput();

int testStr(void *item)
{
    printf("%s\n", (char *)item);
    return 0;
}

int main(int argc, char **argv)
{
    /*
    FILE *fp = NULL;
    FILE *fp2 = NULL;
    char buff[255];
    const int init_size = 10;
    int curr_size;

    char *str1 = malloc(sizeof(char)*init_size);
    char *str2 = malloc(sizeof(char)*init_size);
    int n = 0;

    fp = fopen("test.txt", "r");

    fp2 = fopen("asjdlasfn.txt", "r");

    while (n != -1) {
        curr_size = init_size;
        n = fscanf(fp, "%s %s", str1, str2);
        if (strlen(str1) > curr_size || strlen(str2) > curr_size) {
            curr_size = 2 * curr_size;
            str1 = realloc(sizeof(char)*curr_size);
            str2 = realloc(sizeof(char)*curr_size);
        }
        printf("%lu\n", strlen(str1));
        printf("%s\n", str1);
        printf("%s\n", str2);
        printf("%d\n", n);
    }


    free(str1);
    free(str2);
    */
    printf("%d\n", strlen("ok"));
}

char *stringInput()
{
    char tmp = '\0';
    int strsize = 2;
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
    printf("inside function: %s\n", str);
    return str;
}