/*
 * All the daemon and fuse functions are in here
 *
 * @(#)daemons.c	4.24 (Berkeley) 02/05/99
 *
 * Rogue: Exploring the Dungeons of Doom
 * Copyright (C) 1980-1983, 1985, 1999 Michael Toy, Ken Arnold and Glenn Wichman
 * All rights reserved.
 *
 * See the file LICENSE.TXT for full copyright and licensing information.
 */

#include <curses.h>
#include "rogue.h"

/*
 * doctor:
 *	A healing daemon that restors hit points after rest
 */
void
doctor()
{
    register int lv, ohp;

    lv = pstats.s_lvl;
    ohp = pstats.s_hpt;
    players[currplayer].quiet++;
    if (lv < 8)
    {
		if (players[currplayer].quiet + (lv << 1) > 20)
			pstats.s_hpt++;
    }
    else if (players[currplayer].quiet >= 3)
	    pstats.s_hpt += rnd(lv - 7) + 1;
    if (ISRING(LEFT, R_REGEN))
		pstats.s_hpt++;
    if (ISRING(RIGHT, R_REGEN))
		pstats.s_hpt++;
    if (ohp != pstats.s_hpt)
    {
		if (pstats.s_hpt > max_hp)
			pstats.s_hpt = max_hp;
		players[currplayer].quiet = 0;
    }
}

/*
 * Swander:
 *	Called when it is time to start rolling for wandering monsters
 */
void
swander()
{
    start_daemon(rollwand, 0, BEFORE);
}

/*
 * rollwand:
 *	Called to roll to see if a wandering monster starts up
 */
int between = 0;
void
rollwand()
{

    if (++between >= 4)
    {
	if (roll(1, 6) == 4)
	{
	    wanderer();
	    kill_daemon(rollwand);
	    fuse(swander, 0, WANDERTIME, BEFORE);
	}
	between = 0;
    }
}

/*
 * unconfuse:
 *	Release the poor player from his confusion
 */
void
unconfuse()
{
    players[currplayer].player.t_flags &= ~ISHUH;
    msg("you feel less %s now", choose_str("trippy", "confused"));
}

/*
 * unsee:
 *	Turn off the ability to see invisible
 */
void
unsee()
{
    register THING *th;

    for (th = mlist; th != NULL; th = next(th))
	if (on(*th, ISINVIS) && see_monst(th))
	    mvaddch(th->t_pos.y, th->t_pos.x, th->t_oldch);
    players[currplayer].player.t_flags &= ~CANSEE;
}

/*
 * sight:
 *	He gets his sight back
 */
void
sight()
{
    if (on(players[currplayer].player, ISBLIND))
    {
	extinguish(sight);
	players[currplayer].player.t_flags &= ~ISBLIND;
	if (!(proom->r_flags & ISGONE))
	    enter_room(&hero);
	msg(choose_str("far out!  Everything is all cosmic again",
		       "the veil of darkness lifts"));
    }
}

/*
 * nohaste:
 *	End the hasting
 */
void
nohaste()
{
    players[currplayer].player.t_flags &= ~ISHASTE;
    msg("you feel yourself slowing down");
}

/*
 * stomach:
 *	Digest the hero's food
 */
void
stomach()
{
    register int oldfood;
    int orig_hungry = players[currplayer].hungry_state;

    if (players[currplayer].food_left <= 0)
    {
		if (players[currplayer].food_left-- < -STARVETIME)
			death('s');
		/*
		 * the hero is fainting
		 */
		if (players[currplayer].no_command || rnd(5) != 0)
			return;
		players[currplayer].no_command += rnd(8) + 4;
		players[currplayer].hungry_state = 3;
		if (!terse)
			addmsg(choose_str("the munchies overpower your motor capabilities.  ",
					  "you feel too weak from lack of food.  "));
		msg(choose_str("You freak out", "You faint"));
    }
    else
    {
		oldfood = players[currplayer].food_left;
		players[currplayer].food_left -= ring_eat(LEFT) + ring_eat(RIGHT) + 1 - amulet;

		if (players[currplayer].food_left < MORETIME && oldfood >= MORETIME)
		{
			players[currplayer].hungry_state = 2;
			msg(choose_str("the munchies are interfering with your motor capabilites",
				   "you are starting to feel weak"));
		}
		else if (players[currplayer].food_left < 2 * MORETIME && oldfood >= 2 * MORETIME)
		{
			players[currplayer].hungry_state = 1;
			if (terse)
				msg(choose_str("getting the munchies", "getting hungry"));
			else
				msg(choose_str("you are getting the munchies",
					   "you are starting to get hungry"));
		}
    }
    if (players[currplayer].hungry_state != orig_hungry) { 
        players[currplayer].player.t_flags &= ~ISRUN; 
        players[currplayer].running = FALSE; 
        players[currplayer].to_death = FALSE; 
        players[currplayer].count = 0; 
    } 
}

/*
 * come_down:
 *	Take the hero down off her acid trip.
 */
void
come_down()
{
    register THING *tp;
    register bool seemonst;

    if (!on(players[currplayer].player, ISHALU))
	return;

    kill_daemon(visuals);
    players[currplayer].player.t_flags &= ~ISHALU;

    if (on(players[currplayer].player, ISBLIND))
	return;

    /*
     * undo the things
     */
    for (tp = lvl_obj; tp != NULL; tp = next(tp))
	if (cansee(tp->o_pos.y, tp->o_pos.x))
	    mvaddch(tp->o_pos.y, tp->o_pos.x, tp->o_type);

    /*
     * undo the monsters
     */
    seemonst = on(players[currplayer].player, SEEMONST);
    for (tp = mlist; tp != NULL; tp = next(tp))
    {
	move(tp->t_pos.y, tp->t_pos.x);
	if (cansee(tp->t_pos.y, tp->t_pos.x))
	    if (!on(*tp, ISINVIS) || on(players[currplayer].player, CANSEE))
		addch(tp->t_disguise);
	    else
		addch(chat(tp->t_pos.y, tp->t_pos.x));
	else if (seemonst)
	{
	    standout();
	    addch(tp->t_type);
	    standend();
	}
    }
    msg("Everything looks SO boring now.");
}

/*
 * visuals:
 *	change the characters for the player
 */
void
visuals()
{
    register THING *tp;
    register bool seemonst;

    if (!after || (players[currplayer].running && jump))
	return;
    /*
     * change the things
     */
    for (tp = lvl_obj; tp != NULL; tp = next(tp))
	if (cansee(tp->o_pos.y, tp->o_pos.x))
	    mvaddch(tp->o_pos.y, tp->o_pos.x, rnd_thing());

    /*
     * change the stairs
     */
    if (!players[currplayer].seenstairs && cansee(stairs.y, stairs.x))
		mvaddch(stairs.y, stairs.x, rnd_thing());

    /*
     * change the monsters
     */
    seemonst = on(players[currplayer].player, SEEMONST);
    for (tp = mlist; tp != NULL; tp = next(tp))
    {
	move(tp->t_pos.y, tp->t_pos.x);
	if (see_monst(tp))
	{
	    if (tp->t_type == 'X' && tp->t_disguise != 'X')
		addch(rnd_thing());
	    else
		addch(rnd(26) + 'A');
	}
	else if (seemonst)
	{
	    standout();
	    addch(rnd(26) + 'A');
	    standend();
	}
    }
}

/*
 * land:
 *	Land from a levitation potion
 */
void
land()
{
    players[currplayer].player.t_flags &= ~ISLEVIT;
    msg(choose_str("bummer!  You've hit the ground",
		   "you float gently to the ground"));
}
