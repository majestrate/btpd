# toxn - BitTorrent Protocol Daemon
# See LICENSE file for copyright and license details.

TOXN_SRC    = ${wildcard toxn/*.c}
TOXN_DEPS   = ${wildcard toxn/*.h}
TOXN_OBJ    = ${TOXN_SRC:.c=.o}

TXCLI_SRC   = ${wildcard cli/*.c}
TXCLI_DEPS  = ${wildcard cli/*.h}
TXCLI_OBJ   = ${TXCLI_SRC:.c=.o}

TXINFO_SRC  = ${wildcard info/*.c}
TXINFO_DEPS = ${wildcard info/*.h}
TXINFO_OBJ  = ${TXINFO_SRC:.c=.o}

MISC_SRC    = ${wildcard misc/*.c}
MISC_DEPS   = ${wildcard misc/*.h}
MISC_OBJ    = ${MISC_SRC:.c=.o}

EVLOOP_SRC  = ${wildcard evloop/*.c}
EVLOOP_DEPS = ${wildcard evloop/*.h}
EVLOOP_OBJ  = ${EVLOOP_SRC:.c=.o}

include config.mk

all: options toxn/toxn info/txinfo cli/txcli

options:
	@echo toxn build options:
	@echo "CFLAGS   = ${CFLAGS}"
	@echo "LDFLAGS  = ${LDFLAGS}"
	@echo "CC       = ${CC}"

.c.o:
	${CC} -c ${DEFS} ${CPPFLAGS} ${CFLAGS} $< -o $@

${%_OBJ}: ${%_DEPS}

misc/libmisc.a: ${MISC_OBJ}
	ar rcs $@ ${MISC_OBJ}

evloop/libevloop.a: ${EVLOOP_OBJ}
	ar rcs $@ ${EVLOOP_OBJ}

toxn/toxn: ${TOXN_OBJ} misc/libmisc.a evloop/libevloop.a
	${CC} ${CFLAGS} -o $@ ${TOXN_OBJ}	 misc/libmisc.a evloop/libevloop.a ${LDFLAGS}

info/txinfo: ${TXINFO_OBJ} misc/libmisc.a
	${CC} ${CFLAGS} -o $@ ${TXINFO_OBJ} misc/libmisc.a ${LDFLAGS}

cli/txcli: ${TXCLI_OBJ} misc/libmisc.a
	${CC} ${CFLAGS} -o $@  ${TXCLI_OBJ}  misc/libmisc.a ${LDFLAGS}

clean:
	rm -f toxn/toxn cli/txcli info/txinfo\
		**/*.o **/*.a\
		toxn-${VERSION}.tar.gz

dist: clean
	mkdir -p toxn-${VERSION}
	cp -R COPYRIGHT Makefile README CHANGES configure config.mk toxn cli doc evloop info misc\
		toxn-${VERSION}
	tar -cf toxn-${VERSION}.tar toxn-${VERSION}
	gzip toxn-${VERSION}.tar
	rm -rf toxn-${VERSION}

install: all
	mkdir -p ${DESTDIR}${PREFIX}/bin
	cp -f toxn/toxn cli/txcli info/txinfo ${DESTDIR}${PREFIX}/bin
	chmod 755 ${DESTDIR}${PREFIX}/bin/toxn
	chmod 755 ${DESTDIR}${PREFIX}/bin/txcli
	chmod 755 ${DESTDIR}${PREFIX}/bin/txinfo
	mkdir -p ${DESTDIR}${MANPREFIX}/man1
	cp -f doc/*.1 ${DESTDIR}${MANPREFIX}/man1
	chmod 644 ${DESTDIR}${MANPREFIX}/man1/toxn.1
	chmod 644 ${DESTDIR}${MANPREFIX}/man1/txcli.1
	chmod 644 ${DESTDIR}${MANPREFIX}/man1/txinfo.1

uninstall:
	rm -f ${DESTDIR}${PREFIX}/bin/toxn\
		${DESTDIR}${PREFIX}/bin/txcli\
		${DESTDIR}${PREFIX}/bin/txinfo\
		${DESTDIR}${MANPREFIX}/man1/toxn.1\
		${DESTDIR}${MANPREFIX}/man1/txcli.1\
		${DESTDIR}${MANPREFIX}/man1/txinfo.1

.PHONY: all options clean dist install uninstall
