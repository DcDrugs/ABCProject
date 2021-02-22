#include "ll_cycle.h"
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
}
