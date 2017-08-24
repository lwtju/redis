//
// Created by Administrator on 2017/7/2.
//

#ifndef SF_BIT_OPS_H
#define SF_BIT_OPS_H

#include <stdint.h>

#define S_P 20

#define S_BYTE_SIZE (sizeof(uint8_t) * (1 << S_P))

#define S_MAX_OFFSET ((1 << S_P) * 8)

#define S_MASK (S_MAX_OFFSET - 1)

unsigned int str_hash(const void *key, int len);

unsigned int get_offset(const char* strKey, size_t len);

uint8_t *get_storage();

void set_storage(uint8_t* storage);

int set_bit(uint8_t* storage, int key, int val);

#endif //SF_BIT_OPS_H