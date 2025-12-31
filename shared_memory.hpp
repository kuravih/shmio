#ifndef __SHMIO_SHARED_MEMORY_HPP__
#define __SHMIO_SHARED_MEMORY_HPP__

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstring>
#include <cstdint>
#include <string>
#include <time.h>
#include <vector>
#include <pthread.h>
#include <span>

#define PATH_SIZE 256                // size of path
#define KEYWORD_MAX_STRING 16        // Max keyword name or string length
#define KEYWORD_STR_VAL_MAX_STRING 8 // Max size of a string keyword value
#define KEYWORD_MAX_COMMENT 80       // Max comment size

#define _DATATYPE_UNINITIALIZED 0
#define _DATATYPE_UINT8 1                 // uint8_t = char
#define SIZEOF_DATATYPE_UINT8 1           // sizeof(uint8_t)
#define _DATATYPE_INT8 2                  // int8_t
#define SIZEOF_DATATYPE_INT8 1            // sizeof(int8_t)
#define _DATATYPE_UINT16 3                // uint16_t usually = unsigned short int
#define SIZEOF_DATATYPE_UINT16 2          // sizeof(uint16_t)
#define _DATATYPE_INT16 4                 // int16_t usually = short int
#define SIZEOF_DATATYPE_INT16 2           // sizeof(int16_t)
#define _DATATYPE_UINT32 5                // uint32_t usually = unsigned int
#define SIZEOF_DATATYPE_UINT32 4          // sizeof(uint32_t)
#define _DATATYPE_INT32 6                 // int32_t usually = int
#define SIZEOF_DATATYPE_INT32 4           // sizeof(int32_t)
#define _DATATYPE_UINT64 7                // uint64_t usually = unsigned long
#define SIZEOF_DATATYPE_UINT64 8          // sizeof(uint64_t)
#define _DATATYPE_INT64 8                 // int64_t usually = long
#define SIZEOF_DATATYPE_INT64 8           // sizeof(int64_t)
#define _DATATYPE_HALF 13                 // IEE 754 half-precision 16-bit (uses uint16_t for storage)
#define SIZEOF_DATATYPE_HALF 2            // sizeof(half)
#define _DATATYPE_FLOAT 9                 // IEEE 754 single-precision binary floating-point format: binary32
#define SIZEOF_DATATYPE_FLOAT 4           // sizeof(float)
#define _DATATYPE_DOUBLE 10               // IEEE 754 double-precision binary floating-point format: binary64
#define SIZEOF_DATATYPE_DOUBLE 8          // sizeof(double)
#define _DATATYPE_COMPLEX_FLOAT 11        // complex_float
#define SIZEOF_DATATYPE_COMPLEX_FLOAT 8   // sizeof(complex_float)
#define _DATATYPE_COMPLEX_DOUBLE 12       // complex double
#define SIZEOF_DATATYPE_COMPLEX_DOUBLE 16 // sizeof(complex_double)

namespace shmio
{
    typedef struct
    {
        float re, im;
    } complex_float;

    typedef struct
    {
        double re, im;
    } complex_double;

    enum class KeywordType
    {
        LONG,
        DOUBLE,
        STRING
    };

    struct Keyword
    {
        char name[KEYWORD_MAX_STRING];
        KeywordType type;
        union
        {
            int64_t numl;
            double numf;
            char valstr[KEYWORD_STR_VAL_MAX_STRING];
        } value;
        char comment[KEYWORD_MAX_COMMENT];

        Keyword(const char *_name, const KeywordType _type, int64_t _value, const char *_comment) : type(_type)
        {
            std::strncpy(name, _name, KEYWORD_MAX_STRING - 1);
            value.numl = _value;
            std::strncpy(comment, _comment, KEYWORD_MAX_COMMENT - 1);
        }

        Keyword(const char *_name, const KeywordType _type, uint32_t _value, const char *_comment) : Keyword(_name, _type, (int64_t)_value, _comment) {}
        Keyword(const char *_name, const KeywordType _type, int _value, const char *_comment) : Keyword(_name, _type, (int64_t)_value, _comment) {}

        Keyword(const char *_name, const KeywordType _type, double _value, const char *_comment) : type(_type)
        {
            std::strncpy(name, _name, KEYWORD_MAX_STRING - 1);
            value.numf = _value;
            std::strncpy(comment, _comment, KEYWORD_MAX_COMMENT - 1);
        }

        Keyword(const char *_name, const KeywordType _type, float _value, const char *_comment) : Keyword(_name, _type, (double)_value, _comment) {}

        Keyword(const char *_name, const KeywordType _type, const char *_value, const char *_comment) : type(_type)
        {
            std::strncpy(name, _name, KEYWORD_MAX_STRING - 1);
            std::strncpy(value.valstr, _value, KEYWORD_STR_VAL_MAX_STRING - 1);
            std::strncpy(comment, _comment, KEYWORD_MAX_COMMENT - 1);
        }

        bool operator==(const Keyword &other) const noexcept
        {
            if (type != other.type)
                return false;

            if (std::strncmp(name, other.name, KEYWORD_MAX_STRING - 1) != 0)
                return false;
            if (std::strncmp(comment, other.comment, KEYWORD_MAX_COMMENT - 1) != 0)
                return false;

            switch (type)
            {
            case KeywordType::LONG:
                return value.numl == other.value.numl;
            case KeywordType::DOUBLE:
                return value.numf == other.value.numf;
            case KeywordType::STRING:
                return std::strncmp(value.valstr, other.value.valstr, KEYWORD_STR_VAL_MAX_STRING - 1) == 0;
            }
            return false; // fallback
        }

        bool operator!=(const Keyword &other) const noexcept
        {
            return !(*this == other);
        }
    };

    enum class DataType : uint8_t
    {
        UINT8 = _DATATYPE_UINT8,
        INT8 = _DATATYPE_INT8,
        UINT16 = _DATATYPE_UINT16,
        INT16 = _DATATYPE_INT16,
        UINT32 = _DATATYPE_UINT32,
        INT32 = _DATATYPE_INT32,
        UINT64 = _DATATYPE_UINT64,
        INT64 = _DATATYPE_INT64,
        HALF = _DATATYPE_HALF,
        FLOAT = _DATATYPE_FLOAT,
        DOUBLE = _DATATYPE_DOUBLE,
        COMPLEX_FLOAT = _DATATYPE_COMPLEX_FLOAT,
        COMPLEX_DOUBLE = _DATATYPE_COMPLEX_DOUBLE,
    };

    constexpr size_t DataTypeSize(DataType type)
    {
        switch (type)
        {
        case DataType::UINT8:
            return SIZEOF_DATATYPE_UINT8;
        case DataType::INT8:
            return SIZEOF_DATATYPE_INT8;
        case DataType::UINT16:
            return SIZEOF_DATATYPE_UINT16;
        case DataType::INT16:
            return SIZEOF_DATATYPE_INT16;
        case DataType::UINT32:
            return SIZEOF_DATATYPE_UINT32;
        case DataType::INT32:
            return SIZEOF_DATATYPE_INT32;
        case DataType::UINT64:
            return SIZEOF_DATATYPE_UINT64;
        case DataType::INT64:
            return SIZEOF_DATATYPE_INT64;
        case DataType::HALF:
            return SIZEOF_DATATYPE_HALF;
        case DataType::FLOAT:
            return SIZEOF_DATATYPE_FLOAT;
        case DataType::DOUBLE:
            return SIZEOF_DATATYPE_DOUBLE;
        case DataType::COMPLEX_FLOAT:
            return SIZEOF_DATATYPE_COMPLEX_FLOAT;
        case DataType::COMPLEX_DOUBLE:
            return SIZEOF_DATATYPE_COMPLEX_DOUBLE;
        }
        return _DATATYPE_UNINITIALIZED;
    }

    struct SharedStorage
    {
        pthread_mutex_t mutex;
        pthread_cond_t request_cond;
        pthread_cond_t ready_cond;
        bool request_flag;
        bool ready_flag;
        struct timespec creationtime;   // creation time
        struct timespec lastaccesstime; // last access time
        size_t nkw;                     // Number of keywords
        size_t npx;                     // Size of data array in bytes
        DataType dtype;                 // Data type
    };

    struct SharedMemory
    {
        int fd = -1;
        size_t size = 0;
        std::string name{};
        void *base = nullptr;
        void *data = nullptr;

        SharedMemory() = default;

        SharedMemory(const SharedMemory &) = delete;
        SharedMemory &operator=(const SharedMemory &) = delete;

        SharedMemory(SharedMemory &&other) noexcept : fd(other.fd), size(other.size), name(std::move(other.name)), base(other.base), data(other.data)
        {
            other.fd = -1;
            other.size = 0;
            other.base = nullptr;
            other.data = nullptr;
        }

        SharedMemory &operator=(SharedMemory &&other) noexcept
        {
            if (this != &other)
            {
                fd = other.fd;
                size = other.size;
                name = std::move(other.name);
                base = other.base;
                data = other.data;

                other.fd = -1;
                other.size = 0;
                other.base = nullptr;
                other.data = nullptr;
            }
            return *this;
        }
    };

    inline size_t shared_memory_size(const size_t _nkw, const size_t _npx, const DataType _dtype)
    {
        // shared_memory_size
        //   Calculate the size of the shared memory.
        // Parameters:
        //   const size_t _nkw - number of keywords
        //   const size_t _npx - numper of pixles
        //   const DataType _dtype - data type
        // Return:
        //   size_t size of the shared memory.

        size_t header_size = sizeof(SharedStorage);
        size_t keywords_size = _nkw * sizeof(Keyword);
        size_t pixels_size = _npx * DataTypeSize(_dtype);
        return header_size + keywords_size + pixels_size;
    };

    inline SharedStorage *get_storage_ptr(SharedMemory &_memory)
    {
        // get_storage
        //   get a pointer to the storage.
        // Parameters:
        //   SharedMemory &_memory - shared memory
        // Return:
        //   SharedStorage pointer to the storage.

        return reinterpret_cast<SharedStorage *>(_memory.base);
    }

    inline Keyword *get_keywords_ptr(SharedMemory &_memory)
    {
        // get_keywords
        //   get a pointer to the keywords.
        // Parameters:
        //   SharedMemory &_memory - shared memory
        // Return:
        //   Keyword pointer to the keywords.

        return reinterpret_cast<Keyword *>(reinterpret_cast<char *>(_memory.base) + sizeof(SharedStorage));
    }

    inline std::span<Keyword> get_keywords(SharedMemory &_memory)
    {
        SharedStorage *storage = get_storage_ptr(_memory);
        return std::span<Keyword>(get_keywords_ptr(_memory), storage->nkw);
    }

    inline char *get_pixels_ptr(SharedMemory &_memory)
    {
        // get_data
        //   get a pointer to the pixel data.
        // Parameters:
        //   SharedMemory &_memory - shared memory
        // Return:
        //   char * pointer to the pixel data.

        SharedStorage *storage = get_storage_ptr(_memory);
        Keyword *keywords = get_keywords_ptr(_memory);
        return reinterpret_cast<char *>(keywords) + storage->nkw * sizeof(Keyword);
    }

    template <typename T>
    T *get_pixels_ptr_as(SharedMemory &_memory) // Templated pixel data access with type safety
    {
        SharedStorage *storage = get_storage_ptr(_memory);

        // Basic type checking (could be expanded)
        if constexpr (std::is_same_v<T, uint8_t>)
        {
            if (storage->dtype != DataType::UINT8)
                return nullptr;
        }
        else if constexpr (std::is_same_v<T, int8_t>)
        {
            if (storage->dtype != DataType::INT8)
                return nullptr;
        }
        else if constexpr (std::is_same_v<T, uint16_t>)
        {
            if (storage->dtype != DataType::UINT16)
                return nullptr;
        }
        else if constexpr (std::is_same_v<T, int16_t>)
        {
            if (storage->dtype != DataType::INT16)
                return nullptr;
        }
        else if constexpr (std::is_same_v<T, uint32_t>)
        {
            if (storage->dtype != DataType::UINT32)
                return nullptr;
        }
        else if constexpr (std::is_same_v<T, int32_t>)
        {
            if (storage->dtype != DataType::INT32)
                return nullptr;
        }
        else if constexpr (std::is_same_v<T, uint64_t>)
        {
            if (storage->dtype != DataType::UINT64)
                return nullptr;
        }
        else if constexpr (std::is_same_v<T, int64_t>)
        {
            if (storage->dtype != DataType::INT64)
                return nullptr;
        }
        else if constexpr (std::is_same_v<T, float>)
        {
            if (storage->dtype != DataType::FLOAT)
                return nullptr;
        }
        else if constexpr (std::is_same_v<T, double>)
        {
            if (storage->dtype != DataType::DOUBLE)
                return nullptr;
        }
        else if constexpr (std::is_same_v<T, complex_float>)
        {
            if (storage->dtype != DataType::COMPLEX_FLOAT)
                return nullptr;
        }
        else if constexpr (std::is_same_v<T, complex_double>)
        {
            if (storage->dtype != DataType::COMPLEX_DOUBLE)
                return nullptr;
        }

        return reinterpret_cast<T *>(_memory.data);
    }

    template <typename T>
    inline std::span<T> get_pixels_as(SharedMemory &_memory)
    {
        SharedStorage *storage = get_storage_ptr(_memory);
        T *pixels_ptr = get_pixels_ptr_as<T>(_memory);
        if (!pixels_ptr)
            return {};
        return std::span<T>(pixels_ptr, storage->npx);
    }

    inline int close_shared_memory(SharedMemory &_memory)
    {
        // close_shared_memory
        //   Close an open shared memory.
        // Parameters:
        //   SharedMemory &_memory - memory
        // Return:
        //   0 if stream closed correctly.

        munmap(_memory.base, _memory.size);
        close(_memory.fd);
        // std::string path = std::string("/") + _memory.name + std::string(".shm");
        // shm_unlink(path.c_str()); // TODO: remove the file

        return 0;
    }

    inline bool shared_memory_exists(const char *_name)
    {
        // shared_memory_exists
        //   Check if shared memory exists.
        // Parameters:
        //   const char* _name - name of the shared memory
        // Return:
        //   true if shared memory exists false otherwise
        std::string path = std::string("/") + _name + std::string(".shm");
        int fd = shm_open(path.c_str(), O_RDONLY, 0);
        if (fd == -1)
        {
            return false; // does not exist
        }
        close(fd);
        return true; // exists
    }

    inline int setup_open_shared_memory(SharedMemory &_memory, const size_t _npx, const DataType _dtype, const std::vector<Keyword> &_keywords)
    {
        // setup_open_shared_memory
        //   setup a shared memory. if a shared memory by that name exists an attempt will be made to use it.
        // Parameters:
        //   SharedMemory &_memory - memory
        //   const char *_name - filename
        //   const size_t _npx - number of pixels
        //   const DataType _dtype - type of pixels
        //   const std::vector<Keyword> &_keywords - keywords
        // Return:
        //   0 if stream is created correctly. leaves the stream open.

        if (_memory.name.empty())
        {
            return -1;
        }

        std::string path = std::string("/") + _memory.name + std::string(".shm");
        _memory.fd = shm_open(path.c_str(), O_RDWR, 0); // Open existing shared memory object (read/write)
        if (_memory.fd == -1)
        {
            return -1;
        }

        struct stat file_stat;
        if (fstat(_memory.fd, &file_stat) == -1) // Get the size of the shared memory object
        {
            close(_memory.fd);
            return -1;
        }

        size_t data_size = shared_memory_size(_keywords.size(), _npx, _dtype);
        size_t file_size = static_cast<size_t>(file_stat.st_size);
        if (file_size != data_size)
        {
            return -1;
        }

        _memory.size = file_size;
        _memory.base = mmap(nullptr, _memory.size, PROT_READ | PROT_WRITE, MAP_SHARED, _memory.fd, 0);
        if (_memory.base == MAP_FAILED)
        {
            close(_memory.fd);
            return -1;
        }

        SharedStorage *storage = get_storage_ptr(_memory);
        clock_gettime(CLOCK_REALTIME, &storage->lastaccesstime);

        std::span<Keyword> keywords = get_keywords(_memory);
        for (size_t ikw = 0; ikw < storage->nkw; ++ikw)
        {
            if (std::strncmp(keywords[ikw].name, _keywords[ikw].name, KEYWORD_MAX_STRING) != 0)
            {
                // kato::log::cout << KATO_RED << "setup_open_shared_memory() - keyword " << ikw << "name not matching." << KATO_RESET << std::endl;
                return -1;
            }
            else
            {
                if (std::strncmp(keywords[ikw].comment, _keywords[ikw].comment, KEYWORD_MAX_COMMENT) != 0)
                {
                    // kato::log::cout << KATO_RED << "setup_open_shared_memory() - keyword " << ikw << "comment not matching." << KATO_RESET << std::endl;
                    return -1;
                }
                else
                {
                    if (keywords[ikw].type != _keywords[ikw].type)
                    {
                        // kato::log::cout << KATO_RED << "setup_open_shared_memory() - keyword " << ikw << "type not matching." << KATO_RESET << std::endl;
                        return -1;
                    }
                    else
                    {
                        switch (keywords[ikw].type)
                        {
                        case KeywordType::DOUBLE:
                            keywords[ikw].value.numf = _keywords[ikw].value.numf;
                            break;
                        case KeywordType::LONG:
                            keywords[ikw].value.numl = _keywords[ikw].value.numl;
                            break;
                        case KeywordType::STRING:
                            std::strncpy(keywords[ikw].value.valstr, _keywords[ikw].value.valstr, KEYWORD_MAX_STRING - 1);
                            break;
                        default:
                            break;
                        }
                    }
                }
            }
        }

        _memory.data = get_pixels_ptr(_memory);

        return 0;
    }

    inline int setup_open_shared_memory(SharedMemory &_memory)
    {
        // setup_open_shared_memory
        //   setup a shared memory.
        // Parameters:
        //   SharedMemory &_memory - memory
        // Return:
        //   0 if shared memory is setup correctly otherwise the shared memory will be closed gracefully.

        if (_memory.name.empty())
        {
            return -1;
        }

        std::string path = std::string("/") + _memory.name + std::string(".shm");
        _memory.fd = shm_open(path.c_str(), O_RDWR, 0); // Open existing shared memory object (read/write)
        if (_memory.fd == -1)
        {
            return -1;
        }

        struct stat file_stat;
        if (fstat(_memory.fd, &file_stat) == -1) // Get the size of the shared memory object
        {
            close(_memory.fd);
            return -1;
        }

        size_t file_size = static_cast<size_t>(file_stat.st_size);
        _memory.size = file_size;
        _memory.base = mmap(nullptr, _memory.size, PROT_READ | PROT_WRITE, MAP_SHARED, _memory.fd, 0);
        if (_memory.base == MAP_FAILED)
        {
            close(_memory.fd);
            return -1;
        }

        SharedStorage *storage = get_storage_ptr(_memory);

        clock_gettime(CLOCK_REALTIME, &storage->lastaccesstime);

        _memory.data = get_pixels_ptr(_memory);

        return 0;
    }

    inline int create_open_shared_memory(SharedMemory &_memory, const size_t _npx, const DataType _dtype, const std::vector<Keyword> &_keywords)
    {
        // create_open_shared_memory
        //   create a shared memory. if a shared memory by that name exists an attempt will be made to use it.
        // Parameters:
        //   SharedMemory &_memory - memory
        //   const char *_name - filename
        //   const size_t _npx - number of pixels
        //   const DataType _dtype - type of pixels
        //   const std::vector<Keyword> &_keywords - keywords
        // Return:
        //   0 if stream is created correctly. leaves the stream open.

        if (_memory.name.empty())
        {
            return -1;
        }

        std::string path = std::string("/") + _memory.name + std::string(".shm");
        _memory.fd = shm_open(path.c_str(), O_CREAT | O_RDWR, 0600);
        if (_memory.fd < 0)
        {
            return -1;
        }

        _memory.size = shared_memory_size(_keywords.size(), _npx, _dtype);
        if (ftruncate(_memory.fd, _memory.size) == -1)
        {
            close(_memory.fd);
            return -1;
        }

        _memory.base = mmap(nullptr, _memory.size, PROT_READ | PROT_WRITE, MAP_SHARED, _memory.fd, 0);
        if (_memory.base == MAP_FAILED)
        {
            close(_memory.fd);
            return -1;
        }

        SharedStorage *storage = get_storage_ptr(_memory);

        // Initialize mutex (only once, by creator)
        pthread_mutexattr_t mattr;
        pthread_mutexattr_init(&mattr);
        pthread_mutexattr_setpshared(&mattr, PTHREAD_PROCESS_SHARED);
        pthread_mutex_init(&storage->mutex, &mattr);
        pthread_mutexattr_destroy(&mattr);

        pthread_condattr_t cattr;
        pthread_condattr_init(&cattr);
        pthread_condattr_setpshared(&cattr, PTHREAD_PROCESS_SHARED);
        pthread_cond_init(&storage->request_cond, &cattr);
        pthread_cond_init(&storage->ready_cond, &cattr);

        clock_gettime(CLOCK_REALTIME, &storage->creationtime);
        storage->lastaccesstime = storage->creationtime;
        storage->nkw = _keywords.size();
        storage->npx = _npx;
        storage->dtype = _dtype;
        storage->request_flag = false;
        storage->ready_flag = false;

        char *base = reinterpret_cast<char *>(get_keywords_ptr(_memory));
        std::memcpy(base, _keywords.data(), _keywords.size() * sizeof(Keyword));

        _memory.data = get_pixels_ptr(_memory);

        return 0;
    };

    inline int create_shared_memory(SharedMemory &_memory, const char *_name, const size_t _npx, const DataType _dtype, const std::vector<Keyword> &_keywords)
    {
        // create_shared_memory
        //   Create an opened shared memory. if a shared memory by that name exists an attempt will be made to use it.
        // Parameters:
        //   SharedMemory &_memory - memory
        //   const char *_name - filename
        //   const size_t _npx - number of pixels
        //   const DataType _dtype - type of pixels
        //   const std::vector<Keyword> &_keywords - keywords
        // Return:
        //   0 if stream is created correctly. leaves the stream open.

        _memory.name = _name;
        if (shared_memory_exists(_name))
        {
            return setup_open_shared_memory(_memory, _npx, _dtype, _keywords);
        }
        return create_open_shared_memory(_memory, _npx, _dtype, _keywords);
    }

    inline int open_shared_memory(SharedMemory &_memory, const char *_name)
    {
        // open_shared_memory
        //   open a shared memory by name
        // Parameters:
        //   SharedMemory &_memory - memory
        //   const std::string _name - filename
        // Return:
        //   0 if stream is created correctly. leaves the stream open.

        _memory.name = _name;
        if (shared_memory_exists(_name))
        {
            return setup_open_shared_memory(_memory);
        }
        return -1;
    }

    Keyword *find_keyword(SharedMemory &_memory, const char *name) // Find keyword by name
    {
        for (Keyword &keyword : get_keywords(_memory))
        {
            if (std::strncmp(keyword.name, name, KEYWORD_MAX_STRING) == 0)
            {
                return &keyword;
            }
        }
        return nullptr;
    }

    int update_creation_time(SharedStorage *_storage)
    {
        int ret = clock_gettime(CLOCK_REALTIME, &_storage->creationtime);
        _storage->lastaccesstime = _storage->creationtime; // Update last access time
        return ret;
    }

    int update_last_access_time(SharedStorage *_storage)
    {
        return clock_gettime(CLOCK_REALTIME, &_storage->lastaccesstime);
    }

    int consumer_request_start(SharedStorage *_storage)
    {   /*
            set request flag to true
        */
        // ==== begin critical section ================================================================================
        pthread_mutex_lock(&_storage->mutex);
        _storage->request_flag = true; // request frame from storage
        pthread_cond_signal(&_storage->request_cond);
        pthread_mutex_unlock(&_storage->mutex);
        // ==== end critical section ==================================================================================
        return 0;
    }

    int consumer_wait_for_ready(SharedStorage *_storage)
    {   /*
            wait for ready_flag to become true
            set ready flag to false
        */
        // ==== begin critical section ================================================================================
        pthread_mutex_lock(&_storage->mutex);
        while (!_storage->ready_flag) // wait for frame ready
            pthread_cond_wait(&_storage->ready_cond, &_storage->mutex);
        _storage->ready_flag = false;
        pthread_mutex_unlock(&_storage->mutex);
        // ==== end critical section ==================================================================================
        return 0;
    }

    int producer_wait_for_request(SharedStorage *_storage)
    {   /*
            wait for request flag to become true
        */
        // ==== begin critical section ================================================================================
        pthread_mutex_lock(&_storage->mutex);
        while (!_storage->request_flag) // wait for request
            pthread_cond_wait(&_storage->request_cond, &_storage->mutex);
        pthread_mutex_unlock(&_storage->mutex);
        // ==== end critical section ==================================================================================
        return 0;
    }

    int producer_request_done(SharedStorage *_storage)
    {   /*
            set request flag to false
            set ready flag to true
        */
        // ==== begin critical section ================================================================================
        pthread_mutex_lock(&_storage->mutex);
        _storage->ready_flag = true;
        _storage->request_flag = false;
        pthread_cond_signal(&_storage->ready_cond);
        pthread_mutex_unlock(&_storage->mutex);
        // ==== end critical section ==================================================================================
        return 0;
    }

    int lock(SharedStorage *_storage)
    {
        return pthread_mutex_lock(&_storage->mutex);
    }

    int unlock(SharedStorage *_storage)
    {
        return pthread_mutex_unlock(&_storage->mutex);
    }

}
#endif // __SHMIO_SHARED_MEMORY_HPP__