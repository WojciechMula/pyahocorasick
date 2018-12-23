keys([prefix, [wildcard, [how]]])
----------------------------------------------------------------------

Return an iterator on keys.
If the optional ``prefix`` string is provided, only yield keys starting
with this prefix.

If the optional ``wildcard`` is provided as a single character string,
then the prefix is treated as a simple pattern using this character
as a wildcard.

The optional ``how`` argument is used to control how strings are matched
using one of these possible values:

- **ahocorasick.MATCH_EXACT_LENGTH** (default)
  Yield matches that have the same exact length as the prefix length.
- **ahocorasick.MATCH_AT_LEAST_PREFIX**
  Yield matches that have a length greater or equal to the prefix length.
- **ahocorasick.MATCH_AT_MOST_PREFIX**
  Yield matches that have a length lesser or equal to the prefix length.
