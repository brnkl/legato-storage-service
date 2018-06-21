#ifndef FILE_STORAGE_H
#define FILE_STORAGE_H

#include "legato.h"
#include "interfaces.h"

LE_SHARED le_result_t storage_recordInt(const char* key,
                                        int32_t val,
                                        uint64_t timestamp);

LE_SHARED le_result_t storage_recordDouble(const char* key,
                                           double val,
                                           uint64_t timestamp);

LE_SHARED le_result_t storage_recordString(const char* key,
                                           char* val,
                                           uint64_t timestamp);

LE_SHARED le_result_t storage_getInt(char* key,
                                     int* val,
                                     size_t* size,
                                     uint64_t* timestamp,
                                     size_t* tSize);

LE_SHARED le_result_t storage_getDouble(char* key,
                                        double* val,
                                        size_t* size,
                                        uint64_t* timestamp,
                                        size_t* tSize);

LE_SHARED le_result_t storage_getString(char* key,
                                        char** val,
                                        size_t* size,
                                        uint64_t* timestamp,
                                        size_t* tSize);

#endif
