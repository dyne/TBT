clean:
	find . -name "*~" -exec rm -f {} \;
	rm -rf tbt_site/

sqldatadump:
	mysqldump --opt -u root -p tbt_site > install/tbt_site.sql

createdb:
	mysqladmin -u root -p create tbt_site

dropdb:
	mysqladmin -u root -p drop tbt_site

createtable:
	mysql -u root -p tbt_site < install/tbt_site.sql

grantpriv:
	echo "GRANT ALL ON tbt_site.* TO tbt_site identified by ';.;tbt_s1t3!!!';" | mysql -u root -p 
	mysqladmin -u root -p flush-privileges

tar: 
	chmod 777 src/upload
	chmod 777 src/include/third_part/captcha/tmp/
	mkdir -p tbt_site/
	for i in *; do if [ "$i" != "tbt_site" ]; then cp -rv $i tbt_site/; fi; done
	tar cvz --exclude="*.tbt" --exclude="*tmp*" --exclude="*.tar*" --exclude="*.svn*" -f tbt_site.tar.gz tbt_site/
