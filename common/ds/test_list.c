/*
Used to test the list implimention.
compile with:
	gcc --std=gnu99 list.c test_list.c -o test_list
*/

#include <stdio.h>
#include <stdint.h>
#include "glist.h"
#define LN char

/*     name,fnattr,dattr,dtype,itype */
LIST_DEFINE(LN,static,,char,uint8_t);
char buff[128];
list_t(LN) L = LIST_INITIALIZER(buff);

#define ITER(x) for(size_t i = 0; i < x->sz; i++)

void list_print(list_t(LN) *l) {
	printf("\n{-\ni:\t");
	// the index numbers
	ITER(l) {
		printf("%zd\t",i);
	}
	printf("\nv:\t");
	// the contents (values)
	ITER(l) {
		printf("%d\t", l->buffer[i]);
        }
        printf("\n\t");
        
	{
	// mark elements which are valid members.
	int list_i = 0;
	int buff_i = l->first;
	for( int i=0; i< buff_i; i++ ) 
		printf("\t");
	for( ; list_i < l-> ct; (list_i++, buff_i++) ) {
		if (buff_i >= l->sz) {
			buff_i = 0;
			puts("\r\t");
		}
		printf("^\t");
	}
	printf("\n\t");
	}
	
	// first and end markers.
        ITER(l) {
		bool fst = i==l->first;
		bool end = i==l->end;
		if (fst && end) 
			printf("fe");
		else if (fst)
			printf("f");
		else if (end)
			printf("e");
		else
			printf(" ");
		printf(" \t");
        }
	printf("\n");
	
	
	printf("\n");
	// peak function results.
	printf("\tpeek_front : %d\n", list_peekf(LN)(&L));
	printf("\tpeek_back  : %d\n", list_peekb(LN)(&L));
	
	printf("-}\n\n");
}


int main ( int argc, char **argv ) {
	
	printf("> map (push_front &L) [0..11] ;\n ");
	for(int i = 0; i <= 11; i++ )
		if( list_pushf(LN)(&L,i) ) {
			printf("error, %d\n",i);
		}
	list_print(&L);
	
	printf("> do (pop_back &L) 10 : \t");
	for(int i = 0; i < 9; i++)
		printf("%d\t",list_popb(LN)(&L));
	list_print(&L);
	
	printf("> map (push_back &L) [10..18] ;\n ");
	for(int i = 10; i < 18; i++)
		list_pushb(LN)(&L,i);
	list_print(&L);

	printf("> do (pop_front &L) 5 : \t");
	for(int i = 0; i < 5; i++)
		printf("%d\t",list_popf(LN)(&L));
	list_print(&L);
	
	printf("> map peek &L [0..5] : \t");
	for(int i = 0; i < 6; i++) {
		printf("%d\t",list_peek(LN)(&L,i));
		fflush(stdout);
	}
	list_print(&L);

	return 0;
}
