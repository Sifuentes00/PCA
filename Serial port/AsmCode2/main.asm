section .data
    byte_to_send db 'A'         
    received_byte db 0            

section .text
    global _start

_start:
    ; Инициализация COM1
    mov dx, 0x3F8               
    mov al, 0x80                
    out dx, al                    

    ; Устанавливаем скорость передачи (9600)
    mov dx, 0x3F8               
    mov al, 0x0C                 
    out dx, al                  
    mov al, 0x00                 
    out dx, al                  

    ; Устанавливаем параметры порта: 8 бит, без паритета, 1 стоп-бит
    mov dx, 0x3FB               
    mov al, 0x03              
    out dx, al          

    ; Отправка байта через BIOS
    mov ah, 0x00                 
    mov bx, 0x01               
    mov dl, [byte_to_send]        
    int 0x14                      

    ; Чтение байта через BIOS
    mov ah, 0x01                
    mov bx, 0x01               
    int 0x14                    
    mov [received_byte], al    

    ; Вывод принятого байта на экран
    mov ah, 0x02                
    mov dl, [received_byte]
    int 0x21                   

    ; Завершение программы
    mov ax, 0x4C00            
    int 0x21
