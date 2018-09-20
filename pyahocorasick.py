import ahocorasick
from copy import copy


class TokenAutomatonSearchIter(object):
    def __init__(self, wrapped, index):
        self.wrapped = wrapped
        self.index = index

    def is_root(self):
        return self.wrapped.is_root()

    def pos_id(self):
        return self.wrapped.pos_id()

    def set(self, string, *reset):
        self.wrapped.set(self.index.convert_key(string), *reset)

    def __copy__(self):
        return TokenAutomatonSearchIter(copy(self.wrapped), self.index)

    def __iter__(self):
        return self.wrapped


class TokenIndex(object):
    def __init__(self):
        self.index = {None: 0}
        self.idx = 0

    def add_get_token(self, token):
        if token in self.index:
            return self.index[token]
        self.idx += 1
        self.index[token] = self.idx
        return self.idx

    def get_token(self, token):
        return self.index.get(token, 0)

    def _convert_key(self, key, getter):
        lst = []
        for token in key:
            lst.append(getter(token))
        return tuple(lst)

    def convert_key(self, key):
        return self._convert_key(key, self.get_token)

    def convert_add_key(self, key):
        return self._convert_key(key, self.add_get_token)


class TokenAutomaton(object):
    """
    Implements a similar interface to Automaton, but instead of dealing with
    strings or tuples of integers, deals with tuples of tokens, that is strings
    picked from some vocabulary.
    """

    def __init__(self, store=ahocorasick.STORE_ANY):
        self.index = TokenIndex()
        self.wrapped = ahocorasick.Automaton(store, ahocorasick.KEY_SEQUENCE)

    def add_word(self, key, value):
        self.wrapped.add_word(self.index.convert_add_key(key), value)

    def iter(self, haystack, *args, **kwargs):
        return TokenAutomatonSearchIter(
            self.wrapped.iter(self.index.convert_key(haystack), *args, **kwargs),
            self.index
        )


def mk_index_wrapped_meth(method_name):
    def wrapper(self, key, *args, **kwargs):
        wrapped = getattr(self.wrapped, method_name)
        return wrapped(self.index.convert_key(key), *args, **kwargs)
    return wrapper


for method_name in [
    "exists", "match", "get", "keys", "values", "items"
]:
    setattr(TokenAutomaton, method_name, mk_index_wrapped_meth(method_name))


def mk_plain_wrapped_meth(method_name):
    def wrapper(self, *args, **kwargs):
        wrapped = getattr(self.wrapped, method_name)
        return wrapped(*args, **kwargs)
    return wrapper


for method_name in [
    "clear",
    "make_automaton",
    "get_stats",
    "dump",
]:
    setattr(TokenAutomaton, method_name, mk_plain_wrapped_meth(method_name))
