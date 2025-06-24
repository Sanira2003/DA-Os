# DA-Os - A Simple 16-bit Operating System

DA-Os is a minimalist, text-based, 16-bit operating system built from scratch for x86 architecture. It's an educational project designed to explore the fundamentals of OS development, from bootloading to kernel interaction. It operates entirely in real mode.

## Screenshots

This section is ready for you to add your own screenshots of DA-Os in action.

- **Boot Screen & Welcome Banner:**

  ![welcome](Screenshots/welcome.png)

- **'help' Command Usage:**

  ![help](Screenshots/help.png)

- **'info' Command Usage:**

  ![info](Screenshots/info.png)

## Core Features

- **16-bit Bootloader:** A custom assembly bootloader (`boot.asm`) that loads the kernel from a floppy disk into memory.
- **C-based Kernel:** The kernel (`kernel.c`) is written in C, handling all high-level logic.
- **Interactive Shell:** A simple command-line interface (CLI) that accepts user input and executes commands.
- **Built-in Commands:**
  - `help`: Displays a list of all available commands.
  - `info`: Shows technical details about the OS.
  - `clear`: Clears the terminal screen.
- **Direct Hardware Access:** Manages the screen by writing directly to VGA text mode memory, controls the cursor, and reads raw scancodes from the keyboard.
- **Screen Scrolling:** Automatically scrolls the content up when the screen is full.

## Special Features

DA-Os includes some advanced features that enhance the user experience.

### Dynamic Status Bar

At the bottom of the screen, a status bar provides real-time feedback on operations.

- **Color-Coded Feedback:** The bar changes color to signify the outcome of a command:
  - **Blue (Default):** Idle and ready for a command.
  - **Green:** The command was executed successfully.
  - **Red:** An unknown or invalid command was entered.
- **Informative Messages:** Displays messages like "Help displayed", "Screen cleared", or "Unknown command".

### Command History

The shell keeps a history of your most recent commands to improve efficiency.

- **Navigate History:** Use the **Up Arrow** and **Down Arrow** keys to cycle through the last 10 commands you've entered.
- **Easy Re-execution:** Simply find the command you want to run again and press Enter.

## Architecture

The OS follows a simple, two-stage architecture.

### 1. Bootloader (`boot.asm`)

The boot process begins with `boot.asm`. The BIOS loads this 512-byte sector from the floppy disk into memory at `0x7C00`. Its responsibilities are:

1.  **Setup Environment:** It initializes the segment registers (`ds`, `es`, `ss`) and sets up the stack.
2.  **Load Kernel:** It uses BIOS interrupt `0x13` to read 10 sectors (the kernel) from the disk and place them in memory at address `0x7E00`.
3.  **Transfer Control:** Once the kernel is loaded, the bootloader unconditionally jumps to `0x7E00`, effectively passing control to the C kernel.

### 2. Kernel (`kernel.c`)

The kernel is the heart of DA-Os. Its main components are:

- **Entry Point (`kernel_main`):** This function initializes the screen, displays the welcome banner, and enters an infinite loop to handle user commands.
- **Command Loop:** This loop repeatedly prints the `DA-Os> ` prompt, gets user input, executes the corresponding command, and updates the status bar with the result.

## How to Build and Run

The project is built using a `Makefile` which automates the compilation and linking process.

### Prerequisites

- **NASM:** An assembler for the x86 architecture.
- **GCC Cross-Compiler:** A 32-bit GCC compiler for the i386 target (e.g., `i686-elf-gcc`).
- **LD (Linker):** A linker to combine the object files.
- **QEMU:** An emulator to run the operating system.

### The Build Process

1.  **`boot.asm` -> `boot.bin`**: The `Makefile` uses `nasm` to assemble the bootloader into a flat binary file.
2.  **`kernel.c` -> `kernel.o`**: The C kernel is compiled into an object file.
3.  **Linking (`link.ld`)**: The linker uses the `link.ld` script to combine the boot and kernel binaries, placing the kernel at the correct memory address (`0x7E00`).
4.  **Final Image (`os.img`)**: The final binary is padded to create a 1.44MB floppy disk image.

### Running the OS

You can run the OS with a single command:

```sh
make run

This command will execute all the build steps and then launch the OS image in the QEMU emulator:

qemu-system-i386 -fda os.img

File Structure
.
├── boot.asm            # The 16-bit assembly bootloader
├── kernel.c            # The main C kernel file
├── link.ld             # Linker script to position the kernel
├── Makefile            # Automation script for building and running
└── README.md           # This file
```
