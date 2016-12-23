#include "kuscan_thread.h"

//쓰레드들의 배열을 동적할당해서 만들고, 그다음에 for 문 돌리면서 각각을 실행시킨다
void* sorting_by_thread(void *child_num){
		
	int size = 10*(total/numOfthread);
	int block = size*11;	
	long long get_num[10];
	int number = *(int*)child_num;
	int i;
	List clist;	

	FILE *fp;	
	fp = fopen(filename,"r");
	
	if(fp == NULL){
		printf("파일을 열지 못하였습니다\n");
		return(void*)"fail" ;
	}
	
	initList(&clist);

	fseek(fp,number*block,SEEK_SET);

	while(size != 0 && fscanf(fp,"%lld %lld %lld %lld %lld %lld %lld %lld %lld %lld",&get_num[0],&get_num[1],&get_num[2],&get_num[3],&get_num[4],&get_num[5],&get_num[6],&get_num[7],&get_num[8],&get_num[9]) != EOF){
		
		for(i=0; i<10; i++){		
			Linsert(&clist,get_num[i],1);
		}
		size= size-10;
	}
	if(total%numOfthread != 0 && number == numOfthread-1){
		fseek(fp,(number*block)+block,SEEK_SET);// 마지막 자식이 나머지 줄들을 처리한다
		while(fscanf(fp,"%lld %lld %lld %lld %lld %lld %lld %lld %lld %lld",&get_num[0],&get_num[1],&get_num[2],&get_num[3],&get_num[4],&get_num[5],&get_num[6],&get_num[7],&get_num[8],&get_num[9])!=EOF){
			for(i=0; i<10; i++){		
				Linsert(&clist,get_num[i],1);
			}
		}
	}
	// 리스트에 넣을때만 락걸었다가 풀어주면서 진행해야됨
	clist.cur = clist.head->next;
	while(clist.cur!=clist.tail){
		pthread_mutex_lock(&mutex);
		Linsert(&list,clist.cur->num,clist.cur->cnt);		
		clist.cur=clist.cur->next;
		pthread_mutex_unlock(&mutex);
	}

	return (void*)"success";
}


int main(int argc, char *argv[]){

	FILE *fp;
	char buf[256];

	int i,j;
	int *numbering;
	void* t_return = NULL;
	pthread_t* p_thread;

	numOfthread = atoi(argv[1]); // 쓰레드의 갯수 받아오기
	p_thread = (pthread_t*)malloc(sizeof(pthread_t)*numOfthread);	
	//리스트 생성
	initList(&list);

	//파일열기
	filename = argv[2];
	fp = fopen(filename,"r");
	if(fp == NULL){
		printf("파일을 열지 못하였습니다\n");
		return 1;
	}

	//총 줄수를 계산한다
	while(fgets(buf,256,fp)){
		total++;
	}
	fclose(fp);

	if(total < numOfthread){
		numOfthread=total;
	}

	//쓰레드를 생성한다
	pthread_mutex_init(&mutex,NULL);

	for(i=0; i<numOfthread; i++){
		numbering = (int*)malloc(sizeof(int));		
		*numbering=i;
		pthread_create(&p_thread[i], NULL, sorting_by_thread, (void*)numbering);
	}

	
	for(i=0; i<numOfthread; i++){
		pthread_join(p_thread[i], &t_return);
	}
	
	pthread_mutex_destroy(&mutex);
	all_print(&list);
	return 0;
}



/////////////////////리스트/////////////////////
void all_print(List* plist){
	plist->cur = plist->head->next;
	while(plist->cur != plist->tail){
		printf("%10lld :\t %10d\n",plist->cur->num,plist->cur->cnt);
		plist->cur=plist->cur->next;
	}
}

void initList(List *plist){

	plist->head  = (Node*)malloc(sizeof(Node));
	plist->tail  = (Node*)malloc(sizeof(Node));

	plist->head->prev = NULL;
	plist->head->next = plist->tail;

	plist->tail->prev = plist->head;
	plist->tail->next = NULL;

	plist->cur = plist->head;
}

void Linsert(List *plist, long long get_num, int get_cnt){

	plist->cur = plist->head->next;//처음 노드를 가리키게 한다

	while(plist->cur != plist->tail){

		if(plist->cur->num < get_num){
			plist->cur=plist->cur->next;
		}
		else if(plist->cur->num == get_num){
			plist->cur->cnt += get_cnt;
			return;
		}else if(plist->cur->num > get_num){
			Node *newNode = (Node*)malloc(sizeof(Node));
			newNode->num = get_num;
			newNode->cnt = get_cnt;

			newNode->prev = plist->cur->prev;
			plist->cur->prev->next=newNode;
			newNode->next=plist->cur;
			plist->cur->prev=newNode;
			return;
		}
	}
	Node *newNode = (Node*)malloc(sizeof(Node));
	newNode->num = get_num;
	newNode->cnt = get_cnt;

	newNode->prev = plist->cur->prev;
	plist->cur->prev->next=newNode;
	newNode->next=plist->cur;
	plist->cur->prev=newNode;
}
