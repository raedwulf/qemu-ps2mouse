/*
 * QEMU System Emulator
 *
 * Copyright (c) 2003-2008 Fabrice Bellard
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "net/tap.h"
#include "qemu-common.h"
#include "sysemu.h"
#include "qemu-error.h"

#ifdef __NetBSD__
#include <net/if_tap.h>
#endif

#if defined(__FreeBSD__) || defined(__FreeBSD_kernel__) || defined(__DragonFly__)
#include <libutil.h>
#else
#include <util.h>
#endif

int tap_open(char *ifname, int ifname_size, int *vnet_hdr, int vnet_hdr_required)
{
    int fd;
    char *dev;
    struct stat s;

#if defined(__FreeBSD__) || defined(__FreeBSD_kernel__)
    /* if no ifname is given, always start the search from tap0. */
    int i;
    char dname[100];

    for (i = 0; i < 10; i++) {
        if (*ifname) {
            snprintf(dname, sizeof dname, "/dev/%s", ifname);
        } else {
            snprintf(dname, sizeof dname, "/dev/tap%d", i);
        }
        TFR(fd = open(dname, O_RDWR));
        if (fd >= 0) {
            break;
        }
        else if (errno == ENXIO || errno == ENOENT) {
            break;
        }
        if (*ifname) {
            break;
        }
    }
    if (fd < 0) {
        error_report("warning: could not open %s (%s): no virtual network emulation",
                   dname, strerror(errno));
        return -1;
    }
#else
    TFR(fd = open("/dev/tap", O_RDWR));
    if (fd < 0) {
        fprintf(stderr, "warning: could not open /dev/tap: no virtual network emulation\n");
        return -1;
    }
#endif

    fstat(fd, &s);
    dev = devname(s.st_rdev, S_IFCHR);
    pstrcpy(ifname, ifname_size, dev);

    if (*vnet_hdr) {
        /* BSD doesn't have IFF_VNET_HDR */
        *vnet_hdr = 0;

        if (vnet_hdr_required && !*vnet_hdr) {
            error_report("vnet_hdr=1 requested, but no kernel "
                         "support for IFF_VNET_HDR available");
            close(fd);
            return -1;
        }
    }
    fcntl(fd, F_SETFL, O_NONBLOCK);
    return fd;
}

int tap_set_sndbuf(int fd, QemuOpts *opts)
{
    return 0;
}

int tap_probe_vnet_hdr(int fd)
{
    return 0;
}

int tap_probe_has_ufo(int fd)
{
    return 0;
}

int tap_probe_vnet_hdr_len(int fd, int len)
{
    return 0;
}

void tap_fd_set_vnet_hdr_len(int fd, int len)
{
}

void tap_fd_set_offload(int fd, int csum, int tso4,
                        int tso6, int ecn, int ufo)
{
}
