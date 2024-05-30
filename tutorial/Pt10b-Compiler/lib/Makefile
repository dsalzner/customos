CC=gcc

SRC := main.c
OBJ := x86_64/tinycc_interpreter.o x86_64/libtcc.o x86_64/va_list.o x86_64/main.o
INC := -I./tcc-0.9.27/ -I./tcc-0.9.27/include/ -I.
CFLAGS := -g -DCONFIG_TCC_STATIC -DCONFIG_TCC_SEMLOCK

LD=${CC}

#  x86_64/dtoa.o x86_64/qsort_nr.o x86_64/qsort.o
# x86_64/malloc.o x86_64/tinycc_compat.o
#  x86_64/vsprintf.o

all: ${SRC} ${OBJ}
	$(LD) -o main -O2 ${OBJ}

x86_64/tinycc_interpreter.o:
	${CC} -w ${INC} ${CFLAGS} -c tcc-0.9.27/libtcc.c -o x86_64/libtcc.o
	${CC} -w ${INC} ${CFLAGS} -c tinycc_interpreter.c -o x86_64/tinycc_interpreter.o
	${CC} -w ${INC} ${CFLAGS} -c tcc-0.9.27/lib/va_list.c -o x86_64/va_list.o
	${CC} -w ${INC} ${CFLAGS} -c main.c -o x86_64/main.o

	#${CC} -w ${INC} ${CFLAGS} -c tccboot-0.1/vsprintf.c -o x86_64/vsprintf.o
	#${CC} -w ${INC} ${CFLAGS} -c tccboot-0.1/dtoa.c -o x86_64/dtoa.o
	#${CC} -w ${INC} ${CFLAGS} -c musl-1.2.5/src/stdlib/qsort.c -o x86_64/qsort.o
	#${CC} -w ${INC} ${CFLAGS} -c musl-1.2.5/src/stdlib/qsort_nr.c -o x86_64/qsort_nr.o
	#${CC} -w ${INC} ${CFLAGS} -c tinycc_compat.c -o x86_64/tinycc_compat.o
	#${CC} -w ${INC} ${CFLAGS} -c tccboot-0.1/malloc.c -o x86_64/malloc.o

clean:
	rm -f x86_64/main
	rm -f x86_64/*.o