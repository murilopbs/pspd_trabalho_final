#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <omp.h>
#include <mpi.h>

#define POWMIN 3
#define POWMAX 10

#define ind2d(i,j) (i)*(global_tam+2)+j

double wall_time(void) {
    struct timeval tv;
    struct timezone tz;
    gettimeofday(&tv, &tz);
    return(tv.tv_sec + tv.tv_usec/1000000.0);
}

void UmaVida(int* tabulIn, int* tabulOut, int local_tam, int global_tam) {
    int i, j;
    #pragma omp parallel for private(j)
    for (i = 1; i <= local_tam; i++) {
        for (j = 1; j <= global_tam; j++) {
            int vizviv = tabulIn[ind2d(i-1,j-1)] + tabulIn[ind2d(i-1,j)] +
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

void InitTabul(int* tabul, int global_tam) {
    int ij;
    for (ij = 0; ij < (global_tam + 2) * (global_tam + 2); ij++) {
        tabul[ij] = 0;
    }
    tabul[ind2d(1,2)] = 1;
    tabul[ind2d(2,3)] = 1;
    tabul[ind2d(3,1)] = 1;
    tabul[ind2d(3,2)] = 1;
    tabul[ind2d(3,3)] = 1;
}

int main(int argc, char* argv[]) {
    int pow, i, global_tam, *tabulIn, *tabulOut, *temp_ptr;
    double t0, t1, t2, t3;

    MPI_Init(&argc, &argv);
    int rank, num_procs;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    for (pow = POWMIN; pow <= POWMAX; pow++) {
        global_tam = 1 << pow;

        int local_tam = global_tam / num_procs;
        if (rank == num_procs - 1) {
            local_tam += global_tam % num_procs;
        }

        int local_mem_size = (local_tam + 2) * (global_tam + 2);
        tabulIn = (int*) calloc(local_mem_size, sizeof(int));
        tabulOut = (int*) calloc(local_mem_size, sizeof(int));
        
        t0 = 0.0; t1 = 0.0; t2 = 0.0; t3 = 0.0;

        MPI_Barrier(MPI_COMM_WORLD); 

        if (rank == 0) {
            t0 = wall_time();
            int* full_board = (int*) calloc((global_tam + 2) * (global_tam + 2), sizeof(int));
            InitTabul(full_board, global_tam);

            for(i = 0; i < local_tam + 2; ++i)
                for(int j = 0; j < global_tam + 2; ++j)
                    tabulIn[ind2d(i,j)] = full_board[ind2d(i,j)];

            for (i = 1; i < num_procs; i++) {
                int start_other = i * (global_tam / num_procs);
                int rows_other = (i == num_procs - 1) ? (global_tam/num_procs) + (global_tam % num_procs) : (global_tam/num_procs);
                MPI_Send(&full_board[ind2d(start_other, 0)], (rows_other+2) * (global_tam+2), MPI_INT, i, 0, MPI_COMM_WORLD);
            }
            free(full_board);
        } else {
            MPI_Recv(tabulIn, local_mem_size, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
        
        MPI_Barrier(MPI_COMM_WORLD);
        if(rank==0) t1 = wall_time();

        MPI_Request reqs[4];
        for (i = 0; i < 2 * (global_tam - 3); i++) {
            int prev_rank = (rank == 0) ? MPI_PROC_NULL : rank - 1;
            int next_rank = (rank == num_procs - 1) ? MPI_PROC_NULL : rank + 1;

            MPI_Isend(&tabulIn[ind2d(1, 0)], global_tam + 2, MPI_INT, prev_rank, 0, MPI_COMM_WORLD, &reqs[0]);
            MPI_Isend(&tabulIn[ind2d(local_tam, 0)], global_tam + 2, MPI_INT, next_rank, 0, MPI_COMM_WORLD, &reqs[1]);
            MPI_Irecv(&tabulIn[ind2d(local_tam + 1, 0)], global_tam + 2, MPI_INT, next_rank, 0, MPI_COMM_WORLD, &reqs[2]);
            MPI_Irecv(&tabulIn[ind2d(0, 0)], global_tam + 2, MPI_INT, prev_rank, 0, MPI_COMM_WORLD, &reqs[3]);
            
            MPI_Waitall(4, reqs, MPI_STATUSES_IGNORE);

            UmaVida(tabulIn, tabulOut, local_tam, global_tam);
            
            temp_ptr = tabulIn;
            tabulIn = tabulOut;
            tabulOut = temp_ptr;
        }
        
        MPI_Barrier(MPI_COMM_WORLD);

        if (rank == 0) {
            t2 = wall_time();
            printf("**Resultado (parcial), verificação de corretude desativada no modo Híbrido**\n");
            t3 = wall_time();
            printf("tam=%d; num_procs=%d; tempos: init=%7.7f, comp=%7.7f, fim=%7.7f tot=%7.7f \n\n",
                   global_tam, num_procs, t1 - t0, t2 - t1, t3 - t2, t3 - t0);
        }

        free(tabulIn);
        free(tabulOut);
    }

    MPI_Finalize();
    return 0;
}
