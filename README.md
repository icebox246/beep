# beep

This is a simple program for playing tones using ALSA API.

## Disclaimer

This is an audio application using low level API for playing sound.
Messing with volume value may be dangerous as this application can generate
really loud sounds. Be careful, and use at your own risk.

## Quick start

Dependencies:

- ALSA

```console
make
./beep --help
```

```console
beep [OPTIONS]

OPTIONS
  --freq n     - set frequency to n Hz (default: 440.0)
  --vol n      - set volume to n (default: 0.03)
                 WARNING: Do use _SMALL_ values!
  --dur n      - set duration to n seconds (default: 1.0)
  --sine       - set generator to use sine wave (default)
  --saw        - set generator to use saw wave
  --square     - set generator to use square wave
  --triangle   - set generator to use triangle wave
  --help       - display this message on STDOUT
```

