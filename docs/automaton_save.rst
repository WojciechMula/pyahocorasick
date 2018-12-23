save(path, serializer)
----------------------------------------------------------------------

Save content of automaton in an on-disc file.

``Serializer`` is a callable object that is used when automaton store
type is ``STORE_ANY``. This method converts a python object into
bytes; it can be ``pickle.dumps``.
