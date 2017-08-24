//
// Created by Administrator on 2017/7/2.
//

#include <elf.h>
#include <zmalloc.h>
#include <stdio.h>
#include "bit_ops.h"
#include "sds.h"

static uint32_t dict_hash_function_seed = 5381;

unsigned int str_hash(const void *key, int len) {
    /* 'm' and 'r' are mixing constants generated offline.
     They're not really 'magic', they just happen to work well.  */
    uint32_t seed = dict_hash_function_seed;
    const uint32_t m = 0x5bd1e995;
    const int r = 24;

    /* Initialize the hash to a 'random' value */
    uint32_t h = seed ^ len;

    /* Mix 4 bytes at a time into the hash */
    const unsigned char *data = (const unsigned char *)key;

    while(len >= 4) {
        uint32_t k = *(uint32_t*)data;

        k *= m;
        k ^= k >> r;
        k *= m;

        h *= m;
        h ^= k;

        data += 4;
        len -= 4;
    }

    /* Handle the last few bytes of the input array  */
    switch(len) {
        case 3: h ^= data[2] << 16;
        case 2: h ^= data[1] << 8;
        case 1: h ^= data[0]; h *= m;
        default:
            break;
    };

    /* Do a few final mixes of the hash to ensure the last few
     * bytes are well-incorporated. */
    h ^= h >> 13;
    h *= m;
    h ^= h >> 15;

    return (unsigned int)h;
}

unsigned int get_offset(const char* strKey, size_t len){
    unsigned int key = str_hash(strKey, (int)len);
    key &= S_MASK;

    return key;
}

int set_bit(uint8_t* storage, int key, int val){
    int byte, bit, byte_val, bit_val;
    val &= 0x1;

    byte = key >> 3;
    byte_val = storage[byte];
    bit = 7 - (key & 0x7);
    bit_val = byte_val & (1 << bit);

    byte_val &= ~(1 << bit);
    byte_val |= (val << bit);
    storage[byte] = (uint8_t)byte_val;

    return bit_val > 0 ? 1 : 0;
}

uint8_t* get_storage(){
    uint8_t *storage = (uint8_t*)sdsnewlen(NULL, S_BYTE_SIZE);
//    FILE *fr;
//    fr = fopen("/tmp/storage_bit.log", "r");
//    if(fr != NULL){
//        fread(storage, S_BYTE_SIZE, 1, fr);
//    }
//    fclose(fr);

    return storage;
}

void set_storage(uint8_t* storage){
    FILE *fw;
    fw = fopen("/tmp/storage_bit.log", "w");
    fwrite(storage, S_BYTE_SIZE, 1, fw);
    fclose(fw);
    sdsfree((sds)storage);
}

//void main(){
//    uint8_t* storage = get_storage();
//    int ret = set_bit(storage, 999, 1);
//    int ret2 = set_bit(storage, 999, 1);
//    set_storage(storage);
//    printf("%zu\n", sizeof(uint8_t));
//    printf("%d\n", ret);
//    printf("%d\n", ret2);
//}