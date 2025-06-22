// This is a simple 16-bit kernel for DA-Os, a text-based operating system.
#define VIDEO_MEMORY 0xB8000
#define WHITE_ON_BLACK 0x0F
#define VGA_CTRL 0x3D4
#define VGA_DATA 0x3D5
#define MAX_HISTORY 10
#define MAX_CMD_LENGTH 32
#define STATUS_BAR_ROW 24
#define WHITE_ON_BLUE 0x1F
#define WHITE_ON_RED 0x4F
#define WHITE_ON_GREEN 0x2F

// Global cursor position tracker
static int current_cursor_pos = 0;
static volatile char *vidmem = (volatile char *)VIDEO_MEMORY;

static char history[MAX_HISTORY][MAX_CMD_LENGTH];
static int history_count = 0;
static int history_current = -1;

// Function declarations
void __attribute__((section(".text"))) kernel_main();
void __attribute__((section(".text"))) print_string(const char *str);
void __attribute__((section(".text"))) clear_screen();
void __attribute__((section(".text"))) get_input(char *buffer);
void __attribute__((section(".text"))) print_system_info();
void __attribute__((section(".text"))) update_cursor(int x, int y);
int __attribute__((section(".text"))) strcmp(const char *s1, const char *s2);
void __attribute__((section(".text"))) strcpy(char *dest, const char *src);
int __attribute__((section(".text"))) strlen(const char *str);
void __attribute__((section(".text"))) clear_current_input(char *buffer, int *pos, int start_pos);
void __attribute__((section(".text"))) load_history_command(char *buffer, int *pos, int start_pos);
void __attribute__((section(".text"))) init_status_bar(unsigned char color);
void __attribute__((section(".text"))) update_status_bar(const char *text, unsigned char color);

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
    0x48,                         /* Up Arrow */
    0,                            /* Page Up */
    '-',
    0, /* Left Arrow */
    0,
    0, /* Right Arrow */
    '+',
    0,    /* End */
    0x50, /* Down Arrow */
    0,    /* Page Down */
    0,    /* Insert */
    0,    /* Delete */
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

// Simple string copy
void strcpy(char *dest, const char *src)
{
    while (*src)
    {
        *dest++ = *src++;
    }
    *dest = '\0';
}

// Simple string length
int strlen(const char *str)
{
    int len = 0;
    while (*str++)
    {
        len++;
    }
    return len;
}

// Clear and initialize the status bar
void init_status_bar(unsigned char color)
{
    for (int i = 0; i < 80; i++)
    {
        vidmem[(STATUS_BAR_ROW * 80 + i) * 2] = ' ';
        vidmem[(STATUS_BAR_ROW * 80 + i) * 2 + 1] = color;
    }
}

// Update status bar text (centered)
void update_status_bar(const char *text, unsigned char color)
{
    // Clear the status bar first with specified color
    for (int i = 0; i < 80; i++)
    {
        vidmem[(STATUS_BAR_ROW * 80 + i) * 2] = ' ';
        vidmem[(STATUS_BAR_ROW * 80 + i) * 2 + 1] = color;
    }

    // Center the text
    int len = strlen(text);
    int start_pos = (80 - len) / 2;

    // Write the text with specified color
    for (int i = 0; i < len && (start_pos + i) < 80; i++)
    {
        vidmem[(STATUS_BAR_ROW * 80 + start_pos + i) * 2] = text[i];
        vidmem[(STATUS_BAR_ROW * 80 + start_pos + i) * 2 + 1] = color;
    }
}

// Load command from history into input buffer
void clear_current_input(char *buffer, int *pos, int start_pos)
{
    // Clear the current line
    for (int i = 0; i < *pos; i++)
    {
        current_cursor_pos--;
        vidmem[current_cursor_pos * 2] = ' ';
    }
    *pos = 0;
    update_cursor(start_pos % 80, start_pos / 80);
}

void load_history_command(char *buffer, int *pos, int start_pos)
{
    clear_current_input(buffer, pos, start_pos);

    // Copy history command to buffer and display it
    strcpy(buffer, history[history_count - 1 - history_current]);
    *pos = strlen(buffer);

    // Print the command
    for (int i = 0; i < *pos; i++)
    {
        vidmem[current_cursor_pos * 2] = buffer[i];
        vidmem[current_cursor_pos * 2 + 1] = WHITE_ON_BLACK;
        current_cursor_pos++;
    }
    update_cursor(current_cursor_pos % 80, current_cursor_pos / 80);
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

    init_status_bar(WHITE_ON_BLUE);
    update_status_bar("Ready", WHITE_ON_BLUE);
}

// Print a string to screen
void print_string(const char *str)
{
    int max_row = (current_cursor_pos / 80);
    if (max_row >= STATUS_BAR_ROW)
    {
        // Scroll up before printing
        for (int i = 80 * 2; i < 80 * 24 * 2; i++)
        {
            vidmem[i - 80 * 2] = vidmem[i];
        }
        // Clear the new line (row 23)
        for (int i = (80 * 23 * 2); i < 80 * 24 * 2; i += 2)
        {
            vidmem[i] = ' ';
            vidmem[i + 1] = WHITE_ON_BLACK;
        }
        current_cursor_pos = 80 * 23;
    }

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
    history_current = -1; // Reset history browsing when starting new input

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

            // Add to history if not empty
            if (pos > 0)
            {
                // Shift history down if full
                if (history_count == MAX_HISTORY)
                {
                    for (int i = 0; i < MAX_HISTORY - 1; i++)
                    {
                        strcpy(history[i], history[i + 1]);
                    }
                    history_count--;
                }

                strcpy(history[history_count], buffer);
                history_count++;
            }
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
        else if (key == 0x48)
        { // Up arrow - history previous
            if (history_count > 0 && (history_current < history_count - 1))
            {
                history_current++;
                load_history_command(buffer, &pos, input_start_pos);
            }
        }
        else if (key == 0x50)
        { // Down arrow - history next
            if (history_current > 0)
            {
                history_current--;
                load_history_command(buffer, &pos, input_start_pos);
            }
            else if (history_current == 0)
            {
                history_current = -1;
                // Clear current input
                clear_current_input(buffer, &pos, input_start_pos);
            }
        }
        else if (key < 0x80)
        { // Regular key
            if (history_current != -1)
            {
                // If we were browsing history but now typing, reset
                clear_current_input(buffer, &pos, input_start_pos);
                history_current = -1;
            }

            char c = keyboard_map[key];
            if (c >= 32 && c <= 126)
            {
                if (pos < MAX_CMD_LENGTH - 1)
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
}

// Print system information
void print_system_info()
{
    print_string("\n** System Information **\n");
    print_string("Os: DA-Os v0.1 by Sanira Deneth Adesha.\n");
    print_string("Mode: 16-bit Real Mode.\n");
    print_string("Display: 80x25 Text Mode.\n");
    print_string("Video Memory: 0xB8000.\n");
    print_string("Boot Device: Floppy Disk.\n\n");
}

// Kernel entry point
void kernel_main()
{
    clear_screen();
    init_status_bar(WHITE_ON_BLUE);
    update_status_bar("DA-OS v0.1 | Commands: info, clear, help", WHITE_ON_BLUE);

    char input[32];
    while (1)
    {
        print_string("DA-Os> ");
        get_input(input);

        // Update status bar based on command
        if (strcmp(input, "info") == 0)
        {
            print_system_info();
            update_status_bar("System information displayed", WHITE_ON_GREEN);
        }
        else if (strcmp(input, "clear") == 0)
        {
            clear_screen();
            update_status_bar("Screen cleared", WHITE_ON_GREEN);
        }
        else if (strcmp(input, "help") == 0)
        {
            print_string("Available commands:\n");
            print_string("  info  - Show system information\n");
            print_string("  clear - Clear the screen\n");
            print_string("  help  - Show this help\n");
            update_status_bar("Help displayed", WHITE_ON_GREEN);
        }
        else
        {
            print_string("Unknown command. Type 'help' for help.\n");
            update_status_bar("Unknown command", WHITE_ON_RED);
        }
    }
}