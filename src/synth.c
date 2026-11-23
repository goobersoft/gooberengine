/* The GooberEngine sound chip.
 *
 *   8 voices. Each plays one of 8 waveforms (32 samples of 0-15, drawn in
 *   the sound editor), an instrument, or pitched noise, at a pitch of 0-63
 *   semitones (C2 .. D#7) and a volume of 0-7.
 *
 *   Waves 8-15 are instruments: wave 8 + n plays sfx n at its own speed,
 *   restarting with each note (or sfx step). Its steps are moved by the
 *   note's pitch (a step at C-4 sounds at the note), its volumes scaled by
 *   the note's, and the note's effect goes on top. Its loop sustains it;
 *   without one it falls silent at its end. Inside an instrument, waves
 *   8-15 play drawn waves 0-7 (an instrument can't hold another).
 *
 *   A sound effect is up to 32 steps. Each step has a pitch, a wave, a
 *   volume and an effect, and lasts `speed` ticks (120 ticks a second).
 *   Effects: slide to the next step's pitch, vibrato, drop an octave, fade
 *   in, fade out, arpeggio (the step's pitch, +4, +7 semitones: a major chord).
 *
 * Waves are read without interpolation, so they sound crisp and buzzy.
 * This file only does arithmetic; platform.c runs synth_mix() on the audio
 * thread (or once per frame when headless), with audio_lock() around changes. */
#include "engine.h"
#include <math.h>
#include <string.h>

const char fx_chars[NFX + 1] = ".svd<>a";

typedef struct {
    bool on, releasing;
    int wave;
    double phase, freq;
    float vol, target;          /* current volume follows target smoothly (no clicks) */
    int sfx, step, tick;        /* sfx -1: a plain note, MUSIC: a music track */
    SfxStep cell;               /* music: the note this row started */
    int next_pitch;             /* music: the next note's pitch on this track (for slides), -1 none */
    int note_ticks;             /* note: ticks left, 0 = until stopped */
    long started;
    const SoundBank *bank;      /* the waves (and sfx) it plays from */
    int noise_idx;
    float noise_val;
} Voice;

#define MUSIC (-3)
static Voice voices[NVOICES];

static struct {
    bool on;
    int pos, pattern, row, tick;
    int loop_pattern;           /* >= 0: the editor is looping one pattern */
    const Song *song;           /* the song playing */
} mus;
static double tick_acc;
static float vol_all = 1, vol_music = 1, vol_sfx = 1;   /* the settings' sound levels */

void synth_set_volume(float all, float music, float sfx) { vol_all = all; vol_music = music; vol_sfx = sfx; }
static long start_counter;
static uint32_t lfsr = 0xACE1u;

static void release_music_voices(void);
static void sound(Voice *v, int wave, double p, double vol, int age);
static double pitch_freq(double p) { return 440.0 * pow(2.0, (p + 36 - 69) / 12.0); }

void synth_default_wave(int w, uint8_t *out) {
        for (int i = 0; i < WAVE_LEN; i++) {
            double t = i / (double)WAVE_LEN, v;
            switch (w % 8) {
                case 0: v = i < 16 ? 15 : 0; break;                             /* square */
                case 1: v = i < 8 ? 15 : 0; break;                              /* pulse 1/4 */
                case 2: v = i < 4 ? 15 : 0; break;                              /* pulse 1/8 */
                case 3: v = i < 16 ? i : 31 - i; break;                         /* triangle */
                case 4: v = i / 2; break;                                       /* saw */
                case 5: v = 7.5 + 7.5 * sin(2 * M_PI * t); break;              /* sine */
                case 6: v = 7.5 + 5.0 * sin(2 * M_PI * t) + 2.5 * sin(6 * M_PI * t); break;  /* organ */
                default: v = 7.5 + 7.5 * sin(2 * M_PI * t) * (i < 16 ? 1 : 0.3); break;     /* soft */
            }
            int s = (int)lround(v);
            out[i] = (uint8_t)(s < 0 ? 0 : s > 15 ? 15 : s);
        }
}

void synth_default_sfx(Sfx *s) {
    memset(s, 0, sizeof *s);
    s->speed = 8;
    s->length = SFX_STEPS;
    for (int k = 0; k < SFX_STEPS; k++) s->steps[k].pitch = 24;   /* C4 */
}

void music_defaults(Song *m) {
    memset(m->patterns, 0, sizeof m->patterns);
    for (int p = 0; p < NPATTERNS; p++)
        for (int r = 0; r < PATTERN_ROWS; r++)
            for (int t = 0; t < NTRACKS; t++) m->patterns[p].cells[r][t].pitch = 24;
    for (int p = 0; p < NPATTERNS; p++) m->patterns[p].length = PATTERN_ROWS;
    memset(m->order, 0, sizeof m->order);
    for (int i = 1; i < ORDER_LEN; i++) m->order[i].kind = ORDER_STOP;   /* entry 0: pattern 0 */
    m->song_speed = 8;
}

void synth_defaults(SoundBank *b) {
    for (int w = 0; w < NWAVES; w++) synth_default_wave(w, b->waves[w]);
    for (int n = 0; n < NSFX; n++) synth_default_sfx(&b->sfx[n]);
}

/* a bank or song is about to be freed: nothing may play from it any more */
void synth_forget(const void *bank) {
    for (int i = 0; i < NVOICES; i++)
        if (voices[i].bank == bank) { voices[i].on = false; voices[i].bank = NULL; }
    if (mus.song == bank) { mus.on = false; mus.song = NULL; release_music_voices(); }
}

/* ---- control (callers hold audio_lock) ---- */

static void start_voice(Voice *v) {
    v->bank = cur_snd;
    v->on = true;
    v->releasing = false;
    v->tick = 0;
    v->step = 0;
    v->started = ++start_counter;
}

int synth_sfx(int n, int voice) {
    if (n < 0 || n >= NSFX || !cur_snd) return -1;
    if (voice < 0) {            /* a free voice, else one the music is using, else the oldest sfx */
        for (int i = 0; i < NVOICES && voice < 0; i++) if (!voices[i].on || voices[i].releasing) voice = i;
        for (int i = NVOICES - 1; i >= 0 && voice < 0; i--) if (voices[i].sfx == MUSIC) voice = i;
        if (voice < 0) {
            voice = 0;
            for (int i = 1; i < NVOICES; i++) if (voices[i].started < voices[voice].started) voice = i;
        }
    }
    if (voice >= NVOICES) return -1;
    Voice *v = &voices[voice];
    start_voice(v);
    v->sfx = n;
    return voice;
}

void synth_note(int voice, int pitch, int wave, int vol, int ticks) {
    if (voice < 0 || voice >= NVOICES) return;
    Voice *v = &voices[voice];
    bool was = v->on && !v->releasing;
    start_voice(v);
    if (!was) v->vol = 0;
    v->sfx = -1;
    v->cell = (SfxStep){(uint8_t)pitch, (uint8_t)wave, (uint8_t)vol, FX_NONE};
    sound(v, wave, pitch, vol / 7.0, 0);
    v->note_ticks = ticks > 0 ? ticks : 0;
}

static void release(Voice *v) {
    v->releasing = true;
    v->target = 0;
}

void synth_stop(int voice) {
    if (voice < 0) mus.on = false;      /* stopping everything stops the music too */
    for (int i = 0; i < NVOICES; i++)
        if (voice < 0 || voice == i) release(&voices[i]);
}

int synth_voice_sfx(int voice) {
    if (voice < 0 || voice >= NVOICES || !voices[voice].on || voices[voice].releasing) return -1;
    return voices[voice].sfx >= 0 ? voices[voice].sfx : voices[voice].sfx == MUSIC ? -3 : -2;
}

int synth_voice_step(int voice) {
    if (voice < 0 || voice >= NVOICES) return 0;
    return voices[voice].step;
}

/* ---- the 120 Hz tick: step through sound effects and music ---- */

/* the pitch and volume (0-1) at tick `tick` of a step lasting `speed` ticks;
 * next_pitch is the following note's pitch (for slides), or -1 */
static void shape(const SfxStep *st, int next_pitch, int tick, int speed, int age, double *pitch, double *volume) {
    double t = (tick < speed ? tick : speed) / (double)speed;   /* 0..1 through the step, then held */
    double p = st->pitch;
    double vol = st->vol / 7.0;
    switch (st->fx) {
        case FX_SLIDE: if (next_pitch >= 0) p += (next_pitch - p) * t; break;
        case FX_VIBRATO: p += 0.5 * sin(2 * M_PI * 7.0 * age / TICK_HZ); break;
        case FX_DROP: p -= 12.0 * t; break;
        case FX_FADE_IN: vol *= t; break;
        case FX_FADE_OUT: vol *= 1.0 - t; break;
        case FX_ARPEGGIO: { static const int up[3] = {0, 4, 7}; p += up[(tick / 2) % 3]; break; }
    }
    *pitch = p;
    *volume = vol;
}

/* sets what a voice sounds: wave at pitch p and volume vol; an instrument wave
 * plays its sfx `age` ticks in, moved and scaled by p and vol */
static void sound(Voice *v, int wave, double p, double vol, int age) {
    if (wave >= WAVE_INSTR && wave < WAVE_NOISE) {
        const Sfx *s = v->bank ? &v->bank->sfx[wave - WAVE_INSTR] : NULL;
        int len = !s ? 0 : s->length < 1 ? 1 : s->length > SFX_STEPS ? SFX_STEPS : s->length;
        int speed = !s || s->speed < 1 ? 1 : s->speed;
        int k = age / speed;
        if (s && s->loop_end > s->loop_start && s->loop_end <= len && k >= s->loop_end)
            k = s->loop_start + (k - s->loop_start) % (s->loop_end - s->loop_start);
        if (k >= len) { v->target = 0; return; }        /* played out (or no bank): silent, keep the last wave */
        const SfxStep *st = &s->steps[k];
        double ip, ivol;
        shape(st, k + 1 < len ? s->steps[k + 1].pitch : -1, age % speed, speed, age, &ip, &ivol);
        p += ip - INSTR_PITCH;
        vol *= ivol;
        wave = st->wave >= WAVE_NOISE ? WAVE_NOISE : st->wave % NWAVES;
    }
    v->wave = wave > WAVE_NOISE ? WAVE_NOISE : wave;
    v->freq = pitch_freq(p);
    v->target = (float)vol;
}

static void play_step(Voice *v, const SfxStep *st, int next_pitch, int tick, int speed, int age) {
    double p, vol;
    shape(st, next_pitch, tick, speed, age, &p, &vol);
    sound(v, st->wave, p, vol, tick);
}

static void tick_voice(Voice *v) {
    if (!v->on || v->releasing) return;
    if (v->sfx == MUSIC) {
        int speed = !mus.song || mus.song->song_speed < 1 ? 1 : mus.song->song_speed;
        /* a note carries on through empty rows: slides, drops and fades stay where they ended,
         * vibrato and arpeggio keep going */
        play_step(v, &v->cell, v->next_pitch, v->tick, speed, v->step++);
        if (v->tick < 1 << 20) v->tick++;
        return;
    }
    if (v->sfx < 0) {
        if (v->note_ticks > 0 && --v->note_ticks == 0) { release(v); return; }
        sound(v, v->cell.wave, v->cell.pitch, v->cell.vol / 7.0, v->tick);   /* an instrument moves on */
        if (v->tick < 1 << 20) v->tick++;
        return;
    }
    if (!v->bank) { release(v); return; }
    const Sfx *s = &v->bank->sfx[v->sfx];
    int len = s->length < 1 ? 1 : s->length > SFX_STEPS ? SFX_STEPS : s->length;
    int speed = s->speed < 1 ? 1 : s->speed;
    if (v->step >= len) { release(v); return; }
    play_step(v, &s->steps[v->step], v->step + 1 < len ? s->steps[v->step + 1].pitch : -1, v->tick, speed,
              v->tick + v->step * speed);

    if (++v->tick >= speed) {
        v->tick = 0;
        v->step++;
        if (s->loop_end > s->loop_start && v->step >= s->loop_end) v->step = s->loop_start;
    }
}

/* ---- music ---- */

/* the pattern at order entry pos, following jumps; -1 at a stop */
static int resolve(int *pos) {
    for (int hops = 0; hops <= ORDER_LEN; hops++) {
        if (*pos < 0 || *pos >= ORDER_LEN) return -1;
        const OrderEntry *e = &mus.song->order[*pos];
        if (e->kind == ORDER_PATTERN) return e->value < NPATTERNS ? e->value : -1;
        if (e->kind != ORDER_JUMP) return -1;
        *pos = e->value;
    }
    return -1;                  /* jumps that never reach a pattern */
}

static void release_music_voices(void) {
    for (int i = 0; i < NVOICES; i++) if (voices[i].sfx == MUSIC) release(&voices[i]);
}

void synth_music(int pos, int loop_pattern) {
    release_music_voices();
    mus.on = false;
    mus.song = cur_song;
    if (!mus.song) return;
    mus.loop_pattern = loop_pattern;
    if (loop_pattern >= 0) { mus.pattern = loop_pattern; mus.pos = -1; }
    else {
        mus.pos = pos;
        mus.pattern = resolve(&mus.pos);
        if (mus.pattern < 0) return;
    }
    mus.on = true;
    mus.row = mus.tick = 0;
}

void synth_music_stop(void) {
    mus.on = false;
    release_music_voices();
}

int synth_music_pos(void) { return mus.on ? mus.pos : -1; }
int synth_music_pattern(void) { return mus.on ? mus.pattern : -1; }
int synth_music_row(void) { return mus.on ? mus.row : 0; }

static void start_row(void) {
    const Pattern *pat = &mus.song->patterns[mus.pattern];
    for (int t = 0; t < NTRACKS; t++) {
        const Cell *c = &pat->cells[mus.row][t];
        Voice *v = &voices[t];
        if (v->on && !v->releasing && v->sfx >= 0) continue;     /* a sound effect has this voice */
        if (c->kind == CELL_OFF && v->sfx == MUSIC) release(v);
        if (c->kind != CELL_NOTE) continue;
        bool was = v->on && !v->releasing;
        start_voice(v);
        if (!was) v->vol = 0;
        v->sfx = MUSIC;
        v->cell = (SfxStep){c->pitch, c->wave, c->vol, c->fx};
        v->next_pitch = -1;
        for (int r = mus.row + 1; r < pattern_len(pat); r++) {    /* the note a slide heads for */
            const Cell *n = &pat->cells[r][t];
            if (n->kind == CELL_NOTE) { v->next_pitch = n->pitch; break; }
            if (n->kind == CELL_OFF) break;
        }
    }
}

static void tick_music(void) {
    if (!mus.on) return;
    if (!mus.song) { mus.on = false; return; }
    if (mus.tick == 0) start_row();
    int speed = mus.song->song_speed < 1 ? 1 : mus.song->song_speed;
    if (++mus.tick < speed) return;
    mus.tick = 0;
    if (++mus.row < pattern_len(&mus.song->patterns[mus.pattern])) return;   /* a short pattern ends early */
    mus.row = 0;
    if (mus.loop_pattern >= 0) return;
    mus.pos++;
    mus.pattern = resolve(&mus.pos);
    if (mus.pattern < 0) synth_music_stop();
}

/* ---- the mixer ---- */

void synth_mix(int16_t *out, int n) {
    const double samples_per_tick = (double)SYNTH_RATE / TICK_HZ;
    for (int k = 0; k < n; k++) {
        if ((tick_acc -= 1.0) <= 0) {
            tick_acc += samples_per_tick;
            tick_music();
            for (int i = 0; i < NVOICES; i++) tick_voice(&voices[i]);
        }
        float acc = 0;
        for (int i = 0; i < NVOICES; i++) {
            Voice *v = &voices[i];
            if (!v->on) continue;
            v->vol += (v->target - v->vol) * 0.01f;          /* ~2 ms to settle */
            if (v->releasing && v->vol < 0.001f) { v->on = false; continue; }
            int idx = (int)(v->phase * WAVE_LEN) & (WAVE_LEN - 1);
            float s;
            if (v->wave >= WAVE_NOISE) {                     /* noise: a new random level 32 times a cycle */
                if (idx != v->noise_idx) {
                    v->noise_idx = idx;
                    lfsr = (lfsr >> 1) ^ (-(lfsr & 1u) & 0xB400u);
                    v->noise_val = (float)(lfsr & 0xF) / 7.5f - 1.0f;
                }
                s = v->noise_val;
            } else if (v->bank && v->wave < NWAVES) s = (v->bank->waves[v->wave][idx] - 7.5f) / 7.5f;
            else s = 0;
            acc += s * v->vol * (v->sfx == MUSIC ? vol_music : vol_sfx);
            v->phase += v->freq / SYNTH_RATE;
            v->phase -= floor(v->phase);
        }
        acc *= 0.35f * vol_all;                              /* room for a few voices at once */
        if (acc > 1) acc = 1;
        if (acc < -1) acc = -1;
        out[k] = (int16_t)(acc * 32000);
    }
}
