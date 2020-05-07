#include <stdio.h>

#define RESULT mul(4,6)

int mul(int a, int b){

    return (a*b);
}

#define TOTO mul(4,7)

int main(){

    printf("REsult = %i\n",RESULT);

    printf("Toto = %d\n", TOTO);
}
