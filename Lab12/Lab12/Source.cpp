#include <stdio.h>
#include <Windows.h>
#include <Aclapi.h>
#include <conio.h>

DWORD AddAceToObjectsSecurityDescriptor(
	LPTSTR pszObjName,          // name of object
	SE_OBJECT_TYPE ObjectType,  // type of object
	LPTSTR pszTrustee,          // trustee for new ACE //кому доступ
	TRUSTEE_FORM TrusteeForm,   // format of trustee structure
	DWORD dwAccessRights,       // access mask for new ACE
	ACCESS_MODE AccessMode,     // type of ACE
	DWORD dwInheritance         // inheritance flags for new ACE флаги наследования
)
//AddAceToObjectsSecurityDescriptor(L"F:\\3.txt", SE_FILE_OBJECT  , L"Администраторы", TRUSTEE_IS_NAME , GENERIC_WRITE, DENY_ACCESS , NO_INHERITANCE);


//ок это вяглядит как штука для создания
// dword ассемблерное двойное слово (дабл ворд) . Его размер зависит от архитектуры машины, на современных - 8 байт,

/*
GENERIC_ALL	All possible access rights
GENERIC_EXECUTE	Execute access
GENERIC_READ	Read access
GENERIC_WRITE	Write access
*/

{
	DWORD dwRes = 0;
	PACL pOldDACL = NULL, pNewDACL = NULL;
	PSECURITY_DESCRIPTOR pSD = NULL;
	EXPLICIT_ACCESS ea;

	if (NULL == pszObjName)
		return ERROR_INVALID_PARAMETER;

	// Get a pointer to the existing DACL. //получение указателя на существующий список контроля доступа

	dwRes = GetNamedSecurityInfo(pszObjName, ObjectType,
		DACL_SECURITY_INFORMATION,
		NULL, NULL, &pOldDACL, NULL, &pSD);
	if (ERROR_SUCCESS != dwRes) {
		printf("GetNamedSecurityInfo Error %u\n", dwRes);
		goto Cleanup;
	}

	// Initialize an EXPLICIT_ACCESS structure for the new ACE.

	ZeroMemory(&ea, sizeof(EXPLICIT_ACCESS)); //Заполняет блок памяти нулями.
	ea.grfAccessPermissions = dwAccessRights; //Набор битовых флагов, которые используют формат ACCESS_MASK ,
	//чтобы указать права доступа, которые ACE разрешает, запрещает или проверяет для доверенного лица.
	//Функции, использующие структуру EXPLICIT_ACCESS , не преобразуют, не интерпретируют и не проверяют биты в этой маске.
	ea.grfAccessMode = AccessMode; //этот флаг указывает, разрешает ли ACL или запрещает указанные права доступа.
	ea.grfInheritance = dwInheritance;/*Набор битовых флагов, который определяет, могут ли другие контейнеры или объекты наследовать
	ACE от первичного объекта, к которому присоединен ACL. Значение этого члена соответствует части наследования
	 ( младший байт) члена AceFlags структуры ACE_HEADER . Этот параметр может быть NO_INHERITANCE, чтобы указать,
	  что ACE не наследуется; или это может быть комбинация следующих значений.*/
	ea.Trustee.TrusteeForm = TrusteeForm;
	ea.Trustee.ptstrName = pszTrustee;
	/*TRUSTEE структура , которая идентифицирует пользователя, группу, или программу
	(например, служба Windows) , к которому применяется АСЕ.*/

	// Create a new ACL that merges the new ACE
	// into the existing DACL.

	dwRes = SetEntriesInAcl(1, &ea, pOldDACL, &pNewDACL);
	if (ERROR_SUCCESS != dwRes) {
		printf("SetEntriesInAcl Error %u\n", dwRes);
		goto Cleanup;
	}

	// Attach the new ACL as the object's DACL. присоединить новый

	dwRes = SetNamedSecurityInfo(pszObjName, ObjectType,
		DACL_SECURITY_INFORMATION,
		NULL, NULL, pNewDACL, NULL);
	if (ERROR_SUCCESS != dwRes) {
		printf("SetNamedSecurityInfo Error %u\n", dwRes);
		goto Cleanup;
	}

Cleanup:

	if (pSD != NULL)
		LocalFree((HLOCAL)pSD);
	if (pNewDACL != NULL)
		LocalFree((HLOCAL)pNewDACL);

	return dwRes;
}

int typeName(ACCESS_ALLOWED_ACE * pAce)
{
	/*Определеям имя учетной записи, имя домена и тип учетной записи*/
	PSID sid = &pAce->SidStart;
	SID_NAME_USE typeAccount; //в функции lookup: Указатель на переменную, которая получает значение SID_NAME_USE , указывающее тип учетной записи.
	DWORD sizeNameUser = 0, sizeNameDomain = 0;
	/*Определяем размер буфера для имен домена и учетной записи*/
	LookupAccountSid(NULL, sid, 0, &sizeNameUser, 0, &sizeNameDomain, &typeAccount);
	LPSTR nameDomain = (LPSTR)malloc(sizeNameDomain * sizeof(TCHAR));
	LPSTR nameUser = (LPSTR)malloc(sizeNameUser * sizeof(TCHAR));
	/*Функция LookupAccountSid принимает идентификатор безопасности (SID)
	в качестве входных данных. Он извлекает имя учетной записи для этого SID и имя первого домена, в котором обнаружен этот SID.*/
	if (LookupAccountSid(NULL, sid, nameUser, &sizeNameUser, nameDomain, &sizeNameDomain,
		&typeAccount) == 0) {
		free(nameDomain); free(nameUser);
		printf("LookupAccountSid ERROR");
		return 0;
	}
	char* tAcc = NULL;
	switch (typeAccount) {
	case SidTypeUser:
		tAcc = "Пользователь"; break;
	case SidTypeGroup:
		tAcc = "Группа"; break;
	case SidTypeDomain:
		tAcc = "Домен"; break;
	case SidTypeAlias:
		tAcc = "Псевдоним"; break;
	case SidTypeWellKnownGroup:
		tAcc = "Известная Группа"; break;
	case SidTypeDeletedAccount:
		tAcc = "Удаленный аккаунт"; break;
	case SidTypeInvalid:
		tAcc = "Неверный"; break;
	case SidTypeUnknown:
		tAcc = "Неизвестный"; break;
	case SidTypeComputer:
		tAcc = "Компьютер"; break;
	}
	printf("%s %s : %s \n", tAcc, nameDomain, nameUser);
	free(nameUser);
	free(nameDomain);
	return 0;
}

DWORD getAccMask()
{
	DWORD mask = 0;
	int choise = 1;
	getchar();
	printf("\nВыбор прав :\n");
	printf("1. Полный доступ \n");
	printf("2. Изменение разрешений\n");
	printf("3. Смена владельца \n");
	printf("4. Удаление файла \n");
	printf("5. Чтение из файла \n");
	printf("6. Запись в файл \n");
	printf("7. Выполнение файла \n");
	printf("0. Закончить \n");
	while (choise)
	{
		while (!scanf("%d", &choise) || choise < 0 || choise > 8) {
			fflush(stdin);
			printf("input error.\ntry again: ");
		}
		switch (choise) {
		case 0:
			break;
		case 1:
			mask |= GENERIC_ALL;
			break;
		case 2:
			mask |= WRITE_DAC;
			break;
		case 3:
			mask |= WRITE_OWNER;
			break;
		case 4:
			mask |= DELETE;
			break;
		case 5:
			mask |= GENERIC_READ;
			break;
		case 6:
			mask |= GENERIC_WRITE;
			break;
		case 7:
			mask |= GENERIC_EXECUTE;
			break;
		}
	}
	return mask; //getch();
}

int readACE(char* file)
{
	PACL curDacl; //Указатель на ACL, который содержит ACE для извлечения.
	ACCESS_ALLOWED_ACE* pAce = NULL;
	PSECURITY_DESCRIPTOR pSecDescriptor;

	/*
	BOOL GetAce(
  PACL   pAcl,
  DWORD  dwAceIndex,
  LPVOID *pAce
);*/

	DWORD result = GetNamedSecurityInfo(file, SE_FILE_OBJECT, DACL_SECURITY_INFORMATION, NULL,
		NULL, &curDacl, NULL, &pSecDescriptor);
	if (result != ERROR_SUCCESS) {
		printf("GetNamedSecurityInfo Error %u\n", result);
		return 0;
	}
	/*Выводим права доступа*/
	for (int i = 0; i < curDacl->AceCount; i++) {
		if (GetAce(curDacl, i, (LPVOID*)& pAce) == 0) {
			printf("GetAce Error");
			return 0;
		}
		typeName(pAce);
		if (ACCESS_ALLOWED_ACE_TYPE == pAce->Header.AceType) printf("Разрешено:\n");
		if (ACCESS_DENIED_ACE_TYPE == pAce->Header.AceType) printf("Запрещено:\n");
		if ((pAce->Mask & WRITE_DAC) == WRITE_DAC)
			printf("Изменение DACL\n");
		if ((pAce->Mask & WRITE_OWNER) == WRITE_OWNER)
			printf("Смена владельца\n");
		if ((pAce->Mask & DELETE) == DELETE)
			printf("Удаление файла\n");
		if ((pAce->Mask & FILE_GENERIC_READ) == FILE_GENERIC_READ)
			printf("Чтение файла\n");
		if ((pAce->Mask & FILE_GENERIC_WRITE) == FILE_GENERIC_WRITE)
			printf("Запись в файл\n");
		if ((pAce->Mask & FILE_GENERIC_EXECUTE) == FILE_GENERIC_EXECUTE)
			printf("Выполнение\n");
		printf("\n");
	}
}

int createACE(char* file)
{
	PACL curDacl;
	ACCESS_ALLOWED_ACE* pAce = NULL;
	PSECURITY_DESCRIPTOR pSecDescriptor;
	int number = 0;

	DWORD result = GetNamedSecurityInfo(file, SE_FILE_OBJECT, DACL_SECURITY_INFORMATION, NULL,
		NULL, &curDacl, NULL, &pSecDescriptor);
	if (result != ERROR_SUCCESS) {
		printf("GetNamedSecurityInfo Error %u\n", result);
		return 0;
	}
	printf("\nВыберите субъект доступа: \n");
	for (int i = 0; i < curDacl->AceCount; i++)
	{
		int flag = 0;
		if (GetAce(curDacl, i, (LPVOID*)& pAce) == 0) {
			printf("GetAce Error");
			return 0;
		}
		number++;
		printf("%d. ", number);
		typeName(pAce);
	}
	scanf("%d", &number);
	if (GetAce(curDacl, number - 1, (LPVOID*)& pAce) == 0) {
		printf("GetAce Error");
		return 0;
	}
	PSID psid = &pAce->SidStart;
	PSID sid = (PSID)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, GetLengthSid(psid));
	if (!CopySid(GetLengthSid(psid), sid, psid)) {
		printf("CopySid Error");
		return 0;
	}

	PACL newDacl;
	DWORD AccMask = getAccMask();
	//getAccMask(&AccMask);
	// определяем длину нового DACL
  //dwDaclLength = lpOldDacl->AclSize +
	//sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD) + dwSidLength;
	DWORD sizeNewDacl;
	sizeNewDacl = curDacl->AclSize + sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD) + GetLengthSid(sid);

	//??????
	//sizeNewDacl = (sizeNewDacl + (sizeof(DWORD) - 1)) & 0xfffffffc;

	newDacl = (ACL*)LocalAlloc(LPTR, sizeNewDacl);
	memcpy(newDacl, curDacl, curDacl->AclSize);
	newDacl->AclSize = sizeNewDacl;

	printf("\nСоздать разрешающий ACE - 0, Создать запрещающий ACE - 1\n");
	scanf("%d", &number);
	if (number == 0) result = AddAccessAllowedAce(newDacl, ACL_REVISION_DS, AccMask, sid);
	if (number == 1) result = AddAccessDeniedAce(newDacl, ACL_REVISION_DS, AccMask, sid);
	if (result == 0)
	{
		printf("AddAccess_Ace ERROR");
		return 0;
	}
	DWORD resultSet = SetNamedSecurityInfo(file, SE_FILE_OBJECT, DACL_SECURITY_INFORMATION, NULL,
		NULL, newDacl, 0);
	if (resultSet != ERROR_SUCCESS)
	{
		printf("SetNamedSecurityInfo : end ERROR");
		return 0;
	}

	//AddAceToObjectsSecurityDescriptor(file, SE_FILE_OBJECT, , )
}


void getAccMode(ACCESS_MODE * AccMode)
{
	int num = 0;
	printf("\nСоздать разрешающий ACE - 0, Создать запрещающий ACE - 1\n");
	while (!scanf("%d", &num) && num != 0 && num != 1)
	{
		fflush(stdin);
		printf("input error.\ntry again: ");
	}
	if (num == 0)
		* AccMode = SET_ACCESS;
	else *AccMode = DENY_ACCESS;
}

char* Owner_of_a_File_Object(ACCESS_ALLOWED_ACE * pAce)
{
	PSID sid = &pAce->SidStart;
	SID_NAME_USE typeAccount; //в функции lookup: Указатель на переменную, которая получает значение SID_NAME_USE , указывающее тип учетной записи.
	DWORD sizeNameUser = 0, sizeNameDomain = 0;
	/*Определяем размер буфера для имен домена и учетной записи*/
	LookupAccountSid(NULL, sid, 0, &sizeNameUser, 0, &sizeNameDomain, &typeAccount);
	LPSTR nameDomain = (LPSTR)malloc(sizeNameDomain * sizeof(TCHAR));
	LPSTR nameUser = (LPSTR)malloc(sizeNameUser * sizeof(TCHAR));
	/*Функция LookupAccountSid принимает идентификатор безопасности (SID)
	в качестве входных данных. Он извлекает имя учетной записи для этого SID и имя первого домена, в котором обнаружен этот SID.*/
	if (LookupAccountSid(NULL, sid, nameUser, &sizeNameUser, nameDomain, &sizeNameDomain, &typeAccount) == 0) {
		free(nameDomain); free(nameUser);
		printf("LookupAccountSid ERROR");
		return 0;
	}
	return nameUser;
	//printf("%s %s : %s \n", tAcc, nameDomain, nameUser);
	free(nameUser);
	free(nameDomain);
	return 0;

}

char* get_Subject(char* file)
{
	PACL curDacl;
	ACCESS_ALLOWED_ACE* pAce = NULL;
	PSECURITY_DESCRIPTOR pSecDescriptor;
	int number = 0;

	DWORD result = GetNamedSecurityInfo(file, SE_FILE_OBJECT, DACL_SECURITY_INFORMATION, NULL,
		NULL, &curDacl, NULL, &pSecDescriptor);
	if (result != ERROR_SUCCESS) {
		printf("GetNamedSecurityInfo Error %u\n", result);
		return 0;
	}
	printf("\nВыберите субъект доступа: \n");
	for (int i = 0; i < curDacl->AceCount; i++)
	{
		int flag = 0;
		if (GetAce(curDacl, i, (LPVOID*)& pAce) == 0) {
			printf("GetAce Error");
			return 0;
		}
		number++;
		printf("%d. ", number);
		typeName(pAce);
	}
	scanf("%d", &number);
	if (GetAce(curDacl, number - 1, (LPVOID*)& pAce) == 0) {
		printf("GetAce Error");
		return 0;
	}
	char* s = Owner_of_a_File_Object(pAce);
	return s;
	//
}

int changeACE(char* file)
{
	PACL curDacl;
	ACCESS_ALLOWED_ACE* pAce = NULL;
	PSECURITY_DESCRIPTOR pSecDescriptor;
	int number = 0;
	//Получаем указатель на DACL и дескриптор безопасности
	DWORD result = GetNamedSecurityInfo(file, SE_FILE_OBJECT, DACL_SECURITY_INFORMATION, NULL,
		NULL, &curDacl, 0, &pSecDescriptor);
	if (result != ERROR_SUCCESS) {
		printf("GetNamedSecurityInfo ERROR");
		return 0;
	}
	printf("\nВыберите ACE для изменения: \n");
	for (int i = 0; i < curDacl->AceCount; i++)
	{
		printf("%d. ", i + 1);
		if (GetAce(curDacl, i, (LPVOID*)& pAce) == 0) {
			printf("GetAce ERROR");
			return 0;
		}
		typeName(pAce);
		if (ACCESS_ALLOWED_ACE_TYPE == pAce->Header.AceType) printf("Разрешающий\n");
		if (ACCESS_DENIED_ACE_TYPE == pAce->Header.AceType) printf("Запрещающий\n");
	}
	printf("\nВведите номер изменяемого ACE: ");
	scanf("%d", &number);
	if (GetAce(curDacl, number - 1, (LPVOID*)& pAce) == 0) {
		printf("GetAce ERROR: Некоректный номер ACE");
		return 0;
	}
	//Сохраняем SID и удаляем ACE
	PSID psid = &pAce->SidStart;
	PSID sid = (PSID)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, GetLengthSid(psid));
	if (!CopySid(GetLengthSid(psid), sid, psid)) {
		printf("CopySid ERROR");
		return 0;
	}
	if (!DeleteAce(curDacl, number - 1)) {
		printf("DeleteACE ERROR");
		return 0;
	}

	DWORD resultSet = SetNamedSecurityInfo(file, SE_FILE_OBJECT, DACL_SECURITY_INFORMATION, NULL, NULL, curDacl, 0);
	if (resultSet != ERROR_SUCCESS) {
		printf("SetNamedSecurityInfo ERROR");
		return 0;
	}

	DWORD AccMask = getAccMask();
	//getAccMask(&AccMask);

	if (ACCESS_ALLOWED_ACE_TYPE == pAce->Header.AceType) {
		result = AddAccessAllowedAce(curDacl, ACL_REVISION_DS, AccMask, sid);
	}
	if (ACCESS_DENIED_ACE_TYPE == pAce->Header.AceType) {
		result = AddAccessDeniedAce(curDacl, ACL_REVISION_DS, AccMask, sid);
	}
	if (result == 0) {
		printf("AddAccess_Ace ERROR");
		return 0;
	}
	resultSet = SetNamedSecurityInfo(file, SE_FILE_OBJECT, DACL_SECURITY_INFORMATION, NULL, NULL, curDacl, 0);
	if (resultSet != ERROR_SUCCESS) {
		printf("SetNamedSecurityInfo_end ERROR");
		return 0;
	}
	return 0;
}

int createACE2(char* file)
{
	char* subject = get_Subject(file);
	//printf("%s", subject);
	DWORD AccMask = getAccMask();
	ACCESS_MODE AccMode; getAccMode(&AccMode);

	AddAceToObjectsSecurityDescriptor(file, SE_FILE_OBJECT, subject, TRUSTEE_IS_NAME, AccMask, AccMode, NO_INHERITANCE);
	return 0;
	//SUB_CONTAINERS_AND_OBJECTS_INHERIT
}

int main()
{
	int num = 0;
	char fileName[1024] = { 0 };
	SetConsoleCP(1251);
	SetConsoleOutputCP(1251);
	printf("Введите имя файла: ");
	scanf("%s", fileName);
	while (1)
	{
		printf("\nКакие действия Вы желаете выполнить?\n");
		printf("Для совершения действия выберите соответствующую цифру:\n");
		printf("1. Создание ACE \n");
		printf("2. Чтение ACE \n");
		printf("3. Изменение ACE\n");
		printf("0. Выход \n");
		if (!scanf("%d", &num))
		{
			fflush(stdin);
			printf("input error.\ntry again: ");
		}
		switch (num)
		{
		case 1:
			createACE2(fileName);
			break;
		case 2:
			readACE(fileName);
			break;
		case 3:
			changeACE(fileName);
			break;
		default:
			return 0;
		}
	}

	return 0;
}
