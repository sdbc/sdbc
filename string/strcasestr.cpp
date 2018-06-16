#include <iostream>
using namespace std;
char *strCaseStr(const char *str1, const char *str2)
{
	if(NULL == str1 || NULL == str2)
	{
		return NULL;
	}
	int len1 = strlen(str1);
	int len2 = strlen(str2);
	char *pStr1 = (char *)malloc(len1 + 1);
	char *pStr2 = (char *)malloc(len2 + 1);
	memset(pStr1, 0, len1 + 1); // ÎðÍü¼Ç
	memset(pStr2, 0, len2 + 1); // ÎðÍü¼Ç
	strcpy(pStr1, str1);
	strcpy(pStr2, str2);
	strlwr(pStr1);
	strlwr(pStr2);
	char *p = strstr(pStr1, pStr2);
	if(NULL == p)
	{
		delete pStr1;
		delete pStr2;
		return NULL;
	}
	char *pRet = (char *)(str1 + (p - pStr1));
	delete pStr1;
	delete pStr2;
	return pRet;
}
int main()
{
	char testStr1[][100] = 
	{
		"abc", "abc", "Abc", "Abc", "Abc"
	};
	char testStr2[][100] =
	{
		"bc", "Bc", "Bc", "BC", "ac"
	};
	int size = sizeof(testStr1) / sizeof(testStr1[0]);
	int i = 0;
	char *p = NULL;
	for(i = 0; i < size; i++)
	{
		if(NULL != (p = strCaseStr(testStr1[i],  testStr2[i])))
		{
			cout << p << endl;
		}
		else
		{
			cout << "error" << endl; 
		}
	}
	return 0;
}

