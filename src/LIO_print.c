#include "LIO.h"
#include <stdint.h>
#include <limits.h>
#include <stdarg.h>

#define BUFFER_SIZE 255
#define PARSE_BUFFER_SIZE 31
    
#define UNSIGNED_TYPE *format == 'u' || *format == 'x' || *format == 'o'
    
#define CHECK_INT_TYPE(ARG_GET)\
    uint64_t num;\
    if (!(UNSIGNED_TYPE)) {\
        int64_t arg;\
        if (special_type <= -2){\
            outToCOMport("1");\
            ARG_GET(int)\
        }\
        if (special_type == -1){\
            ARG_GET(int)\
        }\
        if (special_type == 0){\
            ARG_GET(int)\
        }\
        if (special_type == 1){\
            ARG_GET(long)\
        }\
        if (special_type >= 2){\
            ARG_GET(long long)\
        }\
        if (arg < 0) {\
            sign = -1;\
            num = -arg;\
        } else {\
            num = arg;\
        }\
    }\
    if (UNSIGNED_TYPE) {\
        uint64_t arg;\
        if (special_type <= -2){\
            ARG_GET(int)\
        }\
        if (special_type == -1){\
            ARG_GET(int)\
        }\
        if (special_type == 0){\
            ARG_GET(unsigned int)\
        }\
        if (special_type == 1){\
            ARG_GET(unsigned long)\
        }\
        if (special_type >= 2){\
            ARG_GET(unsigned long long)\
        }\
        num = arg;\
    }\
    arg = num;
    
    
#define PARSE_INT(BASE)\
    if (arg == 0) {\
        parse_buffer[0] = '0';\
        pbuff_count = 1;\
    }\
    while (arg != 0) {\
        if (arg % (BASE) < 10) {\
            parse_buffer[pbuff_count] = '0' + (arg % (BASE));\
        } else {\
            parse_buffer[pbuff_count] = 'a' + (arg % (BASE)) - 10;\
        }\
        arg /= (BASE);\
        pbuff_count ++;\
    }\
    if ((BASE) == 16) {\
        parse_buffer[pbuff_count] = 'x';\
        pbuff_count++;\
        parse_buffer[pbuff_count] = '0';\
        pbuff_count++;\
    }\
    if (sign < 0) {\
        parse_buffer[pbuff_count] = '-';\
        pbuff_count++;\
    }\
    for (int l = 0, r = pbuff_count - 1; l < r; l++, r--) {\
        char tmp = parse_buffer[l];\
        parse_buffer[l] = parse_buffer[r];\
        parse_buffer[r] = tmp; \
    }
    
#define PRINT(ARGS_INIT, ARG_GET, ARGS_CLOSE,\
              OUTPUT_INIT, OUTPUT)\
    ARGS_INIT\
    OUTPUT_INIT\
    char buffer[BUFFER_SIZE+1];\
    char parse_buffer[PARSE_BUFFER_SIZE+1];\
    int buff_count = 0;\
    int pbuff_count = 0;\
    int escape = 0;\
    int special = 0;\
    int special_type = 0;\
    int count = 0;\
    while (*format != 0) {\
        if (escape) {\
            buffer[buff_count] = *format;\
            buff_count++;\
            escape = 0;\
            format++;\
            count++;\
        } else {\
            if (special) {\
                if (*format == 'd' || *format == 'i' || *format == 'u' || *format == 'o' || *format == 'x') {\
                    uint64_t arg;\
                    int sign = 1;\
                    pbuff_count = 0;\
                    CHECK_INT_TYPE(ARG_GET);\
                    if (*format == 'o') {\
                        PARSE_INT(8);\
                    }\
                    if (*format == 'x') {\
                        PARSE_INT(16);\
                    }\
                    if (*format == 'd' || *format == 'i' || *format == 'u') {\
                        PARSE_INT(10);\
                    }\
                    if (buff_count + pbuff_count > BUFFER_SIZE){\
                        OUTPUT;\
                    }\
                    for (int i = 0; i < pbuff_count; i++) {\
                        buffer[buff_count + i] = parse_buffer[i];\
                    }\
                    count += pbuff_count;\
                    buff_count += pbuff_count;\
                    special = 0;\
                    special_type = 0;\
                }\
                if (*format == 'c') {\
                    char arg;\
                    ARG_GET(int);\
                    buffer[buff_count] = arg;\
                    buff_count++;\
                    count++;\
                    special = 0;\
                    special_type = 0;\
                }\
                if (*format == 's') {\
                    char* arg;\
                    ARG_GET(char*);\
                    while (*arg != 0) {\
                        buffer[buff_count] = *arg;\
                        buff_count++;\
                        count++;\
                        arg++;\
                        if (buff_count == BUFFER_SIZE) {\
                            OUTPUT;\
                        }\
                    }\
                    special = 0;\
                    special_type = 0;\
                }\
                if (*format == 'h') {\
                    special_type -= 1;\
                }\
                if (*format == 'l') {\
                    special_type += 1;\
                }\
                format++;\
            } else {\
                if (*format == '%') {\
                    special = 1;\
                }\
                if (*format == '\\') {\
                    escape = 1;\
                }\
                if (*format != '%' && *format != '\\') {\
                    buffer[buff_count] = *format;\
                    buff_count++;\
                    count++;\
                }\
                format++;\
            }\
        }\
        if (buff_count == BUFFER_SIZE){\
            OUTPUT;\
        }\
    }\
    OUTPUT;\
    ARGS_CLOSE;\
    return count;
    
#define ELIPSIS_ARGS_INIT\
    va_list args;\
    va_start(args, format);\
    
#define ELIPSIS_ARG_GET(ARG_TYPE)\
    VA_LIST_ARG_GET(ARG_TYPE);

#define ELIPSIS_ARGS_CLOSE\
    VA_LIST_ARGS_CLOSE;

#define VA_LIST_ARGS_INIT\
    
#define VA_LIST_ARG_GET(ARG_TYPE)\
    arg = va_arg(args, ARG_TYPE);

#define VA_LIST_ARGS_CLOSE\
    va_end(args);
    
#define COM_PORT_OUTPUT_INIT
    
#define COM_PORT_OUTPUT\
        buffer[buff_count] = 0;\
        outToCOMport(buffer);\
        buff_count = 0;
        
#define STRING_OUTPUT_INIT\
    uint64_t pos = 0;\
    while (*(s + pos) != 0) pos++;\

#define STRING_OUTPUT\
    for (int i = 0; i < buff_count; i++) {\
        if (pos >= n-1) break;\
        *(s + pos) = buffer[i];\
        pos++;\
    } \
    *(s + pos) = 0;\
    buff_count = 0;
    
    int printf(const char* format, ...) {
        PRINT(ELIPSIS_ARGS_INIT, ELIPSIS_ARG_GET, ELIPSIS_ARGS_CLOSE, COM_PORT_OUTPUT_INIT, COM_PORT_OUTPUT);
    }
    int vprintf(const char* format, va_list args) {
        PRINT(VA_LIST_ARGS_INIT, VA_LIST_ARG_GET, VA_LIST_ARGS_CLOSE, COM_PORT_OUTPUT_INIT, COM_PORT_OUTPUT);
    }
    int snprintf(char* s, uint64_t n, const char* format, ...) {
        PRINT(ELIPSIS_ARGS_INIT, ELIPSIS_ARG_GET, ELIPSIS_ARGS_CLOSE, STRING_OUTPUT_INIT, STRING_OUTPUT);
    }
    int vsnprintf(char* s, uint64_t n, const char* format, va_list args) {
        PRINT(VA_LIST_ARGS_INIT, VA_LIST_ARG_GET, VA_LIST_ARGS_CLOSE, STRING_OUTPUT_INIT, STRING_OUTPUT);
    }
