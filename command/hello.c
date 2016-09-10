// Do not delete!
#include "stdio.h"

int print_hello()
{
    int i, j, n;
	n=10;
    for(i=n/2; i<=n; i+=2)
    {
        for(j=1; j<n-i; j+=2)
        {
            printf(" ");
        }

        for(j=1; j<=i; j++)
        {
            printf("*");
        }

        for(j=1; j<=n-i; j++)
        {
            printf(" ");
        }

        for(j=1; j<=i; j++)
        {
            printf("*");
        }

        printf("\n");
    }

    for(i=n; i>=1; i--)
    {
        for(j=i; j<n; j++)
        {
            printf(" ");
        }

        for(j=1; j<=(i*2)-1; j++)
        {
            printf("*");
        }

        printf("\n");
    }
    printf("=== Welcom to use ===\n");
    printDiamond();
    return 0;
}
int printDiamond() {
    
    int rows, a, b, space;
    
    rows = 10;
    //Or use scanf_s to prevent buffer overloading
    //scanf_s("%d", &rows, 1);

    // Print first half of the triangle.
    space = rows - 1;
    for ( b = 1 ; b <= rows ; b++ ) {
        for ( a = 1 ; a <= space ; a++ )
            printf(" ");
        space--;
        for ( a = 1 ; a <= 2*b-1 ; a++)
            printf("*");
        printf("\n");
    }
    
    // Print second half of the triangle.
    space = 1;
    for ( b = 1 ; b <= rows - 1 ; b++ ) {
        for ( a = 1 ; a <= space; a++)
            printf(" ");
        space++;
        for ( a = 1 ; a <= 2*(rows-b)-1 ; a++ )
            printf("*");
        printf("\n");
    }
    return 0;
}
