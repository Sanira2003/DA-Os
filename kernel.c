#define VIDEO_MEMORY 0xB8000
#define WHITE_ON_BLACK 0x0F

// Function declarations
void __attribute__((section(".text"))) kernel_main();
void __attribute__((section(".text"))) print_string(const char *str);
void __attribute__((section(".text"))) clear_screen();
void __attribute__((section(".text"))) print_system_info();

// Clear the screen
void clear_screen()
{
    volatile char *vidmem = (volatile char *)VIDEO_MEMORY;
    for (int i = 0; i < 80 * 25 * 2; i += 2)
    {
        vidmem[i] = ' ';
        vidmem[i + 1] = WHITE_ON_BLACK;
    }
}

// Print a string to screen
void print_string(const char *str)
{
    volatile char *vidmem = (volatile char *)VIDEO_MEMORY;
    static int pos = 0;

    while (*str)
    {
        if (*str == '\n')
        {
            pos += (80 - (pos % 80));
        }
        else
        {
            vidmem[pos * 2] = *str;
            vidmem[pos * 2 + 1] = WHITE_ON_BLACK;
            pos++;
        }
        str++;
    }
}

// Kernel entry point
void kernel_main()
{
    clear_screen();
    print_string("DA-Os By Sanira Deneth v0.1\n");
}