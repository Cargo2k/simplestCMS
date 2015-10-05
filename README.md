# simplestCMS
An attempt at the simplest (to use) cms possible.

## Overview
This is another concept project, it is probably never going to be production ready.

## Setup
Currently all the paths are hard coded.
 * webRoot - should be the web root of the web server, plan to output composited pages into the tree.  Default path /var/www/html. 
 * dataRoot - root of the input directory currently coded to /var/www/html-data
 * msgLogPath - an output location for log messages, currently /var/log/simplestCMS/message.log

There are 2 files needed to run.
 * *dataRoot*/themes/default/template.mstch - this is the html template for the CMS
```
<html>
<body>
  Welcome to {{ title }} <br>
  {{{ content }}} <br>
  hit #{{ counter }}
</body>
</html>
```
 * *dataRoot*/index.md - a sample markdown to generate the rendered page from
```
<!--
title=Index
-->

# welcome

this is the index page
```

## Usage
 * configure the source with cmake
 * compile
 * make sure the files above are in place
 * configure your web server to see the fastcgi program
 * spawn the fastcgi process
 * visit the page 
