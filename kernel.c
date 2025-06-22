#define VIDEO_MEMORY 0xB8000
#define WHITE_ON_BLACK 0x0F
#define VGA_CTRL 0x3D4
#define VGA_DATA 0x3D5

// Global cursor position tracker
static int current_cursor_pos = 0;
static volatile char *vidmem = (volatile char *)VIDEO_MEMORY;

// Function declarations
void __attribute__((section(".text"))) kernel_main();
void __attribute__((section(".text"))) print_string(const char *str);
void __attribute__((section(".text"))) clear_screen();
void __attribute__((section(".text"))) get_input(char *buffer);
void __attribute__((section(".text"))) print_system_info();
void __attribute__((section(".text"))) update_cursor(int x, int y);
int __attribute__((section(".text"))) strcmp(const char *s1, const char *s2);

// I/O port functions
static inline unsigned char inb(unsigned short port)
{
    unsigned char ret;
    asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void outb(unsigned short port, unsigned char val)
{
    asm volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

// Update hardware cursor position
void update_cursor(int x, int y)
{
    unsigned short pos = y * 80 + x;
    outb(VGA_CTRL, 0x0F);
    outb(VGA_DATA, (unsigned char)(pos & 0xFF));
    outb(VGA_CTRL, 0x0E);
    outb(VGA_DATA, (unsigned char)((pos >> 8) & 0xFF));
}

// Keyboard scancode to ASCII mapping (US QWERTY)
static const unsigned char keyboard_map[128] = {
    0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, /* Ctrl */
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0, /* Left Shift */
    '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',
    0, /* Right Shift */
    '*',
    0,                            /* Alt */
    ' ',                          /* Space */
    0,                            /* Caps Lock */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* F1-F10 */
    0,                            /* Num Lock */
    0,                            /* Scroll Lock */
    0,                            /* Home */
    0,                            /* Up Arrow */
    0,                            /* Page Up */
    '-',
    0, /* Left Arrow */
    0,
    0, /* Right Arrow */
    '+',
    0, /* End */
    0, /* Down Arrow */
    0, /* Page Down */
    0, /* Insert */
    0, /* Delete */
    0, 0, 0,
    0, /* F11 */
    0, /* F12 */
    0  /* All other keys are undefined */
};

// Simple string comparison
int strcmp(const char *s1, const char *s2)
{
    while (*s1 && (*s1 == *s2))
    {
        s1++;
        s2++;
    }
    return *(const unsigned char *)s1 - *(const unsigned char *)s2;
}

// Clear the screen
void clear_screen()
{
    for (int i = 0; i < 80 * 25 * 2; i += 2)
    {
        vidmem[i] = ' ';
        vidmem[i + 1] = WHITE_ON_BLACK;
    }
    current_cursor_pos = 0;
    update_cursor(0, 0);
    print_string("==> Welcome to DA-Os <==\n\n");
}

// Print a string to screen
void print_string(const char *str)
{
    while (*str)
    {
        if (*str == '\n')
        {
            current_cursor_pos += (80 - (current_cursor_pos % 80));
        }
        else
        {
            vidmem[current_cursor_pos * 2] = *str;
            vidmem[current_cursor_pos * 2 + 1] = WHITE_ON_BLACK;
            current_cursor_pos++;
        }
        str++;

        if (current_cursor_pos >= 80 * 25)
        {
            // Scroll screen up
            for (int i = 80 * 2; i < 80 * 25 * 2; i++)
            {
                vidmem[i - 80 * 2] = vidmem[i];
            }
            // Clear new line
            for (int i = (80 * 24 * 2); i < 80 * 25 * 2; i += 2)
            {
                vidmem[i] = ' ';
                vidmem[i + 1] = WHITE_ON_BLACK;
            }
            current_cursor_pos = 80 * 24;
        }
    }
    update_cursor(current_cursor_pos % 80, current_cursor_pos / 80);
}

// Get keyboard input
void get_input(char *buffer)
{
    int pos = 0;
    int input_start_pos = current_cursor_pos;

    while (1)
    {
        // Wait for key press
        while ((inb(0x64) & 0x01) == 0)
            ;

        unsigned char key = inb(0x60);

        if (key == 0x1C)
        { // Enter
            buffer[pos] = 0;
            print_string("\n");
            return;
        }
        else if (key == 0x0E)
        { // Backspace
            if (pos > 0)
            {
                pos--;
                current_cursor_pos--;
                vidmem[current_cursor_pos * 2] = ' ';
                update_cursor(current_cursor_pos % 80, current_cursor_pos / 80);
            }
        }
        else if (key < 0x80)
        { // Regular key
            char c = keyboard_map[key];
            if (c >= 32 && c <= 126)
            {
                buffer[pos++] = c;
                vidmem[current_cursor_pos * 2] = c;
                vidmem[current_cursor_pos * 2 + 1] = WHITE_ON_BLACK;
                current_cursor_pos++;
                update_cursor(current_cursor_pos % 80, current_cursor_pos / 80);
            }
        }
    }
}

// Print system information
void print_system_info()
{
    print_string("\n** System Information **\n");
    print_string("Os: DA-Os v0.1\n");
    print_string("Mode: 16-bit Real Mode\n");
    print_string("Display: 80x25 Text Mode\n");
    print_string("Video Memory: 0xB8000\n");
    print_string("Boot Device: Floppy Disk\n\n");
}

// Kernel entry point
void kernel_main()
{
    clear_screen();
    print_string("Type 'info' for system information\n");
    print_string("Type 'clear' to clear the screen\n");
    print_string("Type 'help' for available commands\n\n");

    char input[32];
    while (1)
    {
        print_string("DA-Os> ");
        get_input(input);

        if (strcmp(input, "info") == 0)
        {
            print_system_info();
        }
        else if (strcmp(input, "clear") == 0)
        {
            clear_screen();
        }
        else if (strcmp(input, "help") == 0)
        {
            print_string("Available commands:\n");
            print_string("  info  - Show system information\n");
            print_string("  clear - Clear the screen\n");
            print_string("  help  - Show this help\n");
        }
        else
        {
            print_string("Unknown command. Type 'help' for help.\n");
        }
    }
}