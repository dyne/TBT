

all: siteconf
	emacs -Q -no-site-file -batch -l siteconf.el -l muse-config.el  \
		-f muse-project-batch-publish "TBT"


siteconf:
	echo "(set 'path \"`pwd`\")" > siteconf.el

clean:
	emacs -Q -no-site-file -batch -l siteconf.el -l muse-config.el  \
		-f muse-project-delete-output-files "TBT"
	rm -f siteconf.el

