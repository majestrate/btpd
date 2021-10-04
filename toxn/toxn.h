#ifndef TOXN_H
#define TOXN_H

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#ifndef AI_ADDRCONFIG
#define AI_ADDRCONFIG 0
#endif

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <inttypes.h>
#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>

#include <benc.h>
#define DAEMON
#include <toxn_if.h>
#undef DAEMON
#include <evloop.h>
#include <metainfo.h>
#include <queue.h>
#include <subr.h>

#include "active.h"
#include "hashtable.h"
#include "net_buf.h"
#include "net_types.h"
#include "net.h"
#include "peer.h"
#include "tlib.h"
#include "torrent.h"
#include "download.h"
#include "upload.h"
#include "content.h"
#include "opts.h"
#include "tracker_req.h"

#define TOXN_VERSION PACKAGE_NAME "/" PACKAGE_VERSION

#define TOXN_L_ALL      0xffffffff
#define TOXN_L_ERROR    0x00000001
#define TOXN_L_TR       0x00000002
#define TOXN_L_CONN     0x00000004
#define TOXN_L_MSG      0x00000008
#define TOXN_L_TOXN     0x00000010
#define TOXN_L_POL      0x00000020
#define TOXN_L_BAD      0x00000040

extern long toxn_seconds;

void toxn_init(void);

__attribute__((format (printf, 2, 3)))
void toxn_log(uint32_t type, const char *fmt, ...);

__attribute__((format (printf, 1, 2), noreturn))
void toxn_err(const char *fmt, ...);

__attribute__((malloc))
void *toxn_malloc(size_t size);
__attribute__((malloc))
void *toxn_calloc(size_t nmemb, size_t size);

void toxn_ev_new(struct fdev *ev, int fd, uint16_t flags, evloop_cb_t cb,
    void *arg);
void toxn_ev_del(struct fdev *ev);
void toxn_ev_enable(struct fdev *ev, uint16_t flags);
void toxn_ev_disable(struct fdev *ev, uint16_t flags);
void toxn_timer_add(struct timeout *to, struct timespec *ts);
void toxn_timer_del(struct timeout *to);

void toxn_shutdown(void);
int toxn_is_stopping(void);

int toxn_id_eq(const void *k1, const void *k2);
uint32_t toxn_id_hash(const void *k);

const uint8_t *toxn_get_peer_id(void);

int toxn_id_eq(const void *id1, const void *id2);
uint32_t toxn_id_hash(const void *id);

void td_acquire_lock(void);
void td_release_lock(void);

void td_post(void (*cb)(void *), void *arg);
void td_post_end();
#define td_post_begin td_acquire_lock

typedef struct ai_ctx * aictx_t;
aictx_t toxn_addrinfo(const char *node, uint16_t port, struct addrinfo *hints,
    void (*cb)(void *, int, struct addrinfo *), void *arg);
void toxn_addrinfo_cancel(aictx_t ctx); 


typedef struct nameconn *nameconn_t;
nameconn_t toxn_name_connect(const char *name, short port,
    void (*cb)(void *, int, int), void *arg);
void toxn_name_connect_cancel(nameconn_t nc);

#endif
