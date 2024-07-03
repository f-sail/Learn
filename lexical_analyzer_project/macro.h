#define _CRT_SECURE_NO_WARNINGS

#ifndef MACRO_H
#define MACRO_H

/*���鳤��*/
#define ARR_SIZE(arr) (sizeof(arr)/sizeof(arr[0]))

/*malloc()����*/
#define MALLOC(num,ElemType) ((ElemType*)malloc(num * sizeof(ElemType)))

/*calloc()����*/
#define CALLOC(num,ElemType) ((ElemType*)calloc(num , sizeof(ElemType)))

/*realloc()����*/
#define REALLOC(arr,size,ElemType) ((ElemType*)realloc(arr, size * sizeof(ElementType)))

/*��������������������Ԫ��*/
#define SWAP_BY_IDX(ElemType,arr,i,j) \
do{ \
    ElemType tmp = (arr)[i]; \
    (arr)[i] = (arr)[j]; \
    (arr)[j] = tmp; \
}while(0)

/*����ָ�뽻����Ԫ��*/
#define SWAP_BY_PTR(ElemType,ptr_1,ptr_2) \
do{ \
    ElemType tmp = *(ptr_1); \
    *(ptr_1) = *(ptr_2); \
    *(ptr_2) = tmp; \
}while(0)

/*�����ж�*/
#define ERROR_CHECK(ret, error_flag, msg) \
do { \
    if ((ret) == (error_flag)) { \
        fprintf(stderr, "error: %s\n", msg); \
        exit(-1); \
    } \
} while(0)

/*�����ж�*/
#define RETURN_CHECK(ret, return_flag, msg, statement, return_value) \
do { \
    if ((ret) == (return_flag)) { \
        printf("%s\n", msg); \
        statement; \
        return return_value; \
    } \
} while(0)


#endif // !MACRO_H