/*
	This is part of pyahocorasick Python module.
	
	<short info>

	Author    : Wojciech Mu³a, wojciech_mula@poczta.onet.pl
	WWW       : http://0x80.pl/proj/pyahocorasick/
	License   : 3-clauses BSD (see LICENSE)
	Date      : $Date$

	$Id$

*/
#ifndef ahocorasick_slist_h_included
#define ahocorasick_slist_h_included

/** base structure for list */
#define LISTITEM_data struct ListItem* __next;

/** list item node */
typedef struct ListItem {
	LISTITEM_data;
} ListItem;

/** List.

	const time of:
	* append
	* prepend
	* pop first
	* get first/last
*/
typedef struct {
	ListItem*	head;	///< first node
	ListItem*	last;	///< last node
} List;


#endif
