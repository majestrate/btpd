#ifndef TOXN_ACTIVE_H
#define TOXN_ACTIVE_H

void active_add(const uint8_t *hash);
void active_del(const uint8_t *hash);
void active_clear(void);
void active_start(void);

#endif
