/*
   $Id$
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <trurl/trurl.h>

#define TEST_STRTOKL    1
#define TEST_STRLEN     1


void test_strtokl(void)
{
    const char **tokens, **p;

    printf("\nTEST n_str_tokl\n");
    tokens = n_str_tokl("ala; ma; kota, i psa", ";, ");

    p = tokens;

    while (*p) {
	printf("token: %s\n", *p++);
    }

    n_str_tokl_free(tokens);
}


void test_strlen(void)
{
    unsigned int len;
    char *arr[] =
    {"ala;", "ma;", " kota,", "i psa", ";, "};
    char *s;

    printf("\nTEST n_str_len\n");

    len = n_str_len(arr[0], arr[1], arr[2], arr[3], arr[4], (char *) NULL);
    s = n_str_concat(arr[0], arr[1], arr[2], arr[3], arr[4], (char *) NULL);

    n_assert(strlen(s) == len);

    printf("length of '%s' = %d\n", s, len);

    free(s);
}


int main()
{

#if TEST_STRTOKL
    test_strtokl();
#endif

#if TEST_STRLEN
    test_strlen();
#endif

    return 0;
}
