/*********************************************************************\
 * File  : multi.c                                Based on CircleMUD *
 * Usage : Controls player multi levels                              *
 * By    : The Finality Development Team                             *
 *   								     *
 * Used with permission from Finality.com                            *
\*********************************************************************/


#include "conf.h"
#include "sysdep.h"
#include "structs.h"
#include "utils.h"
#include "db.h"
#include "interpreter.h"
#include "comm.h"

extern char *pc_class_types[];

/* CUSTOMIZABLE OPTIONS	*/

/* Multi system defines */
#define MULTI_BIOTIC	 (1 << 0) /* Has multied to magic user */
#define MULTI_MEDIC	 (1 << 1) /* Has multied to cleric     */
#define MULTI_BANDIT	 (1 << 2) /* Has multied to thief      */
#define MULTI_SOLDIER (1 << 3) /* Has multied to warrior    */

/* Add new defines above, adding in sequential order. 
 * ie: Next would be (1 << 4) 
 */

/* Set to -1 if you don't want a char to be in a particular room, otherwise
 * set to whatever room you wish.
 */
#define MULTI_ROOM		-1

/* The level you wish to be the multi level */
#define MULTI_LEVEL		(LVL_IMMORT - 1)

/* Stats char should start at after multi */
#define MULTI_HP		   20
#define MULTI_MANA		100
#define MULTI_MOVE		80

struct multi_classes {
  char *name;
  int flag;
} multi_class[] = {
  { "biotic"	, MULTI_BIOTIC	},
  { "medic"	, MULTI_MEDIC		},
  { "bandit"	, MULTI_BANDIT		},
  { "soldier"	, MULTI_SOLDIER		},
  /* Insert new classes here 		*/
  { "\n"	, 0			}
};

/* Multi message - modify this message to set multi message */
/*char *multi_message = 
"You have multied, and must begin anew. However, you will retain knowledge\r\n"
"of your previous skills and spells, you just will not be able to use them\r\n"
"until you reach the appropriate level.\r\n";*/

/* END CUSTOMIZABLE OPTIONS 
 *
 * Only modify code below this point if you know what you are doing. It is set
 * up however, so that if you are a newbie coder, but want a functional multi
 * system, you will not have to modify ANYTHING below here.
 *
 */

#define MULTI_NAME(i)		(multi_class[i].name)
#define MULTI_FLAG(i)		(multi_class[i].flag)

/* Will parse multi_class for proper passed argument */
int find_multi_flag(char *arg)
{
  int i;

  for (i = 0; *MULTI_NAME(i) != '\n'; i++)
    if (is_abbrev(arg, MULTI_NAME(i)))
      return i;

  return -1;
}

int okay_to_multi(struct char_data * ch, int flag)
{
  int room;

  /* Is wanted class same as current class? */
  if (flag == GET_CLASS(ch)) {
    send_to_char(ch, "You are currently a %s!\r\n", pc_class_types[(int)GET_CLASS(ch)]);
    return FALSE;
  }

  /* Has char already completed this class? */
  if (MULTI_FLAGGED(ch, MULTI_FLAG(flag))) {
    send_to_char(ch, "You can not repeat a class already completed.\r\n");
    return FALSE;
  }

  if (GET_LEVEL(ch) < MULTI_LEVEL) {
    send_to_char(ch, "You are only level %d, you must be at least level %d before you can multiclass.\r\n", GET_LEVEL(ch), MULTI_LEVEL);
    return FALSE;
  }

  /* Is the char in the appropriate room? */
  if (MULTI_ROOM >= 0) {
    room = real_room(MULTI_ROOM);
    if (room >= 0 && ch->in_room != room) {
      send_to_char(ch, "You are not in the correct room to multi!\r\n");
      return FALSE;
    }
  }

  /* Everything else is okay, return TRUE! */
  return TRUE;
}

void reset_char_stats(struct char_data * ch, int flag)
{
  GET_MAX_HIT(ch) = MULTI_HP;
  GET_MAX_MANA(ch) = MULTI_MANA;
  GET_MAX_MOVE(ch) = MULTI_MOVE;
 
  GET_HIT(ch) = GET_MAX_HIT(ch);
  GET_MOVE(ch) = GET_MAX_MOVE(ch);
  GET_MANA(ch) = GET_MAX_MANA(ch);

  GET_LEVEL(ch) = 1;
  GET_CLASS(ch) = flag;
  GET_EXP(ch) = 1;
  SET_BIT(MULTI_FLAGS(ch), MULTI_FLAG(flag));
  GET_TOT_LEVEL(ch)++;

  send_to_char(ch, 
  "You have multied, and must begin anew. However, you will retain knowledge\r\n"
  "of your previous skills and spells, you just will not be able to use them\r\n"
  "until you reach the appropriate level.\r\n");
}

ACMD(do_multi)
{
  int flag;
  char arg[MAX_INPUT_LENGTH];

  one_argument(argument, arg);

  if (!*arg) {
    send_to_char(ch, "Usage: multi <class name>\r\n");
    return;
  }

  if ((flag = find_multi_flag(arg)) < 0) {
    send_to_char(ch, "Improper class name, please try again.\r\n");
    return;
  }

  if (!okay_to_multi(ch, flag))
    return;

  reset_char_stats(ch, flag);
}
