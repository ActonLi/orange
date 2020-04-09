
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define TEST 5

static int test5(int argc, char** argv) 
{
    char hexs[8] = {0};
    char *p;
    int len;
    int total;
    int i;

    while (scanf("%s", hexs) != EOF) {
        len = strlen(hexs);

        if (len > 2 && *hexs == '0' && (*(hexs+1) == 'x' || *(hexs+1) == 'X')) {
            p = hexs + 2;
            len -= 2;
        }
        else {
            p = hexs;
        }

        total = 0;

        for (i = len - 1; i >= 0; i--) {
            if(*(p+i) >= '0' && *(p+i) <= '9') {
                total += (*(p+i) - 48)*pow(16, len - i - 1);
            } else if(*(p+i) >= 'A' && *(p+i) <= 'F') {
                total += (*(p+i) - 55)*pow(16, len - i - 1);
            } else if (*(p+i) >= 'a' && *(p+i) <= 'f') {
                total += (*(p+i) - 87)*pow(16, len - i - 1);
            }
        } 

        printf("%d\n", total);

        memset(hexs, 0, 8);
    }

    return 0;
}

static int test4(int argc, char** argv) 
{
    char str[128] = {0};
    int len = 0;
    int i, j;
    int pos;

    while (scanf("%s", str) != EOF) {
        len = strlen(str);
        pos = 0;

        while (len > 0) {
            for (i = 0; i < 8 && i < len; i++) {
                printf("%c", str[pos++]);
            }

            if (i < 8) {
                for (j = 0; j < 8 - i; j++) {
                    printf("0");
                }
            }
            len -= i;
            printf("\n");
        }
        
        memset(str, 0, 128);
    }

    return 0;
}

static int test3(int argc, char** argv)
{
    int count;
    int i,j;
    int tmp;

    while (scanf("%d", &count) != EOF) {
        int* p = malloc(count * sizeof(int));
        if (p) {
            memset(p, 0, count * sizeof(int));
        }
        for (i = 0; i < count; i++) {
            scanf("%d", p+i);
        }

        for (i = 0; i < count; i++) {
            for (j = i+1; j < count; j++) {
                if (*(p+i) > *(p+j)) {
                    tmp = *(p+i);
                    *(p+i) = *(p+j);
                    *(p+j) = tmp;
                }
            }
        }


        for (i = 0; i < count; i++) {
            if (i == 0) {
                printf("%d\n", *(p+i));
            } else {
                if(*(p+i-1) != *(p+i) ) {
                    printf("%d\n", *(p+i));
                }
            }
        }

        free(p);
    }

    return 0;
}

static int test2(int argc, char** argv)
{
    char buf[1024] = {0};
    char chr = 0;
    char* p = buf;
    int count = 0;

    scanf("%s %c", buf, &chr);

    printf("buf: %s, chr: %c\n", buf, chr);

    while (*p != 0) {
        if (*p >= 'a' && *p <= 'z') {
            if (*p == chr || (*p - 32) == chr) {
                count++;
            }
        }
        else if (*p >= 'A' && *p <= 'Z') {
            if (*p == chr || (*p + 32) == chr) {
                count++;
            }
        }
        p++;
    }

    printf("count: %d\n", count);


    return 0;
}

static int test1(int argc, char** argv)
{
    int len = 0;
    char c;
    
    while((c = getchar()) != '\n') {
        if(c == ' ') {
            len = 0;
        }
        else {
            len++;
        }
    }
    
    printf( "%d\n", len );
    
    return 0;
}

int main(int argc, char** argv)
{
    int test = TEST;

    switch (test) {
        case 1:
            test1(argc, argv);
            break;
        case 2:
            test2(argc, argv);
            break;
        case 3:
            test3(argc, argv);
            break;
        case 4:
            test4(argc, argv);
            break;
        case 5:
            test5(argc, argv);
            break;
    }

	return 0;
}
