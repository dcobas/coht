
all:
	echo Make documentation for CTC driver
	rm -f ,*.h
	rm -rf generated-doc
	mkdir -p generated-doc/ 
	../../doxy/doxy.sh -n"CTC Device Driver" -o "generated-doc/" ../libctc.h

clean:
	rm -rf generated-doc/
