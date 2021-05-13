#ifndef UTILS_H
#define UTILS_H

enum FIRST_LAST {
    FIRST,
    LAST,
};

inline b32
findCharacter(const char* pString, char toFind, FIRST_LAST from, size_t* pIndex) {
    size_t length = strlen(pString)/sizeof(char);
    switch (from) {
        case FIRST: {
            for (size_t i = 0; i < length; ++i) {
                if (pString[i] == toFind) {
                    *pIndex = i;
                    return true;
                }
            }
        } break;
        case LAST: {
            for (size_t i = length - 1; i >= 0; --i) {
                if (pString[i] == toFind) {
                    *pIndex = i;
                    return true;
                }
            }
        } break;
    }
    return false;
}

inline b32
haveSameSign(f32 a, f32 b) {
    return ((a >= 0) == (b >= 0));
}

inline u32 reverseBits(u32 value, u32 maxValue = 0) {
    u32 result = 0;
    
    if (maxValue == 0) {
        maxValue = 32;
    }
    
    u32 bitCount = u32(log(maxValue)/log(2));
    
    if (value != 0 && value != maxValue) {
        for (u32 i = 0; i < bitCount; ++i) {
            result |= (value & (1 << i))? (1 << (bitCount - 1 - i)) : 0;
        }
    }
    
    return result;
}

struct ENGINE_API Plane {
    hmm_vec3 normal;
    hmm_vec3 dot;
    f32 d; // ax + by + cz + d
};

struct ENGINE_API Complex {
    f64 r;
    f64 i;
};

inline Complex operator+(Complex compA, Complex compB) {
    Complex result;
    result.r = compA.r + compB.r;
    result.i = compA.i + compB.i;
    
    return result;
}

inline Complex operator-(Complex compA, Complex compB) {
    Complex result;
    result.r = compA.r - compB.r;
    result.i = compA.i - compB.i;
    
    return result;
}

inline Complex operator*(Complex compA, Complex compB) {
    Complex result;
    result.r = compA.r * compB.r - compA.i * compB.i;
    result.i = compA.r * compB.i + compB.r * compA.i;
    
    return result;
}

inline bool operator==(Complex compA, Complex compB) {
    return (compA.r == compB.r) && (compA.i == compB.i);
}

inline Complex operator-(Complex compA) {
    Complex result;
    result.r = -compA.r;
    result.i = -compA.i;
    
    return result;
}

#endif //UTILS_H