#include <stdio.h>
#include <string.h>
#include <stdlib.h>


int main(int argc, char** argv)
{
    
    unsigned int arr[8] = {73, 74, 75, 71, 69, 72, 76, 80};
    int i, j;
    int count;
    int find;

    int pe = 0, pt = 0;

    while (pe < 8) {
        for (pt = pe; pt < 8; pt++) {
            count++;
            if (arr[pt] > arr[pe]) {
                break;
            }
        }
        pe++;
    }
    

    for (i = 0; i < 8; i++) {
        count = 0;
        find = 0;
        for (j = i + 1; j < 8; j++) {
            count++;
            if (arr[j] > arr[i]) {
                find = 1;
                break;
            }
        }

        if (find == 0) {
            count = 0;
        }

        printf("%d ", count);
    }

    printf ("\n");

    return 0;
}


