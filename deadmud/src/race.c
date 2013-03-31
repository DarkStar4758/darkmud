/**************************************************************************
*  File: race.c                                           Part of tbaMUD *
*  Usage: Source file for class-specific code.                            *
*                                                                         *
*  All rights reserved.  See license for complete information.            *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
**************************************************************************/

/** Help buffer the global variable definitions */
#define __RACE_C__

/* This file attempts to concentrate most of the code which must be changed
 * in order for new race to be added.  If you're adding a new class, you
 * should go through this entire file from beginning to end and add the
 * appropriate new special cases for your new race. */

#include "conf.h"
#include "sysdep.h"
#include "structs.h"
#include "utils.h"
#include "db.h"
#include "spells.h"
#include "interpreter.h"
#include "constants.h"
#include "act.h"
#include "comm.h"

/*Names First*/
const char *race_abbrevs[] = {
        "Hu",
        "Mu",
        "Zo",
        "\n"
};

const char *pc_race_types[] = {
        "Human",
        "Mutant",
        "Zombie",
        "\n"
};
/* Race Menu in Interpreter.c*/
const char *race_menu = 
"\r\n"
"+------------------+\r\n"
"|       Race       |\r\n"
"+------------------+\r\n"
"| A) Human         |\r\n"
"| B) Mutant        |\r\n"
"| C) Zombie        |\r\n"
"+------------------|\r\n";

/* The code to interpret a race letter -- used in interpreter.c when a new
 * character is selecting a race and by 'set race' in act.wizard.c. */
int parse_race(char arg)
{
  arg = LOWER(arg);

  switch (arg) {
  case 'a': return RACE_HUMAN;
  case 'b': return RACE_MUTANT;
  case 'c': return RACE_ZOMBIE;
  default:  return RACE_UNDEFINED;
  }
}


bitvector_t find_race_bitvector(const char *arg)
{
  size_t rpos, ret = 0;

  for (rpos = 0; rpos < strlen(arg); rpos++)
    ret |= (1 << parse_race(arg[rpos]));

  return (ret);
}

/* Invalid wear flags */
int invalid_race(struct char_data *ch, struct obj_data *obj) {
  if ((OBJ_FLAGGED(obj, ITEM_ANTI_HUMAN)  && IS_HUMAN(ch)) ||
      (OBJ_FLAGGED(obj, ITEM_ANTI_MUTANT) && IS_MUTANT(ch)) ||
      (OBJ_FLAGGED(obj, ITEM_ANTI_ZOMBIE) && IS_ZOMBIE(ch)))
        return 1;
  else
        return 0;
}
