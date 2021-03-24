#include "ll_cycle.h"
<<<<<<< HEAD
#define NULL 0

bool ll_has_cycle(node *head) {
    node *pnext = head;
    int i = 0;
    while(pnext != NULL)
    {
    	pnext = pnext->next;
    	if (pnext == head)
    		return true;
    	if(i == 2)
    	{
    		i = 0;
    		head = head->next;
    	}
    	i++;
    }
    return false;
=======

bool ll_has_cycle(node *head) {
    /* Ваш код должен быть написан только внутри этой функции */
    return true;
>>>>>>> 288a0d4e7794452fa1c5e2203d20a995068bda9c
}
