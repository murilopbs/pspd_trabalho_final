#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <omp.h>

#define ind2d(i,j) (i)*(tam+2)+j

#define POWMIN 3
#define POWMAX 10

double wall_time(void) {
    struct timeval tv;
    struct timezone tz;
    gettimeofday(&tv, &tz);
    return(tv.tv_sec + tv.tv_usec/1000000.0);
}

void UmaVida (int* tabulIn, int* tabulOut, int tam) {
    int i, j;

    #pragma omp parallel for private(j)
    for (i=1; i<=tam; i++) {
        for (j=1; j<=tam; j++) {
            int vizviv;
            vizviv = tabulIn[ind2d(i-1,j-1)] + tabulIn[ind2d(i-1,j)] +
                     tabulIn[ind2d(i-1,j+1)] + tabulIn[ind2d(i,j-1)] +
                     tabulIn[ind2d(i,j+1)] + tabulIn[ind2d(i+1,j-1)] +
                     tabulIn[ind2d(i+1,j)] + tabulIn[ind2d(i+1,j+1)];

            if (tabulIn[ind2d(i,j)] && vizviv < 2)
                tabulOut[ind2d(i,j)] = 0;
            else if (tabulIn[ind2d(i,j)] && vizviv > 3)
                tabulOut[ind2d(i,j)] = 0;
            else if (!tabulIn[ind2d(i,j)] && vizviv == 3)
                tabulOut[ind2d(i,j)] = 1;
            else
                tabulOut[ind2d(i,j)] = tabulIn[ind2d(i,j)];
        }
    }
}

void InitTabul (int* tabulIn, int* tabulOut, int tam){
    int ij;
    for (ij=0; ij<(tam+2)*(tam+2); ij++) {
        tabulIn[ij] = 0;
        tabulOut[ij] = 0;
    }
    tabulIn[ind2d(1,2)] = 1;
    tabulIn[ind2d(2,3)] = 1;
    tabulIn[ind2d(3,1)] = 1;
    tabulIn[ind2d(3,2)] = 1;
    tabulIn[ind2d(3,3)] = 1;
}

int Correto (int* tabul, int tam){
    int ij, cnt = 0;
    for (ij=0; ij<(tam+2)*(tam+2); ij++)
        cnt += tabul[ij];

    return (cnt == 5 && tabul[ind2d(tam-2,tam-1)] &&
                         tabul[ind2d(tam-1, tam)] &&
                         tabul[ind2d(tam, tam-2)] &&
                         tabul[ind2d(tam,tam-1)] &&
                         tabul[ind2d(tam, tam)]);
}

int main(void) {
    int pow, i, tam, *tabulIn, *tabulOut;
    double t0, t1, t2, t3;

    for (pow=POWMIN; pow<=POWMAX; pow++) {
        tam = 1 << pow;

        t0 = wall_time();

        tabulIn = (int*) malloc ((tam+2)*(tam+2)*sizeof(int));
        tabulOut = (int*) malloc ((tam+2)*(tam+2)*sizeof(int));
        
        InitTabul (tabulIn, tabulOut, tam);

        t1 = wall_time();

        for (i=0; i<2*(tam-3); i++) {
            UmaVida (tabulIn, tabulOut, tam);
            UmaVida (tabulOut, tabulIn, tam);
        }

        t2 = wall_time();

        if (Correto(tabulIn, tam))
            printf("**Ok, RESULTADO CORRETO**\n");
        else
            printf("**Nok, RESULTADO ERRADO**\n");
        
        t3 = wall_time();

        printf("tam=%d; tempos: init=%7.7f, comp=%7.7f, fim=%7.7f tot=%7.7f \n\n",
               tam, t1-t0, t2-t1, t3-t2, t3-t0);

        free(tabulIn); free(tabulOut);
    }
    return 0;
}
