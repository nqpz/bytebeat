bytebeat
========

Based on the idea described on [http://canonical.org/~kragen/bytebeat/]() and
includes tools for easy experimentation.  "Bytebeat" refers to sound generated
by simple formulas sent to an audio output.

  + clive lets you play different tunes grabbed from the site.
  + byteplay lets you play an arbitrary formula.
  + bibgen poorly generates a tune and plays it.
  + exprgen generates a formula, but isn't very good either.


Installation
------------

These tools depend on libtcc to dynamically generate C code for execution;
download from [http://bellard.org/tcc/]().

Just run `make` afterwards.  Should work on at least some platforms.


License
-------

Released under the Do What The Fuck You Want To Public License, Version 2.0; see
http://wtfpl.net/ for the license.


Bugs and the like
-----------------

Needs documentation.  Does some non-standard C things (don't compile with
-pedantic).  Would be nice with a better random tune generator.
