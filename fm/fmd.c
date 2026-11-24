#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include "fm_proto.h"
#include "fm_uart.h"

#include <errno.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <unistd.h>

#define SOCKET_ENV      "ANDROID_SOCKET_fmd"
#define SOCKET_PATH     "/dev/socket/fmd"
#define TTY_PATH        "/dev/ttySAC0"
#define RFKILL_PATH     "/sys/class/rfkill/rfkill0/state"
#define LNA_PATH        "/sys/devices/bluetooth/lna_en"
#define FIRMWARE_PATH   "/vendor/firmware/bcm4345C5_V0069.0172.hcd"

#define BAND_LOW_KHZ    87500
#define BAND_HIGH_KHZ   108000
#define DEFAULT_KHZ     100000

#define ARRAY_SIZE(a)   (sizeof(a) / sizeof((a)[0]))

/* Default Broadcom I2S/PCM pin routing. The FM audio is sent over the I2S
 * pins with the combo chip as the I2S slave: the Exynos I2S controller
 * drives the clocks and the s1402x codec is the clock consumer as well. */
static const uint8_t default_pins[FM_PIN_PAYLOAD_LEN] = {
    0x07, 0x19, 0x18, 0x19, 0x19,
};

struct fmd_state {
    struct fm_uart *uart;
    struct fm_transport *tp;
    uint8_t pins[FM_PIN_PAYLOAD_LEN];
    unsigned int freq_khz;
    int powered;
    int rds_on;
    const char *power_stage;
};

static struct fmd_state fmd;

static void reply(int fd, const char *fmt, ...)
{
    char buf[512];
    va_list ap;
    int n;

    va_start(ap, fmt);
    n = vsnprintf(buf, sizeof(buf) - 2, fmt, ap);
    va_end(ap);
    if (n < 0)
        return;
    buf[n++] = '\n';
    (void)!write(fd, buf, n);
}

static int valid_freq(unsigned int khz)
{
    return khz >= BAND_LOW_KHZ && khz <= BAND_HIGH_KHZ;
}

static int parse_u8(const char *s, uint8_t *out)
{
    char *end;
    long v;

    if (!s)
        return -1;
    errno = 0;
    v = strtol(s, &end, 0);
    if (errno || end == s || *end != '\0' || v < 0 || v > 255)
        return -1;
    *out = (uint8_t)v;
    return 0;
}

static void bytes_to_hex(const uint8_t *buf, int len, char *out, int outsz)
{
    static const char digits[] = "0123456789abcdef";
    int i, pos = 0;

    for (i = 0; i < len && pos + 3 < outsz; i++) {
        out[pos++] = digits[buf[i] >> 4];
        out[pos++] = digits[buf[i] & 0x0f];
        out[pos++] = ' ';
    }
    if (pos > 0)
        pos--;
    out[pos] = 0;
}

static void fm_power_down(void)
{
    if (!fmd.powered)
        return;
    fm_disable(fmd.tp);
    fm_uart_close(fmd.uart);
    fmd.uart = NULL;
    fmd.tp = NULL;
    fmd.powered = 0;
    fmd.rds_on = 0;
}

static int fm_power_up(void)
{
    if (fmd.powered)
        return 0;

    fmd.power_stage = "transport";
    errno = 0;
    fmd.uart = fm_uart_open(TTY_PATH, RFKILL_PATH, LNA_PATH, FIRMWARE_PATH);
    if (!fmd.uart)
        return errno ? -errno : -EIO;
    fmd.tp = fm_uart_transport(fmd.uart);

    fmd.power_stage = "fm-enable";
    if (fm_enable(fmd.tp, 1) < 0)
        goto fail;
    fmd.power_stage = "band";
    if (fm_set_band_europe(fmd.tp) < 0)
        goto fail;
    fmd.power_stage = "audio";
    if (fm_set_audio(fmd.tp, 1, 0, 0) < 0)
        goto fail;
    fmd.power_stage = "pins";
    if (fm_set_pins(fmd.tp, fmd.pins) < 0)
        goto fail;

    fmd.powered = 1;
    fmd.rds_on = 1;
    return 0;

fail:
    fm_disable(fmd.tp);
    fm_uart_close(fmd.uart);
    fmd.uart = NULL;
    fmd.tp = NULL;
    return -EIO;
}

static int selftest_tune_reg(struct fm_transport *t, uint16_t reg, uint16_t *flags)
{
    uint8_t evt[300];
    int n, tries;

    if (fm_reg_write16(t, FM_REG_FM_FREQ, reg) < 0)
        return -1;
    if (fm_reg_write16(t, FM_REG_FM_RDS_MSK,
                       FM_MASK_TUNE_CMPL | FM_MASK_TUNE_FAIL) < 0)
        return -1;
    if (fm_reg_write8(t, FM_REG_SCH_TUNE, FM_TUNE_MODE_PRESET) < 0)
        return -1;
    for (tries = 0; tries < 8; tries++) {
        n = t->wait_event(t->priv, evt, sizeof(evt), 4000);
        if (n < 0)
            return -1;
        if (n > 2 && evt[0] == 0xff && evt[2] == FM_VSE_SUBCODE)
            break;
    }
    if (tries == 8)
        return -1;
    return fm_read_flags(t, flags);
}

static int selftest(void)
{
    static const unsigned int freqs[] = { 87500, 90000, 95000, 100000, 105000, 108000 };
    struct fm_transport *t;
    char name[64] = "";
    unsigned int i, found = 0, got = 0;
    int rssi = 0, snr = 0, rc;

    if (fm_power_up() < 0) {
        fprintf(stderr, "selftest: power-up failed at %s\n",
                fmd.power_stage ? fmd.power_stage : "?");
        return 1;
    }

    t = fmd.tp;
    fm_uart_chip_name(fmd.uart, name, sizeof(name));
    printf("chip=%s\n", name);
    printf("band=%d audio=%d pins=%d\n",
           fm_set_band_europe(t), fm_set_audio(t, 1, 0, 0),
           fm_set_pins(t, fmd.pins));

    for (i = 0; i < (unsigned int)ARRAY_SIZE(freqs); i++) {
        uint16_t flags = 0;
        uint16_t reg10k = (uint16_t)(freqs[i] / 10 - 6400);
        uint16_t regkhz = (uint16_t)(freqs[i] - 64000);
        uint8_t rb[2] = { 0 };

        rssi = 0;
        snr = 0;
        got = 0;
        rc = selftest_tune_reg(t, reg10k, &flags);
        if (rc == 0 && !(flags & FM_MASK_TUNE_FAIL))
            printf("tune=%u fmt=10k reg=0x%04x flags=0x%04x\n",
                   freqs[i], (unsigned)reg10k, (unsigned)flags);
        else {
            rc = selftest_tune_reg(t, regkhz, &flags);
            if (rc == 0 && !(flags & FM_MASK_TUNE_FAIL))
                printf("tune=%u fmt=khz reg=0x%04x flags=0x%04x\n",
                       freqs[i], (unsigned)regkhz, (unsigned)flags);
            else {
                printf("tune=%u failed rc=%d flags=0x%04x\n",
                       freqs[i], rc, (unsigned)flags);
                continue;
            }
        }
        if (fm_reg_read(t, FM_REG_FM_FREQ, 2, rb, sizeof(rb)) == 2)
            got = (unsigned)rb[0] | ((unsigned)rb[1] << 8);
        fm_get_rssi(t, &rssi);
        fm_get_snr(t, &snr);
        printf("  readback_reg=0x%04x rssi=%d snr=%d\n", got, rssi, snr);
    }

    if (fm_seek(t, 100000, 1, &found) < 0)
        printf("seek=failed\n");
    else
        printf("seek=ok %u\n", found);

    fm_power_down();
    return 0;
}

static void handle_pincfg(int fd, int ntok, char **tok)
{
    uint8_t pins[FM_PIN_PAYLOAD_LEN];
    uint8_t raw[64];
    char hex[3 * sizeof(raw) + 1];
    int i, n;

    if (ntok == 1) {
        n = fm_read_pins(fmd.tp, raw, sizeof(raw));
        if (n < 0) {
            reply(fd, "err");
            return;
        }
        bytes_to_hex(raw, n, hex, sizeof(hex));
        reply(fd, "ok %s", hex);
        return;
    }
    if (ntok != 1 + FM_PIN_PAYLOAD_LEN) {
        reply(fd, "err usage: pincfg [b0 b1 b2 b3 b4]");
        return;
    }
    for (i = 0; i < FM_PIN_PAYLOAD_LEN; i++) {
        if (parse_u8(tok[i + 1], &pins[i]) < 0) {
            reply(fd, "err bad-byte");
            return;
        }
    }
    if (fm_set_pins(fmd.tp, pins) < 0) {
        reply(fd, "err");
        return;
    }
    memcpy(fmd.pins, pins, sizeof(fmd.pins));
    reply(fd, "ok");
}

static void handle_pcmroute(int fd, const char *arg)
{
    uint8_t val;
    int n;

    if (!arg) {
        n = fm_reg_read(fmd.tp, FM_REG_PCM_ROUTE, 1, &val, sizeof(val));
        if (n < 1) {
            reply(fd, "err");
            return;
        }
        reply(fd, "ok 0x%02x", val);
        return;
    }
    if (parse_u8(arg, &val) < 0) {
        reply(fd, "err bad-value");
        return;
    }
    reply(fd, fm_reg_write8(fmd.tp, FM_REG_PCM_ROUTE, val) < 0 ? "err" : "ok");
}

static void handle_command(int fd, char *line)
{
    char *tok[8];
    char *p;
    int ntok = 0;
    const char *cmd;

    for (p = strtok(line, " \t"); p != NULL && ntok < (int)ARRAY_SIZE(tok);
         p = strtok(NULL, " \t"))
        tok[ntok++] = p;
    if (ntok == 0)
        return;
    cmd = tok[0];

    if (!strcmp(cmd, "open")) {
        int rc = fm_power_up();
        char name[64] = "";

        if (rc < 0) {
            if (rc == -EBUSY)
                reply(fd, "err bt-enabled");
            else
                reply(fd, "err power-up-failed %s", fmd.power_stage);
            return;
        }
        fm_uart_chip_name(fmd.uart, name, sizeof(name));
        reply(fd, "ok %s", name[0] ? name : "bcm4345C5");
        return;
    }

    if (!strcmp(cmd, "close")) {
        fm_power_down();
        reply(fd, "ok");
        return;
    }

    if (!fmd.powered) {
        reply(fd, "err not-open");
        return;
    }

    if (!strcmp(cmd, "tune")) {
        unsigned int khz = ntok > 1 ? (unsigned int)strtoul(tok[1], NULL, 10)
                                    : DEFAULT_KHZ;
        if (!valid_freq(khz)) {
            reply(fd, "err bad-freq");
            return;
        }
        if (fm_tune(fmd.tp, khz) < 0) {
            reply(fd, "err tune-failed");
            return;
        }
        fmd.freq_khz = khz;
        reply(fd, "ok %u", khz);
    } else if (!strcmp(cmd, "seek")) {
        unsigned int found = 0;
        unsigned int start = ntok > 2 ? (unsigned int)strtoul(tok[2], NULL, 10)
                                      : fmd.freq_khz;
        int up = ntok < 2 || !strcmp(tok[1], "up");
        if (!valid_freq(start))
            start = DEFAULT_KHZ;
        if (fm_seek(fmd.tp, start, up, &found) < 0) {
            reply(fd, "err seek-failed");
            return;
        }
        fmd.freq_khz = found;
        reply(fd, "ok %u", found);
    } else if (!strcmp(cmd, "smtune")) {
        unsigned int khz = ntok > 1 ? (unsigned int)strtoul(tok[1], NULL, 10) : 0;
        int rssi = 0, snr = 0;

        if (!valid_freq(khz)) {
            reply(fd, "err bad-freq");
            return;
        }
        if (fm_tune(fmd.tp, khz) < 0) {
            reply(fd, "err tune-failed");
            return;
        }
        fmd.freq_khz = khz;
        if (fm_get_rssi(fmd.tp, &rssi) < 0) {
            reply(fd, "err");
            return;
        }
        fm_get_snr(fmd.tp, &snr);
        reply(fd, "ok %u %d %d", khz, rssi, snr);
    } else if (!strcmp(cmd, "freq")) {
        unsigned int khz = 0;
        if (fm_get_frequency(fmd.tp, &khz) < 0)
            reply(fd, "err");
        else
            reply(fd, "ok %u", khz);
    } else if (!strcmp(cmd, "rssi")) {
        int rssi = 0;
        if (fm_get_rssi(fmd.tp, &rssi) < 0)
            reply(fd, "err");
        else
            reply(fd, "ok %d", rssi);
    } else if (!strcmp(cmd, "snr")) {
        int snr = 0;
        if (fm_get_snr(fmd.tp, &snr) < 0)
            reply(fd, "err");
        else
            reply(fd, "ok %d", snr);
    } else if (!strcmp(cmd, "mute")) {
        int on = ntok > 1 ? atoi(tok[1]) : 1;
        reply(fd, fm_set_mute(fmd.tp, on) < 0 ? "err" : "ok");
    } else if (!strcmp(cmd, "volume")) {
        int vol = ntok > 1 ? atoi(tok[1]) : 128;
        if (vol < 0)
            vol = 0;
        if (vol > 255)
            vol = 255;
        reply(fd, fm_set_volume(fmd.tp, (uint8_t)vol) < 0 ? "err" : "ok");
    } else if (!strcmp(cmd, "rds")) {
        int on = ntok > 1 ? atoi(tok[1]) : 1;
        if (fm_enable_rds(fmd.tp, on) < 0) {
            reply(fd, "err");
            return;
        }
        fmd.rds_on = on;
        reply(fd, "ok");
    } else if (!strcmp(cmd, "audio")) {
        int i2s = ntok < 2 || !strcmp(tok[1], "i2s") || !strcmp(tok[1], "both");
        int dac = ntok > 1 && (!strcmp(tok[1], "dac") || !strcmp(tok[1], "both"));
        reply(fd, fm_set_audio(fmd.tp, i2s, dac, 0) < 0 ? "err" : "ok");
    } else if (!strcmp(cmd, "flags")) {
        uint16_t flags = 0;
        if (fm_read_flags(fmd.tp, &flags) < 0)
            reply(fd, "err");
        else
            reply(fd, "ok 0x%04x", flags);
    } else if (!strcmp(cmd, "pincfg")) {
        handle_pincfg(fd, ntok, tok);
    } else if (!strcmp(cmd, "pcmroute")) {
        handle_pcmroute(fd, ntok > 1 ? tok[1] : NULL);
    } else if (!strcmp(cmd, "status")) {
        reply(fd, "ok on %u rds %d", fmd.freq_khz, fmd.rds_on);
    } else {
        reply(fd, "err unknown-command");
    }
}

static void serve_client(int fd)
{
    char buf[512];
    int used = 0;

    for (;;) {
        int n = read(fd, buf + used, (int)sizeof(buf) - used - 1);
        char *nl;
        if (n <= 0)
            return;
        used += n;
        buf[used] = 0;
        while ((nl = strchr(buf, '\n')) != NULL) {
            int consumed;
            *nl = 0;
            if (nl > buf && nl[-1] == '\r')
                nl[-1] = 0;
            if (!strcmp(buf, "quit"))
                return;
            handle_command(fd, buf);
            consumed = (int)(nl - buf) + 1;
            memmove(buf, buf + consumed, used - consumed + 1);
            used -= consumed;
        }
        if (used >= (int)sizeof(buf) - 1)
            used = 0;
    }
}

static int listen_socket(const char *path)
{
    struct sockaddr_un addr;
    const char *env = getenv(SOCKET_ENV);
    int fd;

    if (env) {
        fd = atoi(env);
        if (fd > 2 && listen(fd, 4) == 0)
            return fd;
    }
    fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd < 0)
        return -1;
    unlink(path);
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, path, sizeof(addr.sun_path) - 1);
    if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        close(fd);
        return -1;
    }
    chmod(path, 0660);
    if (listen(fd, 4) < 0) {
        close(fd);
        return -1;
    }
    return fd;
}

int main(int argc, char **argv)
{
    const char *path = argc > 1 ? argv[1] : SOCKET_PATH;
    int lfd;

    signal(SIGPIPE, SIG_IGN);

    memcpy(fmd.pins, default_pins, sizeof(fmd.pins));
    fmd.freq_khz = DEFAULT_KHZ;

    if (argc > 1 && !strcmp(argv[1], "--selftest"))
        return selftest();

    lfd = listen_socket(path);
    if (lfd < 0) {
        fprintf(stderr, "fmd: cannot listen on %s: %s\n", path, strerror(errno));
        return 1;
    }

    for (;;) {
        int cfd = accept(lfd, NULL, NULL);
        if (cfd < 0) {
            if (errno == EINTR)
                continue;
            break;
        }
        serve_client(cfd);
        close(cfd);
        fm_power_down();
    }
    close(lfd);
    return 0;
}
