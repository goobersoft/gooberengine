/* Deflate, both ways, without a library (RFC 1951), and the gzip wrapper (RFC 1952).
 *
 * The decoder reads all three block kinds (stored, fixed and dynamic Huffman), so
 * it opens any .zip or .tar.gz that zip, tar, 7-Zip or a file manager writes.
 * It follows the canonical-code method of zlib's "puff": slow-ish, small, and
 * plenty for carts, which are a few hundred kilobytes at most.
 *
 * The encoder is LZ77 (hash chains over the 32K window) with the fixed Huffman
 * codes: no code tables to build or send. Text, which a cart mostly is, comes out
 * at a third to a half of its size. */
#include "engine.h"
#include <stdlib.h>
#include <string.h>

#define MAXBITS 15
#define MAXLCODES 286
#define MAXDCODES 30
#define FIXLCODES 288

static const uint16_t len_base[29] = {3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31,
                                      35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258};
static const uint8_t len_extra[29] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2,
                                      3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0};
static const uint16_t dist_base[30] = {1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193,
                                       257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145, 8193, 12289, 16385, 24577};
static const uint8_t dist_extra[30] = {0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6,
                                       7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13};

/* ---- inflate ---- */

typedef struct {
    const uint8_t *in;
    size_t inlen, pos;
    uint32_t bitbuf;
    int bitcnt;
    uint8_t *out;
    size_t outlen, outcap, limit;
    const char *err;
} Inflate;

typedef struct { short count[MAXBITS + 1]; short symbol[FIXLCODES]; } Huff;

static int bits(Inflate *s, int need) {
    uint32_t v = s->bitbuf;
    while (s->bitcnt < need) {
        if (s->pos >= s->inlen) { s->err = "the compressed data is cut short"; return 0; }
        v |= (uint32_t)s->in[s->pos++] << s->bitcnt;
        s->bitcnt += 8;
    }
    s->bitbuf = v >> need;
    s->bitcnt -= need;
    return (int)(v & ((1u << need) - 1));
}

static bool put(Inflate *s, uint8_t c) {
    if (s->outlen == s->outcap) {
        if (s->outlen >= s->limit) { s->err = "a file in it is too big"; return false; }
        size_t cap = s->outcap ? s->outcap * 2 : 4096;
        if (cap > s->limit) cap = s->limit;
        uint8_t *o = realloc(s->out, cap);
        if (!o) { s->err = "out of memory"; return false; }
        s->out = o;
        s->outcap = cap;
    }
    s->out[s->outlen++] = c;
    return true;
}

static bool stored(Inflate *s) {
    s->bitbuf = 0;            /* to the next byte */
    s->bitcnt = 0;
    if (s->pos + 4 > s->inlen) { s->err = "the compressed data is cut short"; return false; }
    unsigned len = s->in[s->pos] | s->in[s->pos + 1] << 8;
    unsigned nlen = s->in[s->pos + 2] | s->in[s->pos + 3] << 8;
    s->pos += 4;
    if (len != (~nlen & 0xffff)) { s->err = "the compressed data is damaged"; return false; }
    if (s->pos + len > s->inlen) { s->err = "the compressed data is cut short"; return false; }
    while (len--) if (!put(s, s->in[s->pos++])) return false;
    return true;
}

/* one symbol: walk the canonical code a bit at a time */
static int decode(Inflate *s, const Huff *h) {
    int code = 0, first = 0, index = 0;
    for (int len = 1; len <= MAXBITS; len++) {
        code |= bits(s, 1);
        if (s->err) return -1;
        int count = h->count[len];
        if (code - count < first) return h->symbol[index + (code - first)];
        index += count;
        first = (first + count) << 1;
        code <<= 1;
    }
    s->err = "the compressed data is damaged";
    return -1;
}

/* code lengths -> table; returns < 0 for an over-subscribed set */
static int construct(Huff *h, const short *length, int n) {
    for (int len = 0; len <= MAXBITS; len++) h->count[len] = 0;
    for (int sym = 0; sym < n; sym++) h->count[length[sym]]++;
    if (h->count[0] == n) return 0;
    int left = 1;
    for (int len = 1; len <= MAXBITS; len++) {
        left <<= 1;
        left -= h->count[len];
        if (left < 0) return left;
    }
    short offs[MAXBITS + 1];
    offs[1] = 0;
    for (int len = 1; len < MAXBITS; len++) offs[len + 1] = (short)(offs[len] + h->count[len]);
    for (int sym = 0; sym < n; sym++)
        if (length[sym]) h->symbol[offs[length[sym]]++] = (short)sym;
    return left;
}

static bool codes(Inflate *s, const Huff *lencode, const Huff *distcode) {
    for (;;) {
        int sym = decode(s, lencode);
        if (sym < 0) return false;
        if (sym < 256) { if (!put(s, (uint8_t)sym)) return false; continue; }
        if (sym == 256) return true;
        sym -= 257;
        if (sym >= 29) { s->err = "the compressed data is damaged"; return false; }
        int len = len_base[sym] + bits(s, len_extra[sym]);
        int dsym = decode(s, distcode);
        if (dsym < 0) return false;
        if (dsym >= 30) { s->err = "the compressed data is damaged"; return false; }
        size_t dist = dist_base[dsym] + (size_t)bits(s, dist_extra[dsym]);
        if (s->err) return false;
        if (dist > s->outlen) { s->err = "the compressed data is damaged"; return false; }
        while (len--) if (!put(s, s->out[s->outlen - dist])) return false;
    }
}

static bool fixed(Inflate *s) {
    static Huff lencode, distcode;
    static bool built;
    if (!built) {
        short lengths[FIXLCODES];
        int sym = 0;
        for (; sym < 144; sym++) lengths[sym] = 8;
        for (; sym < 256; sym++) lengths[sym] = 9;
        for (; sym < 280; sym++) lengths[sym] = 7;
        for (; sym < FIXLCODES; sym++) lengths[sym] = 8;
        construct(&lencode, lengths, FIXLCODES);
        for (sym = 0; sym < MAXDCODES; sym++) lengths[sym] = 5;
        construct(&distcode, lengths, MAXDCODES);
        built = true;
    }
    return codes(s, &lencode, &distcode);
}

static bool dynamic(Inflate *s) {
    static const uint8_t order[19] = {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};
    short lengths[MAXLCODES + MAXDCODES];
    Huff lencode, distcode;
    int nlen = bits(s, 5) + 257, ndist = bits(s, 5) + 1, ncode = bits(s, 4) + 4;
    if (s->err) return false;
    if (nlen > MAXLCODES || ndist > MAXDCODES) { s->err = "the compressed data is damaged"; return false; }
    int index = 0;
    for (; index < ncode; index++) lengths[order[index]] = (short)bits(s, 3);
    for (; index < 19; index++) lengths[order[index]] = 0;
    if (s->err) return false;
    if (construct(&lencode, lengths, 19) != 0) { s->err = "the compressed data is damaged"; return false; }
    index = 0;
    while (index < nlen + ndist) {
        int sym = decode(s, &lencode);
        if (sym < 0) return false;
        if (sym < 16) { lengths[index++] = (short)sym; continue; }
        short len = 0;
        int rep;
        if (sym == 16) {
            if (!index) { s->err = "the compressed data is damaged"; return false; }
            len = lengths[index - 1];
            rep = 3 + bits(s, 2);
        } else if (sym == 17) rep = 3 + bits(s, 3);
        else rep = 11 + bits(s, 7);
        if (s->err) return false;
        if (index + rep > nlen + ndist) { s->err = "the compressed data is damaged"; return false; }
        while (rep--) lengths[index++] = len;
    }
    if (!lengths[256]) { s->err = "the compressed data is damaged"; return false; }
    int err = construct(&lencode, lengths, nlen);
    if (err < 0 || (err > 0 && nlen - lencode.count[0] != 1)) { s->err = "the compressed data is damaged"; return false; }
    err = construct(&distcode, lengths + nlen, ndist);
    if (err < 0 || (err > 0 && ndist - distcode.count[0] != 1)) { s->err = "the compressed data is damaged"; return false; }
    return codes(s, &lencode, &distcode);
}

uint8_t *ge_inflate(const uint8_t *in, size_t inlen, size_t limit, size_t *outlen, size_t *used, const char **err) {
    Inflate s = {.in = in, .inlen = inlen, .limit = limit};
    int last;
    do {
        last = bits(&s, 1);
        int type = bits(&s, 2);
        if (s.err) break;
        bool ok = type == 0 ? stored(&s) : type == 1 ? fixed(&s) : type == 2 ? dynamic(&s)
                                                                             : (s.err = "the compressed data is damaged", false);
        if (!ok) break;
    } while (!last);
    if (s.err) {
        free(s.out);
        if (err) *err = s.err;
        return NULL;
    }
    if (!s.out) s.out = malloc(1);
    *outlen = s.outlen;
    if (used) *used = s.pos;
    return s.out;
}

/* ---- deflate: LZ77 + the fixed codes ---- */

typedef struct { uint8_t *s; size_t n, cap; uint32_t bitbuf; int bitcnt; } BitOut;

static void out_byte(BitOut *b, uint8_t c) {
    if (b->n == b->cap) { b->cap = b->cap ? b->cap * 2 : 4096; b->s = realloc(b->s, b->cap); }
    b->s[b->n++] = c;
}
static void out_bits(BitOut *b, uint32_t v, int n) {     /* LSB first */
    b->bitbuf |= v << b->bitcnt;
    b->bitcnt += n;
    while (b->bitcnt >= 8) { out_byte(b, (uint8_t)b->bitbuf); b->bitbuf >>= 8; b->bitcnt -= 8; }
}
static void out_code(BitOut *b, uint32_t code, int n) {  /* Huffman codes go MSB first */
    uint32_t r = 0;
    for (int i = 0; i < n; i++) r |= ((code >> i) & 1) << (n - 1 - i);
    out_bits(b, r, n);
}
static void out_lit(BitOut *b, int sym) {
    if (sym < 144) out_code(b, 0x30 + (uint32_t)sym, 8);
    else if (sym < 256) out_code(b, 0x190 + (uint32_t)(sym - 144), 9);
    else if (sym < 280) out_code(b, (uint32_t)(sym - 256), 7);
    else out_code(b, 0xc0 + (uint32_t)(sym - 280), 8);
}
static void out_match(BitOut *b, int len, int dist) {
    int i = 28;
    while (len_base[i] > len) i--;
    out_lit(b, 257 + i);
    out_bits(b, (uint32_t)(len - len_base[i]), len_extra[i]);
    int d = 29;
    while (dist_base[d] > dist) d--;
    out_code(b, (uint32_t)d, 5);
    out_bits(b, (uint32_t)(dist - dist_base[d]), dist_extra[d]);
}

#define WSIZE 32768
#define HBITS 15
#define MAXCHAIN 128

uint8_t *ge_deflate(const uint8_t *in, size_t n, size_t *outlen) {
    BitOut b = {0};
    int32_t *head = malloc(sizeof(int32_t) << HBITS), *prev = malloc(sizeof(int32_t) * WSIZE);
    for (size_t i = 0; i < (1u << HBITS); i++) head[i] = -1;
    out_bits(&b, 1, 1);           /* one block, the last */
    out_bits(&b, 1, 2);           /* fixed codes */
#define HASH(p) ((((uint32_t)in[p] << 10) ^ ((uint32_t)in[p + 1] << 5) ^ in[p + 2]) & ((1u << HBITS) - 1))
#define INSERT(p) do { uint32_t h_ = HASH(p); prev[(p) & (WSIZE - 1)] = head[h_]; head[h_] = (int32_t)(p); } while (0)
    size_t i = 0;
    while (i < n) {
        int best = 0, bdist = 0;
        if (i + 3 <= n) {
            int32_t cand = head[HASH(i)];
            size_t maxlen = n - i < 258 ? n - i : 258;
            for (int chain = 0; cand >= 0 && i - (size_t)cand <= WSIZE - 1 && chain < MAXCHAIN; chain++) {
                size_t l = 0;
                while (l < maxlen && in[(size_t)cand + l] == in[i + l]) l++;
                if ((int)l > best) { best = (int)l; bdist = (int)(i - (size_t)cand); if (l == maxlen) break; }
                int32_t next = prev[cand & (WSIZE - 1)];
                if (next >= cand) break;   /* the slot was reused by a newer position */
                cand = next;
            }
        }
        if (best >= 3) {
            out_match(&b, best, bdist);
            for (int k = 0; k < best; k++, i++) if (i + 3 <= n) INSERT(i);
        } else {
            out_lit(&b, in[i]);
            if (i + 3 <= n) INSERT(i);
            i++;
        }
    }
#undef INSERT
#undef HASH
    out_lit(&b, 256);
    if (b.bitcnt) out_bits(&b, 0, 8 - b.bitcnt);
    free(head);
    free(prev);
    if (!b.s) b.s = malloc(1);
    *outlen = b.n;
    return b.s;
}

/* ---- gzip ---- */

uint8_t *ge_gunzip(const uint8_t *in, size_t n, size_t limit, size_t *outlen, const char **err) {
    if (n < 18 || in[0] != 0x1f || in[1] != 0x8b || in[2] != 8) { *err = "not gzip data"; return NULL; }
    int flg = in[3];
    size_t p = 10;
    if (flg & 4) { if (p + 2 > n) goto cut; p += 2 + (in[p] | (size_t)in[p + 1] << 8); }   /* FEXTRA */
    if (flg & 8) { while (p < n && in[p]) p++; p++; }                                     /* FNAME */
    if (flg & 16) { while (p < n && in[p]) p++; p++; }                                    /* FCOMMENT */
    if (flg & 2) p += 2;                                                                  /* FHCRC */
    if (p >= n) goto cut;
    size_t used;
    uint8_t *out = ge_inflate(in + p, n - p, limit, outlen, &used, err);
    if (!out) return NULL;
    p += used;
    if (p + 8 > n) { free(out); goto cut; }
    uint32_t crc = in[p] | (uint32_t)in[p + 1] << 8 | (uint32_t)in[p + 2] << 16 | (uint32_t)in[p + 3] << 24;
    if (crc != ge_crc32(out, *outlen)) { free(out); *err = "the compressed data is damaged (CRC)"; return NULL; }
    return out;
cut:
    *err = "the compressed data is cut short";
    return NULL;
}

uint8_t *ge_gzip(const uint8_t *in, size_t n, size_t *outlen) {
    size_t dn;
    uint8_t *d = ge_deflate(in, n, &dn);
    uint8_t *out = malloc(dn + 18);
    static const uint8_t hdr[10] = {0x1f, 0x8b, 8, 0, 0, 0, 0, 0, 0, 255};   /* mtime 0: reproducible */
    memcpy(out, hdr, 10);
    memcpy(out + 10, d, dn);
    free(d);
    uint32_t crc = ge_crc32(in, n), sz = (uint32_t)n;
    for (int i = 0; i < 4; i++) { out[10 + dn + i] = (uint8_t)(crc >> (8 * i)); out[14 + dn + i] = (uint8_t)(sz >> (8 * i)); }
    *outlen = dn + 18;
    return out;
}
