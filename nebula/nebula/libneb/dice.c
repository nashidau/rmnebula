#include <stdlib.h>
#include <time.h>

/* static GRand *grand; */

void dice_init(void){
        srand(time(NULL));
        /* grand = g_rand_new();*/
}

int d10(void){
        return 1+(int) (10.0*rand()/(RAND_MAX+1.0));
        /* return g_rand_int_range(grand,1,10); */
}

int d100(void){
        return 1+(int) (100.0*rand()/(RAND_MAX+1.0));
        /*return g_rand_int_range(grand,1,100);*/
}

int d100OEH(void){
        int i;
        i = d100();
        if (i > 95)
                return i + d100OEH();
        return i;
}

int d100OEL(void){
        int i;
        i = d100();
        if (i < 6)
                return i - d100OEH();
        return i;
}

int d100OE(void){
        int i;
        i = d100();
        if (i > 95)
                return i + d100OEH();
        else if (i < 6)
                return i - d100OEH();
        else
                return i;
}

int
nd10(int n){
        int total = 0;
        while(n --)
                total += d10();
        return total;
}

int dX(int X){
        return 1+(int) ((float)X*rand()/(RAND_MAX+1.0));
}

/* Math Functions */

/* Rouding div.  Rounds > 0.5 up.
 * On div by z0 returns 0
 */
int
round_div(int total,int divisor){
        int quotient;
        int remainder;

        if (divisor == 0)
                return 0;
        quotient = total / divisor;
        remainder = total - quotient * divisor;

        if (remainder * 2 >= divisor)
                quotient ++;

        return quotient;
}

