OBJS=list.o phon.o tvz.o
CFLAGS+= -g 
CC=gcc
LD=$(CC)

phone: $(OBJS)
	$(LD) $(CFLAGS) -export-dynamic  `pkg-config --cflags --libs libglade-2.0` -o phone $(OBJS)

clean:
	rm -f $(OBJS)
	rm -f phone

%.o: %.c %.h
	$(CC) -c `pkg-config --cflags libglade-2.0` $(CFLAGS) $< -o $@
