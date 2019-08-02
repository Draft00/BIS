#include <stdio.h>
#include <Windows.h>
#include <Aclapi.h>
#include <conio.h>

DWORD AddAceToObjectsSecurityDescriptor (
    LPTSTR pszObjName,          // name of object
    SE_OBJECT_TYPE ObjectType,  // type of object
    LPTSTR pszTrustee,          // trustee for new ACE //���� ������
    TRUSTEE_FORM TrusteeForm,   // format of trustee structure
    DWORD dwAccessRights,       // access mask for new ACE
    ACCESS_MODE AccessMode,     // type of ACE
    DWORD dwInheritance         // inheritance flags for new ACE ����� ������������
)
//AddAceToObjectsSecurityDescriptor(L"F:\\3.txt", SE_FILE_OBJECT  , L"��������������", TRUSTEE_IS_NAME , GENERIC_WRITE, DENY_ACCESS , NO_INHERITANCE);


//�� ��� �������� ��� ����� ��� ��������
// dword ������������ ������� ����� (���� ����) . ��� ������ ������� �� ����������� ������, �� ����������� - 8 ����,

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

    // Get a pointer to the existing DACL. //��������� ��������� �� ������������ ������ �������� �������

    dwRes = GetNamedSecurityInfo(pszObjName, ObjectType,
          DACL_SECURITY_INFORMATION,
          NULL, NULL, &pOldDACL, NULL, &pSD);
    if (ERROR_SUCCESS != dwRes) {
        printf( "GetNamedSecurityInfo Error %u\n", dwRes );
        goto Cleanup;
    }

    // Initialize an EXPLICIT_ACCESS structure for the new ACE.

    ZeroMemory(&ea, sizeof(EXPLICIT_ACCESS)); //��������� ���� ������ ������.
    ea.grfAccessPermissions = dwAccessRights; //����� ������� ������, ������� ���������� ������ ACCESS_MASK ,
    //����� ������� ����� �������, ������� ACE ���������, ��������� ��� ��������� ��� ����������� ����.
    //�������, ������������ ��������� EXPLICIT_ACCESS , �� �����������, �� �������������� � �� ��������� ���� � ���� �����.
    ea.grfAccessMode = AccessMode; //���� ���� ���������, ��������� �� ACL ��� ��������� ��������� ����� �������.
    ea.grfInheritance= dwInheritance;/*����� ������� ������, ������� ����������, ����� �� ������ ���������� ��� ������� �����������
    ACE �� ���������� �������, � �������� ����������� ACL. �������� ����� ����� ������������� ����� ������������
     ( ������� ����) ����� AceFlags ��������� ACE_HEADER . ���� �������� ����� ���� NO_INHERITANCE, ����� �������,
      ��� ACE �� �����������; ��� ��� ����� ���� ���������� ��������� ��������.*/
    ea.Trustee.TrusteeForm = TrusteeForm;
    ea.Trustee.ptstrName = pszTrustee;
    /*TRUSTEE ��������� , ������� �������������� ������������, ������, ��� ���������
    (��������, ������ Windows) , � �������� ����������� ���.*/

    // Create a new ACL that merges the new ACE
    // into the existing DACL.

    dwRes = SetEntriesInAcl(1, &ea, pOldDACL, &pNewDACL);
    if (ERROR_SUCCESS != dwRes)  {
        printf( "SetEntriesInAcl Error %u\n", dwRes );
        goto Cleanup;
    }

    // Attach the new ACL as the object's DACL. ������������ �����

    dwRes = SetNamedSecurityInfo(pszObjName, ObjectType,
          DACL_SECURITY_INFORMATION,
          NULL, NULL, pNewDACL, NULL);
    if (ERROR_SUCCESS != dwRes)  {
        printf( "SetNamedSecurityInfo Error %u\n", dwRes );
        goto Cleanup;
    }

    Cleanup:

        if(pSD != NULL)
            LocalFree((HLOCAL) pSD);
        if(pNewDACL != NULL)
            LocalFree((HLOCAL) pNewDACL);

        return dwRes;
}

int typeName(ACCESS_ALLOWED_ACE *pAce)
{
/*���������� ��� ������� ������, ��� ������ � ��� ������� ������*/
    PSID sid = &pAce->SidStart;
    SID_NAME_USE typeAccount; //� ������� lookup: ��������� �� ����������, ������� �������� �������� SID_NAME_USE , ����������� ��� ������� ������.
    DWORD sizeNameUser = 0, sizeNameDomain = 0;
    /*���������� ������ ������ ��� ���� ������ � ������� ������*/
    LookupAccountSid(NULL, sid, 0, &sizeNameUser, 0, &sizeNameDomain, &typeAccount);
    LPSTR nameDomain = (LPSTR)malloc(sizeNameDomain*sizeof(TCHAR));
    LPSTR nameUser = (LPSTR)malloc(sizeNameUser*sizeof(TCHAR));
    /*������� LookupAccountSid ��������� ������������� ������������ (SID)
    � �������� ������� ������. �� ��������� ��� ������� ������ ��� ����� SID � ��� ������� ������, � ������� ��������� ���� SID.*/
    if (LookupAccountSid(NULL, sid, nameUser, &sizeNameUser, nameDomain, &sizeNameDomain,
    &typeAccount) == 0) {
        free(nameDomain); free(nameUser);
        printf("LookupAccountSid ERROR");
        return 0;
        }
    char *tAcc = NULL;
    switch (typeAccount) {
        case SidTypeUser:
        tAcc = "������������"; break;
        case SidTypeGroup:
        tAcc = "������"; break;
        case SidTypeDomain:
        tAcc = "�����"; break;
        case SidTypeAlias:
        tAcc = "���������"; break;
        case SidTypeWellKnownGroup:
        tAcc = "��������� ������"; break;
        case SidTypeDeletedAccount:
        tAcc = "��������� �������"; break;
        case SidTypeInvalid:
        tAcc = "��������"; break;
        case SidTypeUnknown:
        tAcc = "�����������"; break;
        case SidTypeComputer:
        tAcc = "���������"; break;
    }
    printf("%s %s : %s \n", tAcc, nameDomain, nameUser);
    free(nameUser);
    free(nameDomain);
    return 0;
}

DWORD getAccMask( )
{
    DWORD mask = 0;
    int choise = 1;
    getchar();
    printf("\n����� ���� :\n");
    printf("1. ������ ������ \n");
    printf("2. ��������� ����������\n");
    printf("3. ����� ��������� \n");
    printf("4. �������� ����� \n");
    printf("5. ������ �� ����� \n");
    printf("6. ������ � ���� \n");
    printf("7. ���������� ����� \n");
    printf("0. ��������� \n");
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

int readACE (char *file)
{
    PACL curDacl; //��������� �� ACL, ������� �������� ACE ��� ����������.
    ACCESS_ALLOWED_ACE *pAce = NULL;
    PSECURITY_DESCRIPTOR pSecDescriptor;

    /*
    BOOL GetAce(
  PACL   pAcl,
  DWORD  dwAceIndex,
  LPVOID *pAce
);*/

    DWORD result = GetNamedSecurityInfo(file, SE_FILE_OBJECT, DACL_SECURITY_INFORMATION, NULL,
    NULL, &curDacl, NULL, &pSecDescriptor);
    if (result != ERROR_SUCCESS){
        printf( "GetNamedSecurityInfo Error %u\n", result);
        return 0;
    }
/*������� ����� �������*/
    for (int i = 0; i < curDacl->AceCount; i++) {
        if (GetAce(curDacl, i, (LPVOID *)&pAce) == 0) {
            printf("GetAce Error");
            return 0;
            }
        typeName(pAce);
        if (ACCESS_ALLOWED_ACE_TYPE == pAce->Header.AceType) printf("���������:\n");
        if (ACCESS_DENIED_ACE_TYPE == pAce->Header.AceType) printf("���������:\n");
        if ((pAce->Mask & WRITE_DAC) == WRITE_DAC)
        printf("��������� DACL\n");
        if ((pAce->Mask & WRITE_OWNER) == WRITE_OWNER)
        printf("����� ���������\n");
        if ((pAce->Mask & DELETE) == DELETE)
        printf("�������� �����\n");
        if ((pAce->Mask & FILE_GENERIC_READ) == FILE_GENERIC_READ)
        printf("������ �����\n");
        if ((pAce->Mask & FILE_GENERIC_WRITE) == FILE_GENERIC_WRITE)
        printf("������ � ����\n");
        if ((pAce->Mask & FILE_GENERIC_EXECUTE) == FILE_GENERIC_EXECUTE)
        printf("����������\n");
        printf("\n");
    }
}

int createACE(char *file)
{
    PACL curDacl;
    ACCESS_ALLOWED_ACE *pAce = NULL;
    PSECURITY_DESCRIPTOR pSecDescriptor;
    int number = 0;

    DWORD result = GetNamedSecurityInfo(file, SE_FILE_OBJECT, DACL_SECURITY_INFORMATION, NULL,
    NULL, &curDacl, NULL, &pSecDescriptor);
    if (result != ERROR_SUCCESS){
        printf( "GetNamedSecurityInfo Error %u\n", result);
        return 0;
    }
    printf("\n�������� ������� �������: \n");
    for (int i = 0; i < curDacl->AceCount; i++)
    {
        int flag = 0;
        if (GetAce(curDacl, i, (LPVOID *)&pAce) == 0) {
                printf("GetAce Error");
            return 0;
        }
        number++;
        printf("%d. ", number);
        typeName(pAce);
    }
    scanf("%d", &number);
    if (GetAce(curDacl, number - 1, (LPVOID *)&pAce) == 0) {
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
    // ���������� ����� ������ DACL
  //dwDaclLength = lpOldDacl->AclSize +
    //sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD) + dwSidLength;
    DWORD sizeNewDacl;
    sizeNewDacl = curDacl->AclSize + sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD) + GetLengthSid(sid);

    //??????
    //sizeNewDacl = (sizeNewDacl + (sizeof(DWORD) - 1)) & 0xfffffffc;

    newDacl = (ACL*)LocalAlloc(LPTR, sizeNewDacl);
    memcpy(newDacl, curDacl, curDacl->AclSize);
    newDacl->AclSize = sizeNewDacl;

    printf("\n������� ����������� ACE - 0, ������� ����������� ACE - 1\n");
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


void getAccMode (ACCESS_MODE *AccMode)
{
    int num = 0;
    printf("\n������� ����������� ACE - 0, ������� ����������� ACE - 1\n");
    while (!scanf("%d", &num) && num != 0 && num != 1)
    {
        fflush(stdin);
        printf("input error.\ntry again: ");
    }
    if (num == 0)
        *AccMode = SET_ACCESS;
    else *AccMode = DENY_ACCESS;
}

char *Owner_of_a_File_Object (ACCESS_ALLOWED_ACE *pAce)
{
    PSID sid = &pAce->SidStart;
    SID_NAME_USE typeAccount; //� ������� lookup: ��������� �� ����������, ������� �������� �������� SID_NAME_USE , ����������� ��� ������� ������.
    DWORD sizeNameUser = 0, sizeNameDomain = 0;
    /*���������� ������ ������ ��� ���� ������ � ������� ������*/
    LookupAccountSid(NULL, sid, 0, &sizeNameUser, 0, &sizeNameDomain, &typeAccount);
    LPSTR nameDomain = (LPSTR)malloc(sizeNameDomain*sizeof(TCHAR));
    LPSTR nameUser = (LPSTR)malloc(sizeNameUser*sizeof(TCHAR));
    /*������� LookupAccountSid ��������� ������������� ������������ (SID)
    � �������� ������� ������. �� ��������� ��� ������� ������ ��� ����� SID � ��� ������� ������, � ������� ��������� ���� SID.*/
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

char *get_Subject (char *file)
{
    PACL curDacl;
    ACCESS_ALLOWED_ACE *pAce = NULL;
    PSECURITY_DESCRIPTOR pSecDescriptor;
    int number = 0;

    DWORD result = GetNamedSecurityInfo(file, SE_FILE_OBJECT, DACL_SECURITY_INFORMATION, NULL,
    NULL, &curDacl, NULL, &pSecDescriptor);
    if (result != ERROR_SUCCESS){
        printf( "GetNamedSecurityInfo Error %u\n", result);
        return 0;
    }
    printf("\n�������� ������� �������: \n");
    for (int i = 0; i < curDacl->AceCount; i++)
    {
        int flag = 0;
        if (GetAce(curDacl, i, (LPVOID *)&pAce) == 0) {
                printf("GetAce Error");
            return 0;
        }
        number++;
        printf("%d. ", number);
        typeName(pAce);
    }
    scanf("%d", &number);
    if (GetAce(curDacl, number - 1, (LPVOID *)&pAce) == 0) {
            printf("GetAce Error");
            return 0;
    }
    char *s = Owner_of_a_File_Object(pAce);
    return s;
    //
}

int changeACE(char *file)
{
    PACL curDacl;
    ACCESS_ALLOWED_ACE *pAce = NULL;
    PSECURITY_DESCRIPTOR pSecDescriptor;
    int number = 0;
    //�������� ��������� �� DACL � ���������� ������������
    DWORD result = GetNamedSecurityInfo(file, SE_FILE_OBJECT, DACL_SECURITY_INFORMATION, NULL,
    NULL, &curDacl, 0, &pSecDescriptor);
    if (result != ERROR_SUCCESS) {
            printf("GetNamedSecurityInfo ERROR");
            return 0;
    }
    printf("\n�������� ACE ��� ���������: \n");
    for (int i = 0; i < curDacl->AceCount; i++)
    {
        printf("%d. ", i + 1);
        if (GetAce(curDacl, i, (LPVOID *)&pAce) == 0) {
                 printf("GetAce ERROR");
                 return 0;
        }
        typeName(pAce);
        if (ACCESS_ALLOWED_ACE_TYPE == pAce->Header.AceType) printf("�����������\n");
        if (ACCESS_DENIED_ACE_TYPE == pAce->Header.AceType) printf("�����������\n");
    }
    printf("\n������� ����� ����������� ACE: ");
    scanf("%d", &number);
    if (GetAce(curDacl, number - 1, (LPVOID *)&pAce) == 0) {
            printf("GetAce ERROR: ����������� ����� ACE");
            return 0;
    }
    //��������� SID � ������� ACE
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
        result = AddAccessAllowedAce(curDacl, ACL_REVISION_DS, AccMask, sid); }
    if (ACCESS_DENIED_ACE_TYPE == pAce->Header.AceType) {
        result = AddAccessDeniedAce(curDacl, ACL_REVISION_DS, AccMask, sid); }
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

int createACE2(char *file)
{
    char *subject = get_Subject(file);
    //printf("%s", subject);
    DWORD AccMask = getAccMask();
    ACCESS_MODE AccMode; getAccMode(&AccMode);

    AddAceToObjectsSecurityDescriptor(file, SE_FILE_OBJECT, subject, TRUSTEE_IS_NAME, AccMask, AccMode, NO_INHERITANCE);
    return 0;
    //SUB_CONTAINERS_AND_OBJECTS_INHERIT
}

int main ()
{
    int num = 0;
    char fileName[1024] = {0};
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);
    printf("������� ��� �����: ");
    scanf("%s", fileName);
    while (1)
    {
        printf("\n����� �������� �� ������� ���������?\n");
        printf("��� ���������� �������� �������� ��������������� �����:\n");
        printf("1. �������� ACE \n");
        printf("2. ������ ACE \n");
        printf("3. ��������� ACE\n");
        printf("0. ����� \n");
        if(!scanf("%d", &num))
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
