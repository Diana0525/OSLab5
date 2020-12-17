#include "msh.h"

#define MAXARGS 20
#define MAXWORD 30
char whitespace[] = " \t\r\n\v";
char args[MAXARGS][MAXWORD];
char after_clear[MAXARGS][MAXWORD];
/* 
* 函数名：getcmd
* 函数功能：负责打印"==> "符号，让用户在该符号之后输入命令
*/
int
getcmd(char *buf, int nbuf)
{
  printf("==> ");
  memset(buf, 0, nbuf);
  gets(buf, nbuf);
  if(buf[0] == 0) // EOF
    return -1;
  return 0;
}
/* 
* 函数名：fork1
* 函数功能：fork1函数负责生成子进程，并在返回错误时打印错误信息
 */
int
fork1(void)
{
  int pid;

  pid = fork();
  if(pid == -1)
    panic("fork");
  return pid;
}

/* 
* 函数名：panic
* 函数功能：panic函数负责输出错误信息
 */
void
panic(char* s)
{
  printf("%s\n", s);
  exit(-1);
}
/* 
* 函数名：clearcmd
* 函数功能：解析输入的函数
 */
void
clearcmd(char* cmd, char* argv[],int* argc){
    
    //让argv的每一个元素都指向args的每一行
    for(int i=0; i<MAXARGS; i++){
        argv[i] = &args[i][0];
    }
    int i;//第i个word
    int j;
    for(i = 0,j = 0; cmd[j] != '\n' && cmd[j] != '\0'; j++){
        //每一轮循环都是寻找一行命令中的一个word
        //跳过之前的空格
        //strchr(char *str,int c),功能为在参数str所指向的字符串中搜索第一次出现字符c的位置
        //返回一个指向该字符串中第一次出现的字符的指针
        //若字符串中不包含该字符则返回NULL空指针
        while(strchr(whitespace,cmd[j])){
            j++;
        }
        //空格之后的命令存入argv
        argv[i++] = cmd+j;
        //寻找下一个空格
        //strchr函数返回0表示当前j指向的不是空格
        while(strchr(whitespace,cmd[j])==0){
            j++;
        }
        cmd[j] = '\0';//则argv能准确读取被两个空格围住的一个字符
    }
    argv[i] = 0;
    *argc = i;
    /* 调试代码 */
    printf("i=%d\n",i);
    printf("argc = %d\n",*argc);
    printf("argv[0]=%s\n",argv[0]);
    printf("argv[1]=%s\n",argv[1]);
    
}
/* 
* 函数名：clearDir
* 函数功能：解析输入的目录
* 返回值：返回目录项的个数，有几根斜线就有几个目录
 */
int
clearDir(char* dir, char* clear_dir[]){
    int i=0,j=0,count=0;
    for(int i=0; i<MAXARGS; i++){
        clear_dir[i] = &after_clear[i][0];
    }
    while(dir[i] == '\0'){ // 当前不是最后一个字符
        if(dir[i] == '/'){ // 当前是“\”，表示目录的字符
            i++;
            count++;// 记录\的数目，一个\表示一个目录
        }
        clear_dir[j++] = dir;
        while(dir[i] != '/'){
            i++;
        }
        dir[i] = '\0'; // 两个斜线之间的字符存入after_clear
        count++; // 斜线数目+1
    }
    return count;
 }
/* 
* 函数名：runcmd
* 函数功能：根据用户输入的命令运行
 */
void 
runcmd(char* argv[],int argc){
    printf("argc = %d",argc);
    int num_dir;
    uint32_t newinodeID;
    char *dirname[MAXNUM_DIR];
    newinodeID = 1 ;// 第一个inode的编号是1
    if(mystrcmp("touch",argv[0])==0){// 是创建文件的命令
        // 解析目录项
        num_dir=clearDir(argv[1], after_clear);
        for(int i = 0; i < num_dir-1; i++){ // num_dir-1表示目录项
            // 新建目录项
            newinodeID = create(after_clear[i], newinodeID, IsDir, 128);
        }
        if(create(after_clear[num_dir-1], newinodeID, IsFile, 0) == 0){ // 默认文件大小为0
            printf("touch file success!\n");
        }
    }
    else if(mystrcmp("mkdir",argv[0])==0){
        // 解析目录项
        num_dir=clearDir(argv[1], after_clear);
        for(int i = 0; i < num_dir; i++){ // num_dir表示目录项
            // 新建目录项
            newinodeID = create(after_clear[i], newinodeID, IsDir, 128);
        }
    }
    else if(mystrcmp("ls",argv[0])==0){ // ls
        num_dir = readInodeMessage(dirname);
        printf(".\n");
        printf("..\n");
        for(int i = 0; i < num_dir; i++){
            printf("%s\n",dirname[i]);
        }
    }
    else if(mystrcmp("shutdown",argv[0])){
        exit(0);
    }
}
int mystrcmp(char* a, char* b){
    int i=0,j=0;
    while(a[i] != '\0' && b[i] != '\0'){
        if(a[i] == b[i]){
            i++;
            continue;
        }
    }
    if(a[i] == '\0' && b[i] == '\0'){
        return 1;
    }
    return 0;
}
int sh(){
    static char buf[100];

    // Read and run input commands.
  while(getcmd(buf, sizeof(buf)) >= 0){
    char* argv[MAXARGS];
    int argc = -1;
    //重组参数格式
    clearcmd(buf, argv, &argc);
    runcmd(argv, argc);
  }
  exit(0);
}
int main(){

    Init();
    /* sh(); */
    /* TestInit(); */
    /* 调试代码 */
    readInodeMessage(char *dirname[])
    /************* 关闭磁盘 *************/
    close_disk();
    return 0;
}