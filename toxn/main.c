#include "toxn.h"

#include <getopt.h>
#include <time.h>

int toxn_daemon_phase = 2;
int first_toxn_comm[2];
int pidfd;

void
first_toxn_exit(char code)
{
    write(first_toxn_comm[1], &code, 1);
    close(first_toxn_comm[0]);
    close(first_toxn_comm[1]);
}

static void
writepid(void)
{
    int nw;
    char pidtxt[100];
    nw = snprintf(pidtxt, sizeof(pidtxt), "%ld", (long)getpid());
    ftruncate(pidfd, 0);
    write(pidfd, pidtxt, nw);
}

static void
setup_daemon(int daemonize, const char *dir)
{
    char c;
    pid_t pid;
    struct timespec ts;

    if (snprintf(NULL, 0, "toxn") != 4)
        toxn_err("snprintf doesn't work.\n");

    if (evtimer_gettime(&ts) != 0)
        toxn_err("evtimer_gettime failed (%s).\n", strerror(errno));

    if (dir == NULL) {
        if ((dir = find_toxn_dir()) == NULL)
            toxn_err("Cannot find the toxn directory.\n");
        if (dir[0] != '/')
            toxn_err("Got non absolute path '%s' from system environment.\n",
                dir);
        toxn_dir = dir;
    }

    if (mkdir(dir, 0777) == -1 && errno != EEXIST)
        toxn_err("Couldn't create home '%s' (%s).\n", dir, strerror(errno));

    if (chdir(dir) != 0)
        toxn_err("Couldn't change working directory to '%s' (%s).\n", dir,
            strerror(errno));

    if (mkdir("torrents", 0777) == -1 && errno != EEXIST)
        toxn_err("Couldn't create torrents subdir (%s).\n", strerror(errno));

    if (toxn_dir == NULL) {
        char wd[PATH_MAX];
        if (getcwd(wd, PATH_MAX) == NULL)
            toxn_err("Couldn't get working directory (%s).\n",
                strerror(errno));
        if ((toxn_dir = strdup(wd)) == NULL)
            toxn_err("Out of memory.\n");
    }

    if (daemonize) {
        if (pipe(first_toxn_comm) < 0)
            toxn_err("Failed to create pipe (%s).\n", strerror(errno));
        if ((pid = fork()) < 0)
            toxn_err("fork() failed (%s).\n", strerror(errno));
        if (pid != 0) {
            read(first_toxn_comm[0], &c, 1);
            exit(c);
        }
        toxn_daemon_phase--;
        if (setsid() < 0)
            toxn_err("setsid() failed (%s).\n", strerror(errno));
        if ((pid = fork()) < 0)
            toxn_err("fork() failed (%s).\n", strerror(errno));
        if (pid != 0)
            exit(0);
    }

    if ((pidfd = open("pid", O_CREAT|O_WRONLY, 0666)) == -1)
        toxn_err("Couldn't open 'pid' (%s).\n", strerror(errno));

    if (lockf(pidfd, F_TLOCK, 0) == -1)
        toxn_err("Another instance of toxn is probably running in %s.\n", dir);

    writepid();
}

static void
usage(void)
{
    printf(
        "toxn is the BitTorrent Protocol Daemon.\n"
        "\n"
        "Usage: toxn [-d dir] [-p port] [more options...]\n"
        "\n"
        "Options:\n"
        "-4\n"
        "\tUse IPv4. If given in conjunction with -6, "
        "both versions are used.\n"
        "\n"
        "-6\n"
        "\tUse IPv6. By default IPv4 is used.\n"
        "\n"
        "--bw-in n\n"
        "\tLimit incoming BitTorrent traffic to n kB/s.\n"
        "\tDefault is 0 which means unlimited.\n"
        "\n"
        "--bw-out n\n"
        "\tLimit outgoing BitTorrent traffic to n kB/s.\n"
        "\tDefault is 0 which means unlimited.\n"
        "\n"
        "-d dir\n"
        "\tThe directory in which to run toxn. Default is '$HOME/.toxn'.\n"
        "\n"
        "--empty-start\n"
        "\tStart toxn without any active torrents.\n"
        "\n"
        "--help\n"
        "\tShow this text.\n"
        "\n"
        "--ip addr\n"
        "\tLet the tracker distribute the given address instead of the one\n"
        "\tit sees toxn connect from.\n"
        "\n"
        "--ipcprot mode\n"
        "\tSet the protection mode of the command socket.\n"
        "\tThe mode is specified by an octal number. Default is 0600.\n"
        "\n"
        "--logfile file\n"
        "\tWhere to put the logfile. By default it's put in the toxn dir.\n"
        "\n"
        "--max-peers n\n"
        "\tLimit the amount of peers to n.\n"
        "\n"
        "--max-uploads n\n"
        "\tControls the number of simultaneous uploads.\n"
        "\tThe possible values are:\n"
        "\t\tn < -1 : Choose n >= 2 based on --bw-out (default).\n"
        "\t\tn = -1 : Upload to every interested peer.\n"
        "\t\tn =  0 : Dont't upload to anyone.\n"
        "\t\tn >  0 : Upload to at most n peers simultaneously.\n"
        "\n"
        "--no-daemon\n"
        "\tKeep the toxn process in the foregorund and log to std{out,err}.\n"
        "\tThis option is intended for debugging purposes.\n"
        "\n"
        "-p n, --port n\n"
        "\tListen at port n. Default is 6881.\n"
        "\n"
        "--prealloc n\n"
        "\tPreallocate disk space in chunks of n kB. Default is 2048.\n"
        "\tNote that n will be rounded up to the closest multiple of the\n"
        "\ttorrent piece size. If n is zero no preallocation will be done.\n"
        "\n"
        "--numwant n\n"
        "\tSet the number of peers to fetch on each request. Default is 50.\n"
        "\n");
    exit(1);
}

static int longval = 0;

static struct option longopts[] = {
    { "port",   required_argument,      NULL,           'p' },
    { "bw-in",  required_argument,      &longval,       1 },
    { "bw-out", required_argument,      &longval,       2 },
    { "prealloc", required_argument,    &longval,       3 },
    { "max-uploads", required_argument, &longval,       4 },
    { "max-peers", required_argument,   &longval,       5 },
    { "no-daemon", no_argument,         &longval,       6 },
    { "logfile", required_argument,     &longval,       7 },
    { "ipcprot", required_argument,     &longval,       8 },
    { "empty-start", no_argument,       &longval,       9 },
    { "ip", required_argument,          &longval,       10 },
    { "logmask", required_argument,     &longval,       11 },
    { "numwant", required_argument,     &longval,       12 },
    { "help",   no_argument,            &longval,       128 },
    { NULL,     0,                      NULL,           0 }
};

int
main(int argc, char **argv)
{
    char *dir = NULL, *log = NULL;
    int daemonize = 1, opt4 = 0, opt6 = 0;

    for (;;) {
        switch (getopt_long(argc, argv, "46d:p:", longopts, NULL)) {
        case -1:
            goto args_done;
        case '4':
            opt4 = 1;
            break;
        case '6':
            opt6 = 1;
            break;
        case 'd':
            dir = optarg;
            break;
        case 'p':
            net_port = atoi(optarg);
            break;
        case 0:
            switch (longval) {
            case 1:
                net_bw_limit_in = atoi(optarg) * 1024;
                break;
            case 2:
                net_bw_limit_out = atoi(optarg) * 1024;
                break;
            case 3:
                cm_alloc_size = atoi(optarg) * 1024;
                break;
            case 4:
                net_max_uploads = atoi(optarg);
                break;
            case 5:
                net_max_peers = atoi(optarg);
                break;
            case 6:
                daemonize = 0;
                break;
            case 7:
                log = optarg;
                break;
            case 8:
                ipcprot = strtol(optarg, NULL, 8);
                break;
            case 9:
                empty_start = 1;
                break;
            case 10:
                tr_ip_arg = optarg;
                break;
            case 11:
                toxn_logmask = atoi(optarg);
                break;
            case 12:
                net_numwant = (unsigned)atoi(optarg);
                break;
            default:
                usage();
            }
            break;
        case '?':
        default:
            usage();
        }
    }
args_done:
    argc -= optind;
    argv += optind;

    if (opt6) {
        net_ipv6 = 1;
        if (!opt4)
            net_ipv4 = 0;
    }

    if (argc > 0)
        usage();

    setup_daemon(daemonize, dir);

    if (evloop_init() != 0)
        toxn_err("Failed to initialize evloop (%s).\n", strerror(errno));

    toxn_init();

    if (daemonize) {
        if (freopen("/dev/null", "r", stdin) == NULL)
            toxn_err("freopen of stdin failed (%s).\n", strerror(errno));
        if (freopen(log == NULL ? "log" : log, "a", stderr) == NULL)
            toxn_err("Couldn't open '%s' (%s).\n", log, strerror(errno));
        if (dup2(fileno(stderr), fileno(stdout)) < 0)
            toxn_err("dup2 failed (%s).\n", strerror(errno));
        first_toxn_exit(0);
    }
    setlinebuf(stdout);
    setlinebuf(stderr);

    toxn_daemon_phase = 0;

    if (!empty_start)
        active_start();
    else
        active_clear();

    evloop();

    toxn_err("Exit from evloop with error (%s).\n", strerror(errno));

    return 1;
}
