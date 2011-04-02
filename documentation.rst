========================================================================
                          pyahocorasick
========================================================================

:Author:		Wojciech Mu³a, wojciech_mula@poczta.onet.pl
:Last update:	$Date$

.. contents:: Contents


Module
------

Module ``ahocorasick`` contains several constants and
class ``Automaton``.

..
	* [TODO] ``FrozenAutomaton`` --- read-only structure trie or Aho-Corasick
	  automaton, occupying less memory then ``Automaton``
	* [TODO] ``DeterminizedAutomaton`` --- read-only determinized Aho-Corasick
	  automaton, occupying **much more** memory then ``Automaton``, but
	  is the fastest. Suitable for small string sets and large amount
	  of data.


Constants
~~~~~~~~~

There are two groups of constants.

1. Type of values associated with strings in ``Automaton``.
See Constructor section for details.

``STORE_ANY``
	Any Python object (default).

``STORE_LENGTH``
	Length of string.

``STORE_INTS``
	32-bit integers.


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
	Some methods are not available when automaton kind is
	``EMPTY`` or isn't an ``AHOCORASICK``. When called then
	exception is raised, however testing this property could
	be better (say faster, more elegant).

``store`` [readonly]
	Type of values stored in trie. By default ``STORE_ANY``
	is used, thus any python object could be used. When ``STORE_INTS``
	or ``STORE_LENGTH`` is used then values are 32-bit integers
	and do not occupy additional memory. See ``add_word`` description
	for details.


Constructor
###########

Constructor accepts just one argument: type of values, one of
constants ``STORE_ANY``, ``STORE_INTS``, ``STORE_LENGTH``.


Dictionary methods
##################

``get(word[, default])``
	Returns value associated with ``word``. Raises ``KeyError`` or
	returns ``default`` value if ``word`` isn't present in dictionary.

``keys([prefix]) => yield bytes object``
	Returns iterator that iterate through words.
	If prefix (a string) is given, then only words sharing this
	prefix are yielded.

``values([prefix]) => yield object``
	Return iterator that iterate through values associated with words.

``items([prefix]) => yield tuple (bytes object, object)``
	Return iterator that iterate through words and associated values.

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
	Returns iterator (object of class AutomatonSearchIter) that
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
	  of :enwiki:`Memory fragmentation|internal memory fragmentation`
	  occurred in memory manager.


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
	for item in A.iter(b"_hershe_"):
	...  print(item)
	... 
	(2, (0, 'he'))
	(3, (1, 'her'))
	(4, (2, 'hers'))
	(6, (3, 'she'))
	(6, (0, 'he'))


Unicode and bytes
-----------------

Trie internally operates on bytes, and this is the reason of following
problem. Unicode objects are read directly --- current version of Python
saves all strings in :enwiki:`UCS-2` (2 bytes per char) or :enwiki:`UCS-4`
(4 bytes per char). To conserve memory ``pyahocorasick`` saves just leading
non-zero bytes, thus keys returned by ``keys``/``items`` are incomplete
``UCS-x`` byte strings.

**Probably this will be redesigned to work with UTF-8.**


License
-------

Library is licensed under very liberal two-clauses BSD license.
Some portions has been realased into public domain.

Full text of license is available in LICENSE file.


