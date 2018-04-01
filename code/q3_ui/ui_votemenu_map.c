/*
===========================================================================
Copyright (C) 2009 Poul Sander

This file is part of the Open Arena source code.

Open Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Open Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Open Arena source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

#include "ui_local.h"

#define ART_BACK0			"menu/art_blueish/back_0"
#define ART_BACK1			"menu/art_blueish/back_1"
#define ART_FIGHT0			"menu/art_blueish/accept_0"
#define ART_FIGHT1			"menu/art_blueish/accept_1"
//#define ART_BACKGROUND                  "menu/art_blueish/addbotframe"
#define ART_ARROWS			"menu/art_blueish/arrows_vert_0"
#define ART_ARROWUP			"menu/art_blueish/arrows_vert_top"
#define ART_ARROWDOWN                   "menu/art_blueish/arrows_vert_bot"
#define ART_ARROWSH			"menu/art_blueish/gs_arrows_0"
#define ART_ARROWSHL			"menu/art_blueish/gs_arrows_l"
#define ART_ARROWSHR			"menu/art_blueish/gs_arrows_r"
#define ART_SELECT			"menu/art_blueish/maps_select"
#define ART_SELECTED			"menu/art_blueish/maps_selected"
#define ART_UNKNOWNMAP           	"menu/art/unknownmap"

#define ID_TYPE				9
#define ID_BACK				10
#define ID_GO				11
#define ID_PICTURES			12	// 12-22
#define ID_PREVPAGE			23
#define ID_NEXTPAGE			24

#define SIZE_OF_NAME                    32

#define MAX_MAPROWS		5
#define MAX_MAPCOLS		2
#define MAX_MAPSPERPAGE	(MAX_MAPROWS * MAX_MAPCOLS)

typedef struct {
	menuframework_s	menu;
	menubitmap_s	arrows;
        menutext_s	banner;
        menutext_s	info;
	menubitmap_s	prevpage;
	menubitmap_s	nextpage;
	menubitmap_s	back;
	menubitmap_s	go;
	menulist_s		type;
	menubitmap_s	mappics[MAX_MAPSPERPAGE];
	menubitmap_s	mapbuttons[MAX_MAPSPERPAGE];

	menutext_s		mapname;
	menutext_s		page;

	//int nummaps;
	int currentmap;
	int pagenum;

	//int		selected;
} votemenu_map_t;

static votemenu_map_t	s_votemenu_map;
static char pagebuffer[64];

#define MAPPAGE_TYPE_ALL 0
#define MAPPAGE_TYPE_RECOMMENDED 1
static const char *mappage_type_items[] = {
	"All",
	"Recommended",
	NULL
};

static const char *getmappage_all_cmd = "getmappage";
static const char *getmappage_recommened_cmd = "getrecmappage";
static const char *getmappage_cmd = "getmappage";

t_mappage mappage;

// XXX: must be multiple of MAX_MAPSPERPAGE
#define MAX_MAP_NUMBER 600
struct {
	int reset;
	int num_sent_cmds;
	int num_cmds;
	//int pagenum;
	int num_maps;
	int loaded_all;
	char mapname[MAX_MAP_NUMBER][MAX_MAPNAME_LENGTH];
} maplist;

static void InitMappage(void) {
    int i;
    //We need to initialize the list or it will be impossible to click on the items
    for(i=0;i<MAX_MAPSPERPAGE;i++) {
        Q_strncpyz(mappage.mapname[i],"----",5);
    }
    mappage.pagenumber = 0;
}

static void InitMaplist(void) {
	memset(&maplist, 0, sizeof(maplist));
}


/*
=================
VoteMapMenu_Event
=================
*/
static void VoteMapMenu_Event( void* ptr, int event )
{
	int mapidx;
	switch (((menucommon_s*)ptr)->id)
	{
            case ID_BACK:
		if (event != QM_ACTIVATED)
                    return;
                UI_PopMenu();
                break;
            case ID_GO:
                if( event != QM_ACTIVATED ) {
                    return;
                }
		if (s_votemenu_map.currentmap < 0 || s_votemenu_map.currentmap >= MAX_MAPSPERPAGE)
			return;
                //if(!Q_stricmp(mappage.mapname[s_votemenu_map.currentmap],"---"))
		mapidx = s_votemenu_map.pagenum * MAX_MAPSPERPAGE + s_votemenu_map.currentmap;
		if (mapidx > maplist.num_maps) {
			return;
		}
                if(!Q_stricmp(maplist.mapname[mapidx],"---"))
                    return; //Blank spaces have string "---"
                trap_Cmd_ExecuteText( EXEC_APPEND, va("callvote map %s", mappage.mapname[mapidx]) );
                UI_PopMenu();
                UI_PopMenu();
                break;
        //    case ID_GO:
        //        if( event != QM_ACTIVATED ) {
        //            return;
        //        }
        //        if(!s_votemenu_map.selected || mappage.mapname[s_votemenu_map.selected-ID_MAPNAME0][0] == 0)
        //            return;
        //        if(!Q_stricmp(mappage.mapname[s_votemenu_map.selected-ID_MAPNAME0],"---"))
        //            return; //Blank spaces have string "---"
        //        trap_Cmd_ExecuteText( EXEC_APPEND, va("callvote map %s", mappage.mapname[s_votemenu_map.selected-ID_MAPNAME0]) );
        //        UI_PopMenu();
        //        UI_PopMenu();
        //        break;
            //default:
            //    if( event != QM_ACTIVATED ) {
            //        return;
            //    }
            //    if(s_votemenu_map.selected != ((menucommon_s*)ptr)->id) {
            //        s_votemenu_map.selected = ((menucommon_s*)ptr)->id;
            //        UI_VoteMapMenuInternal();
            //    }
            //    break;
         }
}

static void UI_VoteMapMenu_PreviousPageEvent( void* ptr, int event ) {
	if (event != QM_ACTIVATED ) {
		return;
	}

        //trap_Cmd_ExecuteText( EXEC_APPEND,va("%s %d",getmappage_cmd, mappage.pagenumber-1) );
	if (s_votemenu_map.pagenum > 0) {
		s_votemenu_map.pagenum--;
		UI_VoteMapMenu_Update();
	}
}


static void UI_VoteMapMenu_NextPageEvent( void* ptr, int event ) {
	if (event != QM_ACTIVATED) {
		return;
	}

	if (maplist.num_maps > (s_votemenu_map.pagenum + 1) * MAX_MAPSPERPAGE) {
		s_votemenu_map.pagenum++;
		UI_VoteMapMenu_Update();
	}

        //trap_Cmd_ExecuteText( EXEC_APPEND,va("%s %d",getmappage_cmd, mappage.pagenumber+1) );
}

static void ResetMaplist(void) {
	InitMaplist();
	InitMappage();
	s_votemenu_map.pagenum = 0;
	s_votemenu_map.currentmap = 0;
	UI_VoteMapMenu_Update();
	//trap_Cmd_ExecuteText( EXEC_APPEND,va("%s 0", getmappage_cmd) );
	//maplist.num_sent_cmds++;
}

static void VoteMapMenu_TypeEvent( void* ptr, int event ) {
	if( event != QM_ACTIVATED) {
		return;
	}
	if (s_votemenu_map.type.curvalue == MAPPAGE_TYPE_RECOMMENDED) {
		getmappage_cmd = getmappage_recommened_cmd;
	} else {
		getmappage_cmd = getmappage_all_cmd;
	}
	if (maplist.loaded_all) {
		ResetMaplist();
		trap_Cmd_ExecuteText( EXEC_APPEND,va("%s 0", getmappage_cmd) );
		maplist.num_sent_cmds++;
	} else {
		maplist.reset = 1;
	}
}

/*
=================
UI_VoteMapMenu_Draw
=================
*/
static void UI_VoteMapMenu_Draw( void ) {
	vec4_t	bg = {0.0, 0.0, 0.0, 0.85};
	//UI_DrawBannerString( 320, 16, "CALL VOTE MAP", UI_CENTER, color_white );
	//UI_DrawNamedPic( 320-233, 240-166, 466, 332, ART_BACKGROUND );
	UI_FillRect( 0, 0, 640, 480, bg );

	// standard menu drawing
	Menu_Draw( &s_votemenu_map.menu );
}

/*
=================
VoteMapMenu_Cache
=================
*/
static void VoteMapMenu_Cache( void )
{
	trap_R_RegisterShaderNoMip( ART_BACK0 );
	trap_R_RegisterShaderNoMip( ART_BACK1 );
	trap_R_RegisterShaderNoMip( ART_FIGHT0 );
	trap_R_RegisterShaderNoMip( ART_FIGHT1 );
	trap_R_RegisterShaderNoMip( ART_ARROWS );
	trap_R_RegisterShaderNoMip( ART_ARROWUP );
	trap_R_RegisterShaderNoMip( ART_ARROWDOWN );
	trap_R_RegisterShaderNoMip( ART_UNKNOWNMAP );
	trap_R_RegisterShaderNoMip( ART_ARROWSH );
	trap_R_RegisterShaderNoMip( ART_ARROWSHL );
	trap_R_RegisterShaderNoMip( ART_ARROWSHR );
	trap_R_RegisterShaderNoMip( ART_SELECT );
	trap_R_RegisterShaderNoMip( ART_SELECTED );

}

//static void setMapMenutext(menutext_s *menu,int y,int id,char * text) {
//    menu->generic.type            = MTYPE_PTEXT;
//    menu->color               = color_red;
//    menu->generic.flags       = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
//    if(s_votemenu_map.selected == id)
//         menu->color       = color_orange;
//    menu->generic.x           = 320-80;
//    menu->generic.y           = y;
//    menu->generic.id          = id;
//    menu->generic.callback    = VoteMapMenu_Event;
//    menu->string              = text;
//    menu->style               = UI_LEFT|UI_SMALLFONT;
//}

/*
===============
VoteMapMenu_LevelshotDraw
===============
*/
static void VoteMapMenu_LevelshotDraw( void *self ) {
	menubitmap_s	*b;
	int				x;
	int				y;
	int				w;
	int				h;
	int				n;
        //const char		*info;

	b = (menubitmap_s *)self;

	if( !b->generic.name ) {
		return;
	}

	if( b->generic.name && !b->shader ) {
		b->shader = trap_R_RegisterShaderNoMip( b->generic.name );
		if( !b->shader && b->errorpic ) {
			b->shader = trap_R_RegisterShaderNoMip( b->errorpic );
		}
	}

	if( b->focuspic && !b->focusshader ) {
		b->focusshader = trap_R_RegisterShaderNoMip( b->focuspic );
	}

	x = b->generic.x;
	y = b->generic.y;
	w = b->width;
	h =	b->height;
	if( b->shader ) {
		UI_DrawHandlePic( x, y, w, h, b->shader );
	}

	x = b->generic.x;
	y = b->generic.y + b->height;
	UI_FillRect( x, y, b->width, 28, colorBlack );

	x += b->width / 2;
	y += 4;
	n = s_votemenu_map.pagenum * MAX_MAPSPERPAGE + b->generic.id - ID_PICTURES;

	if (n > MAX_MAP_NUMBER) {
		n = MAX_MAP_NUMBER;
	}
        
	UI_DrawString( x, y, maplist.mapname[n], UI_CENTER|UI_SMALLFONT, color_orange );

	x = b->generic.x;
	y = b->generic.y;
	w = b->width;
	h =	b->height + 28;
	if( b->generic.flags & QMF_HIGHLIGHT ) {	
		UI_DrawHandlePic( x, y, w, h, b->focusshader );
	}
}

static int CountMappageMaps( void ) {
	int i;
	for (i = 0; i < MAPPAGE_NUM; ++i) {
		if (!Q_stricmp(mappage.mapname[i],"---")) {
			break;
		}
	}
	return i;
}

void UI_VoteMapMenu_Update( void ) {
	int				i;
	int 		top;
	static	char	picname[MAX_MAPSPERPAGE][64];
        const char		*info;
	char			mapname[MAX_MAPNAME_LENGTH];

	top = s_votemenu_map.pagenum * MAX_MAPSPERPAGE;
	//s_votemenu_map.nummaps = maplist.num_maps - s_votemenu_map.pagenumber * MAX_MAPSPERPAGE;

	for (i=0; i<MAX_MAPSPERPAGE; i++)
	{
		if (top + i >= maplist.num_maps)
			break;

		Q_strncpyz( mapname, maplist.mapname[top+i], MAX_MAPNAME_LENGTH );
		Q_strupr( mapname );

		Com_sprintf( picname[i], sizeof(picname[i]), "levelshots/%s", mapname );
                
		s_votemenu_map.mappics[i].generic.flags &= ~((unsigned int)QMF_HIGHLIGHT);
		s_votemenu_map.mappics[i].generic.name   = picname[i];
		s_votemenu_map.mappics[i].shader         = 0;

		// reset
		s_votemenu_map.mapbuttons[i].generic.flags |= QMF_PULSEIFFOCUS;
		s_votemenu_map.mapbuttons[i].generic.flags &= ~((unsigned int)QMF_INACTIVE);
	}

	for (; i<MAX_MAPSPERPAGE; i++)
	{
		s_votemenu_map.mappics[i].generic.flags &= ~((unsigned int)QMF_HIGHLIGHT);
		s_votemenu_map.mappics[i].generic.name   = NULL;
		s_votemenu_map.mappics[i].shader         = 0;

		// disable
		s_votemenu_map.mapbuttons[i].generic.flags &= ~((unsigned int)QMF_PULSEIFFOCUS);
		s_votemenu_map.mapbuttons[i].generic.flags |= QMF_INACTIVE;
	}


	// no maps to vote for
	if( !maplist.num_maps ) {
		s_votemenu_map.go.generic.flags |= QMF_INACTIVE;

		// set the map name
		strcpy( s_votemenu_map.mapname.string, "NO MAPS FOUND" );
	}
	else {
		// set the highlight
		s_votemenu_map.go.generic.flags &= ~((unsigned int)QMF_INACTIVE);
		i = s_votemenu_map.currentmap;
		if ( i >=0 && i < MAX_MAPSPERPAGE ) 
		{
			s_votemenu_map.mappics[i].generic.flags    |= QMF_HIGHLIGHT;
			s_votemenu_map.mapbuttons[i].generic.flags &= ~((unsigned int)QMF_PULSEIFFOCUS);
		}

		// set the map name
		Q_strncpyz( s_votemenu_map.mapname.string, maplist.mapname[top+s_votemenu_map.currentmap], MAX_MAPNAME_LENGTH);
	}
	
	Q_strupr( s_votemenu_map.mapname.string );
	Com_sprintf( pagebuffer, sizeof(pagebuffer), "Page %i", s_votemenu_map.pagenum+1 );
}

/*
=================
VoteMapMenu_MapEvent
=================
*/
static void VoteMapMenu_MapEvent( void* ptr, int event ) {
	if( event != QM_ACTIVATED) {
		return;
	}

	s_votemenu_map.currentmap = (((menucommon_s*)ptr)->id - ID_PICTURES);
	UI_VoteMapMenu_Update();
}


/*
=================
UI_VoteMapMenuInternal
=================
*/
void UI_VoteMapMenuInternal( void )
{
	int i;
	if (maplist.reset) {
		ResetMaplist();
		trap_Cmd_ExecuteText( EXEC_APPEND,va("%s 0",getmappage_cmd ));
		maplist.num_sent_cmds++;
		UI_VoteMapMenu_Update();
		return;
	}
	if (maplist.num_cmds > 0) {
		if (mappage.pagenumber == 0) {
			// returned to page 0, we're finished loading
			maplist.loaded_all = 1;
			Com_Printf("Loaded all maps!\n");
		}
	}
	maplist.num_cmds++;
	if (uis.activemenu != &s_votemenu_map.menu) {
		// menu not showing anymore, ignore
		ResetMaplist();
		return;
	}
	if (!maplist.loaded_all) {
		for (i = 0; i < CountMappageMaps(); ++i) {
			Q_strncpyz(maplist.mapname[maplist.num_maps], mappage.mapname[i], MAX_MAPNAME_LENGTH);
			if (maplist.num_maps < MAX_MAP_NUMBER) {
				maplist.num_maps++;
			}
			Com_Printf("adding map no. %i, %s\n", maplist.num_maps, mappage.mapname[i]);
		}
		trap_Cmd_ExecuteText( EXEC_APPEND,va("%s %d",getmappage_cmd, mappage.pagenumber+1) );
		maplist.num_sent_cmds++;
	}
	UI_VoteMapMenu_Update();
}


/*
=================
UI_VoteMapMenu
 *Called from outside
=================
*/
void UI_VoteMapMenu( void ) {
    int x,y,i;
    static char mapnamebuffer[MAX_MAPNAME_LENGTH*2];
    //memset( &s_votemenu_map, 0 ,sizeof(votemenu_map_t) );

    VoteMapMenu_Cache();

    memset( &s_votemenu_map, 0 ,sizeof(votemenu_map_t) );

    s_votemenu_map.menu.wrapAround = qtrue;
    s_votemenu_map.menu.fullscreen = qfalse;
    s_votemenu_map.menu.draw = UI_VoteMapMenu_Draw;

    s_votemenu_map.banner.generic.type  = MTYPE_BTEXT;
    s_votemenu_map.banner.generic.x	  = 320;
    s_votemenu_map.banner.generic.y	  = 16;
    s_votemenu_map.banner.string		  = "CALL VOTE MAP";
    s_votemenu_map.banner.color	      = color_white;
    s_votemenu_map.banner.style	      = UI_CENTER;

    s_votemenu_map.go.generic.type			= MTYPE_BITMAP;
    s_votemenu_map.go.generic.name			= ART_FIGHT0;
    s_votemenu_map.go.generic.flags		= QMF_RIGHT_JUSTIFY|QMF_PULSEIFFOCUS;
    s_votemenu_map.go.generic.id			= ID_GO;
    s_votemenu_map.go.generic.callback		= VoteMapMenu_Event;
    s_votemenu_map.go.generic.x			= 640;
    s_votemenu_map.go.generic.y			= 480-64;
    s_votemenu_map.go.width  				= 128;
    s_votemenu_map.go.height  				= 64;
    s_votemenu_map.go.focuspic				= ART_FIGHT1;

    s_votemenu_map.back.generic.type		= MTYPE_BITMAP;
    s_votemenu_map.back.generic.name		= ART_BACK0;
    s_votemenu_map.back.generic.flags		= QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
    s_votemenu_map.back.generic.id			= ID_BACK;
    s_votemenu_map.back.generic.callback	= VoteMapMenu_Event;
    s_votemenu_map.back.generic.x			= 0;
    s_votemenu_map.back.generic.y			= 480-64;
    s_votemenu_map.back.width				= 128;
    s_votemenu_map.back.height				= 64;
    s_votemenu_map.back.focuspic			= ART_BACK1;

    s_votemenu_map.type.generic.type		= MTYPE_SPINCONTROL;
    s_votemenu_map.type.generic.name		= "Type:";
    s_votemenu_map.type.generic.flags	= QMF_PULSEIFFOCUS|QMF_SMALLFONT;
    s_votemenu_map.type.generic.callback	= VoteMapMenu_TypeEvent;
    s_votemenu_map.type.generic.id		= ID_TYPE;
    s_votemenu_map.type.generic.x		= 320 - 24;
    s_votemenu_map.type.generic.y		= 70;
    s_votemenu_map.type.itemnames		= mappage_type_items;

    for (i=0; i<MAX_MAPSPERPAGE; i++)
    {
    	//x =	(i % MAX_MAPCOLS) * (128+8) + 188;
    	//y = (i / MAX_MAPROWS) * (128+8) + 96;
    	//x = (640-MAX_MAPROWS*140)/2 + ( (i % MAX_MAPROWS) * 140 );
    	x = (640-MAX_MAPROWS*128)/2 + ( (i % MAX_MAPROWS) * 128 );
    	y = 96 + ( (i / MAX_MAPROWS) * 140 );
    
    	s_votemenu_map.mappics[i].generic.type   = MTYPE_BITMAP;
    	s_votemenu_map.mappics[i].generic.flags  = QMF_LEFT_JUSTIFY|QMF_INACTIVE;
    	s_votemenu_map.mappics[i].generic.x	    = x;
    	s_votemenu_map.mappics[i].generic.y	    = y;
    	s_votemenu_map.mappics[i].generic.id		= ID_PICTURES+i;
    	s_votemenu_map.mappics[i].width  		= 128;
    	s_votemenu_map.mappics[i].height  	    = 96;
    	s_votemenu_map.mappics[i].focuspic       = ART_SELECTED;
    	s_votemenu_map.mappics[i].errorpic       = ART_UNKNOWNMAP;
    	s_votemenu_map.mappics[i].generic.ownerdraw = VoteMapMenu_LevelshotDraw;
    
    	s_votemenu_map.mapbuttons[i].generic.type     = MTYPE_BITMAP;
    	s_votemenu_map.mapbuttons[i].generic.flags    = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS|QMF_NODEFAULTINIT;
    	s_votemenu_map.mapbuttons[i].generic.id       = ID_PICTURES+i;
    	s_votemenu_map.mapbuttons[i].generic.callback = VoteMapMenu_MapEvent;
    	s_votemenu_map.mapbuttons[i].generic.x	     = x - 30;
    	s_votemenu_map.mapbuttons[i].generic.y	     = y - 32;
    	s_votemenu_map.mapbuttons[i].width  		     = 256;
    	s_votemenu_map.mapbuttons[i].height  	     = 248;
    	s_votemenu_map.mapbuttons[i].generic.left     = x;
    	s_votemenu_map.mapbuttons[i].generic.top  	 = y;
    	s_votemenu_map.mapbuttons[i].generic.right    = x + 128;
    	s_votemenu_map.mapbuttons[i].generic.bottom   = y + 128;
    	s_votemenu_map.mapbuttons[i].focuspic         = ART_SELECT;
    }
    
    s_votemenu_map.arrows.generic.type  = MTYPE_BITMAP;
    s_votemenu_map.arrows.generic.name  = ART_ARROWSH;
    s_votemenu_map.arrows.generic.flags = QMF_INACTIVE;
    s_votemenu_map.arrows.generic.x	   = 260;
    s_votemenu_map.arrows.generic.y	   = 400;
    s_votemenu_map.arrows.width  	   = 128;
    s_votemenu_map.arrows.height  	   = 32;
    
    s_votemenu_map.prevpage.generic.type	    = MTYPE_BITMAP;
    s_votemenu_map.prevpage.generic.flags    = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
    s_votemenu_map.prevpage.generic.callback = UI_VoteMapMenu_PreviousPageEvent;
    s_votemenu_map.prevpage.generic.id	    = ID_PREVPAGE;
    s_votemenu_map.prevpage.generic.x		= 260;
    s_votemenu_map.prevpage.generic.y		= 400;
    s_votemenu_map.prevpage.width  		    = 64;
    s_votemenu_map.prevpage.height  		    = 32;
    s_votemenu_map.prevpage.focuspic         = ART_ARROWSHL;
    
    s_votemenu_map.nextpage.generic.type	    = MTYPE_BITMAP;
    s_votemenu_map.nextpage.generic.flags    = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
    s_votemenu_map.nextpage.generic.callback = UI_VoteMapMenu_NextPageEvent;
    s_votemenu_map.nextpage.generic.id	    = ID_NEXTPAGE;
    s_votemenu_map.nextpage.generic.x		= 321;
    s_votemenu_map.nextpage.generic.y		= 400;
    s_votemenu_map.nextpage.width  		    = 64;
    s_votemenu_map.nextpage.height  		    = 32;
    s_votemenu_map.nextpage.focuspic         = ART_ARROWSHR;

    s_votemenu_map.page.generic.type		= MTYPE_TEXT;
    s_votemenu_map.page.generic.flags	= QMF_CENTER_JUSTIFY|QMF_SMALLFONT;
    s_votemenu_map.page.generic.x		= 320;
    s_votemenu_map.page.generic.y		= 368;
    s_votemenu_map.page.string			= pagebuffer;
    s_votemenu_map.page.color         = color_white;
    s_votemenu_map.page.style         = UI_CENTER;

    
    s_votemenu_map.mapname.generic.type  = MTYPE_PTEXT;
    s_votemenu_map.mapname.generic.flags = QMF_CENTER_JUSTIFY|QMF_INACTIVE;
    s_votemenu_map.mapname.generic.x	    = 320;
    s_votemenu_map.mapname.generic.y	    = 440;
    s_votemenu_map.mapname.string        = mapnamebuffer;
    s_votemenu_map.mapname.style         = UI_CENTER|UI_BIGFONT;
    s_votemenu_map.mapname.color         = text_color_normal;
    
    
    Menu_AddItem( &s_votemenu_map.menu, &s_votemenu_map.type );
    
    for (i=0; i<MAX_MAPSPERPAGE; i++)
    {
    	Menu_AddItem( &s_votemenu_map.menu, &s_votemenu_map.mappics[i] );
    	Menu_AddItem( &s_votemenu_map.menu, &s_votemenu_map.mapbuttons[i] );
    }

    Menu_AddItem( &s_votemenu_map.menu, (void*) &s_votemenu_map.banner );
    Menu_AddItem( &s_votemenu_map.menu, (void*) &s_votemenu_map.back );
    Menu_AddItem( &s_votemenu_map.menu, (void*) &s_votemenu_map.go );
    Menu_AddItem( &s_votemenu_map.menu, (void*) &s_votemenu_map.arrows );
    Menu_AddItem( &s_votemenu_map.menu, (void*) &s_votemenu_map.prevpage );
    Menu_AddItem( &s_votemenu_map.menu, (void*) &s_votemenu_map.nextpage );
    Menu_AddItem( &s_votemenu_map.menu, (void*) &s_votemenu_map.mapname );
    Menu_AddItem( &s_votemenu_map.menu, (void*) &s_votemenu_map.page );

    getmappage_cmd = getmappage_all_cmd;

    ResetMaplist();
    UI_VoteMapMenu_Update();
    trap_Cmd_ExecuteText( EXEC_APPEND,va("%s 0", getmappage_cmd) );
    maplist.num_sent_cmds++;

    trap_Cvar_Set( "cl_paused", "0" ); //We cannot send server commands while paused!


    UI_PushMenu( &s_votemenu_map.menu );
}
