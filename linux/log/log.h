#ifndef __LOG_H__
#define __LOG_H__

#include <pthread.h>

#define LOG_BUF_SIZE 1024

/*宏定义：四种打印等级的字符串字面值*/          
#define STR_LEVEL_DEBUG     "LEVEL_DEBUG"       
#define STR_LEVEL_INFO      "LEVEL_INFO"
#define STR_LEVEL_WARNING   "LEVEL_WARNING"
#define STR_LEVEL_ERROR     "LEVEL_ERROR"

/*宏定义：四种打印等级的函数所用的锁*/
#define MUTEX_LOG_DEBUG     "mutex_log_debug"
#define MUTEX_LOG_INFO      "mutex_log_info"
#define MUTEX_LOG_WARNING   "mutex_log_warning"
#define MUTEX_LOG_ERROR     "mutex_log_error"

/*宏定义：四种打印等级的日志路径*/
#define PATH_LEVEL_DEBUG    "./log/LogDebug.log"
#define PATH_LEVEL_INFO     "./log/LogInfo.log"
#define PATH_LEVEL_WARNING  "./log/LogWarning.log"
#define PATH_LEVEL_ERROR    "./log/LogError.log"

/*枚举类型：用于表明日志等级*/
typedef enum{
    LEVEL_DEBUG,    //调试
    LEVEL_INFO,     //普通    
    LEVEL_WARNING,  //警告
    LEVEL_ERROR     //错误
}Level;

/*声明全局变量*/
extern Level g_log_level;           // 全局变量g_log_level，用于表明日志打印等级（默认定义为了LEVEL_INFO等级）
extern pthread_mutex_t g_mutex_log; // 全局变量g_mutex_log，用于防止数据混乱


/*=================================== Log宏函数 ====================================*/
// LOG_DEBUG()                  打印调试日志
// LOG_INFO()                   打印信息日志
// LOG_WARNING()                打印警告日志（若数据与期望的数据不符合，才会打印进日志文件）
// LOG_ERROR_ERRNO()            打印基于 errno 显示的错误信息  （可代替之前的ERROR_CHECK               出现错误会打印进日志文件，但没有封装exit()）
// LOG_ERROR_BY_RIGHT_RET()     打印基于 返回值(是否正确) 显示的错误信息 （可代替之前的ERROR_CHECK_PTHREAD 等    出现错误会打印进日志文件，但没有封装exit()）
// LOG_ERROR_BY_ERROR_RET()     打印基于 返回值(是否错误) 显示的错误信息 （可代替之前的ERROR_CHECK_PTHREAD 等    出现错误会打印进日志文件，但没有封装exit()）

/*宏函数：用于打印调试日志*/
#define LOG_DEBUG(msg) \
do{ \
    char log_date[LOG_BUF_SIZE] = {0}; \ 
    sprintf(log_date, "文件名：%s\t 函数名：%s\t 行数：%d\t // %s\n", __FILE__, __FUNCTION__, __LINE__, msg); \
    logDebug(log_date); \
}while(0)


/*宏函数：用于打印信息日志*/
#define LOG_INFO(msg) \
do{ \
    char log_date[LOG_BUF_SIZE] = {0}; \ 
    sprintf(log_date, "文件名：%s\t 函数名：%s\t 行数：%d\t // %s\n", __FILE__, __FUNCTION__, __LINE__, msg); \
    logInfo(log_date); \
}while(0)


/*宏函数：用于打印警告日志*/
#define LOG_WARNING(value, expected_value, msg) \
do{ \
    if (value != expected_value){ \
        char log_date[LOG_BUF_SIZE] = {0}; \ 
        sprintf(log_date, "文件名：%s\t 函数名：%s\t 行数：%d\t // %s\nCSUSE: value is not expected\n\n", __FILE__, __FUNCTION__, __LINE__, msg); \
        logWarning(log_date); \
    } \
}while(0)


/*
 * 宏函数：基于 errno 显示的错误信息
 * ret          // 函数返回值
 * error_flag   // 错误的标记，（eg：NULL、-1 等）
 * msg          // 需要添加的打印信息
 */
#define LOG_ERROR_ERRNO(ret, error_flag, msg) \
do{ \
    if(ret == error_flag){ \
        pthread_mutex_lock(&g_mutex_log); \
        char log_date[LOG_BUF_SIZE] = {0}; \ 
        sprintf(log_date, "文件名：%s\t 函数名：%s\t 行数：%d\t // %s\n",__FILE__, __FUNCTION__, __LINE__, msg); \
        logError(log_date); \
        int tmp_stderr = dup(STDERR_FILENO); \
        int fd_file = open(PATH_LEVEL_ERROR, O_WRONLY | O_APPEND); \
        dup2(fd_file, STDERR_FILENO); \
        perror("CAUSE"); \
        fprintf(stderr,"\n"); \
        dup2(tmp_stderr, fd_file); \                                
    } \
    pthread_mutex_unlock(&g_mutex_log);\
}while(0)


/*
 * 宏函数：基于 返回值是否正确 判断
 * ret          // 函数返回值
 * right_flag   // 正确返回的标记
 * msg          // 需要添加的打印信息
 */
#define LOG_ERROR_BY_RIGHT_RET(ret, right_flag, msg) \
do{\
    if(ret != right_flag){ \
        pthread_mutex_lock(&g_mutex_log); \
        char log_date[LOG_BUF_SIZE] = {0};\ 
        sprintf(log_date, "文件名：%s\t 函数名：%s\t 行数：%d\t // %s",__FILE__, __FUNCTION__, __LINE__, msg);\
        logError(log_date); \
        int tmp_stderr = dup(STDERR_FILENO); \
        int fd_file = open(PATH_LEVEL_ERROR, O_WRONLY | O_APPEND); \
        dup2(fd_file, STDERR_FILENO); \
        fprintf(stderr, "CAUSE: %s\n\n", strerror(ret)); \          
        dup2(tmp_stderr, fd_file); \                                
    } \
    pthread_mutex_unlock(&g_mutex_log);\
}while(0)


/*
 * 宏函数：基于 返回值是否错误 判断
 * ret          // 函数返回值
 * error_flag   // 错误返回的标记，（eg：NULL、-1 等）
 * msg          // 需要添加的打印信息
 */
#define LOG_ERROR_BY_ERROR_RET(ret, error_flag, msg) \
do{\
    if(ret == error_flag){ \
        pthread_mutex_lock(&g_mutex_log); \
        char log_date[LOG_BUF_SIZE] = {0};\ 
        sprintf(log_date, "文件名：%s\t 函数名：%s\t 行数：%d\t // %s",__FILE__, __FUNCTION__, __LINE__, msg);\
        logError(log_date); \
        int tmp_stderr = dup(STDERR_FILENO); \
        int fd_file = open(PATH_LEVEL_ERROR, O_WRONLY | O_APPEND); \
        dup2(fd_file, STDERR_FILENO); \
        fprintf(stderr, "CAUSE: %s\n\n", strerror(ret)); \          
        dup2(tmp_stderr, fd_file); \                                
    } \
    pthread_mutex_unlock(&g_mutex_log);\
}while(0)



/*================================ 函数：获取与设置等级 ================================*/
/*
 * 功能：获取当前配置文件中的日志等级,
 *       若配置文件中没有，则默认设置为LEVEL_INFO，并写入配置文件
 * 成功返回：0
 * 失败返回：-1
 */
int logGetLevel(
    Level *ptr_level     // 用于接收当前日志等级的Level指针类型变量(传入传出参数)
);


/*
 * 功能：更改需要打印的日志等级,并写入配置文件
 * 成功返回：0
 * 失败返回：-1
 */
int logSetLevel(
    Level level     // 想要修改为的日志打印等级(LEVEL_DEBUG, LEVEL_INFO, LEVEL_WARNING, LEVEL_ERROR)
);


/*================================ 函数：日志分级打印 ================================*/

/*
 * 功能：打印调试信息
 * 成功返回：0
 * 失败返回：-1
 */
int logDebug(
    const char *msg     // 用于传入需要补充的打印信息
);


/*
 * 功能：打印普通信息
 * 成功返回：0
 * 失败返回：-1
 */
int logInfo(
    const char *msg     // 用于传入需要补充的打印信息
);


/*
 * 功能：打印警告信息
 * 成功返回：0
 * 失败返回：-1
 */
int logWarning(
    const char *msg     // 用于传入需要补充的打印信息
);


/*
 * 功能：打印错误信息
 * 成功返回：0
 * 失败返回：-1
 */
int logError(
    const char *msg     // 用于传入需要补充的打印信息
);


/*================================ 函数：level的类型转换 ================================*/
/*
 * 功能：把日志等级由字符串转换为LeveL类型
 * 成功返回：0
 * 失败返回：-1
 */
int logStrToLevel(
    const char *str_level,  // 想要修改的源字符串类型
    Level *ptr_level        // 用于获取转换得到的Level枚举类型（传入传出参数）
);


/*
 * 功能：把日志等级由LeveL类型转换为字符串类型
 * 成功返回：0
 * 失败返回：-1
 */
int logLevelToStr(
    Level level,            // 想要转换的源Level枚举类型
    char *str_level         // 用于获取转换得到的字符串类型（传入传出参数）
);


#endif // !__LOG_H__
