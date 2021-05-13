#ifndef BASE_H
#define BASE_H

#define internal static
#define local_persist static
#define global static

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t   s8;
typedef int16_t  s16;
typedef int32_t  s32;
typedef int64_t  s64;
typedef s8       b8;
typedef s32      b32;
typedef float    f32;
typedef double   f64;
typedef size_t memory_index;

global const f32 PI32 = 3.14159265359f;

// NOTE(Marchin): if expression is false, write into 0 (invalid memory) for a
//plaftorm independent break
#ifdef assert
#undef assert
#endif
#if SLOW
#define assert(expression) if(!(expression)) { *(int*)0 = 0; } 
#else
#define assert(expression) 
#endif

#define INVALID_CODE_PATH assert(!"INVALID_CODE_PATH");

#define kilobytes(value) ((value)*1024LL)
#define megabytes(value) (kilobytes(value)*1024LL)
#define gigabytes(value) (megabytes(value)*1024LL)
#define terabytes(value) (gigabytes(value)*1024LL)
#define arrayCount(array) (sizeof(array)/sizeof((array)[0]))


//Stack Memory

#if 0

internal void
shrinkStackBlock(void* pBuffer, size_t* pOffset, size_t bufferSize, 
                 size_t sizeOfType, void* pElement, size_t oldSize, size_t newSize) {
    
    u8* pBottom = (u8*)pBufferType;
    
    if (newSize >= oldSize) {
        return;
    }
    
    if (pBottom + ((*pOffset) * newSize)*sizeOfType < pBottom + bufferSize*sizeOfType) {
        u8* pFrom = (u8*)pElement + oldSize*sizeOfType;
        u8* pTo = (u8*)pElement + newSize*sizeOfType;
        size_t chunkSize = *pOffset*sizeOfType - (pFrom - pBottom);
        memmove(pTo, pFrom, chunkSize);
    } else {
        assert(false);
    }
}

internal void
freeStackBlock(void* pBuffer, size_t* pOffset, size_t bufferSize, 
               size_t sizeOfType, void* pElement, size_t oldSize) {
    shrinkStackBlock(pBuffer, pOffset, bufferSize, sizeOfType, pElement, oldSize, 0);
}
#endif

#endif //BASE_H