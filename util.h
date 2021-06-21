#pragma once

int string_len(char* in);
void string_cpy(char* out, char* in);
bool string_find(char* in, char* test);
char* decrypt(unsigned int* string, int size, unsigned int key);

static char* utf8cpy(char* dst, const char* src, size_t sizeDest )
{
    if( sizeDest ){
        size_t sizeSrc = strlen(src); // number of bytes not including null
        while( sizeSrc >= sizeDest ){

            const char* lastByte = src + sizeSrc; // Initially, pointing to the null terminator.
            while( lastByte-- > src )
                if((*lastByte & 0xC0) != 0x80) // Found the initial byte of the (potentially) multi-byte character (or found null).
                    break;

            sizeSrc = lastByte - src;
        }
        memcpy(dst, src, sizeSrc);
        dst[sizeSrc] = '\0';
    }
    return dst;
}

void ScheduleUnload();

struct StrEncrypted
{
	StrEncrypted()
	{
		size = 0;
		key = 0;
	}

	void Set(unsigned int* str, int sz, unsigned int k)
	{
		memcpy(string, str, sz);
		size = sz;
		key = k;
	}

	unsigned int string[64];
	int size;
	unsigned int key;
};