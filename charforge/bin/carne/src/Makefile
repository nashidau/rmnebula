

CFLAGS=`pkg-config gtk+-2.0 --cflags`
LDLIBS=`pkg-config gtk+-2.0 --libs`


OBJECTS=carne.o comms.o levelup.o

carne : $(OBJECTS)

clean :
	$(RM) $(OBJECTS) carne core *~
