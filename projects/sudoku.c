#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

typedef struct
{
    int row;
    int column;
} parameters;

int board[9][9] = {
    {5, 3, 4, 6, 7, 8, 9, 1, 2}, {6, 7, 2, 1, 9, 5, 3, 4, 8}, {1, 9, 8, 3, 4, 2, 5, 6, 7}, {8, 5, 9, 7, 6, 1, 4, 2, 3}, {4, 2, 6, 8, 5, 3, 7, 9, 1}, {7, 1, 3, 9, 2, 4, 8, 5, 6}, {9, 6, 1, 5, 3, 7, 2, 8, 4}, {2, 8, 7, 4, 1, 9, 6, 3, 5}, {3, 4, 5, 2, 8, 6, 1, 7, 9}};

int valid[11];

void *check_rows(void *param)
{
    for (int i = 0; i < 9; i++)
    {
        int mask = 0;
        for (int j = 0; j < 9; j++)
            mask |= (1 << board[i][j]);
        if (mask != 1022)
        {
            valid[0] = 0;
            pthread_exit(NULL);
        }
    }
    valid[0] = 1;
    pthread_exit(NULL);
}

void *check_cols(void *param)
{
    for (int i = 0; i < 9; i++)
    {
        int mask = 0;
        for (int j = 0; j < 9; j++)
            mask |= (1 << board[j][i]);
        if (mask != 1022)
        {
            valid[1] = 0;
            pthread_exit(NULL);
        }
    }
    valid[1] = 1;
    pthread_exit(NULL);
}

void *check_subgrid(void *param)
{
    parameters *p = (parameters *)param;
    int id = 2 + (p->row / 3 * 3) + (p->column / 3);
    int mask = 0;
    for (int i = p->row; i < p->row + 3; i++)
        for (int j = p->column; j < p->column + 3; j++)
            mask |= (1 << board[i][j]);

    valid[id] = (mask == 1022);
    free(p);
    pthread_exit(NULL);
}

int main()
{
    pthread_t threads[11];

    pthread_create(&threads[0], NULL, check_rows, NULL);
    pthread_create(&threads[1], NULL, check_cols, NULL);

    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            parameters *p = malloc(sizeof(parameters));
            p->row = i * 3;
            p->column = j * 3;
            pthread_create(&threads[2 + i * 3 + j], NULL, check_subgrid, p);
        }
    }

    for (int i = 0; i < 11; i++)
        pthread_join(threads[i], NULL);

    for (int i = 0; i < 11; i++)
    {
        if (!valid[i])
        {
            printf("Sudoku is INVALID!\n");
            return 0;
        }
    }
    printf("Sudoku is VALID!\n");
    return 0;
}