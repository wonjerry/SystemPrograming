#include "kuscan_process.h"

int main(int argc, char *argv[]){

	FILE *fp;
	FILE *fp2;
	char buf[256];

	int i,j;
	int flag;
	int size;
	int total=0;
	long long get_num[10];
	int proc_num;
	int circle=0;
	int stock;
	int block;

	key_t ipckey;

	int mq_id;
	struct{
		long type;
		long long num;
		int cnt;
	}mymsg;

	proc_num = atoi(argv[1]); // 프로세스의 갯수 받아오기

	//키생성
	if((ipckey = ftok("temp2",43))==-1){
		fp = fopen("temp2","w");
		fclose(fp);
		ipckey = ftok("temp2",43);
	}
	mq_id = msgget(ipckey,IPC_CREAT|0666);//큐생성

	//리스트 생성
	List list;
	initList(&list);

	//파일열기
	fp = fopen(argv[2],"r");
	if(fp == NULL){
		printf("파일을 열지 못하였습니다\n");
		return 1;
	}

	//총 줄수를 계산한다
	while(fgets(buf,256,fp)){
		total++;
	}
	fclose(fp);

	if(total < proc_num){
		proc_num=total;
	}
	size = 10*(total/proc_num);//블록 당 단어의 갯수를 나타낸다
	block = size*11;//한 블록의 바이트수를 나타낸다

	//각 자식마다 한줄씩 읽어서 링크드리스트 처리하고 메세지큐에 넘기기
	for(i=0; i<proc_num; i++){
		if(fork()==0){
			fp2 = fopen(argv[2],"r");
			if(fp2 == NULL){
				printf("파일을 열지 못하였습니다\n");
				return 1;
			}	
			fseek(fp2,i*block,SEEK_SET);//각 자식이 가야할곳을 찍어준다
			while(size != 0 && fscanf(fp2,"%lld %lld %lld %lld %lld %lld %lld %lld %lld %lld",&get_num[0],&get_num[1],&get_num[2],&get_num[3],&get_num[4],&get_num[5],&get_num[6],&get_num[7],&get_num[8],&get_num[9]) != EOF){
				for(j=0; j<10; j++){		
					Linsert(&list,get_num[j],1);
				}
				size= size-10;
			}

			if(total%proc_num != 0 && i == proc_num-1){
				fseek(fp2,(i*block)+block,SEEK_SET);// 마지막 자식이 나머지 줄들을 처리한다
				while(fscanf(fp2,"%lld %lld %lld %lld %lld %lld %lld %lld %lld %lld",&get_num[0],&get_num[1],&get_num[2],&get_num[3],&get_num[4],&get_num[5],&get_num[6],&get_num[7],&get_num[8],&get_num[9])!=EOF){
					for(j=0; j<10; j++){		
						Linsert(&list,get_num[j],1);
					}
				}
			}

			list.cur = list.head->next;
			while(list.cur!=list.tail){
				mymsg.num = list.cur->num;
				mymsg.cnt = list.cur->cnt;
				mymsg.type=1;
				flag = msgsnd(mq_id,(void*)&mymsg,sizeof(mymsg)-sizeof(long),0);
				list.cur = list.cur->next;
			}
			fclose(fp2);
			exit(1);
		}
	}

	//부모는 rcv하면서 링크드 리스트에 넣어준다
	circle = 10*total;
	stock = 0;
	//각 프로세스별 블록 갯수가 있을 것이고 거기에 프로세스의 수를 곱한다
	while(circle>stock){
		if(msgrcv(mq_id,&mymsg,sizeof(mymsg)-sizeof(long),1,0) == -1){
			printf("fail\n");
			break;
		}
		stock += mymsg.cnt;
		Linsert(&list,mymsg.num,mymsg.cnt);
	}

	all_print(&list);

	msgctl(mq_id,IPC_RMID,NULL);

	return 0;
}
////////////////////IPC////////////////////////



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

