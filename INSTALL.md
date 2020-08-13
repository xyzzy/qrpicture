Configure and set prefix to document root

	./configure --prefix=<documentroot>

Build qrcode/qrscq

	make

Create database and load tables

	mysql < qrpicture.sql

	<update config.php>

Install site

	make install

	