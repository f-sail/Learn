#include <my_header.h>
#define N 1024

int directories = 0;
int files = 0;

/*DFS打印*/
static void my_tree(char *path, bool *arr, int arr_len);

/*把权限转换为字符串*/
static void mode_to_str(mode_t st_mode,char* str);



int main(int argc, char* argv[])
{
    char path[N] = {0};
    strcat(path, "./");

    if(argc == 1){
        printf("\033[1;34m");   //改为蓝色
        printf(".\n");          //打印当前目录（"."）
        printf("\033[0m");      //恢复默认颜色
    }else if(argc == 2){
        printf("\033[1;34m");   //改为蓝色
        printf("%s\n",argv[1]); //打印路径
        printf("\033[0m");      //恢复默认颜色

        int ret = chdir(argv[1]); // 改变到提供的根目录
        ERROR_CHECK(ret, -1, "chdir");
    }else{
        fprintf(stderr,"arrgs num error!\n");
    }
    

    /*定义数组，用于记录缩进形式*/
    bool arr[N] = {0};           //是最后一个文件时为true，反之为false
    int arr_len = 0;    //arr_len,表示缩进的层数

    /*DFS遍历*/
    my_tree(".", arr, arr_len + 1);

    /*打印目录数和文件数*/
    printf("\n%d directories, %d files\n", directories, files);

    return 0;
}



/*DFS打印*/
static void my_tree(char *path, bool *arr, int arr_len){

    /*打开目录流，并判断是否打开成功*/
    DIR *dirp = opendir(path);
    ERROR_CHECK_FUNCTION(dirp, NULL, "opendir");    

    struct dirent *entry;       //创建dirent结构体指针用于遍历目录项结点
    while((entry = readdir(dirp)) != NULL){
        /*把相应层级的标记改为false，把之前递归改的数据改回去*/
        arr[arr_len - 1] = false;
        
        /*跳过 “.” 和 “..” 文件夹*/
        if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0){
            continue;
        }

        /*判断是否是最后一个文件项*/
        long curr_flag = telldir(dirp);
        struct dirent *tmp_entry = entry;
        ERROR_CHECK_FUNCTION(curr_flag, -1, "telldir");
        if((tmp_entry = readdir(dirp)) == NULL){            
            arr[arr_len - 1] = true; 
        }else{
            //tmp_entry = readdir(dirp);
            while(tmp_entry != NULL && (strcmp(tmp_entry->d_name, ".") == 0 || strcmp(tmp_entry->d_name, "..") == 0)){
                tmp_entry = readdir(dirp);
            }
            if(tmp_entry  == NULL){            
                arr[arr_len - 1] = true; 
            }
        }
        seekdir(dirp, curr_flag);

        
        /*打印树枝*/
        for(int i = 0; i < arr_len -1; i++){
            /*是否是相应层级的最后一个文件/目录*/
            if(arr[i]){
                /*是*/
                printf("    ");     //打印四个空格
           }else{
                /*不是*/
                printf("│   ");     //打印一个竖杠、三个空格                                                
            }
        }

        /*是最后一个文件/目录打印“└──”，否则打印“├──”*/
        printf("%s", arr[arr_len - 1] ? "└── " : "├── ");


        /*创建stat结构体变量，用于接收entry所指目录项的详细信息*/
        struct stat stat_buf;
        int ret = stat(entry->d_name, &stat_buf);
        ERROR_CHECK_FUNCTION(ret, -1, "stat");

        /*获取文件类型和权限*/
        char mode_str[N] = {0};
        mode_to_str(stat_buf.st_mode, mode_str);

        /*判断文件类型*/
        if(entry->d_type == DT_DIR){
            /*当前文件为：目录文件*/
            directories++;                  //目录数加一

            /*打印文件名*/
            printf("\033[1;34m");           //改为蓝色
            printf("%s\n", entry->d_name);  //打印文件名
            printf("\033[0m");              //恢复默认颜色

            int ret = chdir(entry->d_name); // 改变工作目录到当前遍历的目录
            ERROR_CHECK(ret, -1, "chdir");  // 检查目录是否成功切换
     
            /*递归遍历这个目录文件*/                                            
            my_tree(".", arr, arr_len + 1);
       
            ret = chdir("..");              // 当前目录已经遍历完了，那就返回上>
            ERROR_CHECK(ret, -1, "chdir");  // 检查是否成功返回
        }else if(entry->d_type == DT_REG){
            /*当前文件为：普通文件*/
            files++;                        //文件数加一

            if(mode_str[3] == 'x' || mode_str[6] == 'x' || mode_str[9] == 'x'){
                //有执行权限
                printf("\033[1;32m");       //改为绿色
            }
            printf("%s\n", entry->d_name);  //打印文件名
            printf("\033[0m");              //恢复默认颜色
        }else if(entry->d_type == DT_LNK){
            /*当前文件为：软链接*/
            char link_path_buf[N] = {0};
            ssize_t ret = readlink(entry->d_name, link_path_buf, ARR_SIZE(link_path_buf) - 1);
            ERROR_CHECK_FUNCTION(ret, -1, "readlink");

            printf("\033[1;36m");           //改为青色
            printf("%s -> %s\n", entry->d_name, link_path_buf);
            printf("\033[0m");              //恢复默认颜色
        }else{
            printf("/n");
        }
    }

    closedir(dirp); //关闭目录流
    return;         //返回
}

/*把权限转换为字符串*/
static void mode_to_str(mode_t st_mode,char* str){
    if(S_ISREG(st_mode)) {str[0] = '-';}
    if(S_ISDIR(st_mode)) {str[0] = 'd';}
    if(S_ISLNK(st_mode)) {str[0] = 'l';}

    str[1] = ((st_mode & S_IRUSR) == 0) ? '-' : 'r';
    str[2] = ((st_mode & S_IWUSR) == 0) ? '-' : 'w';
    str[3] = ((st_mode & S_IXUSR) == 0) ? '-' : 'x';

    str[4] = ((st_mode & S_IRGRP) == 0) ? '-' : 'r';
    str[5] = ((st_mode & S_IWGRP) == 0) ? '-' : 'w';
    str[6] = ((st_mode & S_IXGRP) == 0) ? '-' : 'x';

    str[7] = ((st_mode & S_IROTH) == 0) ? '-' : 'r';
    str[8] = ((st_mode & S_IWOTH) == 0) ? '-' : 'w';
    str[9] = ((st_mode & S_IXOTH) == 0) ? '-' : 'x';

    str[10] = '\0';
    return;
}
