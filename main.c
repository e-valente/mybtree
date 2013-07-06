#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "btree.h"

int main(void) 
{
	open_initialize_file();
	
 	insert(offset_root, 500);
	insert(offset_root, 1000);
 	insert(offset_root, 600);
 	insert(offset_root, 700);
	insert(offset_root, 800);
	insert(offset_root, 900);
	insert(offset_root, 1500);
	insert(offset_root, 1600);
	insert(offset_root, 1700);
	insert(offset_root, 1800);
	insert(offset_root, 1900);
	insert(offset_root, 2000);
	insert(offset_root, 123456789123);
//	insert(offset_root, 300);
//	insert(offset_root, 200);
//    insert(offset_root, 100);
//    insert(offset_root, 1501);
//    insert(offset_root, 501);
   // insert(offset_root, 450);
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
