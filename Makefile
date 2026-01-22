# -------- Project --------
TARGET := racer
SOURCES := main.c

# -------- DevkitPro paths (Docker) --------
DEVKITPRO := /opt/devkitpro
DEVKITARM := $(DEVKITPRO)/devkitARM
LIBGBA := $(DEVKITPRO)/libgba

# -------- Tools --------
CC := $(DEVKITARM)/bin/arm-none-eabi-gcc
OBJCOPY := $(DEVKITARM)/bin/arm-none-eabi-objcopy

# -------- Flags --------
CFLAGS := -mthumb -mcpu=arm7tdmi -O2 -Wall \
  -I$(LIBGBA)/include

LDFLAGS := -specs=gba.specs -L$(LIBGBA)/lib -lgba

# -------- Build --------
all: $(TARGET).gba

$(TARGET).elf: $(SOURCES)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

$(TARGET).gba: $(TARGET).elf
	$(OBJCOPY) -O binary $< $@

clean:
	rm -f *.elf *.gba
