#include <dos.h>
#include <conio.h>
#include <stdio.h>

#define BASE 0x3F8  // Базовый адрес COM1

void init_serial() {
    outportb(BASE + 1, 0x00); // Отключаем прерывания IER
    outportb(BASE + 3, 0b10000000); // DLAB = 1 для установления частоты LCR

    outportb(BASE + 0, 0x0C);
    outportb(BASE + 1, 0x00); 

    outportb(BASE + 3, 0b0000011); // 8 бит, без паритета, 1 стоп-бит  LCR
    outportb(BASE + 2, 0b11000111); // Включаем FIFO, очистка буфера
    outportb(BASE + 4, 0x0B); // Включаем DTR, RTS, OUT2   MCR
}

void serial_write(char c) {
    while ((inportb(BASE + 5) & 0x20) == 0); // Ждём освобождения буфера
    outportb(BASE, c);
}

char serial_read() {
    while ((inportb(BASE + 5) & 0x01) == 0); // Ждём доступных данных
    return inportb(BASE);
}

void serial_flush() {
    while (inportb(BASE + 5) & 0x01) {
        inportb(BASE);
    }
}

int main() {
    init_serial();

    while(1) {
        printf("menu:\n1 - send bytes (n)\n2 - receive bytes (n)\n3 - clear buffer\n0 - exit\nYour select: ");
        short inp;
        scanf("%d", &inp);
        if(inp == 1) {
            char arr[1000];
            printf("Enter count of bytes for send(max 1000): ");
            int n;
            scanf("%d", &n);
            printf("Enter string: ");
            scanf("%999s", arr);
            for(int i = 0; i < n; i++) {
                if(arr[i] == '\0') break;
                serial_write(arr[i]);
            }
        } else if(inp == 2) {
            printf("Enter count of bytes for read: ");
            int n;
            scanf("%d", &n);
            printf("Received:\n");
            for(int i = 0; i < n; i++) {
                char received = serial_read();
                printf("%c\n", received);
            }
            printf("\n");
        } else if(inp == 3) {
            serial_flush();
        } else if(inp == 0) {
            break;
        }
    }

    return 0;
}
