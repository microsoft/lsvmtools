.PHONY: a ab ac abc

include ../../defs.mak

all:

a:
	@ $(BINDIR)/lsvmtool cpio split a.cpio
	@ cmp a.cpio a.cpio.1
	@ echo "=== Passed a"

ab:
	@ cat a.cpio b.cpio > ab.0
	@ $(BINDIR)/lsvmtool cpio split ab.0
	@ cmp a.cpio ab.0.1
	@ cmp b.cpio ab.0.2
	@ echo "=== Passed ab"

ac:
	@ cat a.cpio c.cpio.gz > ac.0
	@ $(BINDIR)/lsvmtool cpio split ac.0
	@ cmp a.cpio ac.0.1
	@ cmp c.cpio.gz ac.0.2
	@ echo "=== Passed ac"

abc:
	@ cat a.cpio b.cpio c.cpio.gz > abc.0
	@ $(BINDIR)/lsvmtool cpio split abc.0
	@ cmp a.cpio abc.0.1
	@ cmp b.cpio abc.0.2
	@ cmp c.cpio.gz abc.0.3
	@ echo "=== Passed abc"
	
tests-silent:
	$(MAKE) -s a
	$(MAKE) -s ab
	$(MAKE) -s ac
	$(MAKE) -s abc

tests:
	$(MAKE) -s tests-silent
	@ rm -rf *.[0-9]

clean:
	@ rm -rf *.[0-9]
