#include <my_header.h>
#include <pwd.h>
#include <grp.h>

#define BEGIN_YEAR 1970
#define N 1024
#define TIME_ZONE_OFFSET (8 * 60 * 60)
#define SECONDS_PER_DAY (24 * 60 * 60)
#define SECONDS_PER_HOUR (60 * 60)
#define SECONDS_PER_MINUTE (60)

typedef struct{
    char mode_str[N];
    unsigned long nlinks_count;
    const char *uid;
    const char *gid;
    unsigned long size;
    char time_str[N];
    const char *name;
}FileInfo;

typedef struct{
    FileInfo* arr[N];
    int len;
}FileArray;

/*=========================================== 函数声明 ===========================================*/
/*是否是闰月*/
static bool is_leap_year(int year);

/*目标是否在所给数组中*/
static bool is_in_array(long *arr, int arr_len, long target);

/*获取当前年份共有多少秒数*/
static int get_current_year_seconds(int year);

/*把权限转换为字符串*/
static void mode_to_str(mode_t st_mode,char* str);

/*把最后修改时间转换为字符串*/
static void time_to_str(long total_seconds, char* time_str);

/*排序的规则，用于qsort*/
static int my_cmp(const void *a, const void *b);

/*打印结构体数组*/
static void print_file_array(FileArray *ptr_fileArray);

/*把数据存入结构体数组*/
static void insert_file_array(FileArray *ptr_fileArray, DIR *dirp);

/*=========================================== main函数 ===========================================*/
int main(int argc, char* argv[])
{
    char *path_name;    //用于记录待打印目录的路径名
    /*检测命令行参数*/
    if(argc == 1){
        path_name = ".";
    }else if(argc == 2){
        path_name = argv[1];
    }else{
        fprintf(stderr,"error: args num is not rightful!\n");
    }

    /*打开目录流，并检测是否打开成功*/
    DIR *dirp = opendir(path_name);
    ERROR_CHECK_FUNCTION(dirp,NULL,"opendir");

    /*切换工作目录，改为传入的路径名*/
    int ret = chdir(path_name);
    ERROR_CHECK_FUNCTION(ret,-1,"chdir");

    /*定义结构体数组*/
    FileArray *ptr_fileArray = CALLOC(1, FileArray);
    ERROR_CHECK_FUNCTION(ptr_fileArray, NULL, "CALLOC");


    /*存入目录项*/
    insert_file_array(ptr_fileArray, dirp);

    /*排序*/
    qsort(ptr_fileArray, ptr_fileArray->len, sizeof(ptr_fileArray->arr[0]), my_cmp); 
    
    /*打印*/
    print_file_array(ptr_fileArray);

    /*释放所有数组元素的内存空间*/
    for(int i = 0; i < ptr_fileArray->len; i++){
        free(ptr_fileArray->arr[i]);
    }
    
    free(ptr_fileArray);    //释放ptr_fileArray指向的内存空间
    closedir(dirp);     //关闭目录流
    return 0;           //返回 0
}



/*=========================================== 函数定义 ===========================================*/
/*把权限转换为字符串*/
static void mode_to_str(mode_t st_mode, char* str){
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

/*把最后修改时间转换位字符串*/
static void time_to_str(long total_seconds, char* time_str){
    total_seconds += TIME_ZONE_OFFSET;

    long current_year = BEGIN_YEAR;
    long current_month = 1;
    long current_day = 1;
    long current_hour = 0;
    long current_minute = 0;
    long current_second = 0;

    long arr_big[] = {1, 3, 5 ,7, 8, 10, 12};
    long arr_small[] = {4, 6, 9, 11};

    long current_year_seconds = get_current_year_seconds(current_year);
    
    /*年*/
    while(total_seconds - current_year_seconds >= 0){
        total_seconds -= current_year_seconds;
        current_year++;
        current_year_seconds = get_current_year_seconds(current_year);
    }//!while, current_year为实际年份，total_seconds为剩余秒数
    
    /*月*/
    while(current_month < 13){
        /*大月*/
        if(is_in_array(arr_big, ARR_SIZE(arr_big), current_month)){
            if(total_seconds >= 31 * SECONDS_PER_DAY){
                total_seconds -= 31 * SECONDS_PER_DAY;
                current_month++;
                continue;
            }
            break;
        }

        /*小月*/
        if(is_in_array(arr_small, ARR_SIZE(arr_small), current_month)){
            if(total_seconds >= 30 * SECONDS_PER_DAY){
                total_seconds -= 30 * SECONDS_PER_DAY;
                current_month++;
                continue;
            }
            break;
        }

        /*二月*/
        if(is_leap_year(current_year)){
            /*闰年*/
            if(total_seconds >= 29 * SECONDS_PER_DAY){
                total_seconds -= 29 * SECONDS_PER_DAY;
                current_month++;
                continue;
            }
            break;
        }else{
            /*非闰年*/
            if(total_seconds >= 28 * SECONDS_PER_DAY){
                total_seconds -= 28 * SECONDS_PER_DAY;
                current_month++;
                continue;
            }
            break;
        }
        
    }//!while, current_month为实际月份，total_seconds为剩余秒数

    /*日*/
    while(total_seconds - SECONDS_PER_DAY >= 0){
        total_seconds -= SECONDS_PER_DAY;
        current_day++;
    }//!while, current_day为实际日期，total_seconds为剩余秒数

    /*时*/
    while(total_seconds - SECONDS_PER_HOUR >= 0){
        total_seconds -= SECONDS_PER_HOUR;
        current_hour++;
    }//!while

    /*分*/
    while(total_seconds - SECONDS_PER_MINUTE >= 0){
        total_seconds -= SECONDS_PER_MINUTE;
        current_minute++;
    }//!while

    /*秒*/
    current_second = total_seconds;

    sprintf(time_str," %ld.%02ld.%02ld %02ld:%02ld:%02ld",current_year, current_month, current_day, current_hour, current_minute, current_second);
    return;
}
    
/*目标是否在所给数组中*/
static bool is_in_array(long *arr, int arr_len, long target){
    long *p = arr;
    while(p < arr + arr_len){
        if(*p++ == target){
            return true;
        }
    }
    return false;
}

/*是否是闰月*/
static bool is_leap_year(int year) {
    if (year % 4 != 0) return 0;
    else if (year % 100 != 0)return 1;
    else if (year % 400 != 0) return 0;
    else return 1;
}

/*获得所给年份的总秒数*/
static int get_current_year_seconds(int year){
    if(is_leap_year(year)){
        return 366 * SECONDS_PER_DAY;
    }else{
        return 365 * SECONDS_PER_DAY;
    }
}

/*排序规则，用于qsort*/
static int my_cmp(const void *a, const void *b){ 
    const FileInfo *fileInfo1 = *(const FileInfo **)a;
    const FileInfo *fileInfo2 = *(const FileInfo **)b;
    
    char name1[N],name2[N];
    strcpy(name1,fileInfo1->name);
    strcpy(name2,fileInfo2->name);

    name1[0] = toupper(name1[0]);
    name2[0] = toupper(name2[0]);

    if(name1[0] == name2[0]){
        return strcmp(fileInfo1->name, fileInfo2->name);
    }
    return name1[0] - name2[0];
}

/*打印结构体数组*/
static void print_file_array(FileArray *ptr_fileArray){
    for(int i = 0; i < ptr_fileArray->len; i++){
        printf("%s %lu %s %s %5lu %s ",
            ptr_fileArray->arr[i]->mode_str,
            ptr_fileArray->arr[i]->nlinks_count,
            ptr_fileArray->arr[i]->uid,
            ptr_fileArray->arr[i]->gid,
            ptr_fileArray->arr[i]->size,
            ptr_fileArray->arr[i]->time_str
        );
        if(ptr_fileArray->arr[i]->mode_str[0] == 'd'){
            printf("\033[1;34m");           //改为蓝色
            printf("%s\n", ptr_fileArray->arr[i]->name);    //打印文件名
            printf("\033[0m");              //恢复默认颜色
        }else{
            if(ptr_fileArray->arr[i]->mode_str[3] == 'x' || ptr_fileArray->arr[i]->mode_str[6] == 'x' || ptr_fileArray->arr[i]->mode_str[9] == 'x'){
                //有执行权限
                printf("\033[1;32m");       //改为绿色
            }
            printf("%s\n", ptr_fileArray->arr[i]->name);  //打印文件名
            printf("\033[0m");              //恢复默认颜色                                  
        }
    }
    return;
}

/*把数据存入结构体数组*/
static void insert_file_array(FileArray *ptr_fileArray, DIR *dirp){
    struct dirent *entry;
    /*readdir()读取失败或读到目录流末尾时返回NULL*/
    while((entry = readdir(dirp)) != NULL){
        struct stat stat_buf;   
        int ret = stat(entry->d_name, &stat_buf);
        ERROR_CHECK_FUNCTION(ret,-1,"stat");

        /*为结构体申请空间*/
        ptr_fileArray->arr[ptr_fileArray->len] = CALLOC(1, FileInfo);
        ERROR_CHECK_FUNCTION(ptr_fileArray->arr[ptr_fileArray->len], NULL, "CALLOC");

        /*硬链接数*/
        ptr_fileArray->arr[ptr_fileArray->len]->nlinks_count = stat_buf.st_nlink;

        /*文件类型和权限*/
        mode_to_str(stat_buf.st_mode,ptr_fileArray->arr[ptr_fileArray->len]->mode_str);
        
        /*用户id和组id*/
        ptr_fileArray->arr[ptr_fileArray->len]->uid = getpwuid(stat_buf.st_uid)->pw_name;
        ptr_fileArray->arr[ptr_fileArray->len]->gid = getgrgid(stat_buf.st_gid)->gr_name;

        /*size*/
        ptr_fileArray->arr[ptr_fileArray->len]->size = stat_buf.st_size;

        /*时间*/
        time_to_str(stat_buf.st_mtime, ptr_fileArray->arr[ptr_fileArray->len]->time_str);

        /*名字*/
        //strcpy(ptr_fileArray->arr[ptr_fileArray->len]->name,entry->d_name);
        ptr_fileArray->arr[ptr_fileArray->len]->name = entry->d_name;

        ptr_fileArray->len++;   //数组长度加一
    }
    return;
}
