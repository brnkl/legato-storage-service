#ifndef FILE_STORAGE_H
#define FILE_STORAGE_H

#define MAX_STR_SIZE 2048
#define MAX_KEY_LEN 500
#define MAX_DIR_LIST_STR (MAX_STR_SIZE + 1) * 100

#include "legato.h"
#include "interfaces.h"

typedef enum { INT = 0, DOUBLE, STRING } StorageType;

LE_SHARED le_result_t storage_recordInt(const char* key,
                                        int32_t val,
                                        uint64_t timestamp);

LE_SHARED le_result_t storage_recordDouble(const char* key,
                                           double val,
                                           uint64_t timestamp);

LE_SHARED le_result_t storage_recordString(const char* key,
                                           const char* val,
                                           uint64_t timestamp);

LE_SHARED le_result_t storage_getInt(const char* key,
                                     int* val,
                                     uint64_t* timestamp,
                                     size_t* size);

LE_SHARED le_result_t storage_getDouble(const char* key,
                                        double* val,
                                        uint64_t* timestamp,
                                        size_t* size);

LE_SHARED le_result_t storage_getAllKeys(char* vals, int32_t* types, size_t* tSize);

#endif
