#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>

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

///////////////////thread/////////////////////////

pthread_mutex_t mutex;//키는 1개여야됨
List list;//부모 리스트
int total=0;//총 줄갯수
int numOfthread;//쓰레드의 갯수
char* filename; // 파일이름
