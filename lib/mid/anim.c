#include "../../include/mid.h"
#include "../../include/log.h"
#include "fs.h"
#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

typedef struct Frame Frame;
struct Frame {
	char *file;
	Img *img;
	int tks;
	int nxt;
};

struct Anim {
	int cur;
	int rem;
	int nframes;
	Frame frames[];
};

/* len should include room for the '\0' */
static int readpath(FILE *f, char buf[], int len)
{
	int i = 0;
	int c = fgetc(f);
	while (!isspace(c)) {
		if (i < len - 2)
			buf[i] = c;
		i++;
		c = fgetc(f);
	}
	if (i < len - 1)
		buf[i] = '\0';
	else
		buf[len - 1] = '\0';
	return i;
}

char *strdup(const char s[])
{
	int l = strlen(s);
	char *dst = xalloc(sizeof(*dst) * l + 1, 1);

	int i;
	for (i = 0; i < l; i++)
		dst[i] = s[i];
	dst[i] = '\0';
	return dst;
}

static bool readframes(Rtab *imgs, FILE *f, int n, Anim *a)
{
	int i, err, ms, nxt;
	char file[PATH_MAX + 1];

	for (i = 0; i < n; i++) {
		if (fscanf(f, "%d %d\n", &ms, &nxt) != 2)
			goto err;
		if (readpath(f, file, PATH_MAX + 1) > PATH_MAX)
			goto err;
		a->frames[i].img = resrcacq(imgs, file, NULL);
		if (!a->frames[i].img) {
			seterrstr("Failed to load image: %s", file);
			goto err;
		}
		a->frames[i].file = strdup(file);
		if (!a->frames[i].file) {
			err = errno;
			resrcrel(imgs, file, NULL);
			errno = err;
			goto err;
		}
		a->frames[i].tks = ms / Ticktm;
		a->frames[i].nxt = nxt;
	}

	return true;
err:
	err = errno;
	for (i -= 1; i > 0; i--)
		imgfree(a->frames[i].img);
	errno = err;
	return false;
}

Anim *animnew(const char *path)
{
	assert (imgs);
	int n;
	FILE *f = fopen(path, "r");
	if (!f)
		return NULL;
	if (fscanf(f, "%d", &n) != 1)
		goto err;
	Anim *anim = xalloc(1, sizeof(*anim) + sizeof(Frame[n]));
	if (!readframes(imgs, f, n, anim))
		goto err1;
	anim->nframes = n;
	anim->cur = 0;
	anim->rem = anim->frames[0].tks;
	fclose(f);
	return anim;
err1:
	xfree(anim);
err:
	fclose(f);
	return NULL;
}

void animfree(Anim *a)
{
	for (int i = 0; i < a->nframes; i++) {
		resrcrel(imgs, a->frames[i].file, NULL);
		xfree(a->frames[i].file);
	}
	xfree(a);
}

void animupdate(Anim *a, int n)
{
	a->rem -= n;
	int nxt = a->frames[a->cur].nxt;
	if (a->rem > 0 || nxt < 0)
		return;

	int tks = a->frames[nxt].tks;
	a->cur = nxt;
	a->rem = tks + a->rem; /* a->rem may be negative */
	if (a->rem <= 0) {
		n = -a->rem;
		a->rem = 0;
		animupdate(a, n);
	}
}

void animdraw(Gfx *g, Anim *a, Point p)
{
	imgdraw(g, a->frames[a->cur].img, p);
}

void animreset(Anim *a)
{
	a->cur = 0;
	a->rem = a->frames[0].tks;
}