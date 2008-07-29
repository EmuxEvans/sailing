#ifndef	__DLIST_H__
#define	__DLIST_H__

struct DLIST_ITEM;
typedef struct DLIST_ITEM			DLIST_ITEM;
struct DLIST_CURSOR;
typedef struct DLIST_CURSOR			DLIST_CURSOR;
struct DLIST_HEAD;
typedef struct DLIST_HEAD			DLIST_HEAD;

struct DLIST_ITEM {
	RLIST_ITEM		item;
	RLIST_HEAD		clist;
};

struct DLIST_CURSOR {
	RLIST_ITEM		item;
	DLIST_ITEM*		ditem;
};

struct DLIST_HEAD {
	RLIST_HEAD		head;
	RLIST_HEAD		clist;	
};

ZION_INLINE void dlist_init(DLIST_HEAD* list)//��ʼ��DLIST_HEAD�ڵ�
{
	rlist_init(&list->head);
	rlist_init(&list->clist);
}

ZION_INLINE void dlist_clear(DLIST_ITEM* item, void* ptr)//��ʼ��DLIST_ITEM�ڵ�
{
	rlist_init(&item->clist);
	rlist_clear(&item->item, ptr);
}

ZION_INLINE int dlist_empty(DLIST_HEAD* list)//�ж����HEAD�ڵ��Ƿ�Ϊ��
{
	return rlist_empty(&list->head);
}

ZION_INLINE DLIST_ITEM* dlist_front(DLIST_HEAD* list)//����HEAD�ڵ��һ��˳ʱ��ڵ�
{
	return (DLIST_ITEM*) rlist_front(&list->head);	
}

ZION_INLINE DLIST_ITEM* dlist_back(DLIST_HEAD* list)//����HEAD�ڵ��һ����ʱ��ڵ�
{
	return (DLIST_ITEM*) rlist_back(&list->head);
}

ZION_INLINE DLIST_ITEM* dlist_next(DLIST_HEAD* list, DLIST_ITEM* item)//����ITEM�ڵ����һ���ڵ�
{
	return (DLIST_ITEM*)rlist_next(&(item->item));
}

ZION_INLINE DLIST_ITEM* dlist_prev(DLIST_HEAD* list, DLIST_ITEM* item)//����ITEM�ڵ���һ���ڵ�
{
	return (DLIST_ITEM*)rlist_prev(&(item->item));
}

ZION_INLINE void dlist_insert(DLIST_HEAD* list, DLIST_ITEM* witem, DLIST_ITEM* item)//����ǰ��list����һ���ڵ�
{
	//rlist_insert(&list->head, &witem->item, &item->item);
	rlist_push_front(&list->head, &item->item);
}

ZION_INLINE void dlist_remove(DLIST_HEAD* list, DLIST_ITEM* item)//�Ƴ�ITEM�ڵ�
{
	DLIST_ITEM* next = NULL;
	if( ! dlist_empty(list) )
	{
		next = dlist_next(list,item);
		while( ! rlist_empty(&item->clist) && next != NULL)//��item��ǰ��cursorȫ���Ƶ�itemָ�����һ��item
		{
			rlist_push_front(&next->clist,  rlist_pop_front(&list->clist));
		}
	}
	rlist_remove(&list->head, &item->item);
}

ZION_INLINE void dlist_cursor_init(DLIST_HEAD* list, DLIST_ITEM* witem, DLIST_CURSOR* cur)//���α����List��ָ��witem
{
	rlist_push_front(&list->head, &cur->item);
	cur->ditem = witem;
}

ZION_INLINE void dlist_cursor_front(DLIST_HEAD* list, DLIST_CURSOR* cur)//˳ʱ������α�
{
	rlist_push_front(&list->clist, &cur->item);
}

ZION_INLINE void dlist_cursor_back(DLIST_HEAD* list, DLIST_CURSOR* cur)//��ʱ������α�
{
	rlist_push_back(&list->clist, &cur->item);
}

ZION_INLINE DLIST_ITEM* dlist_cursor_next(DLIST_HEAD* list, DLIST_CURSOR* cur)//��ǰ�ƶ��α�
{
	cur->ditem = (DLIST_ITEM*)rlist_next(&cur->item);
	return cur->ditem;
}

ZION_INLINE DLIST_ITEM* dlist_cursor_prev(DLIST_HEAD* list, DLIST_CURSOR* cur)//����ƶ��α�
{
	cur->ditem = (DLIST_ITEM*)rlist_prev(&cur->item);
	return cur->ditem;
}

#endif

