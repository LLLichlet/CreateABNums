#include<stdio.h>
#include<string.h>
#include<stdint.h>

int add(int x, int y){return x+y;}
int subtr(int x, int y){return x-y;}
int mul(int x, int y){return x*y;}
int div(int x, int y){return !y || x%y ? -1: x/y;}
static int (*const operators[])(int, int) = {add, subtr, mul, div};
static const char oprname[] = "+-*/";

#define NNUMS 10
#define BEGIN 0x108
#define END 0x3ff
#define STAT 0x400
#define INFTY 0xfffffff

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
typedef struct stack{
    uint16_t status[STAT];
    int size;
} stack;
typedef struct hist{
    uint16_t parent;
    operate* opr;
} hist;

void bfs(argopr*, stack*, int*, hist*);
void nonrecur(operate**, char**, int, uint16_t*);

int main(){
    // warning: the size of opr[] must be estimated to avoid overflow
    operate opr[200];
    argopr arg[NNUMS][NNUMS]; 
   
    int nopr = 0;
    int tmp;

    // pre-processing of argoperators
    for(int i=0; i<NNUMS; ++i){
        for(int j=0; j<NNUMS; ++j){
            arg[i][j].size = 0;
            for(int k=0; k<4; ++k){
                tmp = operators[k](i, j);
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
    for(int i=0; i<NNUMS ;++i){
        printf("%d::  ",i);
        for(int j=0 ;j<NNUMS; ++j){
            printf("%d:%d, ",j, arg[i][j].size);
        }
        printf("\b\b  \n");
    }
    printf("\n\n");
#endif

    stack tmpstk;
    int cost[STAT][NNUMS];
    hist histr[STAT];

    for(int i = 0; i < STAT; ++i){
        for(int j = 0; j < NNUMS; ++j){
            cost[i][j] = INFTY;
        }
    }
    cost[BEGIN][3] = cost[BEGIN][8] = 1;
    histr[BEGIN] = (hist){0, NULL};
    tmpstk.status[0] = BEGIN;
    tmpstk.size = 1;

    bfs(arg[0], &tmpstk, cost[0], histr);

    int sum = 0;
    printf("cost of nums:\n");
    for(int i=0; i<10; ++i){
        printf("%d:%d, ", i, cost[END][i]);
        sum += cost[END][i];
    }
    
    printf("\b\b\nsum cost = %d\n\n", sum);
    operate* tmpopr[100];
    operate* numopr[NNUMS];
    memset(numopr, 0, sizeof(operate*)*NNUMS);
    int tmpsize = 0;
    uint16_t this = END;
    while((tmpopr[tmpsize++] = histr[this].opr)){
        numopr[histr[this].opr->res] = histr[this].opr;
        this = histr[this].parent;
    }
    for(int i = tmpsize - 2; i >= 0; i--){
        printf("operation #%d: %d %c %d = %d\n", tmpsize - i - 1, tmpopr[i]->x, oprname[tmpopr[i]->operator], tmpopr[i]->y, tmpopr[i]->res);
    }

    uint16_t stk[20];
    char str[100];
    char* ptr;
    printf("\nAtomic:\n");
    for(int i = 0; i < NNUMS; ++i){
        printf("%d = ", i);
        ptr = str;
        nonrecur(numopr, &ptr, i, stk);
        printf("%s\n", str);
    }
}

// bfs traversal through the given space(int* cost).
void bfs(argopr* args, stack* lastvisit, int* cost, hist* histr){
    int depth = 1;
    stack temp;
    stack* thisvisit = &temp;
    while(cost[END*NNUMS]==INFTY){
        thisvisit->size = 0;
        for(int i = 0; i < lastvisit->size; ++i){
            uint16_t status = lastvisit->status[i];
            for(int a = 0; a < NNUMS; ++a){
                if(!(status & (1<<a)))
                    continue;
                for(int b = 0; b < NNUMS; ++b){
                    if(!(status & (1<<b)))
                        continue;
                    argopr* this = args + a*NNUMS + b;
                    for(int k = 0; k < this->size; ++k){
                        operate* thisopr = this->oprs[k];
                        uint16_t newstat = status | (1 << thisopr->res);
                        int val = cost[status*NNUMS + thisopr->x] + cost[status*NNUMS + thisopr->y] + 1;
                        if(val >= cost[newstat*NNUMS + thisopr->res])
                            continue;
                        memcpy(cost + newstat*NNUMS, cost + status*NNUMS, sizeof(int)*NNUMS);
                        cost[newstat*NNUMS + thisopr->res] = val;
                        histr[newstat].opr = thisopr;
                        histr[newstat].parent = status; 
                        thisvisit->status[thisvisit->size++] = newstat;
                    }
                }   
            }
        }
        stack* ptr = thisvisit;
        thisvisit = lastvisit;
        lastvisit = ptr;
#ifndef NDEBUG
        printf("depth %d completed!\nvisited: ", depth++);
        for(int i = 0; i < thisvisit->size; ++i){
            printf("%x, ", thisvisit->status[i]);
        }
        printf("\n\n");
#endif
    }
}

// Non-recursive traversal with expression written to *str.
// Request pre-allocated array stk. Automatic null termination.
// Why not use the recursive version...???
void nonrecur(operate** numopr, char** str, int n, uint16_t* stk){
    int sstk = 0;
    stk[sstk++] = n;
    loop:
        if(numopr[stk[sstk-1] & 0x7fff]){
            stk[sstk] = numopr[stk[sstk-1] & 0x7fff]->x;
            sstk++;
            *(*str)++ = '(';
            goto loop;
        }
        *str += sprintf(*str, "%d", stk[--sstk] & 0x7fff);
        right:
        if(!sstk)
            goto end;
        if(stk[sstk-1] & 0x8000){
            *(*str)++ = ')';
            sstk--;
            goto right;
        }
        *str += sprintf(*str, " %c ", oprname[numopr[stk[sstk-1]]->operator]);
        stk[sstk] = numopr[stk[sstk-1]]->y;
        stk[sstk-1] |= 0x8000;
        sstk++;
        goto loop;
    end:
    *(*str)++ = '\0';
}
