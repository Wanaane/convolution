#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
typedef struct Matrix3D
{
    int row;
    int col;
    int channels;
    double ***data;
} Matrix3D;

// Function initial matrix
double ***Init3DMatrix(int row, int col, int channels)
{
    double ***ptr = (double ***)malloc(row * sizeof(double **));
    for (int i = 0; i < row; i++)
    {
        ptr[i] = (double **)malloc(col * sizeof(double *));
        for (int j = 0; j < col; j++)
        {
            ptr[i][j] = (double *)malloc(channels * sizeof(double));
        }
    }
    return ptr;
}

// Function free memory storing matrix
void free3DMatrix(Matrix3D matrix)
{
    for (int i = 0; i < matrix.row; i++)
    {
        for (int j = 0; j < matrix.col; j++)
        {
            free(matrix.data[i][j]);
        }
        free(matrix.data[i]);
    }
    free(matrix.data);
}

// Function to read file data input
void getData(Matrix3D matrix, const char *fileInputName)
{
    FILE *file = fopen(fileInputName, "r");
    if (file == NULL)
    {
        printf("Can't open file %s.txt\n", fileInputName);
        exit(1);
    }
    else
    {
        for (int k = 0; k < matrix.channels; k++)
        {
            for (int i = 0; i < matrix.row; i++)
            {
                for (int j = 0; j < matrix.col; j++)
                {
                    fscanf(file, "%lf ", &matrix.data[i][j][k]);
                }
            }
        }

        fclose(file);
    }
}

// Function to read kernel data
void getData(Matrix3D listMatrix[], int size, const char *fileInputName)
{
    FILE *file = fopen(fileInputName, "r");
    if (file == NULL)
    {
        printf("Can't open file %s.txt\n", fileInputName);
        exit(1);
    }
    else
    {

        for (int layer = 0; layer < size; layer++)
        {
            for (int k = 0; k < listMatrix[0].channels; k++)
            {
                for (int i = 0; i < listMatrix[0].col; i++)
                {

                    for (int j = 0; j < listMatrix[0].row; j++)

                    {
                        {
                            fscanf(file, "%lf", &listMatrix[layer].data[i][j][k]);
                        }
                    }
                }
            }

            fclose(file);
        }
    }
}
// function get data from file to bias

void getData(double bias[], int size, const char *fileInputName)
{
    FILE *file = fopen(fileInputName, "r");
    if (file == NULL)
    {
        printf("Can't open file %s.txt\n", fileInputName);
        exit(1);
    }
    else
    {
        for (int i = 0; i < size; i++)
        {
            fscanf(file, "%lf", &bias[i]);
        }

        fclose(file);
    }
}

// Function to print 3D-matrix to console
void print3DMatrix(Matrix3D matrix)
{
    for (int k = 0; k < matrix.channels; k++)
    {
        printf("\t\tLAYER %d\n", k + 1);
        for (int i = 0; i < matrix.row; i++)
        {
            for (int j = 0; j < matrix.col; j++)
            {
                printf("%.2f ", matrix.data[i][j][k]);
            }
            printf("\n");
        }
        printf("\n\n");
    }
}

// Function used to add padding to a 3D matrix
void paddingFunc(Matrix3D &matrix, int stride, int filterSize)
{
    int padding = ((matrix.row - 1) * stride - matrix.row + filterSize) / 2;

    Matrix3D newMatrix;
    newMatrix.channels = matrix.channels;
    newMatrix.row = matrix.row + 2 * padding;
    newMatrix.col = matrix.col + 2 * padding;
    newMatrix.data = Init3DMatrix(newMatrix.row, newMatrix.col, newMatrix.channels);

    int inputCol = 0;
    int inputRow = 0;
    for (int k = 0; k < newMatrix.channels; k++)
    {
        for (int i = 0; i < newMatrix.row; i++)
        {
            for (int j = 0; j < newMatrix.col; j++)
            {
                if (i == 0 || i == newMatrix.row - 1 || j == 0 || j == newMatrix.col - 1)
                    newMatrix.data[i][j][k] = 0;
                else
                    newMatrix.data[i][j][k] = matrix.data[inputRow][inputCol++][k];
            }
            inputCol = 0;
            if (!(i == 0 || i == newMatrix.row - 1))
                inputRow++;
        }
        inputRow = 0;
    }
    matrix = newMatrix;
}
// Function to handle convolution of 3D-matrix input and filter.
Matrix3D convolution(Matrix3D input, Matrix3D kernel[], int numOfKernel, double bias[], int stride)
{
    Matrix3D output;
    output.row = (input.row - kernel[0].row) / stride + 1;
    output.col = (input.col - kernel[0].col) / stride + 1;
    output.channels = numOfKernel;
    output.data = Init3DMatrix(output.row, output.col, output.channels);

    double conv;
    int outputCol = 0, outputRow = 0;

    for (int layer = 0; layer < output.channels; layer++)
    {
        for (int row = 0; row + kernel[layer].row <= input.row; row += stride)
        {
            for (int col = 0; col + kernel[layer].col <= input.col; col += stride)
            {
                conv = bias[layer];
                for (int i = 0; i < kernel[layer].row; i++)
                {
                    for (int j = 0; j < kernel[layer].col; j++)
                    {
                        for (int k = 0; k < kernel[layer].channels; k++)
                        {
                            conv += input.data[row + i][col + j][k] * kernel[layer].data[i][j][k];
                        }
                    }
                }
                output.data[outputRow][outputCol++][layer] = conv;
            }
            outputRow++;
            outputCol = 0;
        }
        outputRow = 0;
    }
    return output;
}
void exportOutput(Matrix3D ouput, const char *fileName)
{
    FILE *file = fopen(fileName, "w");
    if (file == NULL)
    {
        printf("Can't open file %s.txt\n", fileName);
        exit(1);
    }
    else
    {
        for (int k = 0; k < ouput.channels; k++)
        {
            for (int i = 0; i < ouput.col; i++)
            {
                for (int j = 0; j < ouput.col; j++)
                {
                    fprintf(file, "%e\t", ouput.data[i][j][k]);
                }
                fprintf(file, "\n");
            }
            fprintf(file, "\n\n\n");
        }

        fclose(file);
    }
}
void ReLU(Matrix3D &output)
{
    for (int i = 0; i < output.row; i++)
    {
        for (int j = 0; j < output.col; j++)
        {
            for (int k = 0; k < output.channels; k++)
            {
                if (output.data[i][j][k] < 0)
                {
                    output.data[i][j][k] = 0;
                }
            }
        }
    }
}

int main()
{
    const char *inputFileName = "data/data2/ship_n.txt";
    const char *biasFileName = "data/data2/bias_0.txt";
    const char *kernelFileName = "data/data2/W_0.txt";

    // Initial kernel and read its data
    int numOfKernel = 32;
    int kernelSize = 3;
    Matrix3D kernel[32];

    for (int i = 0; i < numOfKernel; i++)
    {
        kernel[i].row = kernel[i].col = kernel[i].channels = kernelSize;
        kernel[i].data = Init3DMatrix(kernel[i].row, kernel[i].col, kernel[i].channels);
    }
    getData(kernel, numOfKernel, kernelFileName);

    // Read data of input
    Matrix3D input;
    input.col = input.row = 32;
    input.channels = 3;
    input.data = Init3DMatrix(input.row, input.col, input.channels);

    getData(input, inputFileName);

    // Read data of bias
    double bias[32];
    int numOfBias = numOfKernel;
    getData(bias, numOfBias, biasFileName);

    // Padding input
    int stride = 1;
    paddingFunc(input, stride, kernelSize);

    // // Calculate con
    Matrix3D output = convolution(input, kernel, numOfKernel, bias, stride);

    // ReLU Layer
    ReLU(output);

    // Export Data
    exportOutput(output, "out.txt");
    return 0;
}
