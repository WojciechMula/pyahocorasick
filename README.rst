========================================================================
                          pyahocorasick
========================================================================

.. contents::

Introduction
============

**pyahocorasick** is a Python module implements two kinds of data
structures: `trie`__ and `Aho-Corasick string matching automaton`__.

**Trie** is a dictionary indexed by strings, which allow to retrieve
associated items in a time proportional to string length. **Aho-Corasick
automaton** allows to find all occurrences of strings from given set in
a single run over text.

(BTW in order to use Aho-Corasick automaton, a trie have to be created;
this is the reason why these two distinct entities exist in a single
module.)

__ http://en.wikipedia.org/wiki/trie
__ http://en.wikipedia.org/wiki/Aho-Corasick%20algorithm


There are two versions:

* **C extension**, compatible only with Python3;
* pure python module, compatible with Python 2 and 3.

Python module API is similar, but isn't exactly the same as C extension.


License
=======

Library is licensed under very liberal two-clauses BSD license.
Some portions has been realased into public domain.

Full text of license is available in LICENSE file.


Author
======

Wojciech Muła, wojciech_mula@poczta.onet.pl


See also
========

* Module `ahocorasick`__ by Danny Yoo --- seems unmaintained (last
  update in 2005) and is licensed under GPL.

* Article about `different trie representations`__ --- this is
  an effect of experiments I made while working on this module.

__ https://hkn.eecs.berkeley.edu/~dyoo/python/ahocorasick/
__ http://0x80.pl/articles/trie-representation.html


Installation
============

Just run:

		$ python setup.py install

If compilation succed, module is ready to use.


API
===

Module
------

Module ``ahocorasick`` contains several constants and
class ``Automaton``.

.. _Unicode and bytes:

Unicode and bytes
-----------------

Type of strings accepted and returned by ``Automaton`` methods
can be either **unicode** or **bytes**, depending on compile time
settings (preprocessor definition ``AHOCORASICK_UNICODE``). Value
of module member ``unicode`` informs about chosen type.

.. warning::
	If unicode is selected, then trie stores 2 or even 4 bytes
	per letter, depending on Python settings. If bytes are
	selected, then just one byte per letter is needed.


Constants
~~~~~~~~~

* ``unicode`` --- see `Unicode and bytes`_
* ``STORE_ANY``, ``STORE_INTS``, ``STORE_LENGTH`` --- see Constructor_
* ``EMPTY``, ``TRIE``, ``AHOCORASICK`` --- see Members_
* ``MATCH_EXACT_LENGTH``, ``MATCH_AT_MOST_PREFIX``, ``MATCH_AT_LEAST_PREFIX``
  --- see description of method keys_


Automaton class
~~~~~~~~~~~~~~~

``Automaton`` class is pickable__ (implements ``__reduce__()``).

__ http://docs.python.org/py3k/library/pickle.html


Members
#######

``kind`` [readonly]
	One of values:

	``EMPTY``
		There are no words saved in automaton.

	``TRIE``
		There are some words, but methods related to Aho-Corasick algorithm
		(``find_all``, ``iter``) won't work.

	``AHOCORASICK``
		Aho-Corasick automaton has been constructed, full functionality is
		available for user.

	Kind is maintained internally by ``Automaton`` object.
	Some methods are not available when automaton kind is
	``EMPTY`` or isn't an ``AHOCORASICK``. When called then
	exception is raised, however testing this property could
	be better (faster, more elegant).

``store`` [readonly]
	Type of values stored in trie. By default ``STORE_ANY``
	is used, thus any python object could be used. When ``STORE_INTS``
	or ``STORE_LENGTH`` is used then values are 32-bit integers
	and do not occupy additional memory. See ``add_word`` description
	for details.


Constructor
###########

Constructor accepts just one argument, a type of values,
one of constants:

``STORE_ANY``
	Any Python object (default).

``STORE_LENGTH``
	Length of string.

``STORE_INTS``
	32-bit integers.


Dictionary methods
##################

``get(word[, default])``
	Returns value associated with ``word``. Raises ``KeyError`` or
	returns ``default`` value if ``word`` isn't present in dictionary.

.. _keys:

``keys([prefix, [wildchar, [how]]]) => yield strings``
	Returns iterator that iterate through words.

	If ``prefix`` (a string) is given, then only words sharing this
	prefix are yielded.
	
	If ``wildchar`` (single character) is given, then prefix is
	treated as a simple pattern with selected wildchar. Optional
	parameter ``how`` controls which strings are matched:

	``MATCH_EXACT_LENGTH`` [default]
		Only strings with the same length as a pattern's length
		are yielded. In other words, literally match a pattern.

	``MATCH_AT_LEAST_PREFIX``
		Strings that have length greater or equal to a pattern's length
		are yielded.

	``MATCH_AT_MOST_PREFIX``
		Strings that have length less or equal to a pattern's length
		are yielded.

	See `Example 2`_.


``values([prefix, [wildchar, [how]]]) => yield object``
	Return iterator that iterate through values associated with words.
	Words are matched as in ``keys`` method.

``items([prefix, [wildchar, [how]]]) => yield tuple (string, object)``
	Return iterator that iterate through words and associated values.
	Words are matched as in ``keys`` method.

``iter()`` protocol
	Equivalent to ``obj.keys()``

``len()`` protocol
	Returns number of distinct words.


Trie
####

``add_word(word, [value]) => bool``
	Add new ``word``, a key, to dictionary and associate with ``value``.
	Returns True if ``word`` didn't exists earlier in dictionary.

	If ``store == STORE_LENGTH`` then ``value`` is not allowed ---
	``len(word)`` is saved.

	If ``store == STORE_INTS`` then ``value`` is optional. If present,
	then have to be an integer, otherwise defaults to ``len(automaton)``.

	If ``store == STORE_ANY`` then ``value`` is required and could
	be any object.

	**This method invalidates all iterators only if new word was
	added (i.e. method returned True).**

``clear() => None``
	Removes all words from dictionary.
	
	**This method invalidates all iterators.**

``exists(word) => bool`` or ``word in ...``
	Returns if word is present in dictionary.

``match(word) => bool``
	Returns if there is a prefix (or word) equal to ``word``.
	For example if word "example" is present in dictionary, then
	all ``match("e")``, ``match("ex")``, ..., ``match("exampl")``,
	``match("example")`` are True. But ``exists()`` is True just
	for the last word.

``longest_prefix(word) => integer``
	Returns length of the longest prefix of word that exists in
	a dictionary.


Aho-Corasick
############

``make_automaton()``
	Creates Aho-Corasick automaton based on trie. This doesn't require
	additional memory. After successful creation ``kind`` become
	``AHOCORASICK``.

	**This method invalidates all iterators.**

``find_all(string, callback, [start, [end]])``
	Perform Aho-Corsick on string; ``start``/``end`` can be used to
	reduce string range. Callback is called with two arguments:

	* index of end of matched string
	* value associated with that string

	(Method called with ``start``/``end`` does similar job
	as ``find_all(string[start:end], callback)``, except index
	values).

``iter(string, [start, [end]])``
	Returns iterator (object of class AutomatonSearchIter_) that
	does the same thing as ``find_all``, yielding tuples instead
	of calling a user function.

	``find_all`` method could be expressed as::

		def find_all(self, string, callback):
			for index, value in self.iter(string):
				callback(index, value)


Other
#####

``dump() => (list of nodes, list of edges, list of fail links)``
	Returns 3 lists describing a graph:

	* nodes: each item is a pair (node id, end of word marker)
	* edges: each item is a triple (node id, label char, child node id)
	* fail: each item is a pair (source node id, node if connected by fail node)

	ID is a unique number and a label is a single byte.

	Module package contains also program ``dump2dot.py`` that shows
	how to convert ``dump`` results to input file for graphviz__ tools.

	__ http://graphviz.org

``get_stats() => dict``
	Returns dictionary containing some statistics about underlaying
	trie:

	* ``nodes_count``	--- total number of nodes
	* ``words_count``	--- same as ``len(automaton)``
	* ``longest_word``	--- length of the longest word
	* ``links_count``	--- number of edges
	* ``sizeof_node``	--- size of single node in bytes
	* ``total_size``	--- total size of trie in bytes (about
	  ``nodes_count * size_of node + links_count * size of pointer``).
	  The real size occupied by structure could be larger, because
	  of `internal memory fragmentation`__ occurred in memory manager.


__ http://en.wikipedia.org/Memory%20fragmentation


.. _AutomatonSearchIter:

AutomatonSearchIter class
~~~~~~~~~~~~~~~~~~~~~~~~~

Class isn't available directly, object of this class is returned
by ``iter`` method of ``Automaton``. Iterator has additional method.

``set(string, [reset]) => None``
	Sets new string to process. When ``reset`` is ``False`` (default),
	then processing is continued, i.e internal state of automaton and
	index aren't touched. This allow to process larger strings in chunks,
	for example::

		it = automaton.iter(b"")
		while True:
			buffer = receive(server_address, 4096)
			if not buffer:
				break

			it.set(buffer)
			for index, value in it:
				print(index, '=>', value)

	When ``reset`` is ``True`` then processing is restarted.
	For example this code::

		for string in set:
			for index, value in automaton.iter(string)
				print(index, '=>', value)

	Does the same job as::

		it = automaton.iter(b"")
		for string in set:
			it.set(it, True)
			for index, value in it:
				print(index, '=>', value)


Example
~~~~~~~

::

	>>> import ahocorasick
	>>> A = ahocorasick.Automaton() 
		
	# add some words to trie
	>>> for index, word in enumerate("he her hers she".split()):
	...   A.add_word(word, (index, word))

	# test is word exists in set
	>>> "he" in A
	True
	>>> "HER" in A
	False
	>>> A.get("he")
	(0, 'he')
	>>> A.get("she")
	(3, 'she')
	>>> A.get("cat", "<not exists>")
	'<not exists>'
	>>> A.get("dog")
	Traceback (most recent call last):
	  File "<stdin>", line 1, in <module>
	KeyError
	>>> 

	# convert trie to Aho-Corasick automaton
	A.make_automaton()

	# then find all occurrences in string
	for item in A.iter("_hershe_"):
	...  print(item)
	... 
	(2, (0, 'he'))
	(3, (1, 'her'))
	(4, (2, 'hers'))
	(6, (3, 'she'))
	(6, (0, 'he'))



.. _example 2:

Example 2
~~~~~~~~~

Demonstration of keys_ behaviour.

::

	>>> import ahocorasick
	>>> A = ahocorasick.Automaton() 

	# add some words to trie
	>>> for index, word in enumerate("cat catastropha rat rate bat".split()):
	...   A.add_word(word, (index, word))

	# prefix
	>>> list(A.keys("cat"))
	["cat", "catastropha"]

	# pattern
	>>> list(A.keys("?at", "?", ahocorasick.MATCH_EXACT_LENGTH))
	["bat", "cat", "rat"]

	>>> list(A.keys("?at?", "?", ahocorasick.MATCH_AT_MOST_PREFIX))
	["bat", "cat", "rat", "rate"]

	>>> list(A.keys("?at?", "?", ahocorasick.MATCH_AT_LEAST_PREFIX))
	["rate"]



Major updates
-------------

2013-10-23
	* fixed bug in constructing output values while seaching

2011-04-13
	* select support for either unicode or bytes
	* simple pattern matching

2011-04-12
	extend ``keys``/``values``/``items`` to match simple patterns
