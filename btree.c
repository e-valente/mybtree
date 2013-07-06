#include "btree.h"

int search(const int offset, int pKey, int* offset_found, int* pos_found)
{
	int pos = 0;
	if (offset == -1)return FAIL;

	page_tree * curr_page;
	curr_page = loadPage(offset);

	if (pageSearch(curr_page, pKey, &pos) == SUCCESS)
	{
		*offset_found = offset;
		*pos_found = pos;
		free(curr_page);
		return SUCCESS;
	}
	*offset_found = offset;
	*pos_found = pos;
	free(curr_page);

	return search(curr_page->child[pos], pKey, offset_found, pos_found);
}

page_tree* loadPage(int offset)
{
	page_tree* return_page;
	fseek(bTreeFile, offset, SEEK_SET);
	return_page = (page_tree*)malloc(sizeof(page_tree));
	fread(return_page, sizeof(page_tree), 1, bTreeFile);
	return return_page;
}

int pageSearch(const page_tree* curr_page, int pKey, int* pos)
{
	int i;
	for (i = 0; i < curr_page->nKeys; i++)
	{
		if (pKey == curr_page->keys[i])
		{
			*pos = i;
			return SUCCESS;
		}
		else if (pKey < curr_page->keys[i])
		{
			*pos = i;
			return FAIL;
		}
	}
	*pos = i;
	return FAIL;	
}

int insert(int offset, int key) {

	int key_promoted = -1;
	int return_value = -1;
	int new_offset = -1;
	int offset_child_right = -1;
	int offset_child_left = -1;
	page_tree *page;

	// verifica se eh necessario criar um novo no raiz
	if ((return_value = insertKey(offset, key, &offset_child_left, &offset_child_right, &key_promoted)) == PROMOTED) {
		// aloca nova pagina na ram
		allocPage(&page);
		// insere chave na nova pagina criada (ainda na RAM)
		pageInsert(page, key_promoted, offset_child_left, offset_child_right);
		//emanuel
		page->child[0] = offset_left;
		page->child[0] = offset_child_left;

		printf("CRIANDO NOVO NÓ COM key %d e left vale %d\n", key_promoted, offset_child_left);
		// move o ponteiro para o final do arquivo para pegar a posicao da nova pagina
		fseek(bTreeFile, 0L, SEEK_END);
		new_offset = ftell(bTreeFile);
		// salva a pagina no disco
		savePage(new_offset, page);
		update_root_offset(new_offset);
		free(page);
	}

	return return_value;
}

int insertKey(int offset, int key, int* child_left_promoted, int* child_right_promoted, int* key_promoted)
{
	page_tree *curr_page; // pagina atual para busca
	page_tree *new_page; // pagina para split
	int pos = -1; // posicao em que a chave deveria ser inserida

	// trata o caso de arvore vazia
	if (offset == NIL) {
		*key_promoted = key;
		*child_right_promoted = NIL;
		*child_left_promoted = NIL;
		return PROMOTED;
	}

	curr_page = loadPage(offset); // carrega a pagina para inspecao
	if (pageSearch(curr_page, key, &pos) == SUCCESS) {
		fprintf(stderr, "Chave ja existe na arvore\n\n");
		return ERROR;
	}

	int return_value = -1;
	int right_offset_child_promoted = -1;
	int left_offset_child_promoted = -1;
	int key_promotion = -1;
	return_value = insertKey(curr_page->child[pos], key, &left_offset_child_promoted, &right_offset_child_promoted, &key_promotion);

	// verifica se nao eh necessaria mais nenhuma operacao na arvore
	// ou seja, a chave foi inserida mas nao ha promocao
	// ou entao a chave ja existia e foi retornado um erro
	if (return_value == NOT_PROMOTED || return_value == ERROR) {
		return return_value;
	}

	// verifica se a chave cabe na pagina atual
	if (curr_page->nKeys < ORDER-1) {
		pageInsert(curr_page, key_promotion, left_offset_child_promoted, right_offset_child_promoted);
		savePage(offset, curr_page);
		return NOT_PROMOTED;
	}

	// se a chave nao couber na pagina, realiza split
	fprintf(stderr, "chamando split... offset vale %d e chave vale %d\n", offset, key_promotion);

	split(key_promotion, right_offset_child_promoted, curr_page, key_promoted, child_left_promoted, child_right_promoted, &new_page);
	*child_left_promoted = offset;
	savePage(offset, curr_page); // salva a pagina atual
	savePage(*child_right_promoted, new_page); // salva a nova pagina apos split

	free(curr_page);
	free(new_page);

	return PROMOTED;
}

void open_initialize_file() {
	bTreeFile = fopen("data_btree.dat", "wb+");
	int offset_root_new = -1;
	fwrite(&offset_root_new, sizeof(int), 1, bTreeFile);
	offset_root = -1;
	offset_left = -1;
}

void update_root_offset(int offset) {
	fseek(bTreeFile, 0, SEEK_SET);
	fwrite(&offset, sizeof(int), 1, bTreeFile);
	offset_root = offset;
}

void savePage(const int offset, const page_tree * newpage) {
	fseek(bTreeFile, offset, SEEK_SET);
	fwrite(newpage, sizeof(page_tree), 1, bTreeFile);
}


void allocPage(page_tree **newpage) {
	int i;
	*newpage = (page_tree*) malloc(sizeof(page_tree));
	for (i = 0; i < ORDER-1; i++) {
		(*newpage)->keys[i] = NIL;
		(*newpage)->child[i] = NIL;
	}
	(*newpage)->child[i] = NIL;
	(*newpage)->nKeys = 0; // total de chaves
}

void resetPage(page_tree *page) {
	int i;
	for (i = 0; i < ORDER-1; i++) {
		page->keys[i] = NIL;
		page->child[i] = NIL;
	}
	page->child[i] = NIL;
	page->nKeys = 0; // total de chaves
}

void pageInsert(page_tree *curr_page, int key, const int offset_left_child, const int offset_right_child) {

	int i;
	// se a pagina for nova, e estiver vazia
	if (curr_page->nKeys == 0) {
		curr_page->nKeys++;
		curr_page->child[1] = offset_right_child;
		curr_page->child[0] = offset_left_child;
		curr_page->keys[0] = key;
		return;
	}

	//printf("===>>>>criando pagina com key %d e left %d\n", key, offset_left_child);

	// percorre a pagina do ultimo elemento para o primeiro
	// copia os elementos maiores do que a chave para a direita
	for (i = curr_page->nKeys-1; i >= 0; i--) {

		if (curr_page->keys[i] > key) {
			// desloca chaves maiores e seus filhos para a direita
			curr_page->keys[i+1] = curr_page->keys[i];
			curr_page->child[i+2] = curr_page->child[i+1];

			if (i == 0) {
				curr_page->keys[i] = key;
				//curr_page->child[0] = offset_left_child;
				curr_page->child[1] = offset_right_child;
				curr_page->nKeys++;
			}
		}
		else {
			curr_page->keys[i+1] = key;
			//curr_page->child[i+1] = offset_left_child;
			curr_page->child[i+2] = offset_right_child;
			curr_page->nKeys++;
			break;
		}
	}
}

void printBTree(int offset, int altura)
{
	if(offset == NIL) return;
	int i;

	page_tree *nextPage;

	nextPage = loadPage(offset);
	//printf("abrindo offset: %d\n", offset);

	printf("Altura: %d | num. Chaves: %d | chaves = [ ", altura, nextPage->nKeys);

	for (i = 0; i < nextPage->nKeys; i++) {
		printf("%d ", nextPage->keys[i]);
	}

	printf("]\n");

	for (i = 0; i < ORDER; i++) {
		printf("filho %d de onde tem a chave %d vale %d\n", i, nextPage->keys[1], nextPage->child[i]);
		printBTree(nextPage->child[i], altura + 1);
	}
}

int searchKeyOnBTree(int offset, int key)
{
	if(offset == NIL) return -1;
	int i;

	page_tree *nextPage;

	nextPage = loadPage(offset);
	//printf("abrindo offset: %d\n", offset);


	for (i = 0; i < nextPage->nKeys; i++) {
		if(nextPage->keys[i] == key) return offset;
	}

	for (i = 0; i < ORDER; i++) {
		offset = nextPage->child[i];
		if(searchKeyOnBTree(offset, key) > 0) return offset;
	}

	return -1;
}

//split(key_promotion, offset_child_promoted, curr_page, key_promoted, child_right_promoted, &new_page);
void split(const int key_input, const int child_right_input, page_tree* page, int* key_promoted, int* child_left_promoted, int* child_right_promoted, page_tree** new_page)
{
	int i, tmp_key;
	int size_split;
	int key_idx, child_idx;
	printf("inserindo chave %d\n", key_input);

	if(key_input == 3)
	{
		printf("vai\n");
	}

	/*aloca nova pagina
	 *note que new_page
	 *é ponteiro **
	 */

	/*size_split terá o numero de chaves
	 * que iremos colocar na pagina nova
	 */

	allocPage(new_page);

	size_split = (ORDER -1)/2;


	/*copia as chaves do split para
	 * a nova pagina
	 */
	for(i = 0, key_idx = ORDER - 2, child_idx = ORDER -1; i < size_split; i++, key_idx--, child_idx --)
	{
		pageInsert(*new_page, page->keys[key_idx], page->child[child_idx-1], page->child[child_idx]);
		page->child[child_idx] = NIL;
		//page->child[child_idx-1] = NIL;
		page->nKeys--;

	}

	/*verifica se a chave atual é maior que a chave da direita
	 * da página antiga. CAso seja maior, será inserido
	 * na nova página, caso contrário, será inserido nova página
	 */


//	printf("comparando %d com %d\n", key_input, page->keys[ORDER-2 - size_split]);
	if(key_input > page->keys[ORDER-2 - size_split])
	{
		/*insere a chave atual*/
		pageInsert(*new_page, key_input, NIL, child_right_input);


	} else{
		/*antes o ultimo nó da pagina antiga é transferido
		 * pra pagina nova. E depois inseriamos a chave atual na pagina antiga
		 */
		pageInsert(*new_page, page->keys[ORDER -2 -size_split], page->child[ORDER-1-size_split-1], page->child[ORDER-1-size_split]);
		page->child[ORDER-1-size_split] = NIL;
		page->nKeys--;
		//page->nKeys--;
		pageInsert(page, key_input, NIL, child_right_input);

	}



	/*promovemos a chave da esquerda da nova pagina
	 * pois será a menor
	 */

	*key_promoted = (*new_page)->keys[0];
	*child_left_promoted = (*new_page)->child[0];

	//printf("==>child left promoted vale %d e key promovida vale %d\n", *child_left_promoted, *key_promoted);



	/*deslocamos as chaves de nova
	 * pagina para esquerda
	 */

	for(i = 0; i < (*new_page)->nKeys-1; i++)
	{
		(*new_page)->keys[i] = (*new_page)->keys[i+1];
		(*new_page)->child[i] = (*new_page)->child[i+1];


	}

	(*new_page)->child[i] = (*new_page)->child[i+1];
	(*new_page)->child[i+1] = NIL;

	/*remove de fato a chave na nova pagina*/
	(*new_page)->nKeys--;


	/*obtem o offset_left: Este offset é o offset
	 * da pagina antiga (que foi particionada)
	 *
	 */

	offset_left = searchKeyOnBTree(offset_root, page->keys[0]);
	*child_left_promoted = offset_left;

	//printf("||SPLIT e child left vale %d, procurei pela chave %d\n", offset_left, page->keys[0]);

	/*obtem o offset da nova pagina:
	 * basta posicionar no fim do arquivo e retornar o valor
	 * do offset em bytes
	 */


	fseek(bTreeFile, 0L, SEEK_END);
	*child_right_promoted = ftell(bTreeFile);


	//printf("conteudo da pagina atual:\n");
	//printf("%d\n", (*new_page)->keys[0]);






}


