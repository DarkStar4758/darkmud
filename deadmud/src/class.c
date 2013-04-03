/**************************************************************************
*  File: class.c                                           Part of tbaMUD *
*  Usage: Source file for class-specific code.                            *
*                                                                         *
*  All rights reserved.  See license for complete information.            *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
**************************************************************************/

/** Help buffer the global variable definitions */
#define __CLASS_C__

/* This file attempts to concentrate most of the code which must be changed
 * in order for new classes to be added.  If you're adding a new class, you
 * should go through this entire file from beginning to end and add the
 * appropriate new special cases for your new class. */

#include "conf.h"
#include "sysdep.h"
#include "structs.h"
#include "utils.h"
#include "db.h"
#include "spells.h"
#include "interpreter.h"
#include "constants.h"
#include "act.h"


/* Names first */
const char *class_abbrevs[] = {
  "Bi",
  "Me",
  "Ba",
  "So",
  "\n"
};

const char *pc_class_types[] = {
  "Biotic",
  "Medic",
  "Bandit",
  "Soldierr",
  "\n"
};

/* The menu for choosing a class in interpreter.c: */
const char *class_menu =
"\r\n"
"+------------------+\r\n"
"|       Class      |\r\n"
"+------------------+\r\n"
"| A) Medic         |\r\n"
"| B) Bandit        |\r\n"
"| C) Soldier       |\r\n"
"| D) Biotic        |\r\n"
"+------------------|\r\n";

/* The code to interpret a class letter -- used in interpreter.c when a new
 * character is selecting a class and by 'set class' in act.wizard.c. */
int parse_class(char arg)
{
  arg = LOWER(arg);

  switch (arg) {
  case 'a': return CLASS_MEDIC;
  case 'b': return CLASS_BANDIT;
  case 'c': return CLASS_SOLDIER;
  case 'd': return CLASS_BIOTIC;
  default:  return CLASS_UNDEFINED;
  }
}

/* bitvectors (i.e., powers of two) for each class, mainly for use in do_who
 * and do_users.  Add new classes at the end so that all classes use sequential
 * powers of two (1 << 0, 1 << 1, 1 << 2, 1 << 3, 1 << 4, 1 << 5, etc.) up to
 * the limit of your bitvector_t, typically 0-31. */
bitvector_t find_class_bitvector(const char *arg)
{
  size_t rpos, ret = 0;

  for (rpos = 0; rpos < strlen(arg); rpos++)
    ret |= (1 << parse_class(arg[rpos]));

  return (ret);
}

/* These are definitions which control the guildmasters for each class.
 * The  first field (top line) controls the highest percentage skill level a
 * character of the class is allowed to attain in any skill.  (After this
 * level, attempts to practice will say "You are already learned in this area."
 *
 * The second line controls the maximum percent gain in learnedness a character
 * is allowed per practice -- in other words, if the random die throw comes out
 * higher than this number, the gain will only be this number instead.
 *
 * The third line controls the minimu percent gain in learnedness a character
 * is allowed per practice -- in other words, if the random die throw comes
 * out below this number, the gain will be set up to this number.
 *
 * The fourth line simply sets whether the character knows 'spells' or 'skills'.
 * This does not affect anything except the message given to the character when
 * trying to practice (i.e. "You know of the following spells" vs. "You know of
 * the following skills" */

#define SPELL	0
#define SKILL	1

/* #define LEARNED_LEVEL	0  % known which is considered "learned" */
/* #define MAX_PER_PRAC		1  max percent gain in skill per practice */
/* #define MIN_PER_PRAC		2  min percent gain in skill per practice */
/* #define PRAC_TYPE		3  should it say 'spell' or 'skill'?	*/

int prac_params[4][NUM_CLASSES] = {
  /* MAG	CLE	THE	WAR */
  { 95,		95,	85,	80	},	/* learned level */
  { 100,	100,	12,	12	},	/* max per practice */
  { 25,		25,	0,	0	},	/* min per practice */
  { SPELL,	SPELL,	SKILL,	SKILL	},	/* prac name */
};

/* The appropriate rooms for each guildmaster/guildguard; controls which types
 * of people the various guildguards let through.  i.e., the first line shows
 * that from room 3017, only MAGIC_USERS are allowed to go south. Don't forget
 * to visit spec_assign.c if you create any new mobiles that should be a guild
 * master or guard so they can act appropriately. If you "recycle" the
 * existing mobs that are used in other guilds for your new guild, then you
 * don't have to change that file, only here. Guildguards are now implemented
 * via triggers. This code remains as an example. */
struct guild_info_type guild_info[] = {

/* Midgaard */
 { CLASS_BIOTIC,        3017,    SOUTH   },
 { CLASS_MEDIC,         3004,    NORTH   },
 { CLASS_BANDIT,        3027,    EAST   },
 { CLASS_SOLDIER,       3021,    EAST   },

/* Brass Dragon */
  { -999 /* all */ ,	5065,	WEST	},

/* this must go last -- add new guards above! */
  { -1, NOWHERE, -1}
};

// NEW SAVING THROWS CODE
byte saving_throws(int class_num, int type, int level)
{
switch (class_num) {
case CLASS_BIOTIC:
switch (type) {
case SAVING_PARA:
/* Paralyzation */
if (level == 0)
return 100;
if (level >= LVL_IMMORT) {
return 0;
} else {
return (70 - ((70 * level) / (LVL_IMMORT - 1)));
}
case SAVING_ROD:
/* Rods */
if (level == 0)
return 100;
if (level >= LVL_IMMORT) {
return 0;
} else {
return (55 - ((55 * level) / (LVL_IMMORT - 1)));
}
case SAVING_PETRI: /* Petrification */
if (level == 0)
return 100;
if (level >= LVL_IMMORT) {
return 0;
} else {
return (65 - ((65 * level) / (LVL_IMMORT - 1)));
}
case SAVING_BREATH: /* Breath weapons */
if (level == 0)
return 100;
if (level >= LVL_IMMORT) {
return 0;
} else {
return (75 - ((75 * level) / (LVL_IMMORT - 1)));
}
case SAVING_SPELL: /* Generic spells */
if (level == 0)
return 100;
if (level >= LVL_IMMORT) {
return 0;
} else {
return (60 - ((60 * level) / (LVL_IMMORT - 1)));
}
default:
log("SYSERR: Invalid saving throw type.");
break;
}
break;
case CLASS_MEDIC:
switch (type) {
case SAVING_PARA:
/* Paralyzation */
if (level == 0)
return 100;
if (level >= LVL_IMMORT) {
return 0;
} else {
return (60 - ((60 * level) / (LVL_IMMORT - 1)));
}
case SAVING_ROD:
/* Rods */
if (level == 0)
return 100;
if (level >= LVL_IMMORT) {
return 0;
} else {
return (70 - ((70 * level) / (LVL_IMMORT - 1)));
}
case SAVING_PETRI: /* Petrification */
if (level == 0)
return 100;
if (level >= LVL_IMMORT) {
return 0;
} else {
return (65 - ((65 * level) / (LVL_IMMORT - 1)));
}
case SAVING_BREATH: /* Breath weapons */
if (level == 0)
return 100;
if (level >= LVL_IMMORT) {
return 0;
} else {
return (80 - ((80 * level) / (LVL_IMMORT - 1)));
}
case SAVING_SPELL: /* Generic spells */
if (level == 0)
return 100;
if (level >= LVL_IMMORT) {
return 0;
} else {
return (70 - ((70 * level) / (LVL_IMMORT - 1)));
}
default:
log("SYSERR: Invalid saving throw type.");
break;
}
break;
case CLASS_BANDIT:
switch (type) {
case SAVING_PARA:
/* Paralyzation */
if (level == 0)
return 100;
if (level >= LVL_IMMORT) {
return 0;
} else {
return (65 - ((65 * level) / (LVL_IMMORT - 1)));
}
case SAVING_ROD:
/* Rods */
if (level == 0)
return 100;
if (level >= LVL_IMMORT) {
return 0;
} else {
return (70 - ((70 * level) / (LVL_IMMORT - 1)));
}
case SAVING_PETRI: /* Petrification */
if (level == 0)
return 100;
if (level >= LVL_IMMORT) {
return 0;
} else {
return (60 - ((60 * level) / (LVL_IMMORT - 1)));
}
case SAVING_BREATH: /* Breath weapons */
if (level == 0)
return 100;
if (level >= LVL_IMMORT) {
return 0;
} else {
return (80 - ((80 * level) / (LVL_IMMORT - 1)));
}
case SAVING_SPELL: /* Generic spells */
if (level == 0)
return 100;
if (level >= LVL_IMMORT) {
return 0;
} else {
return (75 - ((75 * level) / (LVL_IMMORT - 1)));
}
default:
log("SYSERR: Invalid saving throw type.");
break;
}
break;
case CLASS_SOLDIER:
switch (type) {
case SAVING_PARA:
/* Paralyzation */
if (level == 0)
return 100;
if (level >= LVL_IMMORT) {
return 0;
} else {
return (70 - ((70 * level) / (LVL_IMMORT - 1)));
}
case SAVING_ROD:
if (level == 0)
return 100;
/* Rods */
if (level >= LVL_IMMORT) {
return 0;
} else {
return (80 - ((80 * level) / (LVL_IMMORT - 1)));
}
case SAVING_PETRI: /* Petrification */
if (level == 0)
return 100;
if (level >= LVL_IMMORT) {
return 0;
} else {
return (75 - ((75 * level) / (LVL_IMMORT - 1)));
}
case SAVING_BREATH: /* Breath weapons */
if (level == 0)
return 100;
if (level >= LVL_IMMORT) {
return 0;
} else {
return (85 - ((85 * level) / (LVL_IMMORT - 1)));
}
case SAVING_SPELL: /* Generic spells */
if (level == 0)
return 100;
if (level >= LVL_IMMORT) {
return 0;
} else {
return (85 - ((85 * level) / (LVL_IMMORT - 1)));
}
default:
log("SYSERR: Invalid saving throw type.");
break;
}
default:
log("SYSERR: Invalid class saving throw.");
break;
}
/* Should not get here unless something is wrong. */
return 100;
}

// NEW THAC0 CODE
int thaco(int class_num, int level)
{
switch (class_num) {
case CLASS_BIOTIC:
if (level == 0)
return 100;
if (level >= LVL_IMMORT)
{
return 0;
} else {
return (20 - ((20 * level) / (LVL_IMMORT - 1)));
}
case CLASS_MEDIC:
if (level == 0)
return 100;
if (level >= LVL_IMMORT)
{
return 0;
} else {
return (19 - ((19 * level) / (LVL_IMMORT - 1)));
}
case CLASS_BANDIT:
if (level == 0)
return 100;
if (level >= LVL_IMMORT)
{
return 0;
} else {
return (18 - ((18 * level) / (LVL_IMMORT - 1)));
}
case CLASS_SOLDIER:
if (level == 0)
return 100;
if (level >= LVL_IMMORT)
{
return 0;
} else {
return (17 - ((17 * level) / (LVL_IMMORT - 1)));
}
default:
log("SYSERR: Unknown class in thac0 chart.");
}
/* Will not get there unless something is wrong. */
return 100;
}



/* Roll the 6 stats for a character... each stat is made of the sum of the best
 * 3 out of 4 rolls of a 6-sided die.  Each class then decides which priority
 * will be given for the best to worst stats. */
void roll_real_abils(struct char_data *ch)
{
  int i, j, k, temp;
  ubyte table[6];
  ubyte rolls[4];

  for (i = 0; i < 6; i++)
    table[i] = 0;

  for (i = 0; i < 6; i++) {

    for (j = 0; j < 4; j++)
      rolls[j] = rand_number(1, 6);

    temp = rolls[0] + rolls[1] + rolls[2] + rolls[3] -
      MIN(rolls[0], MIN(rolls[1], MIN(rolls[2], rolls[3])));

    for (k = 0; k < 6; k++)
      if (table[k] < temp) {
	temp ^= table[k];
	table[k] ^= temp;
	temp ^= table[k];
      }
  }

  ch->real_abils.str_add = 0;

  switch (GET_CLASS(ch)) {
  case CLASS_BIOTIC:
    ch->real_abils.intel = table[0];
    ch->real_abils.wis = table[1];
    ch->real_abils.dex = table[2];
    ch->real_abils.str = table[3];
    ch->real_abils.con = table[4];
    ch->real_abils.cha = table[5];
    break;
  case CLASS_MEDIC:
    ch->real_abils.wis = table[0];
    ch->real_abils.intel = table[1];
    ch->real_abils.str = table[2];
    ch->real_abils.dex = table[3];
    ch->real_abils.con = table[4];
    ch->real_abils.cha = table[5];
    break;
  case CLASS_BANDIT:
    ch->real_abils.dex = table[0];
    ch->real_abils.str = table[1];
    ch->real_abils.con = table[2];
    ch->real_abils.intel = table[3];
    ch->real_abils.wis = table[4];
    ch->real_abils.cha = table[5];
    break;
  case CLASS_SOLDIER:
    ch->real_abils.str = table[0];
    ch->real_abils.dex = table[1];
    ch->real_abils.con = table[2];
    ch->real_abils.wis = table[3];
    ch->real_abils.intel = table[4];
    ch->real_abils.cha = table[5];
    if (ch->real_abils.str == 18)
      ch->real_abils.str_add = rand_number(0, 100);
    break;
  }
  ch->aff_abils = ch->real_abils;
}

/* Some initializations for characters, including initial skills */
void do_start(struct char_data *ch)
{
  GET_LEVEL(ch) = 1;
  GET_TOT_LEVEL(ch) = 1;
  GET_EXP(ch) = 1;

  set_title(ch, NULL);
  roll_real_abils(ch);

  GET_MAX_HIT(ch)  = 10;
  GET_MAX_MANA(ch) = 100;
  GET_MAX_MOVE(ch) = 82;

  switch (GET_CLASS(ch)) {

  case CLASS_BIOTIC:
    break;

  case CLASS_MEDIC:
    break;

  case CLASS_BANDIT:
    SET_SKILL(ch, SKILL_SNEAK, 10);
    SET_SKILL(ch, SKILL_HIDE, 5);
    SET_SKILL(ch, SKILL_STEAL, 15);
    SET_SKILL(ch, SKILL_BACKSTAB, 10);
    SET_SKILL(ch, SKILL_PICK_LOCK, 10);
    SET_SKILL(ch, SKILL_TRACK, 10);
    break;

  case CLASS_SOLDIER:
    break;
  }

  advance_level(ch);

  GET_HIT(ch) = GET_MAX_HIT(ch);
  GET_MANA(ch) = GET_MAX_MANA(ch);
  GET_MOVE(ch) = GET_MAX_MOVE(ch);

  GET_COND(ch, THIRST) = 24;
  GET_COND(ch, HUNGER) = 24;
  GET_COND(ch, DRUNK) = 0;

  if (CONFIG_SITEOK_ALL)
    SET_BIT_AR(PLR_FLAGS(ch), PLR_SITEOK);
}

/* This function controls the change to maxmove, maxmana, and maxhp for each
 * class every time they gain a level. */
void advance_level(struct char_data *ch)
{
  int add_hp, add_mana = 0, add_move = 0, i;

  add_hp = con_app[GET_CON(ch)].hitp;

  switch (GET_CLASS(ch)) {

  case CLASS_BIOTIC:
    add_hp += rand_number(3, 8);
    add_mana = rand_number(GET_LEVEL(ch), (int)(1.5 * GET_LEVEL(ch)));
    add_mana = MIN(add_mana, 10);
    add_move = rand_number(0, 2);
    break;

  case CLASS_MEDIC:
    add_hp += rand_number(5, 10);
    add_mana = rand_number(GET_LEVEL(ch), (int)(1.5 * GET_LEVEL(ch)));
    add_mana = MIN(add_mana, 10);
    add_move = rand_number(0, 2);
    break;

  case CLASS_BANDIT:
    add_hp += rand_number(7, 13);
    add_mana = 0;
    add_move = rand_number(1, 3);
    break;

  case CLASS_SOLDIER:
    add_hp += rand_number(10, 15);
    add_mana = 0;
    add_move = rand_number(1, 3);
    break;
  }

  ch->points.max_hit += MAX(1, add_hp);
  ch->points.max_move += MAX(1, add_move);

  if (GET_LEVEL(ch) > 1)
    ch->points.max_mana += add_mana;

  if (IS_BIOTIC(ch) || IS_MEDIC(ch))
    GET_PRACTICES(ch) += MAX(2, wis_app[GET_WIS(ch)].bonus);
  else
    GET_PRACTICES(ch) += MIN(2, MAX(1, wis_app[GET_WIS(ch)].bonus));

  if (GET_LEVEL(ch) >= LVL_IMMORT) {
    for (i = 0; i < 3; i++)
      GET_COND(ch, i) = (char) -1;
    SET_BIT_AR(PRF_FLAGS(ch), PRF_HOLYLIGHT);
  }

  snoop_check(ch);
  save_char(ch);
}

/* This simply calculates the backstab multiplier based on a character's level.
 * This used to be an array, but was changed to be a function so that it would
 * be easier to add more levels to your MUD.  This doesn't really create a big
 * performance hit because it's not used very often. */
int backstab_mult(int level)
{
  if (level <= 7)
    return 2;	  /* level 1 - 7 */
  else if (level <= 13)
    return 3;	  /* level 8 - 13 */
  else if (level <= 20)
    return 4;	  /* level 14 - 20 */
  else if (level <= 28)
    return 5;	  /* level 21 - 28 */
  else if (level < LVL_IMMORT)
    return 6;	  /* all remaining mortal levels */
  else
    return 20;	  /* immortals */
}

/* invalid_class is used by handler.c to determine if a piece of equipment is
 * usable by a particular class, based on the ITEM_ANTI_{class} bitvectors. */
int invalid_class(struct char_data *ch, struct obj_data *obj)
{
  if (OBJ_FLAGGED(obj, ITEM_ANTI_BIOTIC) && IS_BIOTIC(ch))
    return TRUE;

  if (OBJ_FLAGGED(obj, ITEM_ANTI_MEDIC) && IS_MEDIC(ch))
    return TRUE;

  if (OBJ_FLAGGED(obj, ITEM_ANTI_SOLDIER) && IS_SOLDIER(ch))
    return TRUE;

  if (OBJ_FLAGGED(obj, ITEM_ANTI_BANDIT) && IS_BANDIT(ch))
    return TRUE;

  return FALSE;
}

/* SPELLS AND SKILLS.  This area defines which spells are assigned to which
 * classes, and the minimum level the character must be to use the spell or
 * skill. */
void init_spell_levels(void)
{
  /* BIOTICS */
  spell_level(SPELL_MAGIC_MISSILE, CLASS_BIOTIC, 1);
  spell_level(SPELL_DETECT_INVIS, CLASS_BIOTIC, 2);
  spell_level(SPELL_DETECT_MAGIC, CLASS_BIOTIC, 2);
  spell_level(SPELL_CHILL_TOUCH, CLASS_BIOTIC, 3);
  spell_level(SPELL_INFRAVISION, CLASS_BIOTIC, 3);
  spell_level(SPELL_INVISIBLE, CLASS_BIOTIC, 4);
  spell_level(SPELL_ARMOR, CLASS_BIOTIC, 4);
  spell_level(SPELL_BURNING_HANDS, CLASS_BIOTIC, 5);
  spell_level(SPELL_LOCATE_OBJECT, CLASS_BIOTIC, 6);
  spell_level(SPELL_STRENGTH, CLASS_BIOTIC, 6);
  spell_level(SPELL_SHOCKING_GRASP, CLASS_BIOTIC, 7);
  spell_level(SPELL_SLEEP, CLASS_BIOTIC, 8);
  spell_level(SPELL_LIGHTNING_BOLT, CLASS_BIOTIC, 9);
  spell_level(SPELL_BLINDNESS, CLASS_BIOTIC, 9);
  spell_level(SPELL_DETECT_POISON, CLASS_BIOTIC, 10);
  spell_level(SPELL_COLOR_SPRAY, CLASS_BIOTIC, 11);
  spell_level(SPELL_ENERGY_DRAIN, CLASS_BIOTIC, 13);
  spell_level(SPELL_CURSE, CLASS_BIOTIC, 14);
  spell_level(SPELL_POISON, CLASS_BIOTIC, 14);
  spell_level(SPELL_FIREBALL, CLASS_BIOTIC, 15);
  spell_level(SPELL_CHARM, CLASS_BIOTIC, 16);
  spell_level(SPELL_IDENTIFY, CLASS_BIOTIC, 20);
  spell_level(SPELL_FLY, CLASS_BIOTIC, 22);
  spell_level(SPELL_ENCHANT_WEAPON, CLASS_BIOTIC, 26);
  spell_level(SPELL_CLONE, CLASS_BIOTIC, 30);

  /* MEDICS */
  spell_level(SPELL_CURE_LIGHT, CLASS_MEDIC, 1);
  spell_level(SPELL_ARMOR, CLASS_MEDIC, 1);
  spell_level(SPELL_CREATE_FOOD, CLASS_MEDIC, 2);
  spell_level(SPELL_CREATE_WATER, CLASS_MEDIC, 2);
  spell_level(SPELL_DETECT_POISON, CLASS_MEDIC, 3);
  spell_level(SPELL_DETECT_ALIGN, CLASS_MEDIC, 4);
  spell_level(SPELL_CURE_BLIND, CLASS_MEDIC, 4);
  spell_level(SPELL_BLESS, CLASS_MEDIC, 5);
  spell_level(SPELL_DETECT_INVIS, CLASS_MEDIC, 6);
  spell_level(SPELL_BLINDNESS, CLASS_MEDIC, 6);
  spell_level(SPELL_INFRAVISION, CLASS_MEDIC, 7);
  spell_level(SPELL_PROT_FROM_EVIL, CLASS_MEDIC, 8);
  spell_level(SPELL_POISON, CLASS_MEDIC, 8);
  spell_level(SPELL_GROUP_ARMOR, CLASS_MEDIC, 9);
  spell_level(SPELL_CURE_CRITIC, CLASS_MEDIC, 9);
  spell_level(SPELL_SUMMON, CLASS_MEDIC, 10);
  spell_level(SPELL_REMOVE_POISON, CLASS_MEDIC, 10);
  spell_level(SPELL_IDENTIFY, CLASS_MEDIC, 11);
  spell_level(SPELL_WORD_OF_RECALL, CLASS_MEDIC, 12);
  spell_level(SPELL_DARKNESS, CLASS_MEDIC, 12);
  spell_level(SPELL_EARTHQUAKE, CLASS_MEDIC, 12);
  spell_level(SPELL_DISPEL_EVIL, CLASS_MEDIC, 14);
  spell_level(SPELL_DISPEL_GOOD, CLASS_MEDIC, 14);
  spell_level(SPELL_SANCTUARY, CLASS_MEDIC, 15);
  spell_level(SPELL_CALL_LIGHTNING, CLASS_MEDIC, 15);
  spell_level(SPELL_HEAL, CLASS_MEDIC, 16);
  spell_level(SPELL_CONTROL_WEATHER, CLASS_MEDIC, 17);
  spell_level(SPELL_SENSE_LIFE, CLASS_MEDIC, 18);
  spell_level(SPELL_HARM, CLASS_MEDIC, 19);
  spell_level(SPELL_GROUP_HEAL, CLASS_MEDIC, 22);
  spell_level(SPELL_REMOVE_CURSE, CLASS_MEDIC, 26);

  /* BANDITS */
  spell_level(SKILL_SNEAK, CLASS_BANDIT, 1);
  spell_level(SKILL_PICK_LOCK, CLASS_BANDIT, 2);
  spell_level(SKILL_BACKSTAB, CLASS_BANDIT, 3);
  spell_level(SKILL_STEAL, CLASS_BANDIT, 4);
  spell_level(SKILL_HIDE, CLASS_BANDIT, 5);
  spell_level(SKILL_TRACK, CLASS_BANDIT, 6);

  /* SOLDIERS */
  spell_level(SKILL_KICK, CLASS_SOLDIER, 1);
  spell_level(SKILL_RESCUE, CLASS_SOLDIER, 3);
  spell_level(SKILL_TRACK, CLASS_SOLDIER, 9);
  spell_level(SKILL_BASH, CLASS_SOLDIER, 12);
  spell_level(SKILL_WHIRLWIND, CLASS_SOLDIER, 16);
}

/* This is the exp given to implementors -- it must always be greater than the
 * exp required for immortality, plus at least 20,000 or so. */
#define EXP_MAX  4100000000

/* Function to return the exp required for each class/level */
int level_exp(int chclass, int level)
{
   // NEW LEVEL EXP CODE
	int mod = 1;
   
   /* always declare variables */
   switch (chclass) {
	
		case CLASS_BIOTIC:
		mod = 45;
			return ((level * mod)*(level * mod));
		break;

		case CLASS_MEDIC:
		mod = 44;
			return ((level * mod)*(level * mod));
		break;

		case CLASS_BANDIT:
		mod = 43;
			return ((level * mod)*(level * mod));
		break;

	case CLASS_SOLDIER:
	mod = 42;
		return ((level * mod)*(level * mod));
	break;
	}

	/*
	* The higher the mod value you use, the more exp is needed per level.
	*
	* When you change your mod values (modifiers) you want to make absolutely
	* sure they don't exceed the maximum level for your mud which you defined
	* earlier as being #define EXP_MAX 1500000000
	*
	* To check this, take the highest Immortal level (The Implementor level)
	* and multiply it by your mod. As an example, if you have 100 total levels
	* on your mod, then your highest mod could be 50. Use the following formula
	* to check yourself:
	(level * mod) * (level * mod)
	*/
	log("SYSERR: XP table error in class.c!");
	return 1234567;
}

/* Default titles of male characters. */
const char *title_male(int chclass, int level)
{
  if (level <= 0 || level > LVL_IMPL)
    return "the Man";
  if (level == LVL_IMPL)
    return "the Implementor";

  switch (chclass) {

    case CLASS_BIOTIC:
    switch (level) {
      case  1: return "the Apprentice of Magic";
      case  2: return "the Spell Student";
      case  3: return "the Scholar of Magic";
      case  4: return "the Delver in Spells";
      case  5: return "the Medium of Magic";
      case  6: return "the Scribe of Magic";
      case  7: return "the Seer";
      case  8: return "the Sage";
      case  9: return "the Illusionist";
      case 10: return "the Abjurer";
      case 11: return "the Invoker";
      case 12: return "the Enchanter";
      case 13: return "the Conjurer";
      case 14: return "the Magician";
      case 15: return "the Creator";
      case 16: return "the Savant";
      case 17: return "the Magus";
      case 18: return "the Wizard";
      case 19: return "the Warlock";
      case 20: return "the Sorcerer";
      case 21: return "the Necromancer";
      case 22: return "the Thaumaturge";
      case 23: return "the Student of the Occult";
      case 24: return "the Disciple of the Uncanny";
      case 25: return "the Minor Elemental";
      case 26: return "the Greater Elemental";
      case 27: return "the Crafter of Magics";
      case 28: return "the Shaman";
      case 29: return "the Keeper of Talismans";
      case 30: return "the Archmage";
      case 31: return "";
      case 32: return "";
      case 33: return "";
      case 34: return "";
      case 35: return "";
      case 36: return "";		
		case 37: return "";
		case 38: return "";
		case 39: return "";
		case 40: return "";
		case 41: return "";
		case 42: return "";
		case 43: return "";
		case 44: return "";
		case 45: return "";
		case 46: return "";
		case 47: return "";
		case 48: return "";
		case 49: return "";
		case 50: return "";
      case LVL_IMMORT: return "the Immortal Warlock";
      case LVL_GOD: return "the Avatar of Magic";
      case LVL_GRGOD: return "the God of Magic";
      default: return "the Mage";
    }
    break;

    case CLASS_MEDIC:
    switch (level) {
      case  1: return "the Believer";
      case  2: return "the Attendant";
      case  3: return "the Acolyte";
      case  4: return "the Novice";
      case  5: return "the Missionary";
      case  6: return "the Adept";
      case  7: return "the Deacon";
      case  8: return "the Vicar";
      case  9: return "the Priest";
      case 10: return "the Minister";
      case 11: return "the Canon";
      case 12: return "the Levite";
      case 13: return "the Curate";
      case 14: return "the Monk";
      case 15: return "the Healer";
      case 16: return "the Chaplain";
      case 17: return "the Expositor";
      case 18: return "the Bishop";
      case 19: return "the Arch Bishop";
      case 20: return "the Patriarch";
      /* no one ever thought up these titles 21-30 */
      case LVL_IMMORT: return "the Immortal Cardinal";
      case LVL_GOD: return "the Inquisitor";
      case LVL_GRGOD: return "the God of Good and Evil";
      default: return "the Cleric";
    }
    break;

    case CLASS_BANDIT:
    switch (level) {
      case  1: return "the Pilferer";
      case  2: return "the Footpad";
      case  3: return "the Filcher";
      case  4: return "the Pick-Pocket";
      case  5: return "the Sneak";
      case  6: return "the Pincher";
      case  7: return "the Cut-Purse";
      case  8: return "the Snatcher";
      case  9: return "the Sharper";
      case 10: return "the Rogue";
      case 11: return "the Robber";
      case 12: return "the Magsman";
      case 13: return "the Highwayman";
      case 14: return "the Burglar";
      case 15: return "the Thief";
      case 16: return "the Knifer";
      case 17: return "the Quick-Blade";
      case 18: return "the Killer";
      case 19: return "the Brigand";
      case 20: return "the Cut-Throat";
      /* no one ever thought up these titles 21-30 */
      case LVL_IMMORT: return "the Immortal Assassin";
      case LVL_GOD: return "the Demi God of Thieves";
      case LVL_GRGOD: return "the God of Thieves and Tradesmen";
      default: return "the Thief";
    }
    break;

    case CLASS_SOLDIER:
    switch(level) {
      case  1: return "the Swordpupil";
      case  2: return "the Recruit";
      case  3: return "the Sentry";
      case  4: return "the Fighter";
      case  5: return "the Soldier";
      case  6: return "the Warrior";
      case  7: return "the Veteran";
      case  8: return "the Swordsman";
      case  9: return "the Fencer";
      case 10: return "the Combatant";
      case 11: return "the Hero";
      case 12: return "the Myrmidon";
      case 13: return "the Swashbuckler";
      case 14: return "the Mercenary";
      case 15: return "the Swordmaster";
      case 16: return "the Lieutenant";
      case 17: return "the Champion";
      case 18: return "the Dragoon";
      case 19: return "the Cavalier";
      case 20: return "the Knight";
      /* no one ever thought up these titles 21-30 */
      case LVL_IMMORT: return "the Immortal Warlord";
      case LVL_GOD: return "the Extirpator";
      case LVL_GRGOD: return "the God of War";
      default: return "the Warrior";
    }
    break;
  }

  /* Default title for classes which do not have titles defined */
  return "the Classless";
}

/* Default titles of female characters. */
const char *title_female(int chclass, int level)
{
  if (level <= 0 || level > LVL_IMPL)
    return "the Woman";
  if (level == LVL_IMPL)
    return "the Implementress";

  switch (chclass) {

    case CLASS_BIOTIC:
    switch (level) {
      case  1: return "the Apprentice of Magic";
      case  2: return "the Spell Student";
      case  3: return "the Scholar of Magic";
      case  4: return "the Delveress in Spells";
      case  5: return "the Medium of Magic";
      case  6: return "the Scribess of Magic";
      case  7: return "the Seeress";
      case  8: return "the Sage";
      case  9: return "the Illusionist";
      case 10: return "the Abjuress";
      case 11: return "the Invoker";
      case 12: return "the Enchantress";
      case 13: return "the Conjuress";
      case 14: return "the Witch";
      case 15: return "the Creator";
      case 16: return "the Savant";
      case 17: return "the Craftess";
      case 18: return "the Wizard";
      case 19: return "the War Witch";
      case 20: return "the Sorceress";
      case 21: return "the Necromancress";
      case 22: return "the Thaumaturgess";
      case 23: return "the Student of the Occult";
      case 24: return "the Disciple of the Uncanny";
      case 25: return "the Minor Elementress";
      case 26: return "the Greater Elementress";
      case 27: return "the Crafter of Magics";
      case 28: return "Shaman";
      case 29: return "the Keeper of Talismans";
      case 30: return "Archwitch";
		case 31: return "";
      case 32: return "";
      case 33: return "";
      case 34: return "";
      case 35: return "";
      case 36: return "";		
		case 37: return "";
		case 38: return "";
		case 39: return "";
		case 40: return "";
		case 41: return "";
		case 42: return "";
		case 43: return "";
		case 44: return "";
		case 45: return "";
		case 46: return "";
		case 47: return "";
		case 48: return "";
		case 49: return "";
		case 50: return "";
      case LVL_IMMORT: return "the Immortal Enchantress";
      case LVL_GOD: return "the Empress of Magic";
      case LVL_GRGOD: return "the Goddess of Magic";
      default: return "the Witch";
    }
    break;

    case CLASS_MEDIC:
    switch (level) {
      case  1: return "the Believer";
      case  2: return "the Attendant";
      case  3: return "the Acolyte";
      case  4: return "the Novice";
      case  5: return "the Missionary";
      case  6: return "the Adept";
      case  7: return "the Deaconess";
      case  8: return "the Vicaress";
      case  9: return "the Priestess";
      case 10: return "the Lady Minister";
      case 11: return "the Canon";
      case 12: return "the Levitess";
      case 13: return "the Curess";
      case 14: return "the Nunne";
      case 15: return "the Healess";
      case 16: return "the Chaplain";
      case 17: return "the Expositress";
      case 18: return "the Bishop";
      case 19: return "the Arch Lady of the Church";
      case 20: return "the Matriarch";
      /* no one ever thought up these titles 21-30 */
      case LVL_IMMORT: return "the Immortal Priestess";
      case LVL_GOD: return "the Inquisitress";
      case LVL_GRGOD: return "the Goddess of Good and Evil";
      default: return "the Cleric";
    }
    break;

    case CLASS_BANDIT:
    switch (level) {
      case  1: return "the Pilferess";
      case  2: return "the Footpad";
      case  3: return "the Filcheress";
      case  4: return "the Pick-Pocket";
      case  5: return "the Sneak";
      case  6: return "the Pincheress";
      case  7: return "the Cut-Purse";
      case  8: return "the Snatcheress";
      case  9: return "the Sharpress";
      case 10: return "the Rogue";
      case 11: return "the Robber";
      case 12: return "the Magswoman";
      case 13: return "the Highwaywoman";
      case 14: return "the Burglaress";
      case 15: return "the Thief";
      case 16: return "the Knifer";
      case 17: return "the Quick-Blade";
      case 18: return "the Murderess";
      case 19: return "the Brigand";
      case 20: return "the Cut-Throat";
      /* no one ever thought up these titles 21-30 */
      case LVL_IMMORT: return "the Immortal Assassin";
      case LVL_GOD: return "the Demi Goddess of Thieves";
      case LVL_GRGOD: return "the Goddess of Thieves and Tradesmen";
      default: return "the Thief";
    }
    break;

    case CLASS_SOLDIER:
    switch(level) {
      case  1: return "the Swordpupil";
      case  2: return "the Recruit";
      case  3: return "the Sentress";
      case  4: return "the Fighter";
      case  5: return "the Soldier";
      case  6: return "the Warrior";
      case  7: return "the Veteran";
      case  8: return "the Swordswoman";
      case  9: return "the Fenceress";
      case 10: return "the Combatess";
      case 11: return "the Heroine";
      case 12: return "the Myrmidon";
      case 13: return "the Swashbuckleress";
      case 14: return "the Mercenaress";
      case 15: return "the Swordmistress";
      case 16: return "the Lieutenant";
      case 17: return "the Lady Champion";
      case 18: return "the Lady Dragoon";
      case 19: return "the Cavalier";
      case 20: return "the Lady Knight";
      /* no one ever thought up these titles 21-30 */
      case LVL_IMMORT: return "the Immortal Lady of War";
      case LVL_GOD: return "the Queen of Destruction";
      case LVL_GRGOD: return "the Goddess of War";
      default: return "the Warrior";
    }
    break;
  }

  /* Default title for classes which do not have titles defined */
  return "the Classless";
}

