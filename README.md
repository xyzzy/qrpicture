Welcome to the world of photo realistic QR's.

Features:
*   93x93 pixel QR code capable of holding 100 characters.
*   The size also makes the scanning responsive and fast.
*   186x186 pixel colour information.
*   Colours are chosen from a high-contrast QR-safe palette.
*   Dithering introduces blurring which reduces sharp constrast edges.
*   Repository hosted on [www.qrpicture.com](https://www.qrpicture.com).
    
Creating photo QR's is a two-part process.

First part (`sqcode`) is to create a 93x93 pixel dithered monochrome image overlaid with mandatory QR pixels. 
The applied dithering performs a best match to ensure the CRC is fully compliant. 

The second part (`sqscq`) is to create a 186x186 pixel dithered color image.
The colour palette is created using Spacial Color Quantification.

Both parts use the same SCQ dithering mechanism to maximize shades and perception to give the result a natural effect.

# Requirements

*   LAMP environment
*   libgd graphical library
*   GD-enabled php

# Installation

Configure and set prefix to document root

```
	./configure --prefix=<documentroot>
```

Build qrcode/qrscq

```
	make
```

Create database and load tables

```
	mysql < qrpicture.sql
	edit config.php
	make "image" directory writable for http server
```

Install site

```
	make install
```
	
# Versioning

Using [SemVer](http://semver.org/) for versioning. For the versions available, see the [tags on this repository](https://github.com/xyzzy/qrpicture/tags).

# License

This project is licensed under the GNU AFFERO General Public License v3 - see the [LICENSE](LICENSE) file for details

## Acknowledgments

* Derrick Coetzee and his amazing work on Spatial Colour Quantinazion.
* Masahiro Hara and his work on setting the QR code standard.
