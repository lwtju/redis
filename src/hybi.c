//
// Created by liuwang-s on 2017/7/3.
//

#include "redis.h"

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <zmalloc.h>

#include "bit_ops.h"
#include "hybi.h"

hybi* get_hybi(){
    hybi* hb = (hybi*)sdsnewlen(NULL, sizeof(hybi));

    hb->b_str = (uint8_t*)sdsnewlen(NULL, S_BYTE_SIZE);

    hb->hll = (struct hllhdr*)sdsnewlen(NULL, sizeof(struct hllhdr));
    memcpy(hb->hll->magic, "HYLL", 4);
    hb->hll->encoding = HLL_DENSE;
    HLL_INVALIDATE_CACHE(hb->hll);
    memset(hb->hll->notused, 0, sizeof(hb->hll->notused));
    memset(hb->hll->registers, 0, HLL_REGISTERS_SIZE);

    hb->hll_tmp = (struct hllhdr*)sdsnewlen(NULL, sizeof(struct hllhdr));
    memcpy(hb->hll_tmp, hb->hll, sizeof(struct hllhdr));

    hb->count = 0;
    hb->group_len = GROUP_DEFAULT_LEN + hb->count / GROUP_SUB_LEN;
    hb->group_count = 0;

    return hb;
}
//
int is_hb_object(robj *o){
    if(stringObjectLen(o) != sizeof(hybi)){
        return -1;
    }
    hybi* hb = (hybi*)o->ptr;
    if(hb->hll->magic[0] != 'H'){
        return -1;
    }

    return 0;
}

void hbaddCommand(redisClient *c){
    robj *o;
    hybi* hb = NULL;

    o = lookupKeyWrite(c->db,c->argv[1]);
    if (o == NULL) {
        o = createObject(REDIS_STRING,sdsempty());
        dbAdd(c->db,c->argv[1],o);
        hb = get_hybi();
        o->ptr = hb;
    } else {
        if (checkType(c,o,REDIS_STRING)) return;
        if (is_hb_object(o) == -1){
            addReplySds(c, sdsnew("-WRONGTYPE Key is not a valid Hybi type.\r\n"));
            return;
        }
        o = dbUnshareStringValue(c->db,c->argv[1],o);
        hb = o->ptr;
    }

    size_t len = strlen(c->argv[2]->ptr);
    hb->group_count ++;

    unsigned int offset = get_offset(c->argv[2]->ptr, len);
    int old_val = set_bit(hb->b_str, offset, 1);

    if(old_val == 0){
        int h_add_ret = hllDenseAdd(hb->hll_tmp->registers, c->argv[2]->ptr, len);
        hb->count ++;
    }
    hllDenseAdd(hb->hll->registers, c->argv[2]->ptr, len);

    if(hb->group_count == hb->group_len){
        hb->count += hllCount(hb->hll, NULL) - hllCount(hb->hll_tmp, NULL);
        //init
        memcpy(hb->hll_tmp, hb->hll, sizeof(struct hllhdr));
        hb->group_count = 0;
        hb->group_len = GROUP_DEFAULT_LEN + hb->count / GROUP_SUB_LEN;
    }

    signalModifiedKey(c->db,c->argv[1]);
    server.dirty++;

    addReplyLongLong(c, hb->count);
}

void hbcountCommand(redisClient *c) {
    robj *o;
    hybi *hb = NULL;
    int ret = 0;

    o = lookupKeyWrite(c->db, c->argv[1]);
    if (o != NULL) {
        if (checkType(c, o, REDIS_STRING) || is_hb_object(o) == -1) {
            addReplySds(c, sdsnew("-WRONGTYPE Key is not a valid Hybi type.\r\n"));
            return;
        }
        o = dbUnshareStringValue(c->db, c->argv[1], o);
        hb = o->ptr;
        ret = hb->count;
    }

    addReplyLongLong(c, ret);
}