#include <stdio.h>
#include <stdlib.h>
int config[3] = { 0 };

int full_config(int config[3])
{
	char str[300] = { 0 };
	int a;
	FILE* fin = fopen("config.txt", "r");
	if (!fin)
	{
		printf("Can't open file!");
		return 0;
	}
	for (int i = 0; i < 3; i++)
	{
		fscanf(fin, "%d", &config[i]);
		fgets(str, 300, fin);
	}
	fclose(fin);
}

void change(FILE* fin, FILE* fout)
{
	srand(time(NULL));
	FILE* fname = fopen("namevar.txt", "r");
	char name[100] = { 0 }, newname[100] = { '\0' };
	char temp[100] = { '\0' };
	int n, j = 0, count;
	fscanf(fname, "%d", &n);
	int c = 2;
	//printf("%d\n", n);
	for (int i = 0; i < n; i++)
	{
		fin = fopen("input.txt", "r");
		fout = fopen("output.txt", "w");
		int c = 2;
		memset(newname, '\0', 100);
		for (int i = 0; i < 100; i++)
			newname[i] = (char)(97 + rand() % (122 - 97 + 1));
		//printf(newname);
		memset(temp, '\0', 100);
		memset(name, '\0', 100);
		fscanf(fname, "%s", &name);
		printf("%s\n", name);
		//c = fgetc(fin);
		//printf("%c", c);
		while (c != EOF)
		{
			c = fgetc(fin);
			if (c == EOF)
				break;
			if (c == '\"')
			{
				fputc(c, fout);
				c = fgetc(fin);
				while (c != '\"')
				{
					if (c == EOF)
						break;
					if (c == '\\') { fputc(c, fout); c = fgetc(fin); fputc(c, fout); c = fgetc(fin); }
					else
					{
						fputc(c, fout);
						c = fgetc(fin);
					}
				}
				//fputc(c, fout);
				//c = fgetc(fin);
			}
			j = 0;
			while (c == name[j] && j < strlen(name))
			{
				printf("%c ", c);
				temp[j] = c;
				j++;
				c = fgetc(fin);
			}
			if (j > 0)
			{
				if (j == strlen(name))
				{
					for (int i = 0; i < j; i++)
						fputc(newname[i], fout);
				}
				else
				{
					fputs(temp, fout);
				}
				memset(temp, '\0', 100);
			}
			fputc(c, fout);
		}
		fclose(fout); fclose(fin);
		fout = fopen("input.txt", "w");
		fin = fopen("output.txt", "r");
		copy(fin, fout);
		fclose(fout); fclose(fin);
	}

	fclose(fname);
}

void delete_comm(FILE * fin, FILE * fout)
{
	fin = fopen("input.txt", "r"), fout = fopen("output.txt", "w");
	int c = 2;
	while (c != EOF)
	{
		c = fgetc(fin);
		if (c == EOF)
		{
			break;
		}
		if (c == '\"')
		{
			fputc(c, fout);
			c = fgetc(fin);
			while (c != '\"')
			{
				if (c == '\\') { fputc(c, fout); c = fgetc(fin); fputc(c, fout); c = fgetc(fin); }
				else
				{
					fputc(c, fout);
					c = fgetc(fin);
				}
			}
			if (config[1] == 1)
			{
				fputc(c, fout);
			}
		}
		else if (c == '\'')
		{
			fputc(c, fout);
			c = fgetc(fin);
			while (c != '\'')
			{
				if (c == '\\') { fputc(c, fout); c = fgetc(fin); fputc(c, fout); c = fgetc(fin); }
				else
				{
					fputc(c, fout); c = fgetc(fin);
				}
			}
			fputc(c, fout);
		}
		else if (c == '/')
		{
			c = fgetc(fin);
			if (c == '*')
			{
				int flag = 0;
				c = fgetc(fin);
				while (flag == 0)
				{
					while ((c != '*') && (c != EOF))
					{
						c = fgetc(fin);
					}
					if (c == EOF)
					{
						break;
					}
					c = fgetc(fin);
					if (c != '/')
					{
						flag = 0;
					}
					else { flag = 1; }
				}
			}
			else if (c == '/')
			{
				int flag = 0;
				while (flag == 0)
				{
					while ((c != '\\') && (c != '\n') && (c != EOF))
					{
						c = fgetc(fin);
					}
					if (c == '\\')
					{
						c = fgetc(fin);
						c = fgetc(fin);
						//if (c == '\n'){c = fgetc(S2); flag = 0;}
					}
					else if (c == '\n')
					{
						flag = 1;
					}
					else if (c == EOF)
					{
						break;
					}
				}
			}
		}
		else
		{
			fputc(c, fout);
		}
	}
	fclose(fin); fclose(fout);
	fin = fopen("output.txt", "r"); fout = fopen("input.txt", "w");
	copy(fin, fout);
	fclose(fout); fclose(fin);
}

void del_format(FILE * fin, FILE * fout)
{
	fin = fopen("input.txt", "r"), fout = fopen("output.txt", "w");
	int c = 10;
	char lib[100] = { 0 };
	while (c != EOF)
	{
		c = fgetc(fin);
		if (c == EOF)
		{
			break;
		}
		if (c == '#')
		{
			fputc(c, fout);
			fgets(lib, 100, fin);
			fputs(lib, fout);
		}
		else if (c == ';')
		{
			fputc(c, fout);
			c = fgetc(fin);
			while (c == ' ' || c == '\n' || c == 9)
				c = fgetc(fin);
			fseek(fin, -1, SEEK_CUR);

		}
		else if (c == '{' || c == ')' || c == '}')
		{
			fputc(c, fout);
			c = fgetc(fin);
			while (c == ' ' || c == '\n' || c == 9)
				c = fgetc(fin);
			fseek(fin, -1, SEEK_CUR);
		}
		else fputc(c, fout);
	}
	fclose(fin); fclose(fout);
	fin = fopen("output.txt", "r"); fout = fopen("input.txt", "w");
	copy(fin, fout);
	fclose(fout); fclose(fin);
}

void add_trash(FILE * fin, FILE * fout)
{
	srand(time(NULL));
	char* var[] = {
		"char ", "int ", "long ", "short ", "double ", "float "
	};
	char* operators[] = {
		"+", "-", "*", "/", "%"
	};
	fin = fopen("input.txt", "r"), fout = fopen("output.txt", "w");
	int c = 2, choise = 0;
	char lib[100] = { 0 }, newname[100] = { 0 };
	while (c != EOF)
	{
		c = fgetc(fin);
		if (c == EOF)
			break;
		/*   if (c == '#')
		   {
			   fputc(c, fout);
			   fgets(lib, 100, fin);
			   fputs(lib, fout);
		   }*/
		if (c == ';')
		{
			fputc(c, fout);
			memset(newname, '\0', 100);
			for (int i = 0; i < 100; i++)
				newname[i] = (char)(97 + rand() % (122 - 97 + 1));
			choise = rand() % 2;
			fprintf(fout, "%s", var[rand() % 6]);
			for (int i = 0; i < 1 + rand() % 10; i++)
			{
				fputc(newname[i], fout);
			}
			fputc('=', fout);
			fprintf(fout, "%d", rand() % 4576);
			if (choise == 0)
			{
				fputc(';', fout);
			}
			else
			{
				fprintf(fout, "%s", operators[rand() % 5]);
				fprintf(fout, "%d", rand() % 1263);
				fputc(';', fout);
			}
		}
		else fputc(c, fout);
	}
	fclose(fin); fclose(fout);

}
void copy(FILE * fin, FILE * fout)
{
	int c = fgetc(fin);
	while (c != EOF)
	{
		fputc(c, fout);
		c = fgetc(fin);
	}
}
int main()
{
	FILE* fin = fopen("input.txt", "r+"), * fout = fopen("output.txt", "r+"), * fcpy = fopen("prevois.txt", "w");
	if (!fin)
	{
		printf("Can't open file!");
		return 0;
	}
	copy(fin, fcpy);
	full_config(config);

	if (config[0] == 1)
	{
		change(fin, fout);
	}
	if (config[1] == 1)
	{
		delete_comm(fin, fout);
	}
	if (config[1] == 1)
	{
		del_format(fin, fout);
		add_trash(fin, fout);
	}
	for (int i = 0; i < 3; i++)
	{
		printf("%d", config[i]);
	}
	//printf("%d", '');
	printf("Hello world!\n");
	return 0;
}
