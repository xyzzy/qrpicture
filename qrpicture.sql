CREATE TABLE `preview` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `jobid` text NOT NULL,
  `status` int(11) NOT NULL,
  `options` text NOT NULL,
  `result` text NOT NULL,
  `txt` text NOT NULL,
  `outlinenr` int(11) NOT NULL,
  `imageb64` mediumtext,
  PRIMARY KEY (`id`),
  UNIQUE KEY `jobid` (`jobid`(6)),
  KEY `status` (`status`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;


CREATE TABLE `queue` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `jobid` text NOT NULL,
  `status` int(11) NOT NULL,
  `options` text NOT NULL,
  `json` text NOT NULL,
  `result` text NOT NULL,
  `imagefilename` text NOT NULL,
  `txt` text NOT NULL,
  `outlinenr` int(11) NOT NULL,
  `imageb64` mediumtext,
  PRIMARY KEY (`id`),
  UNIQUE KEY `jobid` (`jobid`(6)),
  KEY `status` (`status`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
