// __LINE__         当前源代码行号
// __FUNCTION__     当前函数名称的字符串
// __FILE__         源文件名的字符串

#include "log.h"
#include "head.h"
#include "config.h"
      
#define LOG_VALUE_SIZE 128
//#define LOG_BUF_SIZE 1024

/*宏定义：日志打印等级在配置文件中的key值*/       
#define KEY_LEVEL "LogLevel"

///*宏定义：四种打印等级的字符串字面值*/
//#define STR_LEVEL_DEBUG     "LEVEL_DEBUG"       
//#define STR_LEVEL_INFO      "LEVEL_INFO"
//#define STR_LEVEL_WARNING   "LEVEL_WARNING"
//#define STR_LEVEL_ERROR     "LEVEL_ERROR"
//
///*宏定义：四种打印等级的函数所用的锁*/
//#define MUTEX_LOG_DEBUG     "mutex_log_debug"
//#define MUTEX_LOG_INFO      "mutex_log_info"
//#define MUTEX_LOG_WARNING   "mutex_log_warning"
//#define MUTEX_LOG_ERROR     "mutex_log_error"

/*定义全局变量g_log_level为LEVEL_INFO等级*/
Level g_log_level = LEVEL_INFO;

/*互斥锁，用来防止数据混乱*/
pthread_mutex_t g_mutex_log = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t mutex_log_debug = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t mutex_log_info = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t mutex_log_warning = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t mutex_log_error = PTHREAD_MUTEX_INITIALIZER;


/*获取当前日期时间字符串*/
static int getDateTime(char *date_time);


/*把日志等级由字符串转换为LeveL类型*/
int logStrToLevel(const char *str_level, Level *ptr_level){
    if(strncmp(str_level, STR_LEVEL_DEBUG, strlen(STR_LEVEL_DEBUG)) == 0){
        /*LEVEL_DEBUG*/
        *ptr_level = LEVEL_DEBUG;
        return 0;
    }
    if(strncmp(str_level, STR_LEVEL_INFO, strlen(STR_LEVEL_INFO)) == 0){
        /*LEVEL_INFO*/
        *ptr_level = LEVEL_INFO;
        return 0;
    }
    if(strncmp(str_level, STR_LEVEL_WARNING, strlen(STR_LEVEL_WARNING)) == 0){
        /*LEVEL_WARNING*/
        *ptr_level = LEVEL_WARNING;
        return 0;
    }
    if(strncmp(str_level, STR_LEVEL_ERROR, strlen(STR_LEVEL_ERROR)) == 0){
        /*LEVEL_ERROR*/
        *ptr_level = LEVEL_ERROR;
        return 0;
    }

    return -1;
}


/*把日志等级由LeveL转换为字符串类型*/
int logLevelToStr(Level level, char *str_level){
    if(level == LEVEL_DEBUG){
        /*LEVEL_DEBUG*/
        strcpy(str_level, STR_LEVEL_DEBUG);
        return 0;
    }
    if(level == LEVEL_INFO){
        /*LEVEL_INFO*/
        strcpy(str_level, STR_LEVEL_INFO);
        return 0;
    }
    if(level == LEVEL_WARNING){
        /*LEVEL_WARNING*/
        strcpy(str_level, STR_LEVEL_WARNING);
        return 0;
    }
    if(level == LEVEL_ERROR){
        /*LEVEL_ERROR*/
        strcpy(str_level, STR_LEVEL_ERROR);
        return 0;
    }
    return -1;
}



/*功能：获取配置文件中的日志等级，并设置,若配置文件中没有，则写入配置文件 默认等级LEVEL_FINO*/
int logGetLevel(Level *ptr_level){
    const char *key = KEY_LEVEL;
    char value[LOG_VALUE_SIZE] = {0};
    int ret = getParameter(KEY_LEVEL, value);

    if (ret == -1){
        /*配置文件未找到日志等级*/
        setParameter(key,STR_LEVEL_INFO); 
        g_log_level = LEVEL_INFO;
    }else{
        /*配置文件找到日志等级*/
        logStrToLevel(value, ptr_level);
        g_log_level = *ptr_level;
        return 0;
    }
    return -1;
}

/*功能：更改需要打印的日志等级*/
int logSetLevel(Level level){
    /*设置全局变量*/
    g_log_level = level;

    /*设置配置文件*/
    const char *key = KEY_LEVEL;
    char value[LOG_VALUE_SIZE] = {0};
    logLevelToStr(level, value);
    setParameter(key, value);

    return 0;
}


/*功能：打印DEBUG信息*/
int logDebug(const char *msg){
    /*判断打印级别*/
    if(g_log_level > LEVEL_DEBUG){
        /*设置打印级别高于当前级别*/
        return 0;
    };

    /*打印*/
    pthread_mutex_lock(&mutex_log_debug);       //上锁
    FILE *fp = fopen(PATH_LEVEL_DEBUG, "a+");   //打开文件
    if(fp == NULL){
        return -1;                          
    }

    /*创建字符数组并获取时刻信息*/
    char date_time[LOG_BUF_SIZE] = {0};
    getDateTime(date_time);

    /*拼接日志信息*/
    char buf[LOG_BUF_SIZE << 1] = {0};
    sprintf(buf, "Debug：%s\t %s\n", date_time, msg);

    /*写入日志信息*/
    int ret = fwrite(buf, sizeof(char), strlen(buf), fp);
    if(ret == -1){
        return -1;
    }

    fclose(fp);                                 //关闭文件
    pthread_mutex_unlock(&mutex_log_debug);     //解锁

    return 0;
}

/*功能：打印普通信息*/
int logInfo(const char *msg){
    /*判断打印级别*/
    if(g_log_level > LEVEL_INFO){
        /*设置打印级别高于当前级别*/
        return 0;
    };

    /*打印*/
    pthread_mutex_lock(&mutex_log_info);        //上锁
    FILE *fp = fopen(PATH_LEVEL_INFO, "a+");    //打开文件
    if(fp == NULL){
        return -1;
    }

    /*创建字符数组并获取时刻信息*/
    char date_time[LOG_BUF_SIZE] = {0};
    getDateTime(date_time);

    /*拼接日志信息*/
    char buf[LOG_BUF_SIZE << 1] = {0};
    sprintf(buf, "Info：%s\t %s\n", date_time, msg);

    /*写入日志信息*/
    int ret = fwrite(buf, sizeof(char), strlen(buf), fp);
    if(ret == -1){
        return -1;
    }

    fclose(fp);                                 //关闭文件
    pthread_mutex_unlock(&mutex_log_info);      //解锁锁
    return 0;
}

/*功能：打印警告信息*/
int logWarning(const char *msg){
    /*判断打印级别*/                  
    if(g_log_level > LEVEL_WARNING){     
        /*设置打印级别高于当前级别*/  
        return 0;                     
    };

    /*打印*/
    pthread_mutex_lock(&mutex_log_warning);     //上锁
    FILE *fp = fopen(PATH_LEVEL_WARNING, "a+"); //打开文件
    if(fp == NULL){
        return -1;
    }

    /*创建字符数组并获取时刻信息*/
    char date_time[LOG_BUF_SIZE] = {0};
    getDateTime(date_time);

    /*拼接日志信息*/
    char buf[LOG_BUF_SIZE << 1] = {0};
    sprintf(buf, "WARNING：%s\t %s\n", date_time, msg);

    /*写入日志信息*/
    int ret = fwrite(buf, sizeof(char), strlen(buf), fp);
    if(ret == -1){
        return -1;
    }

    fclose(fp);                                 //关闭文件
    pthread_mutex_unlock(&mutex_log_warning);   //解锁
    return 0;
}

/*功能：打印错误信息*/
int logError(const char *msg){
    /*判断打印级别*/                  
    if(g_log_level > LEVEL_ERROR){     
        /*设置打印级别高于当前级别*/  
        return 0;                     
    };     

    /*打印*/
    pthread_mutex_lock(&mutex_log_error);       //上锁
    FILE *fp = fopen(PATH_LEVEL_ERROR, "a+");   //打开文件
    if(fp == NULL){
        return -1;
    }

    /*创建字符数组并获取时刻信息*/
    char date_time[LOG_BUF_SIZE] = {0};
    getDateTime(date_time);

    /*拼接日志信息*/
    char buf[LOG_BUF_SIZE << 1] = {0};
    sprintf(buf, "ERROR: %s\t %s", date_time, msg);

    /*写入日志信息*/
    int ret = fwrite(buf, sizeof(char), strlen(buf), fp);
    if(ret == -1){
        return -1;
    }

    fclose(fp);                                 //关闭文件
    pthread_mutex_unlock(&mutex_log_error);     //解锁
    return 0;       
}




/*获取当前日期时间字符串*/
int getDateTime(char *date_time){
    time_t t;
    struct tm *ptr_timeInfo;

    time(&t);
    ptr_timeInfo = localtime(&t);

    strftime(date_time, 80, "%Y-%m-%d %H:%M:%S", ptr_timeInfo);

    return 0;
}
