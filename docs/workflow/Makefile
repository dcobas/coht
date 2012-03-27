.SUFFIXES: .txt .pdf

DOCNAME=workflow
DBLATEXOPTS=" -P latex.output.revhistory=0 "

.txt.pdf:
	a2x -f pdf --dblatex-opts $(DBLATEXOPTS) $^

all: $(DOCNAME).pdf $(DOCNAME).html
