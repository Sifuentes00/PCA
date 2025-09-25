#include <stdio.h>
#include <dos.h>

#define d2 73u
#define c3 130u
#define D3 156u
#define f3 175u
#define a3 220u
#define A3 233u
#define d4 294u
#define d3 147u
#define D4 311u
#define f4 349u
#define g4 392u
#define a4 440u
#define A4 466u
#define c5 523u
#define d5 587u
#define D5 622u
#define f5 698u
#define g5 784u
#define a5 880u
#define A5 932u
#define C6 1109u
#define e3 164u
#define e4 329u
#define b3 247u
#define g3 196u
#define b4 494u  // Объявление ноты b4

#define duration 200u
#define indent   150u

#define NOTES_AMOUNT 18u  // Исправлено количество нот

unsigned notes[NOTES_AMOUNT][3] = {
    {g4, duration, indent},
    {g4, duration, indent},
    {a4, duration, indent},
    {g4, duration, indent},
    {c5, duration * 2, indent},
    {b4, duration * 2, indent},
    {g4, duration, indent},
    {g4, duration, indent},
    {a4, duration, indent},
    {g4, duration, indent},
    {c5, duration * 2, indent},
    {b4, duration * 2, indent},
    {g4, duration, indent},
    {g4, duration, indent},
    {a4, duration, indent},
    {g4, duration, indent},
    {c5, duration * 2, indent},
    {b4, duration * 2, indent},
};

void state_words(void) {
    unsigned channel, state;
    int ports[] = {0x40, 0x41, 0x42};
    int control_word[] = {226, 228, 232};
    char state_word[] = "76000000";
    int i;

    printf("Status word: \n");
    for (channel = 0; channel < 3; channel++) {
        outp(0x43, control_word[channel]);
        state = inp(ports[channel]);

        for (i = 7; i >= 0; i--) {
            state_word[i] = (char) ((state % 2) + '0');
            state /= 2;
        }
        printf("Channel %d: %s\n", channel, state_word);
    }
}

// Функция для чтения коэффициента деления (значения счетчика CE)
unsigned read_divider(unsigned channel) {
    unsigned low_byte, high_byte, divider;
    int ports[] = {0x40, 0x41, 0x42};

    // Установка команды чтения текущего значения счётчика
    outp(0x43, 0xC0 | (channel << 6));
    low_byte = inp(ports[channel]);
    high_byte = inp(ports[channel]);

    divider = (high_byte << 8) | low_byte;
    return divider;
}

void print_dividers(void) {
    unsigned channel, divider;

    printf("Dividers (CE) in hex: \n");
    for (channel = 0; channel < 3; channel++) {
        divider = read_divider(channel);
        printf("Channel %d: 0x%04X\n", channel, divider);
    }
}

void set_frequency(unsigned divider) {
    unsigned long kd = 1193180 / divider;
    outp(0x43, 0xB6);
    outp(0x42, kd % 256);
    kd /= 256;
    outp(0x42, kd);
}

void play_music(void) {
    int i;
    for (i = 0; i < NOTES_AMOUNT; i++) {
        set_frequency(notes[i][0]);
        outp(0x61, inp(0x61) | 0x03);
        delay(notes[i][1]);
        outp(0x61, inp(0x61) & 0xFC);
        delay(notes[i][2]);
    }
}

int main(void) {
    state_words();
    print_dividers();
    play_music();
    state_words();
    print_dividers();
    return 0;
}