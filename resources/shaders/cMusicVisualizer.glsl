#version 430

struct Complex {
    double r, i;
};

layout(std140, binding = 4) buffer ComI {
    Complex f[ ]; //Input buff
};

layout(std140, binding = 5) buffer ComT {
    Complex t[ ]; //Twiddle buff
};

layout(std430, binding = 6) buffer ComB {
    float b[ ]; //Frequency buff
};

layout(local_size_x = 32, local_size_y = 1, local_size_z = 1) in;

Complex mul(Complex a, Complex b) { 
    return Complex(a.r*b.r - a.i*b.i, a.r*b.i + a.i*b.r);
}

Complex sum(Complex a, Complex b) { 
    return Complex(a.r + b.r, a.i + b.i);
}

uniform int stride;
uniform int size;
uniform int timeSize;

void main() {
    int halfPow2 = 1;
    int fftStride = 2;
    int halfStride = fftStride/2;
    int halfSize = size/2;
    int bitCount = int(log2(size));
    
    for (int i = 0; i < bitCount; ++i) {
        int offset = size/fftStride;
        for (int j = 0; j < size; j += fftStride) {
            int twiddleSelector = 0;
            for (int k = 0; k < halfStride; ++k) {
                int iF = j + k;
                Complex aux = f[iF];
                f[iF] = sum(aux, 
                            mul(t[twiddleSelector],
                                f[iF + halfStride]));
                f[iF + halfStride] = sum(aux,
                                         mul(t[twiddleSelector+halfSize],
                                             f[iF + halfStride]));
                twiddleSelector += offset;
            }
        }
        fftStride *= 2;
        halfStride *= 2;
        halfPow2 *= 2;
    }
    
    int singleRowBandCount = halfSize/stride;
    int bandCount = singleRowBandCount*timeSize;
    int oldBandCount = bandCount - singleRowBandCount;
    
    for (int iBand = oldBandCount - 1; iBand >= 0; --iBand) {
        b[iBand + singleRowBandCount] = b[iBand];
    }
    
    for (int iBand = 0, count = 0; iBand < halfSize; iBand += stride, ++count) {
        b[count] = float(sqrt(f[iBand].r*f[iBand].r +
                              f[iBand].i*f[iBand].i));
    }
    
    b[0] *= 0.5f;
    for (int i = 0; i < 4; ++i) {
        b[i] *= 0.5f;
    }
}