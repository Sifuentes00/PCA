#include <dos.h>
#include <conio.h>
#include <stdio.h>

#define BASE 0x3F8  

void serial_write(char c) {
    while ((inportb(BASE + 5) & 0x20) == 0); 
    outportb(BASE, c);
}

char serial_read() {
    while ((inportb(BASE + 5) & 0x01) == 0);
    return inportb(BASE);
}


int main() {
    outportb(BASE + 4, 0b00010000);
    while(1) {
        printf("menu:\n1 - start\n0 - exit\nYour select: ");
        short inp;
        scanf("%d", &inp);
        if(inp == 1) {
            char symbol;
            printf("Enter byte for send: ");
            getchar();
            scanf("%c", &symbol);
            serial_write(symbol);
            char received = serial_read();
            printf("%c is received\n", received);
        } else if(inp == 0) {
            break;
        }
    }

    return 0;
}
