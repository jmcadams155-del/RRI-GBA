TARGET := racer

CC := arm-none-eabi-gcc
OBJCOPY := arm-none-eabi-objcopy

CFLAGS := -mthumb -O2 -Wall -mcpu=arm7tdmi
LDFLAGS := -specs=gba.specs -lgba

all: $(TARGET).gba

$(TARGET).elf:
	$(CC) $(CFLAGS) main.c -o $@ $(LDFLAGS)

$(TARGET).gba: $(TARGET).elf
	$(OBJCOPY) -O binary $< $@
	gbafix $@

clean:
	rm -f *.elf *.gba
