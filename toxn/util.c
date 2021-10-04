#include "toxn.h"

#include <stdarg.h>
#include <time.h>

int
toxn_id_eq(const void *k1, const void *k2)
{
    return bcmp(k1, k2, 20) == 0;
}

uint32_t
toxn_id_hash(const void *k)
{
    return dec_be32(k + 16);
}

void *
toxn_malloc(size_t size)
{
    void *a;
    if ((a = malloc(size)) == NULL)
        toxn_err("Failed to allocate %d bytes.\n", (int)size);
    return a;
}

void *
toxn_calloc(size_t nmemb, size_t size)
{
    void *a;
    if ((a = calloc(nmemb, size)) == NULL)
        toxn_err("Failed to allocate %d bytes.\n", (int)(nmemb * size));
    return a;
}

void
toxn_ev_new(struct fdev *ev, int fd, uint16_t flags, evloop_cb_t cb, void *arg)
{
    if (fdev_new(ev, fd, flags, cb, arg) != 0)
        toxn_err("Failed to add event (%s).\n", strerror(errno));
}

void
toxn_ev_del(struct fdev *ev)
{
    if (fdev_del(ev) != 0)
        toxn_err("Failed to remove event (%s).\n", strerror(errno));
}

void
toxn_ev_enable(struct fdev *ev, uint16_t flags)
{
    if (fdev_enable(ev, flags) != 0)
        toxn_err("Failed to enable event (%s).\n", strerror(errno));
}

void
toxn_ev_disable(struct fdev *ev, uint16_t flags)
{
    if (fdev_disable(ev, flags) != 0)
        toxn_err("Failed to disable event (%s).\n", strerror(errno));
}

void
toxn_timer_add(struct timeout *to, struct timespec *ts)
{
    if (evtimer_add(to, ts) != 0)
        toxn_err("Failed to add timeout (%s).\n", strerror(errno));
}

void
toxn_timer_del(struct timeout *to)
{
    evtimer_del(to);
}

static const char *
logtype_str(uint32_t type)
{
    switch (type) {
    case TOXN_L_TOXN:  return "toxn";
    case TOXN_L_ERROR: return "error";
    case TOXN_L_CONN:  return "conn";
    case TOXN_L_TR:    return "tracker";
    case TOXN_L_MSG:   return "msg";
    case TOXN_L_POL:   return "policy";
    case TOXN_L_BAD:   return "bad";
    }
    return "";
}

static void
log_common(uint32_t type, const char *fmt, va_list ap)
{
    if (type & toxn_logmask) {
        char tbuf[20];
        time_t tp = time(NULL);
        strftime(tbuf, 20, "%Y %b %e %T", localtime(&tp));
        printf("%s %s: ", tbuf, logtype_str(type));
        vprintf(fmt, ap);
    }
}

extern int toxn_daemon_phase;
extern void first_toxn_exit(char);

void
toxn_err(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    if (toxn_daemon_phase > 0) {
        vprintf(fmt, ap);
        if (toxn_daemon_phase == 1)
            first_toxn_exit(1);
        exit(1);
    } else {
        log_common(TOXN_L_ERROR, fmt, ap);
        abort();
    }
}

void
toxn_log(uint32_t type, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    log_common(type, fmt, ap);
    va_end(ap);
}
