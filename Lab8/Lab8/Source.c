#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <string.h>
#include <math.h>


FILE* original_picture, * cypher_picture, * original_text, * decode_text;
int pack = 0;

enum start_menu
{
	CODE = 1,
	DECODE,
	STOP,
};

int main()
{
	setlocale(LC_ALL, "Russian");
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

int coder()
{
	original_picture = fopen("grey.bmp", "rb");
	original_text = fopen("input.txt", "rb");
	cypher_picture = fopen("pict.bmp", "wb");
	if (!original_text)
	{
		printf("Ошибка при открытии файла с текстом\n");
		return 0;
	}
	if (!original_picture)
	{
		printf("Ошибка при открытии файла с изображением\n");
		return 0;
	}
	printf("Выберите степень упаковки:");
	if (!scanf("%d", &pack) || pack < 1 || pack > 8)
	{
		fflush(stdin);
		printf("input error.\ntry again.");
		return 0;
	}
	long long original_picture_lenght = 0, original_text_lenght = 0;
	original_picture_lenght = lenth(original_picture);
	original_text_lenght = lenth(original_text);
	fseek(original_picture, 0, SEEK_SET);
	if (original_text_lenght * 8 > (original_picture_lenght - 54) * pack)
	{
		printf("Размер текста превышает вместимость стегоконтейнера при данной упаковке.\n");
		printf("Доступно бит для упаковки: %lli\n", (original_picture_lenght - 54) * pack);
		printf("Вы хотите упаковать %lli бит.\n", original_text_lenght * 8);
		return 0;
	}
	head(original_picture, cypher_picture, original_text_lenght);
	int text_buff = 0, read = 0, i = 0;
	int mask = make_mask(pack);
	for (i = 55; i < original_picture_lenght; i++)
	{
		int picbyte = fgetc(original_picture);
		if (picbyte == EOF)
			break;
		picbyte = picbyte & ~mask; //последние pack бит = 0
		if (read < pack) //записываем след. байт текста в буфер
		{
			int text_byte = fgetc(original_text);
			if (text_byte == EOF)
			{
				fputc(picbyte + text_buff, cypher_picture);
				break;
			}
			text_buff = (text_buff << 8) + (char)text_byte;
			read += 8;
		}
		picbyte += (text_buff >> (read - pack));
		fputc(picbyte, cypher_picture);
		read -= pack;
		text_buff = text_buff & make_mask(read);
	}
	for (; i < original_picture_lenght; i++)
		fputc(fgetc(original_picture), cypher_picture);
	fclose(original_picture);
	fclose(cypher_picture);
	fclose(original_text);
}

int decoder()
{
	cypher_picture = fopen("pict2.bmp", "rb");
	if (!cypher_picture)
	{
		printf("Ошибка при открытии файла для чтения.\n");
		return 0;
	}
	printf("Выберите степень упаковки:");
	if (!scanf("%d", &pack) || pack < 1 || pack > 8)
	{
		fflush(stdin);
		printf("input error.\ntry again.");
		return 0;
	}
	long long cypher_picture_lenght = lenth(cypher_picture);
	long long scoded = 0, sdecoded = 0;
	int mask = make_mask(pack), i = 0, bits = 0;
	fseek(cypher_picture, 6, SEEK_SET);
	int size_rash = 0;
	size_rash = fgetc(cypher_picture);
	char filename[20] = "output.";
	//printf("%d - столько символов говорят о длине расширения,\n", size_rash);
	int size_text = 0;
	for (i = 0; i < 3; i++) //Длина зашифрованного текста
		size_text = (size_text << 8) + fgetc(cypher_picture);
	//printf("%d - а это размер текста\n", size_text);
	fseek(cypher_picture, 54, SEEK_SET);
	int n = 0;
	i = 55;
	char rash[10] = { 0 };
	while (n < size_rash)
	{
		int symbol = fgetc(cypher_picture);
		i++;
		bits = (bits << pack) + (symbol & mask);
		scoded += pack;
		if (scoded >= 8)
		{
			char byte = bits >> (scoded - 8);
			scoded -= 8;
			sdecoded++;
			rash[n] = byte;
			n++;
		}
	}
	//printf("%s", rash);
	strcat(filename, rash);
	//printf("%s", filename);
	decode_text = fopen(filename, "wb");
	for (i; i < cypher_picture_lenght; i++)
	{
		if (sdecoded >= size_text)
			break;
		int symbol = fgetc(cypher_picture);
		if (symbol == EOF)
			break;
		bits = (bits << pack) + (symbol & mask);
		scoded += pack;
		if (scoded >= 8)
		{
			char byte = bits >> (scoded - 8);
			scoded -= 8;
			sdecoded++;
			fputc(byte, decode_text);
		}
	}
	fclose(cypher_picture);
}

int make_mask(int pack)
{
	int m = 0;
	for (int i = 0; i < pack; i++)
		m = (m << 1) + 1;
	return m; //число, последние psck бит которого = 1
}

void head(FILE * input, FILE * output, int length) // Копируем заголовок исходного bmp, в новый
{
	char head[54];
	fread(&head, sizeof(unsigned char), 6, input);
	fwrite(&head, sizeof(unsigned char), 6, output);
	fputc(3, output);
	fseek(input, 1, SEEK_CUR);
	fputc(length >> 16, output);
	fputc((length >> 8) & 255, output);
	fputc(length & 255, output);
	fseek(input, 3, SEEK_CUR);
	fread(&head, sizeof(unsigned char), 44, input);
	fwrite(&head, sizeof(unsigned char), 44, output);
}
/*
void pixel_size ()
{
	FILE *input = fopen("grey.bmp", "rb");
	fseek(input, 14, SEEK_SET);
	long long size_bm = 0;
	for (int k = 0; k < 4; k++)
	{
		//size_bm = ((size_bm >> 8)&255) + (int)fgetc(input);
		size_bm += (int)fgetc(input)*pow(2, 8*k);
	}
	printf("%d", size_bm);
	fseek(input, 0, SEEK_SET);
	fclose(input);
}*/
int lenth(FILE * filename)
{
	fseek(filename, 0, SEEK_END);
	int size = ftell(filename);
	fseek(filename, 0, SEEK_SET);
	return size;
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
