#include <assert.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <stdbool.h>
#include "../../include/mid.h"

_Bool itemscan(char *, Item *);
_Bool itemprint(char *, size_t, Item *);
_Bool envscan(char *, Env *);
_Bool envprint(char *, size_t, Env *);
_Bool enemyscan(char *, Enemy *);
_Bool enemyprint(char *, size_t, Enemy *);


static bool readitem(char *buf, Zone *zn);
static bool readenv(char *buf, Zone *zn);
static bool readenemy(char *buf, Zone *zn);
static _Bool readl(char *buf, int sz, FILE *f);

enum { Bufsz = 256 };

Zone *zoneread(FILE *f)
{
	char buf[Bufsz];
	int itms = 0, envs = 0, enms = 0;

	Zone *zn = xalloc(1, sizeof(*zn));
	zn->lvl = lvlread(f);
	if (!zn->lvl)
		return NULL;

	while (readl(buf, Bufsz, f)) {
		if (buf[0] == '\0')
			continue;
		switch (buf[0]) {
		case 'i':
			if (!readitem(buf+1, zn))
				return NULL;
			itms++;
			break;
		case 'e':
			if (!readenv(buf+1, zn))
				return NULL;
			envs++;
			break;
		case 'n':
			if (!readenemy(buf+1, zn))
				return NULL;
			enms++;
			break;
		default:
			seterrstr("Unexpected input line: [%s]", buf);
			return NULL;
		}
	}

	return zn;
}

static bool readitem(char *buf, Zone *zn)
{
	int z, n;
	if (sscanf(buf, "%d%n", &z, &n) != 1) {
		seterrstr("Failed to scan item's z layer");
		return false;
	}

	Item it;
	_Bool ok = itemscan(buf+n, &it);
	if (!ok) {
		seterrstr("Failed to scan item [%s]", buf);
		return false;
	}
	if (!zoneadditem(zn, z, it)) {
		seterrstr("Failed to add item [%s]: too many items", buf);
		return false;
	}

	return true;
}

static bool readenv(char *buf, Zone *zn)
{
	int z, n;
	if (sscanf(buf, "%d%n", &z, &n) != 1) {
		seterrstr("Failed to scan env's z layer");
		return false;
	}

	Env env;
	_Bool ok = envscan(buf+n, &env);
	if (!ok) {
		seterrstr("Failed to scan env [%s]", buf);
		return false;
	}
	if (!zoneaddenv(zn, z, env)) {
		seterrstr("Failed to add env [%s]: too many envs", buf);
		return false;
	}

	return true;
}

static bool readenemy(char *buf, Zone *zn)
{
	int z, n;
	if (sscanf(buf, "%d%n", &z, &n) != 1) {
		seterrstr("Failed to scan enemy's z layer");
		return false;
	}

	Enemy en;
	_Bool ok = enemyscan(buf+n, &en);
	if (!ok) {
		seterrstr("Failed to scan item [%s]", buf);
		return false;
	}
	if (!zoneaddenemy(zn, z, en)) {
		seterrstr("Failed to add enemy [%s]: too many enemies", buf);
		return false;
	}

	return true;
}

static _Bool readl(char *buf, int sz, FILE *f)
{
	char *r = fgets(buf, sz, f);
	if (!r)
		return 0;

	int l = strlen(buf);
	assert (buf[l-1] == '\n');

	buf[l-1] = '\0';

	return 1;
}

void zonewrite(FILE *f, Zone *zn)
{
	lvlwrite(f, zn->lvl);

	for (int z = 0; z < Maxz; z++) {
		Item *itms = zn->itms[z];
		for (int i = 0; i < Maxitms; i++) {
			if (!itms[i].id)
				continue;
			char buf[Bufsz];
			itemprint(buf, Bufsz, &itms[i]);
			fprintf(f, "i %d %s\n", z, buf);
		}
		Env *envs = zn->envs[z];
		for (int i = 0; i < Maxenvs; i++) {
			if (!envs[i].id)
				continue;
			char buf[Bufsz];
			envprint(buf, Bufsz, &envs[i]);
			fprintf(f, "e %d %s\n", z, buf);
		}
		Enemy *enms = zn->enms[z];
		for (int i = 0; i < Maxenms; i++) {
			if (!enms[i].id)
				continue;
			char buf[Bufsz];
			enemyprint(buf, Bufsz, &enms[i]);
			fprintf(f, "n %d %s\n", z, buf);
		}
	}
}

_Bool zoneadditem(Zone *zn, int z, Item it)
{
	int i;
	Item *itms = zn->itms[z];

	for (i = 0; i < Maxitms && itms[i].id; i++)
		;
	if (i == Maxitms)
		return false;

	zn->itms[z][i] = it;
	return true;
}

_Bool zoneaddenv(Zone *zn, int z, Env env)
{
	int i;
	Env *envs = zn->envs[z];

	for (i = 0; i < Maxenvs && envs[i].id; i++)
		;
	if (i == Maxenvs)
		return false;

	zn->envs[z][i] = env;
	return true;
}

_Bool zoneaddenemy(Zone *zn, int z, Enemy enm)
{
	int i;
	Enemy *enms = zn->enms[z];

	for (i = 0; i < Maxenms && enms[i].id; i++)
		;
	if (i == Maxenms)
		return false;

	zn->enms[z][i] = enm;
	return true;
}

void zonefree(Zone *z)
{
	lvlfree(z->lvl);
	free(z);
}

int zonelocs(Zone *zn, int z, _Bool (*p)(Zone *, int, Point), Point pts[], int sz)
{
	int n = 0;
	Point pt;

	for (pt.x = 0; pt.x < zn->lvl->w; pt.x++) {
	for (pt.y = 0; pt.y < zn->lvl->h; pt.y++) {
		if (n >= sz || !p(zn, z, pt))
			continue;
		pts[n] = pt;
		n++;
	}
	}

	return n;
}

_Bool zonefits(Zone *zn, int z, Point loc, Point wh)
{
	wh.x /= Twidth;
	wh.y /= Theight;
	for (int x = loc.x; x < (int) (loc.x + wh.x + 0.5); x++) {
	for (int y = loc.y; y < (int) (loc.y + wh.y + 0.5); y++) {
		Blkinfo bi = blkinfo(zn->lvl, x, y, z);
		if (bi.flags & Tilecollide)
			return false;
	}
	}
	return true;
}

_Bool zoneonground(Zone *zn, int z, Point loc, Point wh)
{
	wh.x /= Twidth;
	int y =	loc.y + wh.y / Theight;
	for (int x = loc.x; x < (int) (loc.x + wh.x + 0.5); x++) {
		if (blk(zn->lvl, x, y, z)->tile != '#')
			return false;
	}
	return true;
}

_Bool zoneoverlap(Zone *zn, int z, Point loc, Point wh)
{
	loc.x *= Twidth;
	loc.y *= Theight;
	Rect r = (Rect) { loc, (Point) { loc.x + wh.x, loc.y + wh.y } };

	for (int i = 0; i < Maxitms; i++) {
		Item *it = &zn->itms[z][i];
		if (it->id && isect(r, it->body.bbox))
			return true;
	}
	for (int i = 0; i < Maxenvs; i++) {
		Env *en = &zn->envs[z][i];
		if (en->id && isect(r, en->body.bbox))
			return true;
	}
	for (int i = 0; i < Maxenms; i++) {
		Enemy *en = &zn->enms[z][i];
		if (en->id && isect(r, en->b.bbox))
			return true;
	}

	return false;
}

void zoneupdate(Zone *zn, Player *p, Point *tr)
{
	int z = zn->lvl->z;

	lvlupdate(zn->lvl);
	playerupdate(p, zn->lvl, tr);

	itemupdateanims();

	Item *itms = zn->itms[z];
	for(size_t i = 0; i < Maxitms; i++)
		if (itms[i].id) itemupdate(&itms[i], p, zn->lvl);

	envupdateanims();

	Env *en = zn->envs[z];
	for(size_t i = 0; i < Maxenvs; i++)
		if (en[i].id) envupdate(&en[i], zn->lvl);

	Enemy *e = zn->enms[z];
	for(size_t i = 0; i < Maxenms; i++) {
		if (!e[i].id)
			continue;
		enemyupdate(&e[i], p, zn->lvl);
		if(e[i].hp <= 0)
			enemyfree(&e[i]);
	}
}

void zonedraw(Gfx *g, Zone *zn, Player *p)
{
	int z = zn->lvl->z;

	lvldraw(g, zn->lvl, true);

	Env *en = zn->envs[z];
	for(size_t i = 0; i < Maxenvs; i++)
		if (en[i].id) envdraw(&en[i], g);

	playerdraw(g, p);

	Item *itms = zn->itms[z];
	for(size_t i = 0; i < Maxitms; i++)
		if (itms[i].id) itemdraw(&itms[i], g);

	Enemy *e = zn->enms[z];
	for(size_t i = 0; i < Maxenms; i++)
		enemydraw(&e[i], g);

	lvldraw(g, zn->lvl, false);

}

