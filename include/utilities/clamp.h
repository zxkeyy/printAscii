#ifndef CLAMP_H
#define CLAMP_H

static inline int clamp(int value, int min, int max) {
    return (value < min) ? min : (value > max) ? max : value;
}

#endif // CLAMP_H