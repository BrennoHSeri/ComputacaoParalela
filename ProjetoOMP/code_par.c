#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>
typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} Pixel;

void applySobelOperator(Pixel *imagem, int width, int height) {
    uint8_t *magnitude = malloc(sizeof(uint8_t) * width * height);
    int maxMagnitude = 0;
    #pragma omp parallel for simd shared(imagem, magnitude) reduction(max:maxMagnitude) collapse(2)
    for (int y = 1; y < height - 1; y++) {
        for (int x = 1; x < width - 1; x++) {
            int pixel_x_r = ((-1) * imagem[(y - 1) * width + (x - 1)].r) + ((-2) * imagem[y * width + (x - 1)].r) + ((-1) * imagem[(y + 1) * width + (x - 1)].r) +
                            (1 * imagem[(y - 1) * width + (x + 1)].r) + (2 * imagem[y * width + (x + 1)].r) + (1 * imagem[(y + 1) * width + (x + 1)].r);

            int pixel_y_r = ((-1) * imagem[(y - 1) * width + (x - 1)].r) + ((-2) * imagem[(y - 1) * width + x].r) + ((-1) * imagem[(y - 1) * width + (x + 1)].r) +
                            (1 * imagem[(y + 1) * width + (x - 1)].r) + (2 * imagem[(y + 1) * width + x].r) + (1 * imagem[(y + 1) * width + (x + 1)].r);

            magnitude[y * width + x] = sqrt((pixel_x_r * pixel_x_r) + (pixel_y_r * pixel_y_r));

            if (magnitude[y * width + x] > maxMagnitude) {
                maxMagnitude = magnitude[y * width + x];
            }
        }
    }

    #pragma omp parallel for simd collapse(2) shared(imagem, magnitude, maxMagnitude)
    for (int y = 1; y < height - 1; y++) {
        for (int x = 1; x < width - 1; x++) {
            imagem[y * width + x].r = (magnitude[y * width + x] * 255) / maxMagnitude;
            imagem[y * width + x].g = (magnitude[y * width + x] * 255) / maxMagnitude;
            imagem[y * width + x].b = (magnitude[y * width + x] * 255) / maxMagnitude;
        }
    }

    free(magnitude);
}

int main(void) {
    FILE *fp = fopen("/content/drive/MyDrive/Trabalho_CAD/Images/macaco.bmp", "r+b");
    if (fp == NULL) {
	printf("Erro ao abrir o arquivo.");
	return 1;
    }

    int width, height;
    fseek(fp, 18, SEEK_SET);
    fread(&width, sizeof(int), 1, fp);
    fread(&height, sizeof(int), 1, fp);

    Pixel *imagem = malloc(sizeof(Pixel) * width * height); // Array unidimensional para representar a imagem

    fseek(fp, 54, SEEK_SET);
    fread(imagem, sizeof(Pixel), width * height, fp); // Lê a imagem diretamente no array de pixels

    omp_set_num_threads(10);
    double start_time = omp_get_wtime();
    applySobelOperator(imagem, width, height);
    double end_time = omp_get_wtime();
    double execution_time = end_time - start_time;
    printf("tempo :%f , ", execution_time);
    fseek(fp, 54, SEEK_SET);
    //fwrite(imagem, sizeof(Pixel), width * height, fp);
    fclose(fp);
    free(imagem);
    return 0;
}
