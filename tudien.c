#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "btree.h"
#define N 1000000 //so luong tu toi da trong tu dien
#define MAX_LIST 40000 
#define SIZE_MEAN 30
#define SIZE_WORD 30

typedef struct{
  char *s;
}String;

void strlwr(char *str);
void readfile(BTA *r,BTA *sr,FILE *f,char *fname);
void writefile(BTA *r,FILE *f,char *fname);
void RandomString(char* str, int strleng);
void CrtRandDict(FILE *f,char *fname);

//ues for soundex
char parse(char c);
void lose_repeated(char *word, char *res);
void lose_char_except_first(char *word, char *res, char c);
int Sndx(char *word);

int search_tab(BTA *root,char *word,String *output);
int soundex_search(BTA *sr,char *fn,String *output);
void search_display(BTA *r,String *output,int maxl);

//use for main
void insert(BTA *r,BTA *sr,char *word,char *mean);
int search(BTA *r,char *word);
void delete(BTA *r,BTA *sr,char *fn);
void auto_search_main(BTA *r,char *fn,String *output);
void soundex_search_main(BTA *r,BTA *sr,char *fn,String *output);

int main(){
  int i=0,k,n;
  int ch,val;
    char me[SIZE_MEAN];//mean
    char wo[SIZE_WORD];//word
    char fn[SIZE_WORD];//find
    char filename[30];
    String list[MAX_LIST],list_s[MAX_LIST];
    BTA *dic,*dics;
    FILE *f1;
    printf("CHUONG TRINH QUAN LI TU DIEN\n");
    printf("Nhap ten file tu dien:");gets(filename);
  /*CrtRandDict(f1,filename);
    printf("Tao file thanh cong!\n");
    return 0;
*/    
    btinit();
       dic=btopn("dictionary",0,0);
       dics=btopn("dictionary_s",0,0);
      if (dic==NULL || dics==NULL) {
        printf("Loading...\n");
	dic = btcrt("dictionary", 0, 0);
	dics = btcrt("dictionary_s", 0, 0); 
	btdups(dics,1);
        readfile(dic,dics,f1,filename);	
      }	
        
    do {
        printf("\tMENU\n1.Them tu\n2.Tim kiem thong thuong\n3.Tim kiem AutoComplete\n4.Tim kiem Suggestion\n5.Xoa tu\n6.Thoat chuong trinh\n");
	do{
	  printf("Ban chon:");scanf("%d",&n);
	  if (n<1 || n>6) printf("->Khong co lua chon nay!\n");
	}while(n<1|| n>6);
        while (getchar()!='\n');
        switch(n){
	case 1:
	  insert(dic,dics,wo,me);
	  break;
	case 2:
	  search(dic,wo);
	  break;
	case 3:
	  auto_search_main(dic,fn,list);
	  break;
	case 4:
	  soundex_search_main(dic,dics,fn,list_s);
	  break;
	case 5:
	  delete(dic,dics,fn);
	  break;
	case 6:
	  printf("Ban co muon luu su thay doi vao file '%s'?(y or n):",filename);
	  if (getchar()=='y') writefile(dic,f1,filename); 
	  btcls(dic);
	  btcls(dics);
	  break;
	}
     }while(n!=6);
     return 0;
}

void strlwr(char *str){ //Doi cac ky tu trong chuoi thanh chu thuong 
  int i;
  for (i=0;str[i]!='\0';i++)
    if (str[i]>='A' && str[i]<='Z')
      str[i]=str[i]+32;
}

void readfile(BTA *r,BTA *sr,FILE *f,char *fname){//sr:soundex root
  char word[SIZE_WORD],mean[SIZE_MEAN];
  char tmp[SIZE_WORD];
    if((f=fopen(fname,"r"))==NULL){
      printf("->Khong tim thay file\n");
      exit(1);
    }
      while(fscanf(f,"%s : %s",word,mean)!=EOF){ 
	strlwr(word);
	strlwr(mean);
	strcpy(tmp,word);
	btins(r,word,mean,SIZE_MEAN);
	Sndx(word);
	btins(sr,word,tmp,SIZE_MEAN);
  }
fclose(f);
	  }

void writefile(BTA *r,FILE *f,char *fname){
  int size;
  char word[SIZE_WORD],mean[SIZE_MEAN];
    if((f=fopen(fname,"w"))==NULL){
      printf("->Khong tim thay file\n");
      exit(1);
    }
    btsel(r,"",mean,SIZE_MEAN,&size);
    while ( btseln(r,word,mean,SIZE_MEAN, &size)==0 ){
	fprintf(f,"%s : %s\n",word,mean);
      }
}

void RandomString(char* str, int strleng){ //sinh chuoi ngau nhien
	char* box = "abcdefjhijklmnopqrstuvwxyz";
	int boxLength = strlen(box);
	int i;
	for (i = 0; i < strleng; i++)
	{
		str[i] = box[rand() % boxLength];
	}
	str[strleng] = '\0';
}

void CrtRandDict(FILE *f,char *fname){ //Tao ngau nhien tu dien
  int i;
  char word[30],mean[30];
if((f=fopen(fname,"w"))==NULL){
      printf("->Khong tim thay file\n");
      exit(1);
    }
  srand(time(NULL));
  for(i=0;i<N;i++){
    RandomString(word,6);
    fprintf(f,"%s : ",word);
    RandomString(mean,8);
    fprintf(f,"%s\n",mean);
  }
  fclose(f);
}

// Tim kiem AutoComplete
int search_tab(BTA *root,char *word,String *output){
  //output luu danh sach cac tu goi y
  int i,total=0;
  BTint value;
  char key[50];
  i=bfndky(root,word,&value);//dinh vi khoa dau tien
  if(i==0){                
    output[total++].s=strdup(word); 
  }
    while(1){
      i=bnxtky(root,key,&value);  //tim kiem khoa tiep theo
      if(i!=0||strncmp(key,word,strlen(word))!=0)
	     break;
      output[total++].s=strdup(key);  //luu khoa tim thay vao output
      if(total==MAX_LIST) break;
    }    
  return total;
}

//Hien thi danh sach cac tu goi y
void search_display(BTA *r,String *output,int maxl){
  char mean[SIZE_MEAN];
  int i,ch,size;
	  if(maxl>0){
	  printf("Tu can tim co the la:\n");
		for(i=0;i<maxl;i++){
		  printf("%3d-%-20s",i+1,output[i].s);
		  if ((i+1)%4==0) printf("\n");
		  if((i+1)%40==0 || i==(maxl-1)){
		    if (i==(maxl-1)) 
		      printf("\nChon STT cua tu ban muon tim hoac nhap '-1' hoac '0' de thoat\n");
		    else
		      printf("Chon STT cua tu ban muon tim hoac nhap '0' de qua danh sach ke tiep, nhap '-1' de thoat\n");
		    do{
		      printf("Moi chon:");
		    scanf("%d",&ch);
		    if (ch<-1 || ch>maxl) printf("->Khong co lua chon nay!\n");
		    }while(ch<-1 || ch>maxl);
		    if (ch==-1) break;
		    if (ch!=0) {		  
		      btsel(r,output[ch-1].s,mean,SIZE_MEAN,&size);
		      printf("-> %s : %s\n", output[ch-1].s,mean);
		      break;
		    }
		  }
		  }
	  }	  
}

char parse(char c) {//use for soundex
	switch(c) {
		case 'a':
		case 'A':
		case 'e':
		case 'E':
		case 'i':
	        case 'I':
		case 'o':
	        case 'O':
		case 'u':
		case 'U':
		case 'y':
		case 'Y':
			return 'v';
		case 'h':
		case 'H':
		case 'w':
		case 'W':
			return 'c';
		case 'b':
		case 'B':
		case 'f':
		case 'F':
		case 'p':
		case 'P':
		case 'v':
		case 'V':
			return '1';
		case 'c':
		case 'C':
		case 'g':
		case 'G':
		case 'j':
		case 'J':
		case 'k':
		case 'K':
		case 'q':
		case 'Q':
		case 's':
		case 'S':
		case 'x':
		case 'X':
		case 'z':
		case 'Z':
			return '2';
		case 'd':
		case 'D':
		case 't':
		case 'T':
			return '3';
		case 'l':
		case 'L':
			return '4';
		case 'm':
		case 'M':
		case 'n':
		case 'N':
			return '5';
		case 'r':
		case 'R':
			return '6';
		default :
			return 'X';
	}
}

void lose_repeated(char *word, char *res) {//use for soundex
	char curr = word[0];
	char next = word[0];
	int c_ind = 0;
	int n_ind = 0;
	while(next != '\0') {
		res[c_ind] = curr;
		c_ind++;
		while(next == curr) {
			n_ind++;
			next = word[n_ind];
		}
		curr = next;
	}
	res[c_ind] = '\0';
}

void lose_char_except_first(char *word, char *res, char c) {//use for soundex
	int ind = 1;
	int i = 1;
	res[0] = word[0];
	while(word[ind] != '\0') {
		if(word[ind] == c) {
			ind++;
		} else {
			res[i] = word[ind];
			i++;
			ind++;
		}
	}
	res[i] = '\0';
}

int Sndx(char *word) {//use for soundex
  int len=strlen(word);
  char first;
  char res[256];
  if(len < 1) {
    res[0] = '0';
    res[1] = '0';
    res[2] = '0';
    res[3] = '\0';
    return 0;
  } else {
    first = word[0];
  }
  int ind = 0;
  for(; ind < len; ind++){
    res[ind] = parse(word[ind]);
  }
  res[ind] = '\0';
  lose_char_except_first(res, word, 'c');
  lose_repeated(word, res);
  lose_char_except_first(res, word, 'v');
  word[0] = first;
  int k = 0;
  while((k = strlen(word)) < 4) {
    word[k] = '0';
    word[k+1] = '\0';
  }
  word[4] = '\0';
  return 0;
}

void insert(BTA *r,BTA *sr,char *word,char *mean){
  char *tmp;
  int val;
  printf("Nhap tu:");
  gets(word);
  strlwr(word);
  if (bfndky(r,word,&val)==0){
    printf("Tu nay da co trong tu dien.\nBan co muon cap nhat du lieu?(y or n):");
    if (getchar()=='y'){
      while (getchar()!='\n');
      printf("Nhap nghia cua tu:");
      gets(mean);
      btupd(r,word,mean,SIZE_MEAN);
    }
  }
  else{
    printf("Nhap nghia cua tu:");
    gets(mean);
    btins(r,word,mean,SIZE_MEAN);
    tmp=strdup(word);
    Sndx(word);
    btins(sr,word,tmp,SIZE_WORD);
  }
}
int search(BTA *r,char *word){
  int size;
  char mean[SIZE_MEAN];
  printf("Nhap tu:");gets(word);
  strlwr(word);
  if ( btsel(r,word,mean,SIZE_MEAN,&size) ){
    printf("->Khong tim thay tu nay trong tu dien!\n");
    return -1;
  }
  else
    printf("Nghia cua tu:%s\n",mean);
}

void auto_search_main(BTA *r,char *fn,String *output){
  int k;
  printf("Nhap tu:");gets(fn);
  strlwr(fn);
  k=search_tab(r,fn,output);
  printf("So tu co chua phan dau '%s' la:%d\n",fn,k);
  search_display(r,output,k);
}

void delete(BTA *r,BTA *sr,char *fn){
  int size,i=0,j,val;
  String wl[1000];//wordlist
  char *t1;
  char word[SIZE_WORD];
  printf("Nhap tu:");gets(fn);
  strlwr(fn);  
  if ( bfndky(r,fn,&val)!=0 ) {
    printf("->Tu nay khong co trong tu dien!\n");
    return;
  }
  t1=strdup(fn);//luu tu goc vao t1 truoc khi chuyen ve ma soundex
  Sndx(fn);
  while(1){
    btsel(sr,fn,word,SIZE_WORD,&size);
    if (strcmp(word,t1)!=0) {
      wl[i++].s=strdup(word);
      btdel(sr,fn);
    }
    else break;
  }
  btdel(sr,fn);
  if (i>0){
  for(j=0;j<i;j++) btins(sr,fn,wl[j].s,size);
  }
  btdel(r,t1);
     printf("->Da xoa!\n");
}

int soundex_search(BTA *sr,char *fn,String *output){
  char *fncode;
  int i,size,total=0;
  char key[30],word[30];
  fncode=strdup(fn);
  Sndx(fncode);
  i=btsel(sr,fncode,word,SIZE_WORD,&size);//tim kiem khoa dau tien
  if (i!=0) return 0;
  else if (i==0) output[total++].s=strdup(word); 
    while(1){
      btseln(sr,key,word,SIZE_WORD,&size); //tim kiem khoa tiep theo
      if (strcmp(key,fncode)!=0) break;
      else
      output[total++].s=strdup(word);  //luu khoa tim thay vao output
      if(total==MAX_LIST) break;
    }    
  return total;
}

void soundex_search_main(BTA *r,BTA *sr,char *fn,String *output){
  int k;
  printf("Nhap tu:");gets(fn);
  strlwr(fn);
  k=soundex_search(sr,fn,output);
  printf("Tong so tu goi y cho '%s' la:%d\n",fn,k);
  search_display(r,output,k);
}
  
