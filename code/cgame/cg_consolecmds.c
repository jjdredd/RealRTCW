/*
===========================================================================

Return to Castle Wolfenstein single player GPL Source Code
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company. 

This file is part of the Return to Castle Wolfenstein single player GPL Source Code (RTCW SP Source Code).  

RTCW SP Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

RTCW SP Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with RTCW SP Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the RTCW SP Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the RTCW SP Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/

/*
 * name:		cg_consolecmds.c
 *
 * desc:		text commands typed in at the local console, or executed by a key binding
 *
*/


#include "cg_local.h"
#include "../ui/ui_shared.h"



void CG_TargetCommand_f( void ) {
	int targetNum;
	char test[4];

	targetNum = CG_CrosshairPlayer();
	if ( targetNum == -1 ) {
		return;
	}

	trap_Argv( 1, test, 4 );
	trap_SendClientCommand( va( "gc %i %i", targetNum, atoi( test ) ) );
}



/*
=================
CG_SizeUp_f

Keybinding command
=================
*/
static void CG_SizeUp_f( void ) {
	trap_Cvar_Set( "cg_viewsize", va( "%i",(int)( cg_viewsize.integer + 10 ) ) );
}


/*
=================
CG_SizeDown_f

Keybinding command
=================
*/
static void CG_SizeDown_f( void ) {
	trap_Cvar_Set( "cg_viewsize", va( "%i",(int)( cg_viewsize.integer - 10 ) ) );
}


/*
=============
CG_Viewpos_f

Debugging command to print the current position
=============
*/
static void CG_Viewpos_f( void ) {
	CG_Printf( "(%i %i %i) : %i\n", (int)cg.refdef.vieworg[0],
			   (int)cg.refdef.vieworg[1], (int)cg.refdef.vieworg[2],
			   (int)cg.refdefViewAngles[YAW] );
}


static void CG_ScoresDown_f( void ) {
	if ( cg.scoresRequestTime + 2000 < cg.time ) {
		// the scores are more than two seconds out of data,
		// so request new ones
		cg.scoresRequestTime = cg.time;
		trap_SendClientCommand( "score" );

		// leave the current scores up if they were already
		// displayed, but if this is the first hit, clear them out
		if ( !cg.showScores ) {
			cg.showScores = qtrue;
			cg.numScores = 0;
		}
	} else {
		// show the cached contents even if they just pressed if it
		// is within two seconds
		cg.showScores = qtrue;
	}
}

static void CG_ScoresUp_f( void ) {
	if ( cg.showScores ) {
		cg.showScores = qfalse;
		cg.scoreFadeTime = cg.time;
	}
}


extern menuDef_t *menuScoreboard;
void Menu_Reset( void );          // FIXME: add to right include file

static void CG_LoadHud_f( void ) {
	char buff[1024];
	const char *hudSet;
	memset( buff, 0, sizeof( buff ) );

	String_Init();
	Menu_Reset();

	trap_Cvar_VariableStringBuffer( "cg_hudFiles", buff, sizeof( buff ) );
	hudSet = buff;
	if ( hudSet[0] == '\0' ) {
		hudSet = "ui/hud.txt";
	}

	CG_LoadMenus( hudSet );
	menuScoreboard = NULL;
}

// TTimo: defined but not used
/*
static void CG_scrollScoresDown_f( void) {
	if (menuScoreboard && cg.scoreBoardShowing) {
		Menu_ScrollFeeder(menuScoreboard, FEEDER_SCOREBOARD, qtrue);
		Menu_ScrollFeeder(menuScoreboard, FEEDER_REDTEAM_LIST, qtrue);
		Menu_ScrollFeeder(menuScoreboard, FEEDER_BLUETEAM_LIST, qtrue);
	}
}


static void CG_scrollScoresUp_f( void) {
	if (menuScoreboard && cg.scoreBoardShowing) {
		Menu_ScrollFeeder(menuScoreboard, FEEDER_SCOREBOARD, qfalse);
		Menu_ScrollFeeder(menuScoreboard, FEEDER_REDTEAM_LIST, qfalse);
		Menu_ScrollFeeder(menuScoreboard, FEEDER_BLUETEAM_LIST, qfalse);
	}
}


static void CG_spWin_f( void) {
	trap_Cvar_Set("cg_cameraOrbit", "2");
	trap_Cvar_Set("cg_cameraOrbitDelay", "35");
	trap_Cvar_Set("cg_thirdPerson", "1");
	trap_Cvar_Set("cg_thirdPersonAngle", "0");
	trap_Cvar_Set("cg_thirdPersonRange", "100");
//	CG_AddBufferedSound(cgs.media.winnerSound);
	//trap_S_StartLocalSound(cgs.media.winnerSound, CHAN_ANNOUNCER);
	CG_CenterPrint("YOU WIN!", SCREEN_HEIGHT * .30, 0);
}

static void CG_spLose_f( void) {
	trap_Cvar_Set("cg_cameraOrbit", "2");
	trap_Cvar_Set("cg_cameraOrbitDelay", "35");
	trap_Cvar_Set("cg_thirdPerson", "1");
	trap_Cvar_Set("cg_thirdPersonAngle", "0");
	trap_Cvar_Set("cg_thirdPersonRange", "100");
//	CG_AddBufferedSound(cgs.media.loserSound);
	//trap_S_StartLocalSound(cgs.media.loserSound, CHAN_ANNOUNCER);
	CG_CenterPrint("YOU LOSE...", SCREEN_HEIGHT * .30, 0);
}
*/

//----(SA)	item (key/pickup) drawing
static void CG_InventoryDown_f( void ) {
	cg.showItems = qtrue;
}

static void CG_InventoryUp_f( void ) {
	cg.showItems = qfalse;
	cg.itemFadeTime = cg.time;
}

//----(SA)	end

static void CG_TellTarget_f( void ) {
	int clientNum;
	char command[128];
	char message[128];

	clientNum = CG_CrosshairPlayer();
	if ( clientNum == -1 ) {
		return;
	}

	trap_Args( message, 128 );
	Com_sprintf( command, 128, "tell %i %s", clientNum, message );
	trap_SendClientCommand( command );
}

static void CG_TellAttacker_f( void ) {
	int clientNum;
	char command[128];
	char message[128];

	clientNum = CG_LastAttacker();
	if ( clientNum == -1 ) {
		return;
	}

	trap_Args( message, 128 );
	Com_sprintf( command, 128, "tell %i %s", clientNum, message );
	trap_SendClientCommand( command );
}


// TTimo: defined but not used
/*
static void CG_NextTeamMember_f( void ) {
  CG_SelectNextPlayer();
}

static void CG_PrevTeamMember_f( void ) {
  CG_SelectPrevPlayer();
}
*/

/////////// cameras

#define MAX_CAMERAS 64  // matches define in splines.cpp
qboolean cameraInuse[MAX_CAMERAS];

int CG_LoadCamera( const char *name ) {
	int i;
	for ( i = 1; i < MAX_CAMERAS; i++ ) {    // start at '1' since '0' is always taken by the cutscene camera
		if ( !cameraInuse[i] ) {
			if ( trap_loadCamera( i, name ) ) {
				cameraInuse[i] = qtrue;
				return i;
			}
		}
	}
	return -1;
}

void CG_FreeCamera( int camNum ) {
	cameraInuse[camNum] = qfalse;
}


static void CG_LoadWeapons_f( void ) {
	int i;

	for ( i = WP_KNIFE; i < WP_NUM_WEAPONS; i++ ) {
			CG_RegisterWeapon( i, qtrue );
	}
}

/*
==============
CG_StartCamera
==============
*/
void CG_StartCamera( const char *name, qboolean startBlack ) {
	char lname[MAX_QPATH];

	//if ( cg.predictedPlayerState.stats[STAT_HEALTH] <= 0 )	// don't allow camera to start if you're dead
	//	return;

	COM_StripExtension( name, lname, sizeof( lname ) );    //----(SA)	added
	Q_strcat( lname, sizeof( lname ), ".camera" );

	if ( trap_loadCamera( CAM_PRIMARY, va( "cameras/%s", lname ) ) ) {
		cg.cameraMode = qtrue;				// camera on in cgame
		if ( startBlack ) {
			CG_Fade( 0, 0, 0, 255, cg.time, 0 );		// go black
		}
		trap_Cvar_Set( "cg_letterbox", "1" ); // go letterbox
		trap_SendClientCommand( "startCamera" );	// camera on in game
		trap_startCamera( CAM_PRIMARY, cg.time );	// camera on in client
	} else {
		//----(SA)	removed check for cams in main dir
		cg.cameraMode = qfalse;                 // camera off in cgame
		trap_SendClientCommand( "stopCamera" );    // camera off in game
		trap_stopCamera( CAM_PRIMARY );           // camera off in client
		CG_Fade( 0, 0, 0, 0, cg.time, 0 );        // ensure fadeup
		trap_Cvar_Set( "cg_letterbox", "0" );
		CG_Printf( "Unable to load camera %s\n",lname );
	}
}

/*
===================
CG_DumpCastAi_f

Dump a ai_zombie definition to the ents file
===================
*/
static void CG_DumpCastAi_f( void ) {
	char aicastfilename[MAX_QPATH];
	char ainame[MAX_STRING_CHARS];
	char aitype[MAX_STRING_CHARS];
	char *aiautoname;
	char *extptr, *buffptr;
	char buff[1024];
	fileHandle_t f;
	int autonumber = 0;


	trap_Cvar_VariableStringBuffer( "cg_entityEditCounter", buff, sizeof( buff ) );
	autonumber = atoi( buff );

	// Check for argument
	if ( trap_Argc() < 2 ) {
		CG_Printf( "Usage: dumpcastai <type> [name]\n" );
		return;
	}
	trap_Argv( 1, aitype, sizeof( aitype ) );

	if ( strcmp( "ai_soldier", aitype ) &&
		 strcmp( "ai_american", aitype ) &&
		 strcmp( "ai_zombie", aitype ) &&
		 strcmp( "ai_warzombie", aitype ) &&
		 strcmp( "ai_venom", aitype ) &&
		 strcmp( "ai_loper", aitype ) &&
		 strcmp( "ai_boss_helga", aitype ) &&
		 strcmp( "ai_boss_heinrich", aitype ) &&
		 strcmp( "ai_eliteguard", aitype ) &&
		 strcmp( "ai_stimsoldier_dual", aitype ) &&
		 strcmp( "ai_stimsoldier_rocket", aitype ) &&
		 strcmp( "ai_stimsoldier_tesla", aitype ) &&
		 strcmp( "ai_supersoldier", aitype ) &&
		 strcmp( "ai_protosoldier", aitype ) &&
		 strcmp( "ai_frogman", aitype ) &&
		 strcmp( "ai_blackguard", aitype ) &&
		 strcmp( "ai_partisan", aitype ) &&
		 strcmp( "ai_civilian", aitype ) &&
		 strcmp( "ai_priest", aitype ) &&
		 strcmp( "ai_xshepherd", aitype ) &&
		 strcmp( "ai_dog", aitype ) &&
		 strcmp( "ai_russian", aitype ) ) {

		CG_Printf( "Wrong type\n" );
		CG_Printf( "Usage: dumpcastai <type> [name]\n" );
		return;
	}

	if ( trap_Argc() == 3 ) {
		trap_Argv( 2, ainame, sizeof( ainame ) );
	} else
	{
		aiautoname = va( "reinforce_%s_%d", aitype, autonumber++ );
		Q_strncpyz( ainame, aiautoname, strlen( aiautoname ) + 1 );
	}

	trap_Cvar_Set( "cg_entityEditCounter", va( "%i",autonumber ) );

	// Open aicast file
	Q_strncpyz( aicastfilename, cgs.mapname, sizeof( aicastfilename ) );
	extptr = aicastfilename + strlen( aicastfilename ) - 4;
	if ( extptr < aicastfilename || Q_stricmp( extptr, ".bsp" ) ) {
		CG_Printf( "Unable to dump, unknown map name?\n" );
		return;
	}
	Q_strncpyz( extptr, ".ents", 6 );
	trap_FS_FOpenFile( aicastfilename, &f, FS_APPEND_SYNC );
	if ( !f ) {
		CG_Printf( "Failed to open '%s' for writing.\n", aicastfilename );
		return;
	}

	// Strip bad characters out
	for ( buffptr = ainame; *buffptr; buffptr++ )
	{
		if ( *buffptr == '\n' ) {
			*buffptr = ' ';
		} else if ( *buffptr == '"' ) {
			*buffptr = '\'';
		}
	}
	// Kill any trailing space as well
	if ( *( buffptr - 1 ) == ' ' ) {
		*( buffptr - 1 ) = 0;
	}

	// Strip bad characters out
	for ( buffptr = aitype; *buffptr; buffptr++ )
	{
		if ( *buffptr == '\n' ) {
			*buffptr = ' ';
		} else if ( *buffptr == '"' ) {
			*buffptr = '\'';
		}
	}
	// Kill any trailing space as well
	if ( *( buffptr - 1 ) == ' ' ) {
		*( buffptr - 1 ) = 0;
	}

	// Build the entity definition
	buffptr = va(   "{\n\"classname\" \"%s\"\n\"origin\" \"%i %i %i\"\n\"ainame\" \"%s\"\n\"angle\" \"%i\"\n\"spawnflags\" \"1\"\n}\n", aitype, (int) cg.snap->ps.origin[0], (int) cg.snap->ps.origin[1], (int) cg.snap->ps.origin[2], ainame, (int)cg.refdefViewAngles[YAW] );

	// And write out/acknowledge
	trap_FS_Write( buffptr, strlen( buffptr ), f );
	trap_FS_FCloseFile( f );
	CG_Printf( "%s (%s) entity dumped to '%s' (%i %i %i).\n", aitype, ainame, aicastfilename,
			   (int) cg.snap->ps.origin[0], (int) cg.snap->ps.origin[1], (int) cg.snap->ps.origin[2] );
}

/*
===================
CG_DumpLocation_f

Dump a target_location definition to a file
===================
*/
static void CG_DumpLocation_f( void ) {
	char locfilename[MAX_QPATH];
	char locname[MAX_STRING_CHARS];
	char *extptr, *buffptr;
	fileHandle_t f;

	// Check for argument
	if ( trap_Argc() < 2 ) {
		CG_Printf( "Usage: dumploc <locationname>\n" );
		return;
	}
	trap_Args( locname, sizeof( locname ) );

	// Open locations file
	Q_strncpyz( locfilename, cgs.mapname, sizeof( locfilename ) );
	extptr = locfilename + strlen( locfilename ) - 4;
	if ( extptr < locfilename || Q_stricmp( extptr, ".bsp" ) ) {
		CG_Printf( "Unable to dump, unknown map name?\n" );
		return;
	}
	Q_strncpyz( extptr, ".ents", 6 );
	trap_FS_FOpenFile( locfilename, &f, FS_APPEND_SYNC );
	if ( !f ) {
		CG_Printf( "Failed to open '%s' for writing.\n", locfilename );
		return;
	}

	// Strip bad characters out
	for ( buffptr = locname; *buffptr; buffptr++ )
	{
		if ( *buffptr == '\n' ) {
			*buffptr = ' ';
		} else if ( *buffptr == '"' ) {
			*buffptr = '\'';
		}
	}
	// Kill any trailing space as well
	if ( *( buffptr - 1 ) == ' ' ) {
		*( buffptr - 1 ) = 0;
	}

	// Build the entity definition
	buffptr = va(   "{\n\"classname\" \"target_location\"\n\"origin\" \"%i %i %i\"\n\"message\" \"%s\"\n}\n\n",
					(int) cg.snap->ps.origin[0], (int) cg.snap->ps.origin[1], (int) cg.snap->ps.origin[2], locname );

	// And write out/acknowledge
	trap_FS_Write( buffptr, strlen( buffptr ), f );
	trap_FS_FCloseFile( f );
	CG_Printf( "Entity dumped to '%s' (%i %i %i).\n", locfilename,
			   (int) cg.snap->ps.origin[0], (int) cg.snap->ps.origin[1], (int) cg.snap->ps.origin[2] );
}

/*
==============
CG_StopCamera
==============
*/
void CG_StopCamera( void ) {
	cg.cameraMode = qfalse;                 // camera off in cgame
	trap_SendClientCommand( "stopCamera" );    // camera off in game
	trap_stopCamera( CAM_PRIMARY );           // camera off in client
	trap_Cvar_Set( "cg_letterbox", "0" );

	// fade back into world
	CG_Fade( 0, 0, 0, 255, 0, 0 );
	CG_Fade( 0, 0, 0, 0, cg.time + 500, 2000 );

}

static void CG_Camera_f( void ) {
	char name[MAX_QPATH];

	trap_Argv( 1, name, sizeof( name ) );

	CG_StartCamera( name, qfalse );
}

static void CG_Fade_f( void ) {
	int r, g, b, a;
	float duration;

	if ( trap_Argc() < 6 ) {
		return;
	}

	r = atof( CG_Argv( 1 ) );
	g = atof( CG_Argv( 2 ) );
	b = atof( CG_Argv( 3 ) );
	a = atof( CG_Argv( 4 ) );

	duration = atof( CG_Argv( 5 ) ) * 1000;

	CG_Fade( r, g, b, a, cg.time, duration );
}

// NERVE - SMF
static void CG_QuickMessage_f( void ) {
	return;
}

static void CG_OpenLimbo_f( void ) {
	return;
}

static void CG_CloseLimbo_f( void ) {
	return;
}

static void CG_LimboMessage_f( void ) {
	return;
}
// -NERVE - SMF

typedef struct {
	char    *cmd;
	void ( *function )( void );
} consoleCommand_t;

static consoleCommand_t commands[] = {
	{ "testgun", CG_TestGun_f },
	{ "testmodel", CG_TestModel_f },
	{ "nextframe", CG_TestModelNextFrame_f },
	{ "prevframe", CG_TestModelPrevFrame_f },
	{ "nextskin", CG_TestModelNextSkin_f },
	{ "prevskin", CG_TestModelPrevSkin_f },
	{ "viewpos", CG_Viewpos_f },
	{ "+scores", CG_ScoresDown_f },
	{ "-scores", CG_ScoresUp_f },
	{ "+inventory", CG_InventoryDown_f },
	{ "-inventory", CG_InventoryUp_f },
//	{ "+zoom", CG_ZoomDown_f },		// (SA) zoom moved to a wbutton so server can determine weapon firing based on zoom status
//	{ "-zoom", CG_ZoomUp_f },
	{ "zoomin", CG_ZoomIn_f },
	{ "zoomout", CG_ZoomOut_f },
	{ "sizeup", CG_SizeUp_f },
	{ "sizedown", CG_SizeDown_f },
	{ "weaplastused", CG_LastWeaponUsed_f },
	{ "weapnextinbank", CG_NextWeaponInBank_f },
	{ "weapprevinbank", CG_PrevWeaponInBank_f },
	{ "weapnext", CG_NextWeapon_f },
	{ "weapprev", CG_PrevWeapon_f },
	{ "weapalt", CG_AltWeapon_f },
	{ "weapon", CG_Weapon_f },
	{ "weaponbank", CG_WeaponBank_f },
	{ "itemnext", CG_NextItem_f },
	{ "itemprev", CG_PrevItem_f },
	{ "item", CG_Item_f },
	{ "tell_target", CG_TellTarget_f },
	{ "tell_attacker", CG_TellAttacker_f },
	{ "tcmd", CG_TargetCommand_f },
	{ "loadhud", CG_LoadHud_f },
	{ "loaddeferred", CG_LoadDeferredPlayers },  // spelling fixed (SA)
	{ "camera", CG_Camera_f },   // duffy
	{ "fade", CG_Fade_f },   // duffy
	{ "loadweapons", CG_LoadWeapons_f },

	// NERVE - SMF
	{ "mp_QuickMessage", CG_QuickMessage_f },
	{ "OpenLimboMenu", CG_OpenLimbo_f },
	{ "CloseLimboMenu", CG_CloseLimbo_f },
	{ "LimboMessage", CG_LimboMessage_f },
	// -NERVE - SMF
	{ "dumploc", CG_DumpLocation_f },
	{ "dumpcastai", CG_DumpCastAi_f }
};


/*
=================
CG_ConsoleCommand

The string has been tokenized and can be retrieved with
Cmd_Argc() / Cmd_Argv()
=================
*/
qboolean CG_ConsoleCommand( void ) {
	const char  *cmd;
	int i;

	cmd = CG_Argv( 0 );

	for ( i = 0 ; i < ARRAY_LEN( commands ) ; i++ ) {
		if ( !Q_stricmp( cmd, commands[i].cmd ) ) {
			commands[i].function();
			return qtrue;
		}
	}

	return qfalse;
}


/*
=================
CG_InitConsoleCommands

Let the client system know about all of our commands
so it can perform tab completion
=================
*/
void CG_InitConsoleCommands( void ) {
	int i;

	for ( i = 0 ; i < ARRAY_LEN( commands ) ; i++ ) {
		trap_AddCommand( commands[i].cmd );
	}

	//
	// the game server will interpret these commands, which will be automatically
	// forwarded to the server after they are not recognized locally
	//
	trap_AddCommand( "kill" );
	trap_AddCommand( "say" );
	trap_AddCommand( "say_team" );
	trap_AddCommand( "say_limbo" );           // NERVE - SMF
	trap_AddCommand( "tell" );
//	trap_AddCommand( "vsay" );
//	trap_AddCommand( "vsay_team" );
//	trap_AddCommand( "vtell" );
//	trap_AddCommand( "vtaunt" );
//	trap_AddCommand( "vosay" );
//	trap_AddCommand( "vosay_team" );
//	trap_AddCommand( "votell" );
	trap_AddCommand( "give" );
	trap_AddCommand( "god" );
	trap_AddCommand( "notarget" );
	trap_AddCommand( "noclip" );
	trap_AddCommand( "where" );
	trap_AddCommand( "team" );
	trap_AddCommand( "follow" );
	trap_AddCommand( "follownext" );
	trap_AddCommand( "followprev" );
	trap_AddCommand( "levelshot" );
	trap_AddCommand( "addbot" );
	trap_AddCommand( "setviewpos" );
	trap_AddCommand( "callvote" );
	trap_AddCommand( "vote" );
//	trap_AddCommand( "callteamvote" );
//	trap_AddCommand( "teamvote" );
	trap_AddCommand( "stats" );
//	trap_AddCommand( "teamtask" );
	trap_AddCommand( "loaddeferred" );        // spelling fixed (SA)

	trap_AddCommand( "startCamera" );
	trap_AddCommand( "stopCamera" );
	trap_AddCommand( "setCameraOrigin" );

	// Rafael
	trap_AddCommand( "nofatigue" );

	// NERVE - SMF
	trap_AddCommand( "setspawnpt" );
	// NERVE - SMF
}
