load(path, deserializer) => Automaton
----------------------------------------------------------------------

Load automaton previously store on disc using ``save`` method.

``Deserializer`` is a callable object which converts bytes back into
python object; it can be ``pickle.loads``.
