org 100h  ; Начало программы для COM файла

section .data
    msg db "IRQ Registers: ", 0
    query_msg db "Request: ", 0
    service_msg db "Service: ", 0
    mask_msg db "Mask: ", 0
    newline db 13, 10, 0

section .bss
    old_vectors resw 16 ; Сохраняем старые обработчики IRQ0—IRQ15

section .text
start:
    ; Сохранение старых векторов
    mov cx, 16          ; Количество линий IRQ (0-15)
    xor di, di          ; Индекс для массива old_vectors (начинаем с 0)
save_old_vectors:
    mov ax, 0           ; Функция INT 21h/AH=35h - Get Interrupt Vector
    mov al, cl          ; Номер прерывания (IRQ)
    int 0x21            ; Получаем адрес обработчика
    mov [old_vectors + di], bx ; Сохраняем в массив
    add di, 2           ; Переходим к следующему месту в массиве
    loop save_old_vectors

    ; Устанавливаем свои обработчики для IRQ0-IRQ15
    mov cx, 16          ; Количество линий IRQ (0-15)
    xor di, di          ; Индекс для массива old_vectors (начинаем с 0)
set_new_vectors:
    mov ax, 0x2500      ; Функция INT 21h/AH=25h - Set Interrupt Vector
    mov al, cl          ; Номер пользовательского прерывания (70h + IRQ номер)
    add al, 70h         ; Смещение к пользовательским векторным прерываниям
    lea dx, [irq_handler] ; Указываем новый обработчик
    int 0x21            ; Устанавливаем обработчик
    loop set_new_vectors

    ; Оставляем программу резидентной
    mov dx, sp          ; Размер стека
    shr dx, 4           ; Конвертируем в параграфы
    add dx, 10          ; Добавляем 10 параграфов резерва
    mov ah, 0x31        ; Функция DOS - Make Program Resident
    int 0x21

; Обработчик прерываний IRQ0-IRQ15
irq_handler:
    ; Сохраняем регистры (используем стек)
    pusha

    ; Определяем номер IRQ
    mov al, 70h         ; Базовый вектор
    sub al, 70h         ; Получаем номер IRQ

    ; Считываем и выводим регистры контроллеров

    ; 1. Регистр запросов
    mov al, 0x0A        ; Команда для выбора регистра запросов
    out 0x20, al        ; На ведущем
    in al, 0x20         ; Читаем значение из порта ведущего
    call print_binary   ; Печатаем в двоичном виде

    mov al, 0x0A        ; Команда для выбора регистра запросов
    out 0xA0, al        ; На ведомом
    in al, 0xA0         ; Читаем значение из порта ведомого
    call print_binary   ; Печатаем в двоичном виде

    ; 2. Регистр обслуживания
    mov al, 0x0B        ; Команда для выбора регистра обслуживания
    out 0x20, al        ; На ведущем
    in al, 0x20         ; Читаем значение из порта ведущего
    call print_binary   ; Печатаем в двоичном виде

    mov al, 0x0B        ; Команда для выбора регистра обслуживания
    out 0xA0, al        ; На ведомом
    in al, 0xA0         ; Читаем значение из порта ведомого
    call print_binary   ; Печатаем в двоичном виде

    ; 3. Регистр масок
    in al, 0x21         ; Читаем регистр масок ведущего
    call print_binary   ; Печатаем в двоичном виде

    in al, 0xA1         ; Читаем регистр масок ведомого
    call print_binary   ; Печатаем в двоичном виде

    ; Вызов стандартного обработчика
    mov di, ax          ; Номер IRQ
    shl di, 1           ; Умножаем на 2 (каждый указатель занимает 2 байта)
    mov bx, [old_vectors + di] ; Адрес стандартного обработчика
    call bx             ; Переходим на стандартный обработчик

    ; Восстанавливаем регистры
    popa
    iret                ; Возвращаемся из прерывания

; Функция для вывода числа в двоичном виде
print_binary:
    pusha               ; Сохраняем регистры
    mov cx, 8           ; 8 бит в байте
print_binary_loop:
    shl al, 1           ; Сдвигаем самый старший бит в CF
    jc print_one        ; Если CF=1, печатаем "1"
    mov ah, '0'         ; Иначе печатаем "0"
    jmp print_char
print_one:
    mov ah, '1'
print_char:
    mov dl, ah
    mov ah, 0x02        ; Функция DOS: вывод символа
    int 0x21
    loop print_binary_loop
    popa                ; Восстанавливаем регистры
    ret

times 510-($-$$) db 0   ; Заполняем до 512 байт
dw 0xAA55               ; Сигнатура конца COM файла