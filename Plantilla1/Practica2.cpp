#include <mpi.h>
#include <iostream>
#include <cstdlib>              // Numeros aleatorios

using namespace std;

constexpr int TAM_MATRIZ = 4;   // Tamaño de la matriz
constexpr int RANK_MASTER = 0;  // Rango del proceso maestro

/*
    Función que se necarga de generar una matriz de tamaño N x N con valores aleatorios.

    @param matrix: matriz que almacena la matriz.
*/
void generateMatrix(int matrix[TAM_MATRIZ][TAM_MATRIZ]);
/*
    Función que se encarga de imprimir una matriz de tamaño N x N.

    @param matrix: matriz que almacena la matriz.
*/
void printMatrix(int matrix[TAM_MATRIZ][TAM_MATRIZ]);
/*
    Función que se encarga de imprimir una línea horizontal de la matriz.
*/
void printLine();

/*
    Función principal.
*/
int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);

    int rank, size = 0;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Verificar que el número de procesos sea igual al tamaño de la matriz
    if (size != TAM_MATRIZ) {
        // Imprimir un mensaje de error en el proceso 0, evitando que los demás procesos muestren el mismo mensaje
        if (rank == RANK_MASTER) {
            fprintf(stderr, "El número de procesos debe ser igual al tamaño de la matriz (%d).\n", TAM_MATRIZ);
        }
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    // Inicializar las matrices A, B y C
    int A[TAM_MATRIZ][TAM_MATRIZ], B[TAM_MATRIZ][TAM_MATRIZ], C[TAM_MATRIZ][TAM_MATRIZ];
    // Inicializar las matrices locales, que almacenan las filas correspondientes a cada proceso
    int localA[TAM_MATRIZ], localB[TAM_MATRIZ], localC[TAM_MATRIZ];

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
    double startTime = MPI_Wtime();

    // Distribuir las filas de las matrices A y B a todos los procesos
    MPI_Bcast(A, TAM_MATRIZ * TAM_MATRIZ, MPI_INT, RANK_MASTER, MPI_COMM_WORLD);
    MPI_Bcast(B, TAM_MATRIZ * TAM_MATRIZ, MPI_INT, RANK_MASTER, MPI_COMM_WORLD);

    // Cada proceso extrae su parte correspondiente
    for (int i = 0; i < TAM_MATRIZ; ++i) {
        localA[i] = A[rank][i];
        localB[i] = B[rank][i];
    }

    // Sumar las filas correspondientes
    for (int i = 0; i < TAM_MATRIZ; ++i) {
        localC[i] = localA[i] + localB[i];
    }

    // Recopilar los resultados en la matriz C en el proceso maestro
    MPI_Gather(localC, TAM_MATRIZ, MPI_INT, C[rank], TAM_MATRIZ, MPI_INT, RANK_MASTER, MPI_COMM_WORLD);

    // Medir el tiempo de finalización
    double endTime = MPI_Wtime();

    if (rank == RANK_MASTER) {
        printf("Matriz C (resultado A+B):\n");
        printMatrix(C);
        printf("Tiempo de ejecución: %f segundos\n", endTime - startTime);
    }

    MPI_Finalize();
    return 0;
}

void generateMatrix(int matrix[TAM_MATRIZ][TAM_MATRIZ]) {
    for (int i = 0; i < TAM_MATRIZ; ++i) {
        for (int j = 0; j < TAM_MATRIZ; ++j) {
            matrix[i][j] = rand() % 10; // Números aleatorios entre 0 y 9
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
    // Imprimir la línea superior
    printLine();

    for (int i = 0; i < TAM_MATRIZ; ++i) {
        // Imprimir los valores de la fila
        printf("|");
        for (int j = 0; j < TAM_MATRIZ; ++j) {
            printf(" %2d |", matrix[i][j]);
        }
        printf("\n");

        // Imprimir las líneas intermedias e inferior de la fila
        printLine();
    }
}


/*
1.	¿Las comunicaciones colectivas facilitan la programación y simplifican el código? ¿Se puede pensar que acortan el tiempo de ejecución de los programas?
•	Las comunicaciones colectivas facilitan la programación y simplifican el código al reducir la cantidad de llamadas a funciones de envío y recepción. Sin embargo, no necesariamente acortan el tiempo de ejecución, ya que el tiempo de comunicación depende de la implementación de MPI y de la red de interconexión.

2.	Explicar qué refleja la medida de tiempo realizada.
•	La medida de tiempo realizada con MPI_Wtime refleja el tiempo total de ejecución del programa, incluyendo tanto el tiempo de comunicación entre procesos como el tiempo de cálculo.

3.	Plantear otras posibilidades de medida de tiempos de ejecución que permitan distinguir los tiempos invertidos en comunicación entre procesos y los tiempos dedicados al cálculo.
•	Para distinguir los tiempos de comunicación y cálculo, se pueden realizar mediciones separadas:
•	Medir el tiempo antes y después de las operaciones de comunicación (e.g., MPI_Scatter y MPI_Gather).
•	Medir el tiempo antes y después de las operaciones de cálculo (e.g., la suma de las filas).
•	Restar los tiempos de comunicación del tiempo total para obtener el tiempo de cálculo.

*/