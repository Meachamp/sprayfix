#pragma once
#include <cstring>

void *FindSignature(char *pBaseAddress, size_t baseLength, const char *pSignature) {
	char *pEndPtr = pBaseAddress + baseLength;
	unsigned int len = strlen(pSignature);

	while (pBaseAddress < pEndPtr)
	{
		bool found = true;
		for (register size_t i = 0; i < len; i++)
		{
			if (pSignature[i] != '?' && pSignature[i] != pBaseAddress[i])
			{
				found = false;
				break;
			}
		}

		if (found)
		{
			return pBaseAddress;
		}

		++pBaseAddress;
	}
	return 0;
}