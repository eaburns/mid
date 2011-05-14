#include "../../include/mid.h"
#include "../../include/log.h"
#include <stdlib.h>
#include "game.h"
#include "resrc.h"

struct Game {
	Point transl;
	int z;			/* current level z-level */
	Lvl *lvl;
	Player *player;
};

Game *gamenew()
{
	Game *gm = malloc(sizeof(*gm));
	if (!gm)
		return NULL;

	gm->transl = (Point) { 0, 0 };
	gm->z = 0;
	gm->lvl = resrcacq(lvls, "lvl/0.lvl", NULL);
	gm->player = playernew(256, 256);

	return gm;
}

void gamefree(Scrn *s)
{
	Game *gm = s->data;
	playerfree(gm->player);
	resrcrel(lvls, "lvl/0.lvl", NULL);
	free(gm);
}

void gameupdate(Scrn *s, Scrnstk *stk)
{
	Game *gm = s->data;
	lvlupdate(anim, gm->lvl);
	playerupdate(gm->player);
}

void gamedraw(Scrn *s, Gfx *g)
{
	Game *gm = s->data;
	gfxclear(g, (Color){ 96, 96, 96, 0 });
	lvldraw(g, anim, gm->lvl, gm->z, gm->transl);
	playerdraw(g, gm->player, gm->transl);
	gfxflip(g);
}

void gamehandle(Scrn *s, Scrnstk *stk, Event *e)
{
	Game *gm = s->data;
	playerhandle(gm->player, &gm->transl, e);
}

Scrnmt gamemt = {
	gameupdate,
	gamedraw,
	gamehandle,
	gamefree,
};
