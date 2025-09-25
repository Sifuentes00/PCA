#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <dos.h>

#define TIMER_REG_AMOUNT 3
#define TIME_REG_AMOUNT 6

typedef unsigned char byte;

enum time_registers { // Адреса регистров RTC
    sec = 0x00,
    sec_timer = 0x01,
    min = 0x02,
    min_timer = 0x03,
    hour = 0x04,
    hour_timer = 0x05,
    day = 0x07,
    month = 0x08,
    year = 0x09,
};

enum state_registers {
    A = 0x0A, // Регистр состояния A
    B = 0x0B, // Регистр состояния B
    C = 0x0C, // Регистр состояния C
    D = 0x0D, // Регистр состояния D
};

unsigned time[TIME_REG_AMOUNT];
const byte time_registers[TIME_REG_AMOUNT] = {sec, min, hour, day, month, year};
const byte timer_registers[TIMER_REG_AMOUNT] = {sec_timer, min_timer, hour_timer};

unsigned bcd_to_dec(unsigned bcd) {
    return (bcd / 16 * 10) + (bcd % 16);
}

unsigned dec_to_bcd(unsigned dec) {
    return (dec / 10 * 16) + (dec % 10);
}

void wait_for_rtc_update(void) {
    // Ждём, пока бит UIP (обновление RTC) не станет 0
    do {
        outp(0x70, A);
    } while (inp(0x71) & 0x80); // UIP = 1 -> обновление идёт
}

void print_time(void) {
    int i;

    wait_for_rtc_update(); // Ждём окончания обновления RTC

    for (i = 0; i < TIME_REG_AMOUNT; ++i) {
        outp(0x70, time_registers[i]); // Устанавливаем адрес регистра
        time[i] = inp(0x71);           // Читаем данные из регистра
        time[i] = bcd_to_dec(time[i]); // Преобразуем из BCD в десятичный формат
    }

    printf("Current time: %02u:%02u:%02u %02u.%02u.20%02u\n",
           time[2], time[1], time[0], time[3], time[4], time[5]);
}

void set_time(void) {
    int i;

    puts("Enter time in format hours:min:sec:");
    if (scanf("%u:%u:%u", &time[2], &time[1], &time[0]) != 3) {
        fprintf(stderr, "Failed to enter time\n");
        return;
    }

    puts("Enter date in format day.month.year:");
    if (scanf("%u.%u.%u", &time[3], &time[4], &time[5]) != 3) {
        fprintf(stderr, "Failed to enter date\n");
        return;
    }

    for (i = 0; i < TIME_REG_AMOUNT; ++i) {
        time[i] = dec_to_bcd(time[i]); // Преобразуем в BCD формат
    }

    disable(); // Отключаем прерывания

    wait_for_rtc_update(); // Ждём окончания обновления RTC

    // Отключаем обновление RTC
    outp(0x70, B);
    outp(0x71, inp(0x71) | 0x80); // Устанавливаем бит UPD (запрещаем обновление)

    // Устанавливаем новое время
    for (i = 0; i < TIME_REG_AMOUNT; ++i) {
        outp(0x70, time_registers[i]);
        outp(0x71, time[i]);
    }

    // Включаем обновление RTC
    outp(0x70, B);
    outp(0x71, inp(0x71) & 0x7F); // Сбрасываем бит UPD (разрешаем обновление)

    enable(); // Включаем прерывания

    puts("Time set successfully.");
}

void interrupt (*old_interrupt)(void); // Старый обработчик прерываний

void interrupt new_interrupt(void) {
    puts("Alarm triggered! Time reached!");

    outp(0x70, C); // Читаем регистр состояния C, чтобы сбросить флаг прерывания
    inp(0x71);

    // Завершаем прерывание
    outp(0x20, 0x20); // EOI для мастера
    outp(0xA0, 0x20); // EOI для слэйва
}

void set_alarm(void) {
    int i;

    puts("Enter alarm time in format hours:min:sec:");
    if (scanf("%u:%u:%u", &time[2], &time[1], &time[0]) != 3) {
        fprintf(stderr, "Failed to enter time\n");
        return;
    }

    for (i = 0; i < TIMER_REG_AMOUNT; ++i) {
        time[i] = dec_to_bcd(time[i]); // Преобразуем в BCD формат
    }

    disable();

    old_interrupt = getvect(0x70);    // Сохраняем старый обработчик
    setvect(0x70, new_interrupt);    // Устанавливаем новый обработчик

    outp(0xA1, inp(0xA1) & 0xFE);    // Разрешаем прерывания от будильника

    wait_for_rtc_update();           // Ждём окончания обновления RTC

    // Устанавливаем время будильника
    for (i = 0; i < TIMER_REG_AMOUNT; ++i) {
        outp(0x70, timer_registers[i]);
        outp(0x71, time[i]);
    }

    // Разрешаем прерывания от будильника
    outp(0x70, B);
    outp(0x71, inp(0x71) | 0x20); // Устанавливаем бит AIE (разрешаем прерывания)

    enable();

    puts("Alarm set successfully.");
}

void set_delay(void) {
    int frequency;

    puts("Enter frequency in Hz (1-1024):");
    if (scanf("%d", &frequency) != 1 || frequency <= 0 || frequency > 1024) {
        fprintf(stderr, "Invalid frequency. Must be between 1 and 1024.\n");
        return;
    }

    disable();

    wait_for_rtc_update(); // Ждём окончания обновления RTC

    // Устанавливаем частоту
    outp(0x70, A);
    outp(0x71, (inp(0x71) & 0xF0) | (frequency & 0x0F)); // Устанавливаем частоту

    enable();

    printf("Delay frequency set to %d Hz\n", frequency);
}

int main(void) {
    while (1) {
        puts("1 - Print time");
        puts("2 - Set time");
        puts("3 - Set alarm");
        puts("4 - Set delay frequency");
        puts("0 - Exit");

        switch (getch()) {
            case '1':
                print_time();
                break;

            case '2':
                set_time();
                break;

            case '3':
                set_alarm();
                break;

            case '4':
                set_delay();
                break;

            case '0':
                exit(EXIT_SUCCESS);

            default:
                puts("Invalid option. Try again.");
                break;
        }
    }
}