#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "btree.h"

int main(void) 
{
	open_initialize_file();
	
 	insert(offset_root, 5);
	insert(offset_root, 10);
 	insert(offset_root, 6);
 	insert(offset_root, 7);
	insert(offset_root, 8);
	insert(offset_root, 9);
	insert(offset_root, 15);
	insert(offset_root, 4);
	insert(offset_root, 3);
	insert(offset_root, 2);
	insert(offset_root, 1);
	//insert(offset_root, 1);

  //	printBTree();
//  	insert(offset_root, 8);
//  	printBTree();
//  	insert(offset_root, 15);
//  	printBTree();
// 	
// 	insert(offset_root, 15);
// 	insert(offset_root, 11);
// 	printBTree();
// 
// 	insert(offset_root, 12);
// 	insert(offset_root, 30);
 	printBTree(offset_root, 0);
// 	getchar();
// 	printf("\n");
	return 0;
}
