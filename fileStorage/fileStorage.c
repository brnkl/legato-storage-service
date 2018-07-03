#include "legato.h"
#include "util.h"
#include <sys/mman.h>
#include "fileStorage.h"

// TODO write a helper to make
// sure all the directories we need are ready
// to go
static const char* SD_DEV_PATH = "/dev/mmcblk0p1";
static const char* SD_MNT_PATH = "/mnt/userrw/sd/";
static const char* FLASH_STORAGE_PATH = "/mnt/flash/storage/";
static const char* SD_STORAGE_PATH = "/mnt/userrw/sd/storage/";
static const char* SERIES_PATH = "%sseries";
// TODO fill in the third %s as a type
// such that we know what type it is when we grab it
static const char* SERIES_FILENAME = "%s/%s.%s.series";

typedef struct { bool hasSdCard; } StorageState;
typedef struct {
  int (*scan)(char* recordLine, void* value, uint64_t* timestamp);
  void* (*deref)(void* value, int i);
  StorageType type;
} GetCallbacks;

StorageState state = {.hasSdCard = false};

static bool mountSdCard() {
  if (!util_alreadyMounted(SD_DEV_PATH)) {
    int mkDirRes = 0;
    if (!util_fileExists(SD_MNT_PATH)) {
      mkDirRes = mkdir(SD_MNT_PATH, 777);
    }
    int mountRes = mount(SD_DEV_PATH, SD_MNT_PATH, "minix", 0, "");
    return !(mkDirRes || mountRes);
  }
  return true;
}

void getSeriesDir(char* res) {
  const char* storageLocation =
      state.hasSdCard ? SD_STORAGE_PATH : FLASH_STORAGE_PATH;
  // build the dir e.g /mnt/flash/userrw/sd/storage/series
  snprintf(res, MAX_STR_SIZE, SERIES_PATH, storageLocation);
}

void getSeriesPath(const char* key, char* res, StorageType type) {
  char dir[MAX_STR_SIZE];
  char* typestr;
  switch(type) {
    case INT:
      typestr = "int";
    break;
    case DOUBLE:
      typestr = "double";
    break;
    case STRING:
      typestr = "string";
    break;
  }
  getSeriesDir(dir);
  // build the path e.g /mnt/flash/userrw/sd/storage/series/pressure1.double.series
  snprintf(res, MAX_STR_SIZE, SERIES_FILENAME, dir, key, typestr);
}

int32_t stringToStorageType(char* typestr) {
  if(!strcmp(typestr, "int")) return INT;
  if(!strcmp(typestr, "double")) return DOUBLE;
  if(!strcmp(typestr, "string")) return STRING;
  return INT;
}

// TODO add a space check with some type
// of pruning
le_result_t storage_record(const char* key,
                           void* value,
                           uint64_t timestamp,
                           StorageType type,
                           int (*formatCallback)(void* input,
                                                 char* output,
                                                 size_t n)) {
  char seriesPath[MAX_STR_SIZE];
  getSeriesPath(key, seriesPath, type);

  char formattedVal[MAX_STR_SIZE];
  formatCallback(value, formattedVal, MAX_STR_SIZE);

  char newRecord[MAX_STR_SIZE];
  size_t len = snprintf(newRecord, MAX_STR_SIZE, "%" PRId64 ",%s\n", timestamp,
                        formattedVal);

  ioutil_appendToFile(seriesPath, newRecord, sizeof(char), len);
  return LE_OK;
}

int formatInt(void* input, char* output, size_t n) {
  int* v = input;
  return snprintf(output, n, "%d", *v);
}

int formatDouble(void* input, char* output, size_t n) {
  double* v = input;
  return snprintf(output, n, "%f", *v);
}

int formatString(void* input, char* output, size_t n) {
  char* v = input;
  return snprintf(output, n, "%s", v);
}

int parseIntRecord(char* recordLine, void* value, uint64_t* timestamp) {
  int* v = value;
  return sscanf(recordLine, "%" PRId64 ",%d", timestamp, v);
}

int parseDoubleRecord(char* recordLine, void* value, uint64_t* timestamp) {
  double* v = value;
  return sscanf(recordLine, "%" PRId64 ",%lf", timestamp, v);
}

int parseStringRecord(char* recordLine, void* value, uint64_t* timestamp) {
  char* v = value;
  return sscanf(recordLine, "%" PRId64 ",%s", timestamp, v);
}

le_result_t storage_recordInt(const char* key,
                              int32_t val,
                              uint64_t timestamp) {
  return storage_record(key, (void*)&val, timestamp, INT, formatInt);
}

le_result_t storage_recordDouble(const char* key,
                                 double val,
                                 uint64_t timestamp) {
  return storage_record(key, (void*)&val, timestamp, DOUBLE, formatDouble);
}

le_result_t storage_recordString(const char* key,
                                 const char* val,
                                 uint64_t timestamp) {
  return storage_record(key, (void*)val, timestamp, STRING, formatString);
}

void* derefInt(void* value, int i) {
  int* v = value;
  return (void*)&v[i];
}

void* derefDouble(void* value, int i) {
  double* v = value;
  return (void*)&v[i];
}

void* derefString(void* value, int i) {
  char* v = value;
  return (void*)&v[i];
}

// the routines below will read the latest
// n records where n = size
//
// if size is larger than the number of
// stored records, all records are read
// and the size is adjusted
le_result_t storage_get(const char* key,
                        void* val,
                        uint64_t* timestamp,
                        size_t* size,
                        GetCallbacks* callbacks) {
  char seriesPath[MAX_STR_SIZE];
  getSeriesPath(key, seriesPath, callbacks->type);
  int fd = open(seriesPath, O_RDWR);

  if (fd < 0) {
    goto ioError;
  }
  struct stat buf;
  int statRes = fstat(fd, &buf);
  if (statRes != 0) {
    goto ioError;
  }
  if (buf.st_size <= 0) {
    goto empty;
  }

  char recordLine[MAX_STR_SIZE];
  char* file =
      mmap(NULL, buf.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  int lastNewline = -1;
  int len, i = 0;
  // TODO this indexing scheme seems
  // to work but I'm not sure why (i.e the worst
  // thing to say as a programmer)
  //
  // I think mmap throws some spaces in this bitch
  // for some reason?
  while (buf.st_size > 0 && i < *size) {
    if (file[buf.st_size] == '\n') {
      if (lastNewline > 0) {
        len = lastNewline - (buf.st_size - 2);
        char* src = file + (buf.st_size + 1);
        memcpy(recordLine, (void*)src, len);
        recordLine[len] = '\0';
        void* valDest = callbacks->deref(val, i);
        callbacks->scan(recordLine, valDest, &timestamp[i]);
        i++;
      }
      ftruncate(fd, buf.st_size);
      lastNewline = buf.st_size;
    }
    buf.st_size--;
  }
  ftruncate(fd, buf.st_size);
  close(fd);
  if (*size > i) {
    *size = i;
  }
  return LE_OK;
empty:
  *size = 0;
  close(fd);
  return LE_UNAVAILABLE;
ioError:
  *size = 0;
  close(fd);
  return LE_IO_ERROR;
}

le_result_t storage_getInt(const char* key,
                           int* val,
                           uint64_t* timestamp,
                           size_t* size) {
  GetCallbacks c = {.scan = parseIntRecord, .deref = derefInt, .type = INT};
  return storage_get(key, (void*)val, timestamp, size, &c);
}

le_result_t storage_getDouble(const char* key,
                              double* val,
                              uint64_t* timestamp,
                              size_t* size) {
  GetCallbacks c = {.scan = parseDoubleRecord, .deref = derefDouble, .type = DOUBLE};
  return storage_get(key, (void*)val, timestamp, size, &c);
}

// TODO figure out why this seg faults
// (it might be related to the pointer I
// passed when testing)
le_result_t storage_getString(const char* key,
                              char* val[],
                              uint64_t* timestamp,
                              size_t* size) {
  GetCallbacks c = {.scan = parseStringRecord, .deref = derefString, .type = STRING};
  return storage_get(key, (void*)val, timestamp, size, &c);
}

void parseDirList(char* dirList, char* vals, int32_t* type, size_t* tSize) {
  char* entrySep = ",";
  char* keyTypeSep = ".";
  char* entryEnd;
  char* keyTypeTokenEnd;
  char* entryToken = strtok_r(dirList, entrySep, &entryEnd);
  char* keyTypeToken;
  int i = 0, nScanned = 0;
  char entryCopy[600], keystr[500], typestr[100];
  while(entryToken != NULL) {
    strncpy(entryCopy, entryToken, 600);
    keyTypeToken = strtok_r(entryCopy, keyTypeSep, &keyTypeTokenEnd);
    // TODO die with magic numbers
    while(keyTypeToken != NULL && i < 2) {
      char* storage = i++ == 0 ? keystr : typestr;
      sscanf(keyTypeToken, "%s", storage);
      keyTypeToken = strtok_r(NULL, keyTypeSep, &keyTypeTokenEnd);
    }
    if (nScanned > 0) {
      strcat(vals, ",");
    }
    type[nScanned++] = stringToStorageType(typestr);
    strcat(vals, keystr);
    i = 0;
    entryToken = strtok_r(NULL, entrySep, &entryEnd);
  }
  *tSize = nScanned;
}

// Note that we need to perform a scan here
// to avoid the risk of "zombie files"
le_result_t storage_getAllKeys(char* vals, int32_t* type, size_t* tSize) {
  char dirList[MAX_DIR_LIST_STR];
  char dir[MAX_STR_SIZE];
  getSeriesDir(dir);
  util_listDir(dir, dirList, MAX_DIR_LIST_STR);
  parseDirList(dirList, vals, type, tSize);
  return LE_OK;
}

COMPONENT_INIT {
  state.hasSdCard = mountSdCard();
}
