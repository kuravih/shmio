# shmio

**shmio** is a C++ library for working with shared memory.
It provides an abstraction layer for creating, accessing, and managing shared memory regions across multiple processes on Unix-like systems.
It is a stripped down version of [ImageStreamIO](https://github.com/milk-org/ImageStreamIO).

## Key Features

- **POSIX Shared Memory**: Uses POSIX shared memory (`shm_open`, `mmap`) under the hood
- **Data Type Support**: Supports a wide range of numeric data types including:
  - Integer types: `DataType::UINT8`, `DataType::INT8`, `DataType::UINT16`, `DataType::INT16`, `DataType::UINT32`, `DataType::INT32`, `DataType::UINT64`, `DataType::INT64`
  - Floating point: `DataType::HALF` (16-bit), `DataType::FLOAT` (32-bit), `DataType::DOUBLE` (64-bit)
  - Complex numbers: `DataType::COMPLEX_FLOAT` (2 x 32-bit), `DataType::COMPLEX_DOUBLE` (2 x 64-bit)
- **Keyword Metadata**: Supports metadata keywords with multiple types ('KeywordType::LONG', 'KeywordType::DOUBLE' and 'KeywordType::STRING')
- **Thread-Safe**: Uses POSIX threads (`pthread`) for synchronization

## Core Components

The main header file `shared_memory.hpp` defines:
- Data type enumerations and size definitions
- A `Keyword` struct for storing metadata (name, type, value, comment)
- The `shmio` namespace containing the library's API

## Use Cases

This library is designed for inter-process communication and data sharing scenarios where multiple processes need to efficiently access the same memory region with minimal overhead. See [pyshmio](https://github.com/kuravih/pyshmio) for the python binding.