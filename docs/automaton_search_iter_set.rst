set(string, reset=False)
----------------------------------------------------------------------

Set a new string to search. When the reset argument is False (default)
then the Aho-Corasick procedure is continued and the internal state of the
Automaton and end index of the string being searched are not reset. This allow
to search for large strings in multiple smaller chunks. 
