#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define single(I)       (0 == (I & (I-1)))
#define likely(x)       __builtin_expect((x),1)
#define unlikely(x)     __builtin_expect((x),0)
#define population      __builtin_popcount

/*
 * Sudoku Hammer
 *
 * Apr 2012
 * ssadler@mashi.org
 *
 * Implementation of Peter Norvig's Sudoku algorithm:
 * http://norvig.com/sudoku.html
 */


char example_names[][10] = {"grid1", "grid2", "norvig1", "norvig2"};
char example_grids[][81] = {
    "  93  2   2  8  4 4    6  3  7 9   4 3 2 1 8 1   3 7  9  7    5 8  1  3   2  39  ",
    "    19 4   48  6  75      2 9 1 2  4     3   5  4 6 3 8      73  6  84   1 29    ",
    "4     8 5 3          7      2     6     8 4      1       6 3 7 5  2     1 4      ",
    "     6    59     82    8    45        3        6  3 54   325  6                  ",
};

short units_x[][9] = {{0,  1,  2,  3,  4,  5,  6,  7,  8},
                      {9,  10, 11, 12, 13, 14, 15, 16, 17},
                      {18, 19, 20, 21, 22, 23, 24, 25, 26},
                      {27, 28, 29, 30, 31, 32, 33, 34, 35},
                      {36, 37, 38, 39, 40, 41, 42, 43, 44},
                      {45, 46, 47, 48, 49, 50, 51, 52, 53},
                      {54, 55, 56, 57, 58, 59, 60, 61, 62},
                      {63, 64, 65, 66, 67, 68, 69, 70, 71},
                      {72, 73, 74, 75, 76, 77, 78, 79, 80}};

short units_y[][9] = {{0, 9,  18, 27, 36, 45, 54, 63, 72},
                      {1, 10, 19, 28, 37, 46, 55, 64, 73},
                      {2, 11, 20, 29, 38, 47, 56, 65, 74},
                      {3, 12, 21, 30, 39, 48, 57, 66, 75},
                      {4, 13, 22, 31, 40, 49, 58, 67, 76},
                      {5, 14, 23, 32, 41, 50, 59, 68, 77},
                      {6, 15, 24, 33, 42, 51, 60, 69, 78},
                      {7, 16, 25, 34, 43, 52, 61, 70, 79},
                      {8, 17, 26, 35, 44, 53, 62, 71, 80}};

short units_z[][9] = {{0,  1,  2,  9,  10, 11, 18, 19, 20},
                      {3,  4,  5,  12, 13, 14, 21, 22, 23},
                      {6,  7,  8,  15, 16, 17, 24, 25, 26},
                      {27, 28, 29, 36, 37, 38, 45, 46, 47},
                      {30, 31, 32, 39, 40, 41, 48, 49, 50},
                      {33, 34, 35, 42, 43, 44, 51, 52, 53},
                      {54, 55, 56, 63, 64, 65, 72, 73, 74},
                      {57, 58, 59, 66, 67, 68, 75, 76, 77},
                      {60, 61, 62, 69, 70, 71, 78, 79, 80}};

short peers[1620];

short toNum(short c) {
    short ii = 1;
    while (c > 1) {
        c >>= 1;
        ii++;
    }
    return ii;
}

init_peers() {
    short i, ii, offset = 0;
    short *x, *y, *z;
    for (i=0; i<81; i++) {
        x = units_x[i / 9];
        y = units_y[i % 9];
        z = units_z[3 * (i/27) + (i/3) % 3];

        for (ii=0; ii<9; ii++) {
            if (i != x[ii])
                peers[offset++] = x[ii];

            if (i != y[ii])
                peers[offset++] = y[ii];

            if (z[ii]/9 != i/9 && z[ii]%9 != i%9)
                peers[offset++] = z[ii];
        }
    }
}

short assign(short* g, short k, short c);
short eliminate(short* g, short k, short c);
short propagate(short* g, short* keys, short c);


short assign(short* g, short k, short c) {
    short i;
    for (i=1; i<512; i<<=1) {
        if (0 == (i & c)) {
            if (0 == eliminate(g, k, i)) {
                return 0;
            }
        }
    }
    return 1;
}


short eliminate(short* g, short k, short c) {
    short i;
    if (0 == (g[k] & c)) return 1; // already done
    g[k] &= ~c;
    if (0 == g[k]) return 0; // fuck
    
    if (unlikely(single(g[k]))) {
        for (i=(k*20); i<(k*20)+20; i++) {
            if (0 == eliminate(g, peers[i], g[k])) {
                return 0;
            }
        }
    }
    
    return 1 == propagate(g, units_x[k/9], c) &&
           1 == propagate(g, units_y[k%9], c) &&
           1 == propagate(g, units_z[3 * (k/27) + (k/3) % 3], c);
}


short propagate(short* g, short* keys, short c) {
    short dk, i, ii = 0;
    
    for (i=0; i<9; i++) {
        if (0 < (c & g[keys[i]])) {
            ii++;
            dk = keys[i];
        }
    }
    if (ii == 0) {
        return 0;
    } else if (ii == 1) {
        return assign(g, dk, c);
    } else return 1;
}

short search(short* g) {
    short i, k, c;
    
    for (i=0; i<81; i++) {
        if (!single(g[i])) break;
    }
    if (81 == i) return 1;
    
    short* g2 = malloc(81 * sizeof(short));
    memcpy(g2, g, 81*sizeof(short));
    
    for (i=2; i<10; i++)
        for (k=0; k<81; k++)
            if (population(g[k]) == i)
                goto found;
    
    found:
    for (c=1; c<512; c<<=1) {
        if (g2[k] & c) {
            if (1 == assign(g, k, c) && 1 == search(g)) {
                break;
            }
            memcpy(g, g2, 81*sizeof(short));
        }
    }
    
    free(g2);
    return c < 512 ? 1 : 0;
}

short check_grid(short* grid) {
    short i, k, c, s;
    short* unit;
    for (i=0; i<27; i++) {
        unit = i/9==0 ? units_x[i] : ( i/9==1 ? units_y[i%9] : units_z[i%9] );
        c = 0; s = 0;
        for (k=0; k<9; k++) {
            s = grid[unit[k]];
            if (single(s)) {
                if (0 != (c & s)) {
                    return 0;
                }
                c |= s;
            }
        }
    }
    return 1;
}

print_grid(short* g) {
    short i;
    for (i=0; i<81;) {
        if (single(g[i])) {
            printf("%d ", toNum(g[i]));
        } else {
            printf("  ");
        }
        
        ++i;
        
        if (i == 27 || i == 54) printf("\n------+-------+------");
        if (i % 9 == 0) printf("\n");
        else if (i % 3 == 0) printf("| ");
    }
}

run_grid(char* data) {
    short i, c, out;
    
    short *grid = malloc(81 * sizeof(short));
    for (i=0; i<81; i++) grid[i] = 511;

    for (i=0; i<81; i++) {
        if (data[i] > '0' && data[i] <= '9') {
            out = assign(grid, i, 1 << (data[i] - '1'));
            if (out == 0) {
                printf("failed\n");
                break;
            }
        }
    }
    
    print_grid(grid);
    
    if (1 == out && 1 == search(grid)) {
        print_grid(grid);
        printf(check_grid(grid) == 1 ? "OK\n" : "FAIL\n");
    } else {
        printf("no solution\n");
    }
    free(grid);
}

read_stdin_grid(char *data) {
    int i; char c;
    for (i=0; i<81; i++) {
        c = getc(stdin);
        if (c == '\n') {
            printf("Invalid line encountered\n");
            return 0;
        } else if (c == EOF) {
            if (i > 0) printf("EOF reached\n");
            return 0;
        }
        data[i] = c;
    }
    while ((c = getc(stdin)) != '\n' && c != EOF);
    return 1;
}

int main(int argc, char *argv[]) {
    init_peers();
    char input[81];
    short j, i;
    
    if (argc > 1) {
        for (j=1; j<argc; j++) {
            if (strlen(argv[j]) < 81) {
                for (i=0; i<sizeof(example_names)/10; i++) {
                    if (0 == strcmp(argv[j], example_names[i])) {
                        run_grid(example_grids[i]);
                        break;
                    }
                }
            } else {
                run_grid(argv[i]);
            }
        }
    } else {
        while (0 != read_stdin_grid(input)) {
            run_grid(input);
        }
    }
    
    return 0;
}
