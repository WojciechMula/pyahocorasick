dump()
----------------------------------------------------------------------

Return a three-tuple of lists describing the Automaton as a graph
of **nodes**, **edges**, **failure links**.

- nodes: each item is a pair (node id, end of word marker)
- edges: each item is a triple (node id, label char, child node id)
- failure links: each item is a pair (source node id, node if connected
  by fail node)

For each of these, the node id is a unique number and a label is
a number.
