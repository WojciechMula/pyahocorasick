# -*- coding: utf-8 -*-
"""
    This is part of pyahocorasick Python module.

    Unit tests for the C-based ahocorasick module.

    Author    : Wojciech Muła, wojciech_mula@poczta.onet.pl
    WWW       : http://0x80.pl/proj/pyahocorasick/
    License   : public domain
"""

import ahocorasick

if ahocorasick.unicode:
    conv = lambda x: x
else:
    conv = lambda x: bytes(x, encoding='utf-8')
