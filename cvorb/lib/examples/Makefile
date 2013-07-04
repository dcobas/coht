include /acc/src/dsc/co/Make.auto

INCDIR := ../../include
CFLAGS := -Wall -D_GNU_SOURCE -g -I. -I$(INCDIR)
LDLIBS := -lm
LIBCVORB := ../libcvorb.$(CPU).a

SRCS :=	status.c \
	lock.c \
	version.c \
	pcb.c


OBJS := $(SRCS:.c=.$(CPU))

all : $(LIBCVORG) $(OBJS)

.PHONY : all clean

status.$(CPU):		$(@:.$(CPU)=.$c) $(LIBCVORB)
lock.$(CPU):		$(@:.$(CPU)=.$c) $(LIBCVORB)
version.$(CPU):		$(@:.$(CPU)=.$c) $(LIBCVORB)
pcb.$(CPU):			$(@:.$(CPU)=.$c) $(LIBCVORB)

clean:
	$(RM) *.$(CPU) *.o $(BAKS)
