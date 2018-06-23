#ifndef FILE_STORAGE_H
#define FILE_STORAGE_H

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

LE_SHARED le_result_t storage_getString(const char* key,
                                        char* val[],
                                        uint64_t* timestamp,
                                        size_t* size);

#endif
