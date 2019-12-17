#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define TLB_NUMBER (16)
#define PAGE_TABLE_NUM (1<<8)
//#define PAGE_TABLE_NUM (128)

#define MAX_LEN_65535 65535
static char bin_file_cxt[MAX_LEN_65535] = {0};

typedef struct{
	char bin_file[256];
	char addr_file[256];
	int algorithm; // 1:fifo; 2; lru
	int  frame;   //the max numbe of frame
}user_opt_t;

typedef struct{
	int page_number;
	int t;
	int tlru;
	int tfifo;
	int frame;
}page_t;

static page_t page_table_list[PAGE_TABLE_NUM] = {{0}};
static page_t tlb_table_list[TLB_NUMBER] = {{0}};
static int frame_max_num = 256;
int parser_user_opt(user_opt_t *user_opt, int argc, char* argv[])
{
	int opt;
	char algorithm[256] = {0};

	user_opt->algorithm = 1;
	user_opt->frame = 256;
	while ((opt = getopt(argc, argv, "b:f:p:n:")) != -1) {
		switch (opt) {
			case 'b':
				if(sscanf(optarg, "%s", user_opt->bin_file) != 1){
					printf("option -b set bin file is invalid %s,pls check\n", optarg);	
					exit(0);
				}
				break;
			case 'f':
				if(sscanf(optarg, "%s", user_opt->addr_file) != 1){
					printf("option -f set txt file is invalid %s,pls check\n", optarg);	
					exit(0);
				}
				break;	
			case 'p':
				if(sscanf(optarg, "%s", algorithm) != 1){
					printf("option -p set way of lru and fifo is invalid %s,pls check\n", optarg);	
					exit(0);
				}
				
				if (strcmp(algorithm,"FIFO") == 0){
					user_opt->algorithm = 1;                    	
				}else{
					user_opt->algorithm = 2;   
				}
				break;
			case 'n':
				if(sscanf(optarg, "%d", &user_opt->frame) != 1){
					printf("option -n set page number is invalid %s,pls check\n", optarg);	
					exit(0);
				}
				break;
			default:
				exit(0);
		}
	}

	return 0;
}

void load_bin_file(char* bin_file_name, char* buffer, int len)
{
	
	FILE* fp = fopen(bin_file_name,"rb");
	if(NULL == fp){
		printf("Err:failed to open file %s\n", bin_file_name);
		exit(0);
	}

	fread(bin_file_cxt, 1, sizeof(bin_file_cxt), fp);
	fclose(fp);	
}

int tlb_find(int line, int page, int* frame)
{
	int i;

	for(i = 0; i< TLB_NUMBER; i++){
		if (tlb_table_list[i].t == -1){
			break;
		}
		if (tlb_table_list[i].page_number == page){
			tlb_table_list[i].t = line;
			tlb_table_list[i].tlru = line;
			*frame = tlb_table_list[i].frame;
			return 0;
		}
	}
	return -1;
}

int page_table_find(int line, int page, int* frame, int* index)
{
	int i;

	for(i = 0; i < frame_max_num; i++){
		if (page_table_list[i].page_number == page){
			page_table_list[i].tlru = line;
			
			*index = i;
			*frame = page_table_list[i].frame;			
			return 0;
		}
	}
	return -1;
}

int update_tlb(int algorithm, int page_number, int index)
{
	int i;
	int	ififo = 0;
	int tfifo = -1;

	int ilru = 0;
	int tlru = -1;;
	
	for(i = 0; i < TLB_NUMBER; i++){
		//if tlb have empty slot, insert it
		if (tlb_table_list[i].t == -1){
			tlb_table_list[i].page_number= page_table_list[index].page_number;
			tlb_table_list[i].t = page_table_list[index].t;
			tlb_table_list[i].tlru = page_table_list[index].tlru;
			tlb_table_list[i].tfifo = page_table_list[index].tfifo;
			tlb_table_list[i].frame = page_table_list[index].frame;			
		}
		
		//if this page is in the tlb, do nothing and return
		if (tlb_table_list[i].page_number == page_number){
			//tlb_table_list[i].tlru = page_table_list[index].tlru;
			return;
		}

		//find the first page which enter the tlb
		if (tfifo >tlb_table_list[i].tfifo){
			ififo = i;
			tfifo  = tlb_table_list[i].tfifo;
		}
		//find the least use
		if (tlru > tlb_table_list[i].tlru){
			ilru = i;
			tlru  = tlb_table_list[i].tlru;
		}
	}
	
	if (algorithm == 1){
		tlb_table_list[ififo].page_number= page_table_list[index].page_number;
		tlb_table_list[ififo].t = page_table_list[index].t;
		tlb_table_list[ififo].tlru = page_table_list[index].tlru;
		tlb_table_list[ififo].tfifo = page_table_list[index].tfifo;
		tlb_table_list[ififo].frame = page_table_list[index].frame;
	}else{
		tlb_table_list[ilru].page_number= page_table_list[index].page_number;
		tlb_table_list[ilru].t = page_table_list[index].t;
		tlb_table_list[ilru].tlru = page_table_list[index].tlru;
		tlb_table_list[ilru].tfifo = page_table_list[index].tfifo;
		tlb_table_list[ilru].frame = page_table_list[index].frame;
	}
	return 0;	
}

int update_page_table(int algorithm, int line, int page_num, int *frame, int* index)
{
	int i;
	int pos = 0;
	int min = -1;


	//if have empty slot, insert this record and return
	for(i = 0; i < frame_max_num; i++){
		if (page_table_list[i].t == -1){
			page_table_list[i].t = line;
			page_table_list[i].tfifo= line;
			page_table_list[i].tlru= line;
			page_table_list[i].frame = i;	
			page_table_list[i].page_number = page_num;	
			*frame = i;
			*index = i;
			return 0;
		}
	}
	
	if (algorithm == 1){//fifo
		min = page_table_list[0].tfifo;
		for(i = 1; i < frame_max_num; i++){
			if (min > page_table_list[i].tfifo){
				min = page_table_list[i].tfifo;
				pos = i;
			}
		}
	}else{//lru
		min = page_table_list[0].tlru;
		for(i = 1; i < frame_max_num; i++){			
			if (min > page_table_list[i].tlru){
				min = page_table_list[i].tlru;
				pos = i;
			}
		}	
	}
	
	*index = pos;
	*frame = page_table_list[pos].frame;

	//clear the page from tlb table list
	for(i =0;i < TLB_NUMBER; i++){
		if (tlb_table_list[i].page_number == page_table_list[pos].page_number){
			tlb_table_list[i].t = -1;
			tlb_table_list[i].page_number = 0;
			tlb_table_list[i].tfifo = 0;
			tlb_table_list[i].tlru = 0;
			tlb_table_list[i].frame = 0;
		}
	}


	
	if (algorithm == 1){
		//printf("FIFO:%d %d %d\n", pos, page_table_list[pos].tfifo, page_table_list[pos].page_number);
		//printf("\treplace FIFO:%d %d %d\n", pos, line, page_num);
		page_table_list[pos].t = line;
		page_table_list[pos].tfifo = line;
		page_table_list[pos].page_number = page_num;
	}else{
		//printf("LRU:%d %d %d\n", pos, page_table_list[pos].tlru, page_table_list[pos].page_number);
		//printf("\treplace LRU:%d %d %d\n", pos, line, page_num);
		page_table_list[pos].t = line;
		page_table_list[pos].tlru= line;
		page_table_list[pos].page_number = page_num;
	}
	return 0;
}
void init_table_list()
{
	int i;

	memset(tlb_table_list, 0, sizeof(tlb_table_list));
	memset(page_table_list, 0, sizeof(page_table_list));

	for(i = 0; i<PAGE_TABLE_NUM;i++){
		page_table_list[i].t = -1;
	}
	
	for(i = 0; i<TLB_NUMBER;i++){
		tlb_table_list[i].t = -1;
	}
}
int main(int argc, char* argv[])
{
	int va;
	int res;
	int line = 0;
	FILE* fp = 0;
	int tlb_hit = 0;
	int page_fault = 0;
	char buffer[MAX_LEN_65535] = {0};
	user_opt_t user_opt;
	memset(&user_opt, 0, sizeof(user_opt_t));

	init_table_list();
	parser_user_opt(&user_opt, argc, argv);
	load_bin_file(user_opt.bin_file, buffer, sizeof(buffer));

	//printf("usr opt:%s %s %d %d\n",user_opt.bin_file, user_opt.addr_file, user_opt.algorithm, user_opt.frame);
	fp = fopen(user_opt.addr_file, "r");
	if (fp == NULL){
		printf("Err:failed to open address file,pls check\n");
		return 0;
	}		
	if (user_opt.frame == 128){
		frame_max_num = 128;
	}
	//printf("frame_max_num:%d\n", frame_max_num);
	while(!feof(fp)){
		char tmp[128] = { 0 };
		char* result = 0;

		result =fgets(tmp, sizeof(tmp), fp);
		if (NULL == result){
			break;
		}
		line ++;

		//delete 0x0a 0x0d
		if (tmp[strlen(tmp) - 1] == 0x0a){
			tmp[strlen(tmp) - 1] =0;
		}
		if (tmp[strlen(tmp) - 1] == 0x0d){
			tmp[strlen(tmp) - 1] =0;
		} 
		
		va = atoi(tmp);	

		//start to calc 
		int page_num = (va >> 8)&0xff;
		int offset = va&0xff;
		int frame = 0;
		int index = 0;
		int i = 0;
		
		//printf("vm:%x;page_num:%x offset:%x\n", va, page_num, offset);
		//find from tlb
		res = tlb_find(line, page_num, &frame);
		if (res == 0){
			tlb_hit++;	

			//if lru,update page table
			for(i = 0; i < frame_max_num; i++){
				if(page_table_list[i].page_number == page_num){
					page_table_list[i].t = line;
					page_table_list[i].tlru = line;
				}
			}
		}else{
			res = page_table_find(line, page_num, &frame, &index);
			if (res == 0){
				;//hit
			}else{
				page_fault++;
				update_page_table(user_opt.algorithm, line, page_num, &frame, &index);			
				update_tlb(user_opt.algorithm, page_num, index);					
			}	
						
		}
		printf("Virtual address: %d Physical address: %d Value: %d\n", va, (frame << 8)| offset, bin_file_cxt[(page_num<<8)|offset]);		
	}
	
	if(NULL == fp){
		fclose(fp);
	}
	printf("line: %d\n", line);
	printf("Page Faults = %u\n", page_fault);
	printf("Page Fault Rate = %g\n", (double)page_fault/line);
	printf("TLB Hits = %u\n", tlb_hit);
	printf("TLB Hits = %g\n",(double)tlb_hit/line);
	return 0;
}
