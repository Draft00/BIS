#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <string.h>

FILE* fin, * fout;

int size;
int poz_print = 0, block = 0;
unsigned char buftext[64] = { 0 };
unsigned char text[72] = { 0 };
int kontrol[] = { 0, 1, 3, 7, 15, 31, 63 };

unsigned char kodtext[72] = { 0 };
unsigned char dekodtext[64] = { 0 };

enum
{
	CODE = 1,
	DECODE,
	STOP,
};
void form_char()
{
	for (int i = 0; i < (size + poz_print) / 8; i++)
	{
		int k = 0;
		for (int t = 0; t < 8; t++)
		{
			k = k << 1;
			k += (int)text[t];
		}
		int c = (char)k;
		printf("\n%d - and this is k", k);
		k = 0;
		fprintf(fout, "%c", c);
		for (int m = 0; m < 8 * ((size + poz_print) / 8 - 1); m++)
			text[m] = text[m + 8];
		printf("Now its a shift text: ");
		for (int i = 0; i < 8; i++)
			printf("%d", text[i]);
	}
	int l = 0;
	for (int i = (size + poz_print) / 8 * 8; i < size + poz_print; i++)
	{
		text[l] = text[i];
		l++;
	}
	poz_print = l;
	for (int i = poz_print; i < size + poz_print; i++)
		text[i] = 0;

}

void size_and_blocks()
{
	printf("Введите размер блока: ");
	scanf("%d", &block);
	while (block != 8 && block != 12 && block != 16 && block != 24 && block != 32 && block != 48 && block != 64)
	{
		printf("Некорректное значение! Доступно: 8, 12, 16, 24, 32, 48, 64. Попробуйте снова: ");
		fflush(stdin);
		scanf("%d", &block);
	}
	if (block == 8)
		size = 12;
	if (block == 12)
		size = 17;
	if (block == 16)
		size = 21;
	if (block == 24)
		size = 29;
	if (block == 32)
		size = 38;
	if (block == 48)
		size = 54;
	if (block == 64)
		size = 71;
}

void form_kontrol_bits()
{
	int z = 0;
	for (int i = 0; i < size; i++)
	{
		if (i == 0 || i == 1 || i == 3 || i == 7 || i == 15 || i == 31 || i == 63)
			text[i + poz_print] = 0;
		else
		{
			text[i + poz_print] = buftext[z];
			z++;
		}
	}
	for (int m = 0; m < size; m++)
	{
		if (text[m + poz_print])
			kontrol_bits(m);
	}
}

void kontrol_bits(int poz1)
{
	int binview[8] = { 0 };
	//добавлено
	for (int i = 0; i < 7; i++)
	{
		if (poz1 == kontrol[i])
			return;
	}

	poz1++;
	for (int i = 0; i < 8; i++)
	{
		binview[i] = poz1 % 2;
		poz1 /= 2;
	}
	for (int i = 0; i < 8; i++)
	{
		if (binview[i])
			text[kontrol[i] + poz_print] ^= 1;
	}
}
void coder()
{
	fin = fopen("input.txt", "rb");
	fout = fopen("output.txt", "wb");
	size_and_blocks();
	memset(buftext, 0, 72);
	int poz = 0, ostatok = 0, count = 0;
	int c = 2;
	while (c != EOF && c != 10)
	{
		c = fgetc(fin);
		if (c == EOF || c == 10)
		{
			ostatok += count;
			if (ostatok == 1)
				size = 3;
			else if (ostatok <= 4)
				size = ostatok + 3;
			else if (ostatok <= 11)
				size = ostatok + 4;
			else if (ostatok <= 26)
				size = ostatok + 5;
			else if (ostatok <= 57)
				size = ostatok + 6;
			else size = ostatok + 7;
			printf("poz_print - %d", poz_print);
			printf("buftext: ");
			for (int i = 0; i < ostatok; i++)
				printf("%d", buftext[i]);
			printf("size - %d\n", size);
			printf("Text: ");
			for (int i = 0; i < size + poz_print; i++)
				printf("%d", text[i]);
			printf("poz_pront - %d", poz_print);
			if (count + ostatok != 0)
			{
				printf("No null ");
				printf("form kont bits ");
				form_kontrol_bits();
			}

			for (int i = 0; i < size + poz_print; i++)
				printf("%d", text[i]);
			form_char();
			//printf("\n%d\n", poz_print);
			printf("after form char ");
			for (int i = 0; i < size + poz_print; i++)
				printf("%d", text[i]);
			printf("\n\n%d, %d", size, poz_print);
			if (poz_print != 0)
			{
				int k = 0;
				for (int t = 0; t < 8; t++)
				{
					k = k << 1;
					k += (int)text[t];

				}
				int c = (char)k;
				k = 0;
				fprintf(fout, "%c", c);
			}
			break;
		}
		if (ostatok + count < block)
		{
			for (int k = 0; k < 8; k++)
			{
				buftext[poz + ostatok] = ((c >> (8 - k - 1)) & 1);
				poz++;
			}
			count += 8;
		}
		printf("Buf after full: ");
		for (int i = 0; i < count + ostatok; i++)
			printf("%d", buftext[i]);
		printf("\n");

		if (ostatok + count >= block)
		{
			poz = 0;
			//printf("dolzno %c", c);
			printf("With control bits: ");
			form_kontrol_bits();
			for (int i = 0; i < size + poz_print; i++)
				printf("%d", text[i]);
			form_char();
			printf("\n");
			printf("Text after shift: ");
			for (int i = 0; i < poz_print; i++)
				printf("%d", text[i]);
			printf("poloz - %d", poz_print);
			int z = 0;
			printf("\nbuf after chift: ");
			for (int y = block; y < count; y++)
			{
				buftext[z] = buftext[y];
				z++;
			}
			for (int i = z; i < count + ostatok; i++)
				buftext[i] = 0;
			for (int i = 0; i < size; i++)
				printf("%d", buftext[i]);
			printf("\n%d count before", count);
			int s = ostatok + count - block;
			ostatok = s;
			count = 0;
			printf("\n%d, %d - ostatok and count\n", ostatok, count);
		}
	}
	fclose(fin);
	fclose(fout);
}

void find_kon_bits_for_decoder(int poz1, int new_kontrol_bits[])
{
	int binview[8] = { 0 };
	for (int i = 0; i < 7; i++)
		if (poz1 == kontrol[i])
			return;

	poz1++;
	for (int i = 0; i < 8; i++)
	{
		binview[i] = poz1 % 2;
		poz1 /= 2;
	}
	for (int i = 0; i < 8; i++)
	{
		if (binview[i])
			new_kontrol_bits[i] ^= 1;
	}
}

void del_control_bits()
{
	char new_symbol = 0;
	int read = 0, i = 0;
	while (read != block)
	{
		if (i == 0 || i == 1 || i == 3 || i == 7 || i == 15 || i == 31 || i == 63)
		{
		}
		else
		{
			dekodtext[read + poz_print] = kodtext[i];
			read++;
		}
		i++;
	}
}

void form_char_for_decode()
{
	for (int i = 0; i < (block + poz_print) / 8; i++)
	{
		int k = 0;
		for (int t = 0; t < 8; t++)
		{
			k = k << 1;
			k += (int)dekodtext[t];
		}
		int c = (char)k;
		k = 0;
		fprintf(fout, "%c", c);
		for (int m = 0; m < 8 * ((block + poz_print) / 8 - 1); m++)
			dekodtext[m] = dekodtext[m + 8];
	}
	int l = 0;
	for (int i = (block + poz_print) / 8 * 8; i < block + poz_print; i++)
	{
		dekodtext[l] = dekodtext[i];
		l++;
	}
	poz_print = l;
	for (int i = poz_print; i < block + poz_print; i++)
		dekodtext[i] = 0;
}
void decoder()
{
	fin = fopen("output.txt", "rb");
	fout = fopen("final.txt", "wb");
	size_and_blocks();
	poz_print = 0;
	int old_kontrol_bits[7] = { 0 };
	int new_kontrol_bits[7] = { 0 };
	int c = 2, ostatok = 0, count = 0, poz = 0;
	while (c != EOF)
	{
		c = fgetc(fin);
		if (c == EOF)
		{
			printf("ENDENDEND\n\n");
			for (int i = 0; i < 18; i++)
				printf("%d", dekodtext[i]);
			ostatok += count;
			//if (ostatok == 1)
			   // block = 3;
			if (ostatok <= 8)
				block = ostatok - 3;
			else if (ostatok <= 16)
				block = ostatok - 4;
			else if (ostatok <= 32)
				block = ostatok - 5;
			else if (ostatok <= 64)
				block = ostatok - 6;
			else block = ostatok - 7;
			printf("block is - %d\n", block);
			printf("kodtext is \n");
			for (int i = 0; i < ostatok; i++)
				printf("%d", kodtext[i]);
			printf("Decode text now: ");
			for (int i = 0; i < poz_print; i++)
				printf("%d", dekodtext[i]);
			printf("poz - %d\n", poz_print);
			del_control_bits();
			printf("Now kontr bis was deleted: ");
			for (int i = 0; i < block + poz_print; i++)
				printf("%d", dekodtext[i]);
			form_char_for_decode();
			printf("After char form: ");
			for (int i = 0; i < block + poz_print; i++)
				printf("%d", dekodtext[i]);
			/* if (poz_print != 0)
			 {
				 int k = 0;
				 for (int t = 0; t < 8; t++)
				 {
					 k = k << 1;
					 k += (int)text[t];
				 }
				 int c = (char)k;
				 k = 0;
				 fprintf(fout, "%c", c);
			 }*/
			break;
		}

		if (ostatok + count < size)
		{
			for (int k = 0; k < 8; k++)
			{
				kodtext[poz + ostatok] = ((c >> (8 - k - 1)) & 1);
				poz++;
			}
			count += 8;
		}
		printf("Kodtext after full: ");
		for (int i = 0; i < count + ostatok; i++)
			printf("%d", kodtext[i]);
		printf("\n");
		if (ostatok + count >= size)
		{
			poz = 0;
			for (int i = 0; i < 7; i++)
				if (kontrol[i] < size)
					old_kontrol_bits[i] = kodtext[kontrol[i]];
				else old_kontrol_bits[i] = 0;
			for (int i = 0; i < 7; i++)
				new_kontrol_bits[i] = 0;

			for (int m = 0; m < size; m++)
				if (kodtext[m])//m+ostatok
					find_kon_bits_for_decoder(m, new_kontrol_bits);

			printf("This is the old and new kontrol bits: ");
			for (int i = 0; i < 7; i++)
				printf("%d", old_kontrol_bits[i]);
			printf("  ");
			for (int i = 0; i < 7; i++)
				printf("%d", new_kontrol_bits[i]);
			printf("\n");
			int mistake_index = 0;
			for (int i = 0; i < 7; i++)
				if (old_kontrol_bits[i] != new_kontrol_bits[i])
					mistake_index += kontrol[i];
			if (mistake_index >= 0)
				kodtext[mistake_index] ^= 1;

			del_control_bits();
			printf("(dekodtext) Delete kontrol bits: ");
			for (int i = 0; i < block + poz_print; i++)
				printf("%d", dekodtext[i]);
			form_char_for_decode();
			printf("(dekodtext) after char was formed: ");
			for (int i = 0; i < 72; i++)
				printf("%d", dekodtext[i]);
			int z = 0;
			for (int y = size; y < count + ostatok; y++)
			{
				kodtext[z] = kodtext[y];
				z++;
			}
			for (int i = z; i < count + ostatok; i++)
				kodtext[i] = 0;
			printf("\nKodtext after shift: ");
			for (int i = 0; i < count; i++)
				printf("%d", kodtext[i]);
			int s = ostatok + count - size;
			ostatok = s;
			count = 0;
			printf("\nOstatok and count: %d, %d\n", ostatok, count);
		}

	}
}
int main()
{
	setlocale(0, "");
	int n = menu();
	while (n != 3)
	{
		switch (n)
		{
		case CODE:
			coder();
			break;
		case DECODE:
			decoder();
		case STOP:
			exit(0);
		}
		n = menu();
	}
	return 0;
}

int menu()
{
	int num = 0;
	printf("\nКакие действия Вы желаете выполнить?\n");
	printf("Для совершения действия выберите соответствующую цифру:\n");
	printf("1. Кодирование.\n");
	printf("2. Декодирование.\n");
	printf("3. Выход.\n");
	if (!scanf("%d", &num))
	{
		fflush(stdin);
		printf("input error.\ntry again: ");
	}
	return (num);
}
