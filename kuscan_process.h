#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/wait.h>

typedef struct node{
	long long num;
	int cnt;
	struct node *prev;
	struct node *next;
}Node;

typedef struct _DLinkedList{
	Node *cur;
	Node *head;
	Node *tail;
	int total;
}List;

///////////////////리스트 관련 함수/////////////////
void initList(List* plist);
void Linsert(List *plist, long long get_num, int get_cnt);
void all_print(List* plist);
void PLinsert(List *plist, long long get_data, int num_count);
