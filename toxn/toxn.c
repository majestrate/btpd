#include "toxn.h"

#include <openssl/sha.h>
#include <signal.h>

static uint8_t m_peer_id[20];
static struct timeout m_heartbeat;
static int m_signal;
static int m_shutdown;
static int m_ghost;

long toxn_seconds;

void
toxn_exit(int code)
{
    toxn_log(TOXN_L_TOXN, "Exiting.\n");
    exit(code);
}

extern int pidfd;
void ipc_shutdown(void);
void net_shutdown(void);

static void
death_procedure(void)
{
    assert(m_shutdown);
    if (torrent_count() == 0)
        toxn_exit(0);
    if (!m_ghost && torrent_count() == torrent_ghosts()) {
        toxn_log(TOXN_L_TOXN, "Entering pre exit mode. Bye!\n");
        fclose(stderr);
        fclose(stdout);
        net_shutdown();
        ipc_shutdown();
        close(pidfd);
        m_ghost = 1;
    }
}

void
toxn_shutdown(void)
{
    m_shutdown = 1;
    struct torrent *tp, *next;
    TOXNQ_FOREACH_MUTABLE(tp, torrent_get_all(), entry, next)
        torrent_stop(tp, 0);
    death_procedure();
}

int toxn_is_stopping(void)
{
    return m_shutdown;
}

const uint8_t *
toxn_get_peer_id(void)
{
    return m_peer_id;
}

static void
signal_handler(int signal)
{
    m_signal = signal;
}

static void
heartbeat_cb(int fd, short type, void *arg)
{
    toxn_timer_add(&m_heartbeat, (& (struct timespec) { 1, 0 }));
    toxn_seconds++;
    net_on_tick();
    torrent_on_tick_all();
    if (m_signal) {
        toxn_log(TOXN_L_TOXN, "Got signal %d.\n", m_signal);
        m_signal = 0;
        if (!m_shutdown)
            toxn_shutdown();
    }
    if (m_shutdown)
        death_procedure();
}

void tr_init(void);
void ipc_init(void);
void td_init(void);
void addrinfo_init(void);

void
toxn_init(void)
{
    struct sigaction sa;
    unsigned long seed;
    uint8_t idcon[1024];
    struct timeval now;
    int n;

    bzero(&sa, sizeof(sa));
    sa.sa_handler = SIG_IGN;
    sa.sa_flags = SA_RESTART;
    sigfillset(&sa.sa_mask);
    sigaction(SIGPIPE, &sa, NULL);
    sa.sa_handler = signal_handler;
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGINT, &sa, NULL);

    gettimeofday(&now, NULL);
    n = snprintf(idcon, sizeof(idcon), "%ld%ld%d", (long)now.tv_sec,
        (long)now.tv_usec, net_port);
    if (n < sizeof(idcon))
        gethostname(idcon + n, sizeof(idcon) - n);
    idcon[sizeof(idcon) - 1] = '\0';
    n = strlen(idcon);

    SHA1(idcon, n, m_peer_id);
    bcopy(m_peer_id, &seed, sizeof(seed));
    bcopy(TOXN_VERSION, m_peer_id, sizeof(TOXN_VERSION) - 1);
    m_peer_id[sizeof(TOXN_VERSION) - 1] = '|';

    srandom(seed);

    td_init();
    addrinfo_init();
    net_init();
    ipc_init();
    ul_init();
    cm_init();
    tr_init();
    tlib_init();

    evtimer_init(&m_heartbeat, heartbeat_cb, NULL);
    toxn_timer_add(&m_heartbeat, (& (struct timespec) { 1, 0 }));
}
