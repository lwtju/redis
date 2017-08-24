//
// Created by liuwang-s on 2017/7/3.
//

#ifndef SF_HYBI_H
#define SF_HYBI_H

#include "hyperloglog.h"

#define GROUP_DEFAULT_LEN 1
#define GROUP_SUB_LEN 10000

typedef struct hybi{
    struct hllhdr *hll;
    struct hllhdr *hll_tmp;
    uint8_t *b_str;
    int group_len;
    int group_count;
    int count;    //当前总共计数
} hybi;

#endif //SF_HYBI_H
