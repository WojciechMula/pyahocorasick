========================================================================
                          pyahocorasick
========================================================================

:Author:		Wojciech Mu³a, wojciech_mula@poczta.onet.pl
:Last update:	2011-03-2x

.. contents::

Introduction
------------

**pyahocorasick** is a Python module implements two kinds of data
structures: :enwiki:`trie` and :enwiki:`Aho-Corasick algorithm`.
Extension is compatible only with Python3.

**Trie** is a dictionary indexed by strings, which allow to retrieve
associated items in a time proportional to string length. **Aho-Corasick
string matching automaton** allows to find all occurrences of strings
from given set in a single run over string.

(BTW in order to use Aho-Corasick automaton, a trie have to be created;
this is the reason why these two distinct entities exist in a single
module.)


Last changes
------------

*none*


Documentation
-------------

Module ``pyahocorasick`` contains some constants and
following classes:

* ``Automaton`` --- the most flexible structure

..
	* [TODO] ``FrozenAutomaton`` --- read-only structure trie or Aho-Corasick
	  automaton, occupying less memory then ``Automaton``
	* [TODO] ``DeterminizedAutomaton`` --- read-only determinized Aho-Corasick
	  automaton, occupying **much more** memory then ``Automaton``, but
	  is the fastest. Suitable for small string sets and large amount
	  of data.
 


Module constants
~~~~~~~~~~~~~~~~

There are two groups of constants.

1. Type of values associated with strings in ``Automaton``.
See Constructor section for details.

``STORE_LEN``
	Length of string (implicit).

``STORE_INT``
	32-bit integers.

``STORE_ANY``
	Any Python object.


2. Kind of ``Automaton`` object:

``EMPTY``
	There are no words saved in automaton.

``TRIE``
	There are some words, but methods related to Aho-Corasick algorithm
	(``find_all``, ``iter``) won't work.

``AHOCORASICK``
	Aho-Corasick automaton has been constructed, full functionality is
	available for user.


Automaton class
~~~~~~~~~~~~~~~

``Automaton`` class is pickable__ (implements ``__reduce__()``).

__ http://docs.python.org/py3k/library/pickle.html


Members
#######

``kind`` [readonly]
	One of ``EMPTY``, ``TRIE``, ``AHOCORASICK``.

	Kind is maintained internally by ``Automaton`` object.
	Some methods are not available when automaton is ``EMPTY``
	or isn't an ``AHOCORASICK``.

	Class raises proper exceptions, however testing this
	property could be better (faster, more elegant.)

``store`` [readonly]
	Type of values stored in trie. By default ``STORE_ANY``
	is used, thus any python object could be used. When ``STORE_INT``
	or ``STORE_LEN`` is used then values are 32-bit integers
	and do not occupy additional memory. See ``add_word`` description
	to find details.


Constructor
###########

Constructor accepts just one argument: type of values, one of
constants ``STORE_ANY``, ``STORE_INT``, ``STORE_LEN``.


Dictionary methods
##################

``get(word[, default])``
	Returns value associated with ``word``. Raises ``KeyError`` or
	returns ``default`` value if ``word`` isn't present in dictionary.

``keys() => yield bytes object``
	Returns iterator that iterate through words.

``values() => yield object``
	Return iterator that iterate through values associated with words.

``items() => yield tuple (bytes object, object)``
	Return iterator that iterate through words and associated values.

``len()`` protocol
	Returns number of distinct words.


Trie
####

``add_word(word, [value]) => bool``
	Add new ``word``, a key, to dictionary and associate with ``value``.
	Returns True if ``word`` didn't exists earlier in dictionary.

	If ``store == STORE_LEN`` then ``value`` is not allowed ---
	``len(word)`` is saved.

	If ``store == STORE_INT`` then ``value`` is optional. If given
	have to be integer, otherwise defaults to ``len(automaton)``.

	If ``store == STORE_ANY`` then ``value`` is required and could
	be any object.

	**This method invalidates all iterators.**

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
	* value associated with string

	Method called with ``start``/``end`` does similar job
	as ``find_all(string[start:end], callback)``.

``iter(string, [start, [end]])``
	Returns iterator that does the same thing as ``find_all``,
	yielding tuples instead of calling a user function.


Other
#####

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
	  of :enwiki:`Memory fragmentation|internal memory fragmentation`
	  happened in memory manager.


Examples
~~~~~~~~

*TODO*


Unicode and bytes
-----------------

Trie internally operates on bytes, and because of that there following
caveats. Unicode objects are read directly --- Python saves all strings
in :enwiki:`UCS-2` (2 bytes per char) or :enwiki:`UCS-4` (4 bytes per
char). To conserve memory ``pyahocorasick`` saves just leading non-zero
bytes, thus keys returned by ``keys``/``items`` are incompleted ``UCS-x``
byte strings.

**This will be redesigned!**


Download
--------

Library is licensed under very liberal two-clauses BSD license.

Single archive is available:

* `pyahocorasick.tgz`__ [size: ?, md5sum: ?]

__ pyahocorasick.tgz

File list:

* ``LICENSE``
* ``README``
* ``pyahocorasick.c``
* ``setup.py``
* ``unittests.py``
* ``trie.h``, ``trie.c``
* ``slist.h``, ``slist.c``
* ``Automaton.h``, ``Automaton.c``, ``Automaton_pickle.c``,
* ``AutomatonItemsIter.h``, ``AutomatonItemsIter.c``
* ``AutomatonSearchIter.h``, ``AutomatonSearchIter.c``

