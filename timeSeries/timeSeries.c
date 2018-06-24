#include "legato.h"
#include "util.h"
#include "interfaces.h"
#include "fileStorage.h"

le_result_t brnkl_series_recordInt(const char* key,
                                   int32_t val,
                                   uint64_t timestamp) {
  return storage_recordInt(key, val, timestamp);
}

le_result_t brnkl_series_recordDouble(const char* key,
                                      double val,
                                      uint64_t timestamp) {
  return storage_recordDouble(key, val, timestamp);
}

le_result_t brnkl_series_recordString(const char* key,
                                      const char* val,
                                      uint64_t timestamp) {
  return storage_recordString(key, val, timestamp);
}

le_result_t brnkl_series_getInt(const char* key,
                                int32_t* val,
                                size_t* size,
                                uint64_t* timestamp,
                                size_t* tSize) {
  return LE_OK;
}

le_result_t brnkl_series_getDouble(const char* key,
                                   double* val,
                                   size_t* size,
                                   uint64_t* timestamp,
                                   size_t* tSize) {
  return LE_OK;
}

le_result_t brnkl_series_getString(const char* key,
                                   char* val[],
                                   size_t* size,
                                   uint64_t* timestamp,
                                   size_t* tSize) {
  return LE_OK;
}

le_result_t brnkl_series_clear(const char* key,
                               int32_t clearAll,
                               uint32_t* size) {
  return LE_OK;
}

COMPONENT_INIT {
  LE_INFO("YO");
}
