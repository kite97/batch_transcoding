#include <stdbool.h>
#ifndef _WAVE_H_
#define _WAVE_H_

#ifdef __cplusplus
extern "C" {
#endif

#define WAVE_PCM 1
#define WAVE_ALAW 6
#define WAVE_MULAW 7

#pragma pack(push, 1)

typedef struct {
    char chunk_id[4];
    uint32_t chunk_size;
    char wave_id[4];
} riff_t;

typedef struct {
    char chunk_id[4];
    uint32_t chunk_size;
    uint16_t format_tag;
    uint16_t nchannels;
    uint32_t sample_rate;
    uint32_t bytes_rate;
    uint16_t block_align;
    uint16_t bits_per_sample;
} fmt_t;

typedef struct {
    uint16_t cb_size;
    char chunk_id[4];
    uint32_t chunk_size;
    uint32_t dw_sample_length;
} fact_t;

typedef struct {
    char chunk_id[4];
    uint32_t chunk_size;
    char buf[];
} data_t;

typedef struct {
    riff_t riff;
    fmt_t fmt;
    union {
        data_t pcm;
        struct {
            fact_t fact;
            data_t data;
        } law;
    } type;
} wave_t;

#pragma pack(pop)

bool wave_check_chunk(wave_t *wav);
void wave_parse_chunk(wave_t *wav, FILE *fp, int *data_frame, int ms);
int wave_fread_frame(char *buf, int frame_size, FILE *fp);

#ifdef __cplusplus
}
#endif

#endif
