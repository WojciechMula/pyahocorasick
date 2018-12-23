load(path, deserializer) => Automaton
----------------------------------------------------------------------

Load automaton previously stored on disc using ``save`` method.

``Deserializer`` is a callable object which converts bytes back into
python object; it can be ``pickle.loads``.
