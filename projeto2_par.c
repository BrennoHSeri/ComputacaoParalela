#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>

#define TAM 10 // Definindo a variável global TAM

void popularMatrizAleatoria(int *matriz, int tamanho) {
    srand(time(NULL)); // Semente única para todos os processos
    for (int i = 0; i < tamanho * TAM; i++) {
        matriz[i] = rand() % 100; // Gera um número aleatório entre 0 e 99
    }
}

void imprimirMatriz(int *matriz, int linhas_por_processo) {
    for (int i = 0; i < linhas_por_processo; i++) {
        for (int j = 0; j < TAM; j++) {
            printf("%d ", matriz[i * TAM + j]);
        }
        printf("\n");
    }
}

int encontrarMaiorElemento(int *matriz, int tamanho) {
    int maior = matriz[0];
    for (int i = 1; i < tamanho * TAM; i++) {
        if (matriz[i] > maior) {
            maior = matriz[i];
        }
    }
    return maior;
}

int encontrarMenorElemento(int *matriz, int tamanho) {
    int menor = matriz[0];
    for (int i = 1; i < tamanho * TAM; i++) {
        if (matriz[i] < menor) {
            menor = matriz[i];
        }
    }
    return menor;
}

void calcularSomaLinhas(int *matriz, int *somas_linhas, int linhas_por_processo) {
    for (int i = 0; i < linhas_por_processo; i++) {
        somas_linhas[i] = 0;
        for (int j = 0; j < TAM; j++) {
            somas_linhas[i] += matriz[i * TAM + j];
        }
    }
}

void calcularSomaColunas(int *matriz, int *somas_colunas, int linhas_por_processo) {
    for (int j = 0; j < TAM; j++) {
        somas_colunas[j] = 0;
        for (int i = 0; i < linhas_por_processo; i++) {
            somas_colunas[j] += matriz[i * TAM + j];
        }
    }
}

int main(int argc, char *argv[]) {
    int meu_rank, num_processos;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &meu_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_processos);

    int linhas_por_processo = TAM / num_processos;
    int *matriz_local = (int *)malloc(linhas_por_processo * TAM * sizeof(int));

    int *matriz_completa = NULL;
    if (meu_rank == 0) {
        matriz_completa = (int *)malloc(TAM * TAM * sizeof(int));
        popularMatrizAleatoria(matriz_completa, TAM);
        printf("Matriz Completa (Processo 0):\n");
        imprimirMatriz(matriz_completa, TAM);
        printf("\n");
    }

    double tempos[15]; // Armazenar os tempos de execução
    for (int t = 0; t < 15; t++) {
        MPI_Barrier(MPI_COMM_WORLD);
        double tempo_inicio = MPI_Wtime(); // Medir o tempo inicial

        MPI_Scatter(matriz_completa, linhas_por_processo * TAM, MPI_INT,
                    matriz_local, linhas_por_processo * TAM, MPI_INT, 0, MPI_COMM_WORLD);

        int menor_local = encontrarMenorElemento(matriz_local, linhas_por_processo);
        int maior_local = encontrarMaiorElemento(matriz_local, linhas_por_processo);

        int menor_global, maior_global;

        MPI_Reduce(&menor_local, &menor_global, 1, MPI_INT, MPI_MIN, 0, MPI_COMM_WORLD);
        MPI_Reduce(&maior_local, &maior_global, 1, MPI_INT, MPI_MAX, 0, MPI_COMM_WORLD);

        int *somas_linhas = (int *)malloc(linhas_por_processo * sizeof(int));
        int *somas_colunas = (int *)malloc(TAM * sizeof(int));

        calcularSomaLinhas(matriz_local, somas_linhas, linhas_por_processo);
        calcularSomaColunas(matriz_local, somas_colunas, linhas_por_processo);

        int *soma_colunas_global = (int *)malloc(TAM * sizeof(int));
        int *soma_linhas_global = NULL;
        if (meu_rank == 0) {
            soma_linhas_global = (int *)malloc(TAM * sizeof(int));
        }

        MPI_Gather(somas_linhas, linhas_por_processo, MPI_INT, soma_linhas_global, linhas_por_processo, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Reduce(somas_colunas, soma_colunas_global, TAM, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

        if (meu_rank == 0) {
            printf("Soma das Linhas:\n");
            for (int i = 0; i < TAM; i++) {
                printf("Soma Linha %d = %d\n", i, soma_linhas_global[i]);
            }
            printf("\n");

            printf("Soma das Colunas:\n");
            for (int j = 0; j < TAM; j++) {
                printf("Soma Coluna %d = %d\n", j, soma_colunas_global[j]);
            }
            printf("\n");
        }

        free(somas_linhas);
        free(somas_colunas);
        free(soma_linhas_global);
        free(soma_colunas_global);

        MPI_Barrier(MPI_COMM_WORLD);
        double tempo_fim = MPI_Wtime(); // Medir o tempo final

        tempos[t] = tempo_fim - tempo_inicio; // Calcular o tempo de execução
    }

    if (meu_rank == 0) {
        printf("\nTempos de Execução: [");
        for (int i = 0; i < 15; i++) {
            printf("%f", tempos[i]);
            if (i < 14) {
                printf(", ");
            }
        }
        printf("]\n");
    }

    free(matriz_local);
    free(matriz_completa);
    MPI_Finalize();
    return 0;
}
