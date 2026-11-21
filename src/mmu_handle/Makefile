TARGET = mmu_handle

OPTFLAGS=-O2 -fomit-frame-pointer -g0

#KOS_CFLAGS+= -Ikos-patch/kernel/arch/dreamcast/include -I. -I$(KOS_BASE)/../kos-ports/include/SDL -I$(KOS_BASE)/../kos-ports/zlib-1.2.1 -I$(KOS_BASE)/../kos-ports/libpng-1.2.5 $(OPTFLAGS) 
KOS_CFLAGS+= -Ikos-patch/kernel/arch/dreamcast/include -I. $(OPTFLAGS)  -DDREAMCAST -DMMU_HANDLE_BEGINMEM=0 -DMMU_HANDLE_MAX=0x10000

#KOS_CFLAGS+= -DMMU_HANDLE_DEBUG

KOS_CPPFLAGS+=${KOS_CFLAGS}

all: $(TARGET).bin

include $(KOS_BASE)/Makefile.rules

.SRCS	=	main.cpp mmu_handle.cpp \

OBJS = $(.SRCS:.cpp=.o) damedato.o

clean:
	rm -f $(OBJS) $(TARGET).bin $(TARGET).elf 

$(TARGET).elf: $(OBJS)
	$(KOS_CC) $(KOS_CFLAGS) $(KOS_LDFLAGS) -o $(TARGET).elf $(KOS_START) \
		$(OBJS) -lm $(OBJEXTRA) $(KOS_LIBS)

$(TARGET).bin: $(TARGET).elf
	$(KOS_OBJCOPY) -R .stack -O binary $(TARGET).elf $(TARGET).bin


run: $(TARGET).bin
	$(KOS_LOADER) $(TARGET).bin

