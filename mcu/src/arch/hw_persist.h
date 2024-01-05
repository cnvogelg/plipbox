#ifndef HW_PERSIST_H
#define HW_PERSIST_H

#define HW_PERSIST_OK           0
#define HW_PERSIST_WRONG_CRC    1
#define HW_PERSIST_NOT_READY    2

struct hw_persist_base {
  u16 crc;
};
typedef struct hw_persist_base hw_persist_base_t;

extern u08 hw_persist_save(hw_persist_base_t *base, u16 size);
extern u08 hw_persist_load(hw_persist_base_t *base, u16 size);

#endif
