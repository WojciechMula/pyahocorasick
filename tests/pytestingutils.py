# -*- coding: utf-8 -*-
"""
This is part of pyahocorasick Python module.

Unit tests for the C-based ahocorasick module.

Author    : Wojciech Muła, wojciech_mula@poczta.onet.pl
WWW       : http://0x80.pl/proj/pyahocorasick/
License   : public domain
"""
import sys

import ahocorasick

if ahocorasick.unicode:
    conv = lambda x: x
else:
    conv = lambda x: bytes(x, encoding='utf-8')


sys_platform = str(sys.platform).lower()
on_linux = sys_platform.startswith('linux')
on_windows = 'win32' in sys_platform
on_mac = 'darwin' in sys_platform
on_freebsd = 'freebsd' in sys_platform
