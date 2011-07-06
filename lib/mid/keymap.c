#include "../../include/mid.h"
#include <string.h>

_Bool keymapread(char km[], char *fname){
	FILE *f = fopen(fname, "r");
	if(!f)
		return 1;

	char act[8];
	char k;
	while(fscanf(f, "%7s %c", act, &k) != EOF){
		if(strcmp(act, "left") == 0)
			km[Mvleft] = k;
		else if(strcmp(act, "right") == 0)
			km[Mvright] = k;
		else if(strcmp(act, "jump") == 0)
			km[Mvjump] = k;
		else if(strcmp(act, "act") == 0)
			km[Mvact] = k;
		else if(strcmp(act, "inv") == 0)
			km[Mvinv] = k;
		else if(strcmp(act, "sword") == 0)
			km[Mvsword] = k;
	}

	if(ferror(f)){
		fclose(f);
		return 1;
	}
	fclose(f);
	return 0;
}

char kmap[] = {
	[Mvleft] = 's',
	[Mvright] = 'f',
	[Mvact] = 'u',
	[Mvjump] = 'e',
	[Mvinv] = 'n',
	[Mvsword] = 'j',
};