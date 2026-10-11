/* Unicode scalar I/O. Terminal and internal strings are always UTF-8. */
#include <stdio.h>
#include <iconv.h>
#include "psy.h"

int psy_encoding = PSY_UTF8;
int psy_read_encoding = PSY_AUTO;

static int utf8_length(int c)
{
    if (c < 0x80) return 1;
    if (c >= 0xc2 && c <= 0xdf) return 2;
    if (c >= 0xe0 && c <= 0xef) return 3;
    if (c >= 0xf0 && c <= 0xf4) return 4;
    return 0;
}

static int decode(const char_t *bytes, int n)
{
    int c, i;
    if (n == 1) return bytes[0];
    c = bytes[0] & ((1 << (7 - n)) - 1);
    for (i = 1; i < n; i++) {
        if ((bytes[i] & 0xc0) != 0x80) return PSY_INVALID;
        c = (c << 6) | (bytes[i] & 0x3f);
    }
    if ((n == 2 && c < 0x80) || (n == 3 && c < 0x800) ||
        (n == 4 && c < 0x10000) || c > 0x10ffff ||
        (c >= 0xd800 && c <= 0xdfff)) return PSY_INVALID;
    return c;
}

int psy_utf8_encode(int c, char_t out[4])
{
    if (c < 0 || c > 0x10ffff || (c >= 0xd800 && c <= 0xdfff)) return 0;
    if (c < 0x80) { out[0] = c; return 1; }
    if (c < 0x800) {
        out[0] = 0xc0 | (c >> 6); out[1] = 0x80 | (c & 63); return 2;
    }
    if (c < 0x10000) {
        out[0] = 0xe0 | (c >> 12); out[1] = 0x80 | ((c >> 6) & 63);
        out[2] = 0x80 | (c & 63); return 3;
    }
    out[0] = 0xf0 | (c >> 18); out[1] = 0x80 | ((c >> 12) & 63);
    out[2] = 0x80 | ((c >> 6) & 63); out[3] = 0x80 | (c & 63); return 4;
}

int psy_next_char(char_t **s)
{
    int n = utf8_length(**s), i, c;
    if (!n) { ++*s; return PSY_INVALID; }
    for (i = 1; i < n; i++)
        if (!(*s)[i]) { ++*s; return PSY_INVALID; }
    c = decode(*s, n);
    *s += n;
    return c;
}

static int convert_char(iconv_t cd, char *in, size_t in_size,
                        char *out, size_t *out_size)
{
    size_t result;
    if (cd == (iconv_t)-1) return 0;
    iconv(cd, NULL, NULL, NULL, NULL);
    result = iconv(cd, &in, &in_size, &out, out_size);
    return result != (size_t)-1 && result == 0 && in_size == 0;
}

int psy_read_char(FILE *fp, int encoding)
{
    char_t bytes[4];
    int c = fp == stdin ? psy_terminal_byte() : getc(fp), n, i;
    if (c == EOF) return EOF;
    bytes[0] = c;
    if (encoding == PSY_UTF8) {
        n = utf8_length(c);
        if (!n) return PSY_INVALID;
        for (i = 1; i < n; i++) {
            c = fp == stdin ? psy_terminal_byte() : getc(fp);
            if (c == EOF) return PSY_INVALID;
            if ((c & 0xc0) != 0x80) { if (fp == stdin) psy_terminal_unread(c); else ungetc(c, fp); return PSY_INVALID; }
            bytes[i] = c;
        }
        return decode(bytes, n);
    } else {
        static iconv_t cd = (iconv_t)-1;
        unsigned char out[4];
        size_t remaining = sizeof(out);
        n = ((c >= 0x81 && c <= 0x9f) || (c >= 0xe0 && c <= 0xfc)) ? 2 : 1;
        if (n == 2) {
            c = fp == stdin ? psy_terminal_byte() : getc(fp);
            if (c == EOF) return PSY_INVALID;
            bytes[1] = c;
        }
        if (cd == (iconv_t)-1) cd = iconv_open("UTF-32LE", "CP932");
        if (!convert_char(cd, (char *)bytes, n, (char *)out, &remaining) || remaining)
            return PSY_INVALID;
        return out[0] | (out[1] << 8) | (out[2] << 16) | (out[3] << 24);
    }
}

int psy_write_char(int c, FILE *fp, int encoding)
{
    char_t bytes[4];
    int n;
    if (encoding == PSY_UTF8) n = psy_utf8_encode(c, bytes);
    else {
        static iconv_t cd = (iconv_t)-1;
        unsigned char in[4];
        size_t remaining = sizeof(bytes);
        in[0] = c; in[1] = c >> 8; in[2] = c >> 16; in[3] = c >> 24;
        if (cd == (iconv_t)-1) cd = iconv_open("CP932", "UTF-32LE");
        if (!convert_char(cd, (char *)in, sizeof(in), (char *)bytes, &remaining))
            return EOF;
        n = sizeof(bytes) - remaining;
    }
    if (!n || fwrite(bytes, 1, n, fp) != (size_t)n) return EOF;
    return c;
}

int psy_getwc(FILE *fp) { return psy_read_char(fp, PSY_UTF8); }
int psy_putwc(int c, FILE *fp) { return psy_write_char(c, fp, PSY_UTF8); }
int psy_ungetwc(int c, FILE *fp)
{
    char_t bytes[4];
    int n = psy_utf8_encode(c, bytes);
    if (!n) return EOF;
    while (n) if (ungetc(bytes[--n], fp) == EOF) return EOF;
    return c;
}

int psy_char_width(int c)
{
    int width = wcwidth((wchar_t)c);
    return width < 0 ? 1 : width;
}

/* UTF-8 wins when both encodings are valid. ASCII is identical in both. */
int psy_detect_encoding(FILE *fp)
{
    int c, valid = 1;
    while ((c = psy_read_char(fp, PSY_UTF8)) != EOF) {
        if (c == PSY_INVALID || !c) { valid = 0; break; }
    }
    if (ferror(fp)) return PSY_INVALID;
    rewind(fp);
    if (valid) return PSY_UTF8;
    while ((c = psy_read_char(fp, PSY_SJIS)) != EOF) {
        if (c == PSY_INVALID || !c) { rewind(fp); return PSY_INVALID; }
    }
    if (ferror(fp)) return PSY_INVALID;
    rewind(fp);
    return PSY_SJIS;
}

FILE *psy_config_utf8(FILE *fp)
{
    FILE *converted;
    int c, encoding = psy_detect_encoding(fp);
    if (encoding == PSY_INVALID) { fclose(fp); return NULL; }
    converted = tmpfile();
    if (!converted) { fclose(fp); return NULL; }
    while ((c = psy_read_char(fp, encoding)) != EOF) {
        if (c == PSY_INVALID || !c || psy_putwc(c, converted) == EOF) {
            fclose(fp); fclose(converted); return NULL;
        }
    }
    if (ferror(fp) || fflush(converted)) {
        fclose(fp); fclose(converted); return NULL;
    }
    fclose(fp);
    rewind(converted);
    return converted;
}
