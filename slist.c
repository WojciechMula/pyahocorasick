typedef struct ListItem {
	void* data;
	struct ListItem* next;
} ListItem;


typedef struct {
	ListItem*	head;
	ListItem*	last;
} List;


ListItem*
list_item_new(void* data) {
	ListItem* item = (ListItem*)memalloc(sizeof(ListItem));
	item->data = data;
	item->next = 0;
	return item;
}


void
list_item_delete(ListItem* item) {
	memfree(item);
}


void
list_init(List* list) {
	if (list) {
		list->head = 0;
		list->last = 0;
	}
}


int
list_delete(List* list) {
	ASSERT(list);
	ListItem* item = list->head;
	ListItem* tmp;
	while (item) {
		tmp = item;
		item = item->next;
		memfree(tmp);
	}

	list->head = list->last = NULL;
	return 0;
}


ListItem*
list_append(List* list, void* data) {
	ASSERT(list);

	struct ListItem* item = list_item_new(data);
	if (item) {
		if (list->last) {
			list->last->next = item;	// append
			list->last = item;			// set as last node
		}
		else
			list->head = list->last = item;
	}
	
	return item;
}


void*
list_pop_first(List* list) {
	ASSERT(list);
	ListItem* item;
	void* data;
	
	if (list->head) {
		item = list->head;
		list->head = item->next;

		if (!list->head)
			list->last = 0;

		data = item->data;
		memfree(item);
		return data;
	}
	else
		return NULL;
}

