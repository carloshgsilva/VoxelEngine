#pragma once

#include <fstream>
#include <string>

#include "Core/Core.h"
#include "Log.h"

// Stream of Values
// Used to Write to File, Memory Buffers
class Stream {
protected:
    uint8 StreamIsLoading : 1;
    uint8 StreamIsLittleEndian : 1 = 1;

public:
    void SetIsLittleEndian(bool v) { StreamIsLittleEndian = v; }
    bool IsLoading() { return StreamIsLoading; }

    virtual bool HasData() { assert(false); return true; }
    virtual size_t GetSize() { assert(false); return 0; }
    virtual size_t GetPointer() { assert(false); return 0; }
    virtual void SetPointer(uint64_t p) { assert(false);  }
    virtual void Serialize(void* Data, size_t Size) { }
    virtual std::string GetIdentifier() {
        return "Not Identified";
    }

    void Skip(uint64_t bytes) {
        SetPointer(GetPointer() + bytes);
    }

    template <typename T, typename = std::enable_if_t<std::is_fundamental_v<T>>>
    void Serialize(T& Value) {
        if(StreamIsLoading) {
            if(StreamIsLittleEndian) {
                Serialize(&Value, sizeof(T));
            } else {
                union {
                    T v;
                    uint8_t u8[sizeof(T)];
                } src, dst;
                Serialize(&src, sizeof(T));
                for (size_t i = 0; i < sizeof(T); i++) {
                    dst.u8[i] = src.u8[sizeof(T) - i - 1];
                }
                Value = dst.v;
            }
        } else {
            assert(false);
        }
    }

    //Fundamentals (int, float, double)
    template <typename T>
    friend std::enable_if_t<std::is_fundamental_v<T>, Stream&> 
    operator|(Stream& S, T& Value) {
        S.Serialize(&Value, sizeof(T));
        return S;
    }

    //TODO: Swap the default std::string
    friend Stream& operator|(Stream& S, std::string& Value) {
        int32 Size = static_cast<int32>(Value.size());
        S | Size;

        if (S.IsLoading()) {
            Value.resize(Size);
        }

        S.Serialize(Value.data(), Value.size());
        return S;
    }

};

class MemoryWriter : public Stream {
    size_t pointer;
    char* buffer;
    char* end;
public:

    MemoryWriter(char* buffer, size_t size) : buffer(buffer), pointer(0) {
        end = buffer + size;
    }

    virtual void Serialize(void* data, size_t size) {
        if (buffer >= end)CHECK(0);
        memcpy(buffer + pointer, data, size);
        pointer += size;
    }
    virtual size_t GetPointer() { return pointer; }
};
class MemoryReader : public Stream {
    size_t pointer;
    char* buffer;
    char* end;
public:
    MemoryReader(char* buffer, size_t size) : buffer(buffer), pointer(0) {
        end = buffer + size;
    }

    virtual void Serialize(void* data, size_t size) {
        if (buffer >= end)CHECK(0);
        memcpy(data, buffer+ pointer, size);
        pointer += size;
    }
    virtual size_t GetPointer() { return pointer; }
};

class FileWriter : public Stream{
public:
    std::ofstream fs;
    std::string path;

    FileWriter(const std::string& path) :
        path(path), 
        fs(path, std::ios::binary) {
        StreamIsLoading = false;
    }

    virtual void Serialize(void* Data, size_t Size) {
        fs.write(reinterpret_cast<char*>(Data), Size);
    }

    virtual std::string GetIdentifier() { return path; }
};
class FileReader : public Stream {
    std::ifstream fs;
    std::string path;
    size_t size;
public:
    FileReader(const std::string& path) :
        path(path),
        fs(path, std::ios::binary|std::ios::ate) {

        if (!fs.is_open()) {//Probably the file is already being used or is not found
            Log::error("Failed to open file {}", path);
        }

        //Get Size
        size = fs.tellg();
        fs.seekg(0, std::ios::beg);

        StreamIsLoading = true;
    }

    virtual size_t GetSize() { return size; }
    virtual size_t GetPointer() { return fs.tellg(); }
    virtual void SetPointer(uint64_t p) { fs.seekg(p);  }
    virtual void Serialize(void* Data, size_t Size) {
        fs.read(reinterpret_cast<char*>(Data), Size);
    }
    virtual std::string GetIdentifier() { return path; }
};


//glm Stream wrapers
#include "glm/glm.hpp"

static Stream& operator|(Stream& S, glm::mat4& V) {
    S.Serialize(&V[0], sizeof(glm::mat4));
    return S;
}
static Stream& operator|(Stream& S, glm::vec3& V) {
    S.Serialize(&V[0], sizeof(glm::vec3));
    return S;
}