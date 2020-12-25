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
* 函数名：clearcmd
* 函数功能：解析输入的命令
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
}
/* 
* 函数名：clearDir
* 函数功能：解析输入的目录
* 返回值：返回目录项的个数，有几根斜线就有几个目录
 */
int
clearDir(char* dir, char* clear_dir[]){
    int i=0,j=0,count=0;
    if(dir[0] == '\0' && dir[1] == '/'){
        return 0;// 表示只有一个斜杠，没有目录项，默认为根目录
    }
    for(int i=0; i<MAXARGS; i++){
        clear_dir[i] = &after_clear[i][0];
    }
    /* 计算一共有多少个斜线 */
    while(dir[i] != '\0'){
        if(dir[i] == '/'){
            count ++;
        }
        i++;
    }
    for(i = 0,j = 0; i < count-1; j++){
        while(dir[j] == '/'){
            j++;
        }
        clear_dir[i++] = dir+j;
        
        while(dir[j] != '/'){
            j++;
        }
        dir[j] = '\0';
    }
    if(count == 1){ // 针对只有一个目录项的特殊情况
        clear_dir[0] = dir+1;
    }
    else{
        clear_dir[i] = dir+j;
    }
    return count;
 }
/* 
* 函数名：runcmd
* 函数功能：根据用户输入的命令运行
 */
void 
runcmd(char* argv[],int argc){
    int num_dir,num_file;// num_dir表示有多少级目录，num_file表示该目录最终有多少文件
    uint32_t newinodeID = 1,temp=1,old_InodeID;// 第一个inode的编号是1
    char *dirname[MAXNUM_DIR];// 存储ls输出的内容
    char* clear_dir[MAXARGS]; // 存储/之间的目录项名称
    int i = 0;
    if(mystrcmp("touch",argv[0]) == 1){// 是创建文件的命令
        // 解析路径
        num_dir=clearDir(argv[1], clear_dir);
        for(i = 0; i < num_dir-1; i++){ // 依次判断目录项是否已经存在，若存在则返回inodeID
            temp = newinodeID;
            newinodeID = findInodeforItem(newinodeID, clear_dir[i], IsDir);
            if(newinodeID == 0){ // 表示目录项不存在，需要创建
                newinodeID = temp;
                break;
            }
        }
        for( ; i < num_dir-1; i++){ // num_dir-1表示目录项
            // 新建目录项
            newinodeID = create(clear_dir[i], newinodeID, IsDir, 128);
        }
        newinodeID = create(clear_dir[num_dir-1], newinodeID, IsFile, 128); // 创建指向文件的目录项
        if(create(clear_dir[num_dir-1], newinodeID, IsFile, 0) == 1){ // 默认文件大小为0
            printf("touch file success!\n");
        }
    }
    else if(mystrcmp("mkdir",argv[0]) == 1){
        // 解析路径
        num_dir=clearDir(argv[1], clear_dir);
        for(i = 0; i < num_dir-1; i++){ // 依次判断目录项是否已经存在，若存在则返回inodeID
            temp = newinodeID;
            newinodeID = findInodeforItem(newinodeID, clear_dir[i], IsDir);
            if(newinodeID == 0){ // 表示目录项不存在，需要创建
                newinodeID = temp;
                break;
            }
        }
        for( ; i < num_dir; i++){ // num_dir-1表示目录项
            // 新建目录项
            newinodeID = create(clear_dir[i], newinodeID, IsDir, 128);
        }
    }
    else if(mystrcmp("ls",argv[0]) == 1){ // ls
        // 解析路径
        if(argv[1] != 0){ // 若后面接着目录项，则解析目录
            num_dir = clearDir(argv[1], clear_dir);
        }
        if(argc == 1 || (argc == 2 && mystrcmp("/",argv[1]) == 1)){ // 指定根目录
            num_file = lsDirName(1, dirname);
        }
        else{
            for(i = 0; i < num_dir; i++){
                newinodeID = readInodeMessage(newinodeID, clear_dir[i]);
                if(newinodeID == 0){ // 找不到对应目录
                    printf("error path!\n");
                    return;
                }
            }
            num_file = lsDirName(newinodeID, dirname);
        }
        printf(".\n");
        printf("..\n");
        for(i = 0; i < num_file; i++){
            printf("%s\n",dirname[i]);
        }
    }
    else if (mystrcmp("cp",argv[0]) == 1)
    {
        if(argc < 3){
            printf("lack for something!\n");
            return 0;// 命令错误
        }
        // argv[1] 中存放被复制文件的路径
        // argv[2] 中存放需要新建的文件路径
        // 解析路径
        num_dir=clearDir(argv[1], clear_dir); // 解析被复制的文件的目录
        for ( i = 0; i < num_dir-1; i++) // 根据目录项寻找inodeID
        {
            newinodeID = findInodeforItem(newinodeID, clear_dir[i], IsDir);
            if(newinodeID == 0){
                printf("can't find file!");// 找不到相应的目录
            }
        }
        old_InodeID = findInodeforItem(newinodeID, clear_dir[num_dir-1], IsFile);// 找到被复制文件的inodeID
        newinodeID = 1; // 重置为1，准备新建目的目录的文件
        num_dir=clearDir(argv[2], clear_dir); // 解析被复制的文件的路径
        for( i = 0; i < num_dir-1; i++){
            temp = newinodeID;
            newinodeID = findInodeforItem(newinodeID, clear_dir[i], IsDir);
            if(newinodeID == 0){ // 表示目录项不存在，需要创建
                newinodeID = temp;
                break;
            }
        }
        for( ; i < num_dir-1; i++){ // num_dir-1表示目录项
            // 新建目录项
            newinodeID = create(clear_dir[i], newinodeID, IsDir, 128);
        }
        newinodeID = create(clear_dir[num_dir-1], newinodeID, IsFile, 128); // 创建指向文件的目录项
        if(copyFile(old_InodeID, newinodeID) == 1){
            printf("copy successfully!\n");
        }
    }
    else if(mystrcmp("shutdown",argv[0]) == 1){
        /************* 关闭磁盘 *************/
        close_disk();
        exit(0);
    }
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
    sh();
    return 0;
}