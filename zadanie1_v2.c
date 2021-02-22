// zadanie1.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#define MAX 5000
#include <stdio.h>
char* pam;
void* memory_alloc(unsigned int size) {
	int* p_fb = pam;
	int* p; //blok ktory budem alokovat
	p = *p_fb;
	int* prev = pam;
	if (size < 4) size += 4; // o 4B musim pridat lebo inac by sa mi nezmestil pointer 
	unsigned int size_block = *p >> 1;
	//uz nie je free blok
	if (*p == NULL) return NULL;
	//ak je moc maly hladam pokial nenajdem alebo pokial neskoncim na konci
	size_block = *p >> 1;
	while ((size_block < size) && p != NULL) {
		prev = p;
		if (*(p + 1) == NULL) return NULL;
		p = *(p + 1);
		//if (p == NULL) return NULL;
		if (*p == 0) return NULL;
		size_block = *p >> 1;
	}
	//ak som na konci dovi
	if (p == NULL) return NULL;
	int new_size = size_block - size - sizeof(int) - sizeof(int *); //nova celkova velkost free bloku ktory by zostal -8 este hlavicka+pointer na next free
	//printf("Velmi tazky vypocet new_size = %d - %d - %d\n", size_block, size, sizeof(int*));
	if (new_size <= 0) { //ak by sa neoplatilo ani sekat ten blok na dalsi free lebo by bol moc maly, dam mu vacsiu velkost ked tak
		//printf("\nNechavam cely blok, pretoze nasledujuci by bol moc maly (nezmesil by sa tam ani pointer na dalsi free blok)\n");
		printf("\nNesekam blok, pretoze by zostal moc maly\n");
		*p = (size_block << 1) + 1;
		if (prev != pam) *(prev + 1) = *(p + 1);
		else *prev = *(p + 1);
		*(p + 1) = 0; //vymazem ten pointer odtial na next_fb
		//printf("Velkost obsadeneho bloku je:%d\n", size_block);
		return p + 1;
	}
	int* next_fb = *(p + 1);
	*p = (size << 1) + 1; //zakodujem velkost a ze je obsadene cize si velkost posuniem o 1 bit

	int* new_fb = ((char*)p + (4 + size));
	//printf("\nNew fb adress:%p", new_fb);
	*new_fb = (new_size+4) << 1; //+4 lebo ukazovatel na novy fb sa pocita do velkosti bloku (iba hlavicka sa nepocita)
	*(new_fb + 1) = next_fb;
	//printf("\nNext_fb %p\n", next_fb);
	if (prev != pam) {
		*(prev + 1) = new_fb;
	}
	else {
		*prev = new_fb;
	}
	//printf("Velkost obsadeneho bloku: %d a velkost noveho free bloku %d\n", *p >> 1,*new_fb>>1);
	return p+1;
}
int memory_free(void* val_ptr) {
	int* head_val_ptr = (int*)val_ptr - 1;
	printf("\nSom vo free pre %p\n",val_ptr);
	int* p = pam; //p sluzi potom aj ako next_fb za miestom valid_ptr
	int size_block = *p >> 1; //toto neviem naco mi je
	int free = (*(int*)head_val_ptr) & 1;
	int* prev_fb = pam;
	//hladam 2 vedlajsie free bloky z oboch stran
	while (p < head_val_ptr && *p!=0) { //toto skonci bud ak som na konci pamate cize blok co idem mazat je hned pri konci, alebo ked p>valid_ptr cize mam prvy volny blok za valid_ptr
		prev_fb = p; //keby tu nemam ten if tak prev_fb by mohol byt aj napravo od toho bloku co idem uvolnovat...
		if (p == pam) p = *p;
		else p = *(p + 1);
	}
	printf("p:%p\n", p);
	int size_block_ptr = (*((int*)head_val_ptr)) >> 1; //tu je chyba treba to posunut o 2 ale neviem preco
	int* is_nextr = ((char*)head_val_ptr + (4 + size_block_ptr));
	//new verzia
	if (*p == NULL) {
		printf("\nP je null cize zapisem do next null");
		//*(int*)valid_ptr = *(int*)valid_ptr >> 1;
		*(int*)head_val_ptr = *(int*)head_val_ptr & ~1;
		*((int*)head_val_ptr + 1) = p;
	}
	else {
		if (is_nextr == p) {//je hned za tym blokom pamate
			int new_fb_size = (*(int*)head_val_ptr >> 1) + (*p >> 1) + sizeof(int*);
			*(int*)head_val_ptr =new_fb_size <<1; //zapisem si novu velkost pamate
			//*(int*)head_val_ptr = *(int*)head_val_ptr & ~1;
			printf("\nP nie je null a je hned za tym blokom pamate cize zapisem  %p adresu ako ukazovatel do noveho free bloku a nova velkost bude %d\n", (p + 1),new_fb_size);
			*((int*)head_val_ptr + 1) = *(p + 1);
		}
		else { //nie je za tym blokom pamate
			*(int*)head_val_ptr = *(int*)head_val_ptr & ~1;
			printf("\nP nie je null a nie je hned za tym blokom pamate cize zapisem tam %p", (p));;
			*((int*)head_val_ptr + 1) = p;
		}
	}
	if (prev_fb != pam) { //prev je niekde dalej ako prvy pointer na zaciatku pamate
		printf("\nPrev nie je pam (prvy pointer na free blok na zaciatku pamate");
		int size_prev = *prev_fb >> 1;
		int* is_nextl = ((char*)prev_fb + (4 + size_prev));
		if (is_nextl == head_val_ptr) { //prev je hned vedla a mozem ho spojit
			printf("\nPrev je hned vedla cize spojim to cele a budem ukazovat na next_fb %p", (p));;
			size_prev = size_prev + (*(int*)head_val_ptr >> 1) + sizeof(int*); //
			*(prev_fb + 1) = *((int*)head_val_ptr + 1); //tu som mal predtym = p COULD BE A PROBLEM
			*(int*)head_val_ptr = *(int*)head_val_ptr & ~1;
			*(prev_fb) = size_prev << 1;
		}
		else {//prev nie je hned vedla ale ani to nie je ten ukazoval na zaciatku pamate, cize ich spojm
			*(prev_fb + 1) = head_val_ptr;
		}
	}
	else { //potom je prev pointer na prvy free block na zaciatku pamati a medzi prev a novym blokom ktory idem mazat nie je nic free cize prev ide ukazovat na valid_ptr
		//printf("\nPrev nie je hned vedla cize budem ukazovat na next_fb %p", (head_val_ptr));;
		printf("\nVelkost noveho free bloku je %d\n", *head_val_ptr >> 1);
		*(prev_fb) = head_val_ptr;
	}
	return 1;
}
void stav_pamate() {
	for (int i = 0; i < 100; i++) {

	}
	
}
int memory_check(void* ptr) {
	int* point = (int *)pam+1;//aby som vyskocil z toho prveho ukazovatela
	int s_add = (int* )ptr - 1;
	while (point != NULL && point != s_add) {
		int free = *point & 1;
		if (point == s_add && free) return 1;
		int size = *point >> 1;
		point = ((char*)point + (4 + size));
	}
	if (point == ptr) return 1;
	return 0;
}
void memory_init(void* ptr, unsigned int size) {
	int real_size = size - sizeof(int) - 2*sizeof(int*);
	//printf("\nreal size %d\n", real_size);
	int* foot = ((char*)ptr + 8 + real_size);
	*(int*)ptr = (int *)ptr+1;
	*((int*)ptr+1) = (size - sizeof(int) - 2*sizeof(int*)) << 1;
	*((int*)ptr + 2) = foot;
	char* test = ((char*)ptr + 4 + real_size); //+8 lebo 4B ma integer ktory urcuje velkost bloku,po nom ide pointer na dalsi novy blok, ten je na zaciatku pointer na patu celeho bloku
	*foot = NULL;
	printf("Koncim init: foot: %p next_free: %p prvy ukazovatel %p\n", foot, *((int*)ptr), ptr);
}


int main()
{
	char pamat[MAX];
	char* k = pamat;
	int* k2 = pamat;
	printf("k:%p k2:%p, k+4:%p k2+1:%p\n", k, k2, (k + 4), (k2 + 1));
	for (int i = 4; i < 10; i++) {
		pamat[i] = 99;
	}
	int* ptr = pamat;
	int* test;
	char* test2;
	printf("\n%d %d", sizeof(test), sizeof(test2));
	*ptr = 513;
	int free = *ptr & 1;
	int size = *ptr & ~1;

	int* p = pamat;
	pam = pamat;

	/* TEST1
	memory_init(pamat, 100);
	printf("%p\n", pamat);
	printf("Po init  %d %p\n", pamat[96], &pamat[96]);
	printf("%d", *p);
	char* p1 = (char*)memory_alloc(8);
	printf("\nsize:%d free:%d, %p", *((int*)p1-1) >> 1, *(int*)p1 & 1, p1);
	char* p2 = (char*)memory_alloc(20);
	char* p3 = (char*)memory_alloc(150);
	printf("\np3:%p", p3);
	memory_free(p1);
	printf("\n%p\n", p2);
	printf("%d", memory_check(p2));
	char* p4 = (char*)memory_alloc(52);
	printf("\np4:%p", p4);
	memory_free(p2);
	memory_free(p4);
	p1 = (char*)memory_alloc(8);
	p2 = (char*)memory_alloc(24);
	p3 = (char*)memory_alloc(12);
	p4 = (char*)memory_alloc(8);
	char *p5 = (char*)memory_alloc(8);
	printf("\nfree p3: %d",memory_free(p3));
	printf("\nfree p5: %d", memory_free(p5));
	*/
		/* TEST 2
	char* p1 = (char*)memory_alloc(8);
	printf("Pointer p1:%p\n", p1);
	char* p2 = (char*)memory_alloc(8);
	printf("Pointer p2:%p\n", p2);
	char* p3 = (char*)memory_alloc(8);
	printf("Pointer p3:%p\n", p3);
	char* p4 = (char*)memory_alloc(8);
	printf("Pointer p4:%p\n", p4);
	char* p5 = (char*)memory_alloc(8);
	printf("Pointer p5:%p\n", p5);
	char* p6 = (char*)memory_alloc(8);
	printf("Pointer p6:%p\n", p6);
	printf("Memory free p4:%d", memory_free(p4));
	char* p7 = (char*)memory_alloc(8);
	printf("Pointer p7:%p\n", p7);
	printf("Memory free p1:%d", memory_free(p1));
	char* p8 = (char*)memory_alloc(8);
	printf("Pointer p8:%p\n", p8);
	*/

	//TEST 3
	/*memory_init(pamat, 100);
	char* p1 = (char*)memory_alloc(8);
	printf("P1:%p\n",p1);
	char* p2 = (char*)memory_alloc(16);
	printf("P2:%p\n", p2);
	char* p3 = (char*)memory_alloc(24);
	printf("P3:%p\n", p3);
	printf("free p3: %d\n", memory_free(p3));
	printf("free p1: %d\n", memory_free(p1));
	printf("free p2: %d\n", memory_free(p2));
	int* free_b = *(int  *)pam;
	printf("free size:%d\n", *free_b >> 1);*/

	//TEST 4
	memory_init(pamat, 5000);
	char* p1 = (char*)memory_alloc(500);
	printf("P1:%p\n", p1);
	char* p2 = (char*)memory_alloc(650);
	printf("P2:%p\n", p2);
	char* p3 = (char*)memory_alloc(800);
	printf("P3:%p\n", p3);
	printf("free p1: %d\n", memory_free(p1));
	printf("free p2: %d\n", memory_free(p2));
	printf("free p3: %d\n", memory_free(p3));
	int* free_b = *(int*)pam;
	printf("free size:%d\n", *free_b >> 1); 
	printf("%d", sizeof(int*));
	return 0;
}

