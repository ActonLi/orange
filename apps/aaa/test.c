#include <stdio.h>
#include <string.h>
#include <stdlib.h>


int main(int argc, char** argv)
{
    char strs[3][64] = {"apple", "apple", "banana"};

    int i, j;
    int count;
     

    for (i = 0; i < 3; i++) {
        count = 0;
        for (j = 0; j < 3; j++) {
            if (i == j) {
                continue;
            }
            if (strcmp(strs[i], strs[j]) == 0) {
                count++;
            }
        }
        if (count == 0) {
            printf("%s\n", strs[i]);
        }
    }
    

    return 0;
}
