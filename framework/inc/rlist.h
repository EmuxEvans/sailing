#ifndef	__RLIST_H__
#define	__RLIST_H__

struct RLIST_ITEM;
typedef struct RLIST_ITEM RLIST_ITEM;
struct RLIST_HEAD;
typedef struct RLIST_HEAD RLIST_HEAD;

struct RLIST_ITEM {
	RLIST_ITEM*		next;
	RLIST_ITEM*		prev;
	void* ptr;
};

struct RLIST_HEAD {
	RLIST_ITEM		item;
};

ZION_INLINE void rlist_init(RLIST_HEAD* list)//��head�ڵ���г�ʼ��
{
	list->item.next = &list->item;
	list->item.prev = &list->item;
	list->item.ptr  = NULL;
}

ZION_INLINE void rlist_clear(RLIST_ITEM* item, void* ptr)//��item�ڵ���г�ʼ��
{
	item->next	= NULL;
	item->prev	= NULL;
	item->ptr	= ptr;
}

ZION_INLINE int rlist_empty(RLIST_HEAD* list)//�жϵ�ǰ�ĵ������Ƿ�Ϊ��
{
	return list->item.next==&list->item;
}

ZION_INLINE int rlist_is_head(RLIST_HEAD* list, RLIST_ITEM* item)//�ж�item�ڵ��Ƿ�Ϊͷ�ڵ�
{
	return(item==&list->item);
}

ZION_INLINE RLIST_ITEM* rlist_front(RLIST_HEAD* list)//��õ�һ��item�ڵ㣨ͷ�ڵ���һ���ڵ㣩
{
	return(list->item.next);
}

ZION_INLINE RLIST_ITEM* rlist_back(RLIST_HEAD* list)//������һ��item�ڵ�(ͷ�ڵ�ǰһ���ڵ�)
{
	return(list->item.prev);
}

ZION_INLINE RLIST_ITEM* rlist_next(RLIST_ITEM* item)//��õ�ǰ�ڵ�ĺ�һ���ڵ�
{
	return(item->next);
}

ZION_INLINE RLIST_ITEM* rlist_prev(RLIST_ITEM* item)//��õ�ǰ�ڵ��ǰһ���ڵ�
{
	return(item->prev);
}

ZION_INLINE void rlist_set_userdata(RLIST_ITEM* item, void* ptr)//����item�ڵ��ֵ
{
	item->ptr = ptr;
}

ZION_INLINE void* rlist_get_userdata(RLIST_ITEM* item)//���item�ڵ��ֵ
{
	return(item->ptr);
}

ZION_INLINE void rlist_insert(RLIST_HEAD* list, RLIST_ITEM* witem, RLIST_ITEM* item)//��ָ���ڵ�����ڵ�
{
	item->prev = witem->prev;
	item->next = witem;
	witem->prev->next = item;
	witem->prev = item;
}

ZION_INLINE void rlist_append(RLIST_HEAD* list, RLIST_ITEM* witem, RLIST_ITEM* item)//��ָ���ڵ�ǰ����ڵ�
{
	item->prev = witem;
	item->next = witem->next;
	witem->next->prev = item;
	witem->next = item;
}

ZION_INLINE RLIST_ITEM* rlist_remove(RLIST_HEAD* list, RLIST_ITEM* item)//�Ƴ��ڵ�
{
	item->prev->next = item->next;
	item->next->prev = item->prev;
	item->next = NULL;
	item->prev = NULL;
	return(item);
}

#define rlist_push_front(list, aitem)		rlist_append((list), &(list)->item, aitem)
#define rlist_push_back(list, aitem)		rlist_insert((list), &(list)->item, aitem)
#define rlist_pop_front(list)				(rlist_empty(list)?NULL:rlist_remove(list, (list)->item.next))
#define rlist_pop_back(list)				(rlist_empty(list)?NULL:rlist_remove(list, (list)->item.prev))

#endif
