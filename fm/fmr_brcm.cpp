/*
 * Broadcom BCM4345C5 FM radio backend for the LineageOS FMRadio application.
 *
 * The combo chip is driven over its HCI UART by the fmd daemon; this file is
 * the libfmjni glue that turns the FM JNI callback table into fmd commands.
 *
 * Copyright (C) 2014 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <errno.h>
#include <poll.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/un.h>
#include <unistd.h>

#include "fmr.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "FMLIB_BRCM"

#define FMD_SOCKET_PATH     "/dev/socket/fmd"
#define FMD_TIMEOUT_MS      25000
#define FMD_VALID_RSSI_DBM  (-103)
#define BRCM_CHIP_ID        0x4356

static int fmd_write_all(int fd, const char *buf, int len)
{
    int done = 0;

    while (done < len) {
        ssize_t n = send(fd, buf + done, len - done, MSG_NOSIGNAL);
        if (n <= 0) {
            if (n < 0 && errno == EINTR)
                continue;
            return -1;
        }
        done += (int)n;
    }
    return 0;
}

static int fmd_read_line(int fd, char *buf, int bufsz)
{
    int pos = 0;

    for (;;) {
        char c;
        ssize_t n;

        if (pos >= bufsz - 1)
            return -1;
        n = recv(fd, &c, 1, 0);
        if (n <= 0) {
            if (n < 0 && errno == EINTR)
                continue;
            return -1;
        }
        if (c == '\n')
            break;
        if (c != '\r')
            buf[pos++] = c;
    }
    buf[pos] = 0;
    return 0;
}

static int fmd_command(int fd, char *reply, int replysz, const char *fmt, ...)
{
    char cmd[256];
    char line[256];
    va_list ap;
    int n;

    va_start(ap, fmt);
    n = vsnprintf(cmd, sizeof(cmd), fmt, ap);
    va_end(ap);
    if (n < 0 || n >= (int)sizeof(cmd))
        return -1;

    if (fmd_write_all(fd, cmd, n) < 0 || fmd_write_all(fd, "\n", 1) < 0)
        return -1;
    if (fmd_read_line(fd, line, sizeof(line)) < 0)
        return -1;
    if (strncmp(line, "ok", 2) != 0) {
        LOGD("fmd \"%s\" -> \"%s\"", cmd, line);
        return -1;
    }
    if (reply && replysz > 0) {
        strncpy(reply, line, (size_t)replysz - 1);
        reply[replysz - 1] = 0;
    }
    return 0;
}

static int brcm_open_dev(const char *pname, int *fd)
{
    struct sockaddr_un addr;
    struct timeval tv;
    int s;

    (void)pname;

    if (fd == NULL)
        return -ERR_INVALID_PARA;

    s = socket(AF_UNIX, SOCK_STREAM, 0);
    if (s < 0) {
        LOGE("%s: socket: %s", __func__, strerror(errno));
        return -1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, FMD_SOCKET_PATH, sizeof(addr.sun_path) - 1);
    if (connect(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        LOGE("%s: connect(%s): %s", __func__, FMD_SOCKET_PATH, strerror(errno));
        close(s);
        return -1;
    }

    tv.tv_sec = FMD_TIMEOUT_MS / 1000;
    tv.tv_usec = (FMD_TIMEOUT_MS % 1000) * 1000;
    setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    setsockopt(s, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));

    *fd = s;
    return 0;
}

static int brcm_close_dev(int fd)
{
    fmd_command(fd, NULL, 0, "close");
    return close(fd);
}

static int brcm_pwr_up(int fd, int band, int freq)
{
    (void)band;

    if (fmd_command(fd, NULL, 0, "open") < 0)
        return -1;
    if (freq > 0 && fmd_command(fd, NULL, 0, "tune %u", (unsigned int)freq * 100) < 0)
        return -1;
    return 0;
}

static int brcm_pwr_down(int fd, int type)
{
    (void)type;
    return fmd_command(fd, NULL, 0, "close");
}

static int brcm_tune(int fd, int freq, int band)
{
    (void)band;
    return fmd_command(fd, NULL, 0, "tune %u", (unsigned int)freq * 100);
}

static int brcm_seek(int fd, int *freq, int band, int dir, int lev)
{
    char reply[128];
    unsigned int khz;

    (void)band;
    (void)lev;

    if (freq == NULL)
        return -ERR_INVALID_PARA;
    if (fmd_command(fd, reply, sizeof(reply), "seek %s %u",
                    dir ? "up" : "down", (unsigned int)*freq * 100) < 0)
        return -1;
    if (sscanf(reply, "ok %u", &khz) != 1)
        return -1;
    *freq = (int)(khz / 100);
    return 0;
}

static int brcm_set_mute(int fd, int mute)
{
    return fmd_command(fd, NULL, 0, "mute %d", mute ? 1 : 0);
}

/*
 * RDS is not decoded by fmd yet. The application still probes for support
 * and polls for data, so report the feature and answer every data request
 * with "no event" instead of failing the whole FM session.
 */
static int brcm_is_rdsrx_support(int fd, int *supt)
{
    (void)fd;
    if (supt == NULL)
        return -ERR_INVALID_PARA;
    *supt = 1;
    return 0;
}

static int brcm_turn_on_off_rds(int fd, int onoff)
{
    return fmd_command(fd, NULL, 0, "rds %d", onoff == FMR_RDS_ON ? 1 : 0);
}

static int brcm_get_chip_id(int fd, int *chipid)
{
    (void)fd;
    if (chipid == NULL)
        return -ERR_INVALID_PARA;
    *chipid = BRCM_CHIP_ID;
    return 0;
}

static int brcm_read_rds_data(int fd, RDSData_Struct *rds, uint16_t *rds_status)
{
    (void)fd;
    (void)rds;
    if (rds_status == NULL)
        return -ERR_INVALID_PARA;
    *rds_status = 0;
    return 0;
}

static int brcm_get_ps(int fd, RDSData_Struct *rds, uint8_t **ps, int *ps_len)
{
    (void)fd;
    (void)rds;
    (void)ps;
    (void)ps_len;
    return -1;
}

static int brcm_get_rt(int fd, RDSData_Struct *rds, uint8_t **rt, int *rt_len)
{
    (void)fd;
    (void)rds;
    (void)rt;
    (void)rt_len;
    return -1;
}

static int brcm_active_af(int fd, RDSData_Struct *rds, int band,
                          uint16_t cur_freq, uint16_t *ret_freq)
{
    (void)fd;
    (void)rds;
    (void)band;
    (void)cur_freq;
    (void)ret_freq;
    return -1;
}

static int brcm_ana_switch(int fd, int antenna)
{
    (void)fd;
    (void)antenna;
    return 0;
}

static int brcm_soft_mute_tune(int fd, fm_softmute_tune_t *para)
{
    char reply[160];
    unsigned int khz;
    int rssi;
    int snr;

    if (para == NULL)
        return -ERR_INVALID_PARA;

    /* para->freq is in 10 kHz units (8750 for 87.5 MHz), fmd wants kHz. */
    if (fmd_command(fd, reply, sizeof(reply), "smtune %u",
                    (unsigned int)para->freq * 10) < 0)
        return -1;
    if (sscanf(reply, "ok %u %d %d", &khz, &rssi, &snr) != 3)
        return -1;

    para->rssi = rssi;
    para->valid = (rssi > FMD_VALID_RSSI_DBM) ? fm_true : fm_false;
    return 0;
}

static int brcm_desense_check(int fd, int freq, int rssi)
{
    (void)fd;
    (void)freq;
    (void)rssi;
    return 0;
}

static int brcm_pre_search(int fd)
{
    fmd_command(fd, NULL, 0, "mute 1");
    return 0;
}

static int brcm_restore_search(int fd)
{
    fmd_command(fd, NULL, 0, "mute 0");
    return 0;
}

void FM_interface_init(struct fm_cbk_tbl *cbk_tbl)
{
    if (cbk_tbl == NULL)
        return;

    memset(cbk_tbl, 0, sizeof(*cbk_tbl));
    cbk_tbl->open_dev = brcm_open_dev;
    cbk_tbl->close_dev = brcm_close_dev;
    cbk_tbl->pwr_up = brcm_pwr_up;
    cbk_tbl->pwr_down = brcm_pwr_down;
    cbk_tbl->seek = brcm_seek;
    cbk_tbl->tune = brcm_tune;
    cbk_tbl->set_mute = brcm_set_mute;
    cbk_tbl->is_rdsrx_support = brcm_is_rdsrx_support;
    cbk_tbl->turn_on_off_rds = brcm_turn_on_off_rds;
    cbk_tbl->get_chip_id = brcm_get_chip_id;
    cbk_tbl->read_rds_data = brcm_read_rds_data;
    cbk_tbl->get_ps = brcm_get_ps;
    cbk_tbl->get_rt = brcm_get_rt;
    cbk_tbl->active_af = brcm_active_af;
    cbk_tbl->ana_switch = brcm_ana_switch;
    cbk_tbl->soft_mute_tune = brcm_soft_mute_tune;
    cbk_tbl->desense_check = brcm_desense_check;
    cbk_tbl->pre_search = brcm_pre_search;
    cbk_tbl->restore_search = brcm_restore_search;
}
