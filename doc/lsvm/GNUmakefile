all:
	texi2dvi lsvm.texi
	dvipdf lsvm.dvi lsvm.pdf
	makeinfo lsvm.texi
	makeinfo --no-split --html lsvm.texi

CLEAN = \
    lsvm.aux \
    lsvm.cp \
    lsvm.dvi \
    lsvm.fn \
    lsvm.ky \
    lsvm.log \
    lsvm.pg \
    lsvm.toc \
    lsvm.tp \
    lsvm.pdf \
    lsvm.vr \
    lsvm.info \
    lsvm.html

clean:
	rm -f $(CLEAN)
