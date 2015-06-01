#include "DoubleArray.h"

/*
int
main(int argc, char **argv)
{
	FILE *fp;
  struct da *da = da_create();
  char buf[100];
  da_add(da, "bd", 3);
  da_add(da, "aa", 3);
  da_add(da, "ba", 3);
  da_dump(da);
  while(fgets(buf, 100, fp)!=NULL){
	  da_add(da, buff, strlen(buff));
  }
  //printf("発見:%d\n",da_find(da, (const unsigned char *)"d", 1));
  //da_traverse(da, 1, buf, 0);
  return 0;
}
*/
int main(int argc, const char **argv){
	wchar_t str[SIZE_T];
	FILE *fp;
	setlocale(LC_CTYPE, "");
	DoubleArrayLib *dal = new DoubleArrayLib;

	if(argc==3 && argv[1][0]=='-' && argv[1][1]=='f'){
		fp=fopen(argv[2], "rb");
		if(!fp){
			perror("存在しないファイル名です。");
			return  EXIT_FAILURE;
		}
	}
	else{
		//usage();
		return EXIT_SUCCESS;
	}
	//fstat(fp->_file, (struct stat *)&st);
	//str=(char *)malloc(st.st_size);
	//fread(str, 1, st.st_size, fp);
	while(fgetws(str, SIZE_T, fp)!=NULL){
		//da_add(da, str, wcslen(str));
		dal->add_str(str, wcslen(str));
	}
	fclose(fp);

	dal->da_dump();

	//da_dump(da);
	//da_traverse(da, 1, str, 0);
	while(1){
		printf("どの文字を取る？:");
		wscanf(L"%s", str);
		dal->da_traverse(1, str, wcslen(str)+1);
		//printf("%d個\n", dal->da_traverse(1, str, wcslen(str)));,dal->search_str(str, wcslen(str)));
	}

	return EXIT_SUCCESS;
}

