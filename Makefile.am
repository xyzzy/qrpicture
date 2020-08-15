ACLOCAL_AMFLAGS = -I m4
EXTRA_DIST = \
	LICENSE CHANGELOG.md INSTALL.md README.md \
	.gitignore \
	config.php \
	qrpicture.sql \
	www.qrpicture.com/index-old.php \
	$(SITEFILES) $(SITEFILES_ASSETS) $(SITEFILES_IMAGES)
    
SITEFILES = \
	www.qrpicture.com/favicon.ico \
	www.qrpicture.com/index.php \
	www.qrpicture.com/qrcode-mask-93x93.png \
	www.qrpicture.com/qrscq-mask-93x93.png \
	www.qrpicture.com/mootools-core-1.4.5.js \
	www.qrpicture.com/mootools-more-1.4.0.1.js \
	www.qrpicture.com/qrpicture.js \
	www.qrpicture.com/robots.txt \
	www.qrpicture.com/status.php \
	www.qrpicture.com/submit.php \
	www.qrpicture.com/worker.php
	
SITEFILES_ASSETS = \
	www.qrpicture.com/assets/8S5rXB.png \
	www.qrpicture.com/assets/index.html \
	www.qrpicture.com/assets/outline0-97x97.png \
	www.qrpicture.com/assets/outline1-97x97.png \
	www.qrpicture.com/assets/outline2-97x97.png \
	www.qrpicture.com/assets/outline3-97x97.png \
	www.qrpicture.com/assets/outline4-97x97.png \
	www.qrpicture.com/assets/outline5-97x97.png \
	www.qrpicture.com/assets/p2G4MC.png \
	www.qrpicture.com/assets/qrAwesome.anim.col.210x210.gif \
	www.qrpicture.com/assets/qrSpiral.anim.col.210x210.gif

SITEFILES_IMAGES = \
	www.qrpicture.com/images/index.html

bin_PROGRAMS = qrwork qrscq

qrwork_SOURCES = qrwork.cc
qrwork_LDADD = -lgd

qrscq_SOURCES = qrscq.cc
qrscq_LDADD = -lgd

install-data-hook:
	install -d $(prefix) $(prefix)/assets
	install -d -m 775 $(prefix)/images
	cd $(top_srcdir); install $(SITEFILES) $(prefix)
	cd $(top_srcdir); install $(SITEFILES_ASSETS) $(prefix)/assets
	cd $(top_srcdir); install $(SITEFILES_IMAGES) $(prefix)/images
	if test ! -e "$(prefix)/config.php"; then install "$(top_srcdir)/config.php"  "$(prefix)/config.php"; fi

uninstall-local:
	$(RM) $(prefix)/assets/{8S5rXB.png,index.html,outline0-97x97.png,outline1-97x97.png,outline2-97x97.png,outline3-97x97.png,outline4-97x97.png,outline5-97x97.png,p2G4MC.png,qrAwesome.anim.col.210x210.gif,qrSpiral.anim.col.210x210.gif}
	$(RM) $(prefix)/images/index.html
	$(RM) $(prefix)/{favicon.ico,index.php,mask-93x93.png,mootools-core-1.4.5.js,mootools-more-1.4.0.1.js,qrcode-mask-93x93.png,qrpicture.js,qrscq-mask-93x93.png,robots.txt,status.php,submit.php,worker.php,config.php}
