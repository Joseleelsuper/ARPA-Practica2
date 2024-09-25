#include <mpi.h>
#include <iostream>             
#include <stdlib.h>             // Numeros pseudoaleatorios

using namespace std;

constexpr int TAM_MATRIZ = 4;   // Tama�o de la matriz
constexpr int RANK_MASTER = 0;  // Rango del proceso maestro

/*
    Funci�n que se necarga de generar una matriz de tama�o N x N con valores aleatorios.

    @param matrix: matriz que almacena la matriz.
*/
void generateMatrix(int matrix[TAM_MATRIZ][TAM_MATRIZ]);
/*
    Funci�n que se encarga de imprimir una matriz de tama�o N x N.

    @param matrix: matriz que almacena la matriz.
*/
void printMatrix(int matrix[TAM_MATRIZ][TAM_MATRIZ]);
/*
    Funci�n que se encarga de imprimir una l�nea horizontal de la matriz.
*/
void printLine();

/*
    Funci�n principal.
*/
int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);

    int rank, size = 0;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Verificar que el n�mero de procesos sea igual al tama�o de la matriz
    if (size != TAM_MATRIZ) {
        // Imprimir un mensaje de error en el proceso 0, evitando que los dem�s procesos muestren el mismo mensaje
        if (rank == RANK_MASTER) {
            fprintf(stderr, "El n�mero de procesos debe ser igual al tama�o de la matriz (%d).\n", TAM_MATRIZ);
        }
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    // Inicializar las matrices A, B y C
    int A[TAM_MATRIZ][TAM_MATRIZ], B[TAM_MATRIZ][TAM_MATRIZ], C[TAM_MATRIZ][TAM_MATRIZ];
    // Inicializar las matrices locales, que almacenan las filas correspondientes a cada proceso
    int localA[TAM_MATRIZ], localB[TAM_MATRIZ], localC[TAM_MATRIZ];

	srand(time(NULL)); // Inicializar la semilla para generar n�meros pseudoaleatorios
    // Generar las matrices A y B en el proceso 0 y mostrarlas
    if (rank == RANK_MASTER) {
        generateMatrix(A);
        generateMatrix(B);
        printf("Matriz A:\n");
        printMatrix(A);
        printf("Matriz B:\n");
        printMatrix(B);
    }

	// Medir el tiempo de inicio
    double commStartTime, commEndTime, calcStartTime, calcEndTime = 0;

    commStartTime = MPI_Wtime();
    // Distribuir las filas de las matrices A y B a todos los procesos
    MPI_Scatter(A, TAM_MATRIZ, MPI_INT, localA, TAM_MATRIZ, MPI_INT, RANK_MASTER, MPI_COMM_WORLD);
    MPI_Scatter(B, TAM_MATRIZ, MPI_INT, localB, TAM_MATRIZ, MPI_INT, RANK_MASTER, MPI_COMM_WORLD);
    commEndTime = MPI_Wtime();

    // Sumar las filas correspondientes
    calcStartTime = MPI_Wtime();
    for (int i = 0; i < TAM_MATRIZ; ++i) {
        localC[i] = localA[i] + localB[i];
    }
    calcEndTime = MPI_Wtime();

    // Recopilar los resultados en la matriz C en el proceso maestro
	commStartTime += MPI_Wtime();
    MPI_Gather(localC, TAM_MATRIZ, MPI_INT, C, TAM_MATRIZ, MPI_INT, RANK_MASTER, MPI_COMM_WORLD);
	commEndTime += MPI_Wtime();

    if (rank == RANK_MASTER) {
        printf("Matriz C (resultado A+B):\n");
        printMatrix(C);
        printf("Tiempo de comunicaci�n: %f segundos\n", commEndTime - commStartTime);
        printf("Tiempo de c�lculo: %f segundos\n", calcEndTime - calcStartTime);
    }

    MPI_Finalize();
    return 0;
}

void generateMatrix(int matrix[TAM_MATRIZ][TAM_MATRIZ]) {
    for (int i = 0; i < TAM_MATRIZ; ++i) {
        for (int j = 0; j < TAM_MATRIZ; ++j) {
            matrix[i][j] = rand() % 10; // N�meros pseudoaleatorios entre 0 y 9
        }
    }
}

void printLine() {
    printf("+");
    for (int j = 0; j < TAM_MATRIZ; ++j) {
        printf("-----");
    }
    printf("+\n");
}

void printMatrix(int matrix[TAM_MATRIZ][TAM_MATRIZ]) {
    // Imprimir la l�nea superior
    printLine();

    for (int i = 0; i < TAM_MATRIZ; ++i) {
        // Imprimir los valores de la fila
        printf("|");
        for (int j = 0; j < TAM_MATRIZ; ++j) {
            printf(" %2d |", matrix[i][j]);
        }
        printf("\n");

        // Imprimir las l�neas intermedias e inferior de la fila
        printLine();
    }
}


/*
1.	�Las comunicaciones colectivas facilitan la programaci�n y simplifican el c�digo? �Se puede pensar que acortan el tiempo de ejecuci�n de los programas?
�	Las comunicaciones colectivas facilitan la programaci�n y simplifican el c�digo al reducir la cantidad de llamadas a funciones de env�o y recepci�n. Sin embargo, no necesariamente acortan el tiempo de ejecuci�n, ya que el tiempo de comunicaci�n depende de la implementaci�n de MPI y de la red de interconexi�n.

2.	Explicar qu� refleja la medida de tiempo realizada.
�	La medida de tiempo realizada con MPI_Wtime refleja el tiempo total de ejecuci�n del programa, incluyendo tanto el tiempo de comunicaci�n entre procesos como el tiempo de c�lculo.

3.	Plantear otras posibilidades de medida de tiempos de ejecuci�n que permitan distinguir los tiempos invertidos en comunicaci�n entre procesos y los tiempos dedicados al c�lculo.
�	Para distinguir los tiempos de comunicaci�n y c�lculo, se pueden realizar mediciones separadas:
�	Medir el tiempo antes y despu�s de las operaciones de comunicaci�n (e.g., MPI_Scatter y MPI_Gather).
�	Medir el tiempo antes y despu�s de las operaciones de c�lculo (e.g., la suma de las filas).
�	Restar los tiempos de comunicaci�n del tiempo total para obtener el tiempo de c�lculo.
double commStartTime, commEndTime, calcStartTime, calcEndTime;

commStartTime = MPI_Wtime();
MPI_Scatter(A.data(), N, MPI_INT, localA.data(), N, MPI_INT, 0, MPI_COMM_WORLD);
MPI_Scatter(B.data(), N, MPI_INT, localB.data(), N, MPI_INT, 0, MPI_COMM_WORLD);
commEndTime = MPI_Wtime();

calcStartTime = MPI_Wtime();
for (int i = 0; i < N; ++i) {
    localC[i] = localA[i] + localB[i];
}
calcEndTime = MPI_Wtime();

commStartTime = MPI_Wtime();
MPI_Gather(localC.data(), N, MPI_INT, C.data(), N, MPI_INT, 0, MPI_COMM_WORLD);
commEndTime = MPI_Wtime();

if (rank == 0) {
    std::cout << "Tiempo de comunicaci�n: " << (commEndTime - commStartTime) << " segundos" << std::endl;
    std::cout << "Tiempo de c�lculo: " << (calcEndTime - calcStartTime) << " segundos" << std::endl;
}
*/