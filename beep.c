#include <alsa/asoundlib.h>
#include <math.h>
#include <stdio.h>

#define SOUND_DEVICE "default"
#define SAMPLE_RATE 48000
#define DT (1.0f / SAMPLE_RATE)
#define PI 3.141592653589793238f
#define TWO_PI (2 * PI)

void atry(int err) {
    if (err < 0) {
        fprintf(stderr, "[ERRR] ALSA error: %s\n", snd_strerror(err));
        exit(EXIT_FAILURE);
    }
}

#define BUFFER_SIZE (4800)
struct synth_data {
    float buffer[BUFFER_SIZE];
    size_t last_time;
    float frequency;
    float volume;
};

void generate_square_wave(struct synth_data* data) {
    size_t t, i, target_time = data->last_time + BUFFER_SIZE;
    for (i = 0, t = data->last_time; t < target_time; i++, t++) {
        if ((int)(t * DT * data->frequency * 0.5f) & 1)
            data->buffer[i] = data->volume;
        else
            data->buffer[i] = -data->volume;
    }
    data->last_time = target_time;
}

void generate_saw_wave(struct synth_data* data) {
    size_t t, i, target_time = data->last_time + BUFFER_SIZE;
    float f;
    for (i = 0, t = data->last_time; t < target_time; i++, t++) {
        f = (t * DT * data->frequency);
        data->buffer[i] = data->volume * ((f - floorf(f)) * 2 - 1);
    }
    data->last_time = target_time;
}

void generate_triangle_wave(struct synth_data* data) {
    size_t t, i, target_time = data->last_time + BUFFER_SIZE;
    float f;
    for (i = 0, t = data->last_time; t < target_time; i++, t++) {
        f = (t * DT * data->frequency * 2);
        if ((int)f & 1) {
            data->buffer[i] = data->volume * ((f - floorf(f)) * 2 - 1);
        } else {
            data->buffer[i] = data->volume * ((1 - f + floorf(f)) * 2 - 1);
        }
    }
    data->last_time = target_time;
}

void generate_sine_wave(struct synth_data* data) {
    size_t t, i, target_time = data->last_time + BUFFER_SIZE;
    for (i = 0, t = data->last_time; t < target_time; i++, t++) {
        data->buffer[i] =
            sinf(TWO_PI * t * DT * data->frequency) * data->volume;
    }
    data->last_time = target_time;
}

void usage(FILE* f) {
    fprintf(f, "beep [OPTIONS]\n");
    fprintf(f, "\n");
    fprintf(f, "OPTIONS\n");
    fprintf(f, "  --freq n     - set frequency to n Hz (default: 440.0)\n");
    fprintf(f, "  --vol n      - set volume to n (default: 0.03)\n");
    fprintf(f, "                 WARNING: Do use _SMALL_ values!\n");
    fprintf(f, "  --dur n      - set duration to n seconds (default: 1.0)\n");
    fprintf(f, "  --sine       - set generator to use sine wave (default)\n");
    fprintf(f, "  --saw        - set generator to use saw wave\n");
    fprintf(f, "  --square     - set generator to use square wave\n");
    fprintf(f, "  --triangle   - set generator to use triangle wave\n");
    fprintf(f, "  --help       - display this message on STDOUT\n");
}

int main(int argc, char* argv[]) {
    snd_pcm_t* handle;
    snd_async_handler_t* ahandler;
    int i;
    int duration;
    void (*generator_function)(struct synth_data*);
    struct synth_data data = {0};

    data.frequency = 440;
    data.volume = 0.03;
    duration = 10;
    generator_function = generate_sine_wave;

    argv++;
    while (*argv) {
        if (strcmp(*argv, "--freq") == 0) {
            argv++;
            if (*argv == 0) {
                fprintf(stderr, "Missing frequency number!\n");
                exit(EXIT_FAILURE);
            }
            if (!sscanf(*argv, "%f", &data.frequency)) {
                fprintf(stderr, "Failed to parse frequency number!\n");
                exit(EXIT_FAILURE);
            }
        } else if (strcmp(*argv, "--vol") == 0) {
            argv++;
            if (*argv == 0) {
                fprintf(stderr, "Missing volume number!\n");
                exit(EXIT_FAILURE);
            }
            if (!sscanf(*argv, "%f", &data.volume)) {
                fprintf(stderr, "Failed to parse volume number!\n");
                exit(EXIT_FAILURE);
            }
        } else if (strcmp(*argv, "--dur") == 0) {
            float d;
            argv++;
            if (*argv == 0) {
                fprintf(stderr, "Missing duration number!\n");
                exit(EXIT_FAILURE);
            }
            if (!sscanf(*argv, "%f", &d)) {
                fprintf(stderr, "Failed to parse duration number!\n");
                exit(EXIT_FAILURE);
            }
            duration = d * 10;
        } else if (strcmp(*argv, "--sine") == 0) {
            generator_function = generate_sine_wave;
        } else if (strcmp(*argv, "--square") == 0) {
            generator_function = generate_square_wave;
        } else if (strcmp(*argv, "--saw") == 0) {
            generator_function = generate_saw_wave;
        } else if (strcmp(*argv, "--triangle") == 0) {
            generator_function = generate_triangle_wave;
        } else if (strcmp(*argv, "--help") == 0) {
            usage(stdout);
            exit(EXIT_SUCCESS);
        } else {
            fprintf(stderr,
                    "Invalid option '%s'\n"
                    "Try 'beep --help' for more infomation.\n",
                    *argv);
            exit(EXIT_FAILURE);
        }

        argv++;
    }

    atry(snd_pcm_open(&handle, SOUND_DEVICE, SND_PCM_STREAM_PLAYBACK, 0));

    atry(snd_pcm_set_params(handle, SND_PCM_FORMAT_FLOAT_LE,
                            SND_PCM_ACCESS_RW_INTERLEAVED, 1, SAMPLE_RATE, 1,
                            10000));

    for (i = 0; i < duration; i++) {
        generator_function(&data);
        snd_pcm_writei(handle, data.buffer, BUFFER_SIZE);
    }

    atry(snd_pcm_drain(handle));

    snd_pcm_close(handle);
    return 0;
}
