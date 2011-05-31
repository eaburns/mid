#include "../../include/mid.h"
#include "../../include/log.h"
#include "game.h"
#include "resrc.h"
#include <stdlib.h>

typedef struct Invscr Invscr;
struct Invscr{
	Inv *inv;
	Lvl *lvl;
	Item *curitem;
	int z;
};

enum { Iconw = 32, Iconh = 32 };
enum { Pad = 1 };
enum { Width = Iconw * Invcols + Pad * (Invcols - 1),
       Height = Iconh * Invrows + Pad * (Invrows - 1) };
enum { Xmin = Scrnw - Width - 1, Ymin = 15 };

static const char *moneystr = "gold";

static void update(Scrn*,Scrnstk*);
static void draw(Scrn*,Gfx*);
static void handle(Scrn*,Scrnstk*,Event*);
static Item *invat(Inv *inv, int x, int y);
static void invfree(Scrn*);
static void curdraw(Gfx *g, Item *inv);
static void moneydraw(Gfx *g, Inv *inv);
static void griddraw(Gfx *g, Inv *inv, Item *cur);
static Txt *gettxt(void);

static Scrnmt invmt = {
	update,
	draw,
	handle,
	invfree,
};

Scrn *invscrnnew(Inv *i, Lvl *lvl, int z){
	Invscr *inv = malloc(sizeof(*inv));
	if(!inv)
		return NULL;
	inv->inv = i;
	inv->lvl = lvl;
	inv->z = z;
	inv->curitem = NULL;

	Scrn *s = malloc(sizeof(*s));
	if(!s)
		return NULL;
	s->mt = &invmt;
	s->data = inv;
	return s;
}

static void update(Scrn *s, Scrnstk *stk){
	Invscr *i = s->data;
	Inv *inv = i->inv;
	for (int i = 0; i < Invcols * Invrows; i++) {
		Item *it = inv->items[i];
		if (!it)
			continue;
		animupdate(it->icon, 1);
	}
}

static void draw(Scrn *s, Gfx *g){
	gfxclear(g, (Color){ 127, 255, 127 });

	Invscr *i = s->data;
	lvlminidraw(g, i->lvl, i->z, (Point){0,0});

	Inv *inv = i->inv;
	moneydraw(g, inv);
	griddraw(g, inv, i->curitem);
	if (i->curitem)
		curdraw(g, i->curitem);

	gfxflip(g);
}

static void moneydraw(Gfx *g, Inv *inv)
{
	Txt *invtxt = gettxt();
	Point d = txtdims(invtxt, moneystr);
	txtdraw(g, invtxt, (Point) { Scrnw - d.x, 1 }, moneystr);
	d.x += txtdims(invtxt, "%d ", inv->money).x;
	txtdraw(g, invtxt, (Point) { Scrnw - d.x , 1 }, "%d ", inv->money);
}

static void griddraw(Gfx *g, Inv *inv, Item *cur)
{
	for (int r = 0; r < Invrows; r++) {
		for (int c = 0; c < Invcols; c++) {
			int x0 = Xmin + r * Pad;
			int y0 = Ymin + c * Pad;
			Point a = (Point) { r * Iconw + x0, c * Iconh + y0 };
			Point b = (Point) { (r + 1) * Iconw + x0, (c + 1) * Iconh + y0 };
			Rect rect = (Rect){ a, b };
			Item *it = inv->items[r * Invcols + c];
			if (cur && it == cur)
				gfxfillrect(g, rect, (Color){255,255,255,100});
			gfxdrawrect(g, rect, (Color){0});

			if (it)
				animdraw(g, it->icon, a);
		}
	}
}

static void curdraw(Gfx *g, Item *inv)
{
	Txt *invtxt = gettxt();
	Point d = txtdims(invtxt, inv->name);
	Point p = (Point) { .x = Scrnw - d.x, .y = Height + Ymin + Pad };
	txtdraw(g, invtxt, p, inv->name);
}

static Txt *gettxt(void)
{
	static Txt *invtxt;
	static Txtinfo txtinfo = (Txtinfo) { .size = 12, .color = (Color) {0} };
	if (!invtxt) {
		invtxt = resrcacq(txt, "txt/FreeSans.ttf", &txtinfo);
		if (!invtxt)
			fatal("Failed to load inventory text");
	}
	return invtxt;
}

static void handle(Scrn *s, Scrnstk *stk, Event *e){
	if (e->type == Mousemv) {
		Invscr *i = s->data;
		i->curitem = invat(i->inv, e->x, e->y);
	}

	if(e->type != Keychng || e->repeat)
		return;

	if(e->down && e->key == kmap[Mvinv]){
		scrnstkpop(stk);
		return;
	}
}

static Item *invat(Inv *inv, int x, int y)
{
	if (x < Xmin || x > Xmin + Width || y < Ymin || y > Ymin + Height)
		return NULL;

	int i = (x - Xmin) / (Iconw + Pad);
	int j = (y - Ymin) / (Iconh + Pad);

	if (x > Xmin + i * (Iconw + Pad) + Iconw
	    || y > Ymin + j * (Iconh + Pad) + Iconh)
		return NULL;	/* In padding */

	return inv->items[i * Invcols + j];
}

static void invfree(Scrn *s){
	free(s->data);
	free(s);
}