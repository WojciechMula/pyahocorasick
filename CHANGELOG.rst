1.4.0 (2019-01-24)
--------------------------------------------------

- Change internal trie representation thanks to that performance
  of common operation is 1.5 - 2.5 times faster. Details are
  presented in https://github.com/WojciechMula/pyahocorasick/pull/107
  Warning: this change breaks compatibility of pickle and ``save()``
  format, this won't be possible to load files created in the
  previous version.

1.3.0 (2018-12-20)
--------------------------------------------------

- Add alternative pickling mechanism ``save()``/``load``, which
  requires less memory than the standard pickle solution (issue #102)

1.2.0 (2018-12-13)
--------------------------------------------------

- Add methods ``remove_word()``/``pop()`` (issue #79)

1.1.13.1 (2018-12-11)
--------------------------------------------------

- Fix mantifest file

1.1.13 (2018-12-11)
--------------------------------------------------

- Fix pickling of large automatons (issue #50);
  The fix wouldn't be possible without great help and
  patience of all people involved:

  * **Emil Stenström** (@EmilStenstrom)
  * **David Woakes** (@woakesd)
  * **@Dobatymo**
  * **Philippe Ombredanne** (@pombredanne)
    
  The fix wouldn't also be possible without **Daniel Lemire** (@lemire),
  who gave me access to decent machines and I was able to test fixes
  on large data.

1.1.12 (2018-12-03)
--------------------------------------------------

- Add support for tuples of ints to ``iter()`` (by **Frankie Robertson**)

1.1.11 (2018-12-02)
--------------------------------------------------

- Reworked pickling code
- Fix pickling crash (issue #68)
- Fix pickling memory leak (issue #62)
- Fix documentation (by **Philippe Ombredanne**)
- Fix several latent bugs and problems

1.1.10 (2018-10-25)
--------------------------------------------------

- Fix handling of unicode in Python 3 (by **Frankie Robertson**)

1.1.9 (2018-10-25)
--------------------------------------------------

- Fix documentation typos (by **Sylvain Zimmer**)
- Add ability to skip white spaces in the input strings (by **@gladtosee**; issue #84)

1.1.8 (2018-04-25)
--------------------------------------------------

- Fix memory leak (issue #81)
- Add link to Python implementation from Abusix (by **Frederik Petersen**)
- Fix unit tests (by **Renat Nasyrov**)

1.1.7 (2018-02-23)
--------------------------------------------------

- Minor documentation fixes (by **Edward Betts**)
- Some internal improvements

1.1.6 (2017-11-27)
--------------------------------------------------

- Fix PyPI building (by **Philippe Ombredanne**; issue #71)

1.1.5 (2017-11-22)
--------------------------------------------------

- Fix handling of UCS2-encoded string (issue #53)
- Fix pickling error
- Several minor fixes and corrections to documentation
  and infrastructure (thanks to: **Jan Fan**, **@blackelk**,
  **David Woakes** and **Xiaopeng Xu**)

1.1.4 (2016-08-08)
--------------------------------------------------

- Fix URL in documentation (by **Philippe Ombredanne**)

1.1.3 (2016-08-07)
--------------------------------------------------

- Rewrite documentation and fix PyPI presentation (by **Philippe Ombredanne**)

1.1.2 (2016-08-06)
--------------------------------------------------

- Rewrite documentation continued (by **Philippe Ombredanne**)

1.1.1 (2016-05-29)
--------------------------------------------------

- Rewrite documentation, setup readthedocs.io__ page (by **Philippe Ombredanne**)
- Make the module compilable in Windows using MSVC compiler (issue #11)
- Fix ``get()`` method that crashed when trie was empty (issue #22)
- Fix pickling problem (issue #26)
- Add ``__sizeof__()`` method (issue #25)

__ https://pyahocorasick.readthedocs.io/en/latest/

1.1.0 (2016-04-26)
--------------------------------------------------

- Support for Python 2 (with help from **Philippe Ombredanne**; issue #12)

1.0.3 (2016-04-24)
--------------------------------------------------

- Fix memory leak (by **Jonathan Grs**; issue #9)

1.0.2 (2016-04-23)
--------------------------------------------------

- Fix range parsing (by **Jonathan Grs**; issue #10)
- Fix pickling on 64-bit machines (issue #20)
- Update documentation regarding wildcards

1.0.1 (2016-04-19)
--------------------------------------------------

- Fix Unicode handling during automaton build (issue #8)
- Fix some 64-bit code issues (issue #5)
- Fix documentation (thanks to **Pastafarianist**)

1.0.0 (2014-11-25)
--------------------------------------------------

- The first version available through PyPi
