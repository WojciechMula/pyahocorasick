from distutils.core import setup, Extension

module1 = Extension(
	'ahocorasick',
	sources = ['pyahocorasick.c'],
	depends = [
		'Automaton.c',
		'AutomatonKeysIter.c',
		'AutomatonSearchIter.c',
		'trie.c',
		'slist.c',
		'utils.c'
	]
)

setup(
	name = 'Aho-Corasick automaton',
	ext_modules = [module1]
)
