#include<stdio.h>

int add(int x, int y){return x+y;}
int subtr(int x, int y){return x-y;}
int mul(int x, int y){return x*y;}
int divs(int x, int y){return !y || x%y ? -1: x/y;}
static int (*const oprfunc[])(int, int) = {add, subtr, mul, divs};
static const char oprname[] = "+-*/";

#define NNUMS 10
#define INFTY 0xffffffu

typedef struct operate{
    unsigned char x;
    unsigned char y;
    unsigned char res;
    unsigned char operator;
} operate;
typedef struct argopr{
    operate* oprs[NNUMS];
    int size;
} argopr;
typedef struct status{
    unsigned cost;
    operate* parent;
} status;

void recur(status*, char**, int);
void nonrecur(status*, char**, int, unsigned*);

int main(){
    
    static operate opr[NNUMS*NNUMS*2];
    static argopr arg[NNUMS][NNUMS];

    int nopr = 0;
    int tmp;

    // pre-processor of operators
    for(int i=0; i<NNUMS; ++i){
        for(int j=0; j<NNUMS; ++j){
            arg[i][j].size = 0;
            for(int k=0; k<4; ++k){
                tmp = oprfunc[k](i, j);
                // out of range
                if(tmp < 0 || tmp >= NNUMS)
                    continue;
                // excessive identity operation
                if(tmp == i || tmp == j)
                    continue;
                // commutative property of add&mul
                if(!(k%2) && i>j)
                    continue;
                opr[nopr] = (operate){i, j, tmp, k};
                arg[i][j].oprs[arg[i][j].size++] = &opr[nopr++];
            }
        }
    }

#ifndef NDEBUG
    printf("full operation table:\n");
    for(int i=0; i<nopr; ++i){
        printf("%u%c%u=%u, ", opr[i].x, oprname[opr[i].operator], opr[i].y, opr[i].res);
        if(i % 10 == 9)
            printf("\n");
    }
    printf("\nsum operations = %d\n\n", nopr);
    printf("operations sorted by arguments:\n");
    for(int i=0; i<NNUMS ; ++i){
        printf("%d::  ",i);
        for(int j=0 ; j<NNUMS; ++j){
            printf("%d:%d, ",j, arg[i][j].size);
        }
        printf("\n");
    }
    printf("\n\n");
#endif

    status stat[NNUMS];
    for(int i = 0; i < NNUMS; ++i){
        stat[i].cost = INFTY;
    }
    // default initialization
    stat[3].cost = stat[8].cost = 1;
    stat[3].parent = stat[8].parent = NULL;
    int istransf = 1;
    
    while(istransf--){
        for(int i = 0; i < NNUMS; ++i){
            for(int j = 0; j < NNUMS; ++j){
                for(int k = 0; k < arg[i][j].size; ++k){
                    tmp = arg[i][j].oprs[k]->res;
                    if(stat[i].cost + stat[j].cost + 1 >= stat[tmp].cost)
                        continue;
                    stat[tmp].cost = stat[i].cost + stat[j].cost + 1;
                    stat[tmp].parent = arg[i][j].oprs[k];
                    istransf = 1;
                }
            }
        }
    }

    int sum = 0;
    printf("cost of nums:\n");
    for(int i = 0; i < NNUMS; ++i){
        sum += stat[i].cost;
        printf("%d:%d, ", i, stat[i].cost);
    }
    printf("\nsum cost = %d\n", sum);
    operate* thisopr;
    printf("\noperations:\n");
    for(int i = 0; i < NNUMS; ++i){
        if((thisopr = stat[i].parent))
            printf("%d = %d %c %d\n", i, thisopr->x, oprname[thisopr->operator], thisopr->y);
        else
            printf("%d = %d\n", i, i);
    }
    printf("\n");

    unsigned stk[100];
    char str[100];
    char* ptr;
    printf("atomic:\n");
    for(int i = 0; i < NNUMS; ++i){
        ptr = str;
        ptr += sprintf(ptr, "%d = ", i);
        //recur(stat, &ptr, i);
        nonrecur(stat, &ptr, i, stk);
        //*ptr++ = '\0';
        printf("%s\n", str);
    }
}


// Atomic oprs with expr written to *res
// Manual null termination is required.
void recur(status* stat, char** res, int n){
    operate* thisopr;
    if(!(thisopr = stat[n].parent)){
        *res += sprintf(*res, "%d", n);
        return;
    }
    *(*res)++ = '(';
    recur(stat, res, thisopr->x);
    *res += sprintf(*res, " %c ", oprname[thisopr->operator]);
    recur(stat, res, thisopr->y);
    *(*res)++ = ')';
}

// Deprecated non-recursive visit with expr written to *res.
// Request pre-allocated array stk. Automatic null termination.
// Why not use the recursive version...???
void nonrecur(status* stat, char** res, int n, unsigned* stk){
    int sstk = 0;
    stk[sstk++] = n;
    while(1){
        if(stat[stk[sstk-1] & 0x7fffffff].parent){
            stk[sstk] = stat[stk[sstk-1] & 0x7fffffff].parent->x;
            sstk++;
            *(*res)++ = '(';
            continue;
        }
        *res += sprintf(*res, "%d", stk[--sstk] & 0x7fffffff);
        right:
        if(!sstk)
            break;
        if(stk[sstk-1] & 0x80000000){
            *(*res)++ = ')';
            sstk--;
            goto right;
        }
        *res += sprintf(*res, " %c ", oprname[stat[stk[sstk-1]].parent->operator]);
        stk[sstk] = stat[stk[sstk-1]].parent->y;
        stk[sstk-1] |= 0x80000000;
        sstk++;
    }
    *(*res)++ = '\0';
}

