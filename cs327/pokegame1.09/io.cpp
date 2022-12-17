#include <unistd.h>
#include <ncurses.h>
#include <ctype.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <vector>
#include <cmath>

#include "io.h"
#include "poke327.h"
#include "pokemon.h"
#include "db_parse.h"

typedef struct io_message {
  /* Will print " --more-- " at end of line when another message follows. *
   * Leave 10 extra spaces for that.                                      */
  char msg[71];
  struct io_message *next;
} io_message_t;

static io_message_t *io_head, *io_tail;

void io_init_terminal(void)
{
  initscr();
  raw();
  noecho();
  curs_set(0);
  keypad(stdscr, TRUE);
  start_color();
  init_pair(COLOR_RED, COLOR_RED, COLOR_BLACK);
  init_pair(COLOR_GREEN, COLOR_GREEN, COLOR_BLACK);
  init_pair(COLOR_YELLOW, COLOR_YELLOW, COLOR_BLACK);
  init_pair(COLOR_BLUE, COLOR_BLUE, COLOR_BLACK);
  init_pair(COLOR_MAGENTA, COLOR_MAGENTA, COLOR_BLACK);
  init_pair(COLOR_CYAN, COLOR_CYAN, COLOR_BLACK);
  init_pair(COLOR_WHITE, COLOR_WHITE, COLOR_BLACK);
}

void io_reset_terminal(void)
{
  endwin();

  while (io_head) {
    io_tail = io_head;
    io_head = io_head->next;
    free(io_tail);
  }
  io_tail = NULL;
}

void io_queue_message(const char *format, ...)
{
  io_message_t *tmp;
  va_list ap;

  if (!(tmp = (io_message_t *) malloc(sizeof (*tmp)))) {
    perror("malloc");
    exit(1);
  }

  tmp->next = NULL;

  va_start(ap, format);

  vsnprintf(tmp->msg, sizeof (tmp->msg), format, ap);

  va_end(ap);

  if (!io_head) {
    io_head = io_tail = tmp;
  } else {
    io_tail->next = tmp;
    io_tail = tmp;
  }
}

static void io_print_message_queue(uint32_t y, uint32_t x)
{
  while (io_head) {
    io_tail = io_head;
    attron(COLOR_PAIR(COLOR_CYAN));
    mvprintw(y, x, "%-80s", io_head->msg);
    attroff(COLOR_PAIR(COLOR_CYAN));
    io_head = io_head->next;
    if (io_head) {
      attron(COLOR_PAIR(COLOR_CYAN));
      mvprintw(y, x + 70, "%10s", " --more-- ");
      attroff(COLOR_PAIR(COLOR_CYAN));
      refresh();
      getch();
    }
    free(io_tail);
  }
  io_tail = NULL;
}

/**************************************************************************
 * Compares trainer distances from the PC according to the rival distance *
 * map.  This gives the approximate distance that the PC must travel to   *
 * get to the trainer (doesn't account for crossing buildings).  This is  *
 * not the distance from the NPC to the PC unless the NPC is a rival.     *
 *                                                                        *
 * Not a bug.                                                             *
 **************************************************************************/
static int compare_trainer_distance(const void *v1, const void *v2)
{
  const character *const *c1 = (const character * const *) v1;
  const character *const *c2 = (const character * const *) v2;

  return (world.rival_dist[(*c1)->pos[dim_y]][(*c1)->pos[dim_x]] -
          world.rival_dist[(*c2)->pos[dim_y]][(*c2)->pos[dim_x]]);
}

static character *io_nearest_visible_trainer()
{
  character **c, *n;
  uint32_t x, y, count;

  c = (character **) malloc(world.cur_map->num_trainers * sizeof (*c));

  /* Get a linear list of trainers */
  for (count = 0, y = 1; y < MAP_Y - 1; y++) {
    for (x = 1; x < MAP_X - 1; x++) {
      if (world.cur_map->cmap[y][x] && world.cur_map->cmap[y][x] !=
          &world.pc) {
        c[count++] = world.cur_map->cmap[y][x];
      }
    }
  }

  /* Sort it by distance from PC */
  qsort(c, count, sizeof (*c), compare_trainer_distance);

  n = c[0];

  free(c);

  return n;
}

void io_display()
{
  uint32_t y, x;
  character *c;

  clear();
  for (y = 0; y < MAP_Y; y++) {
    for (x = 0; x < MAP_X; x++) {
      if (world.cur_map->cmap[y][x]) {
        mvaddch(y + 1, x, world.cur_map->cmap[y][x]->symbol);
      } else {
        switch (world.cur_map->map[y][x]) {
        case ter_boulder:
        case ter_mountain:
          attron(COLOR_PAIR(COLOR_MAGENTA));
          mvaddch(y + 1, x, '%');
          attroff(COLOR_PAIR(COLOR_MAGENTA));
          break;
        case ter_tree:
        case ter_forest:
          attron(COLOR_PAIR(COLOR_GREEN));
          mvaddch(y + 1, x, '^');
          attroff(COLOR_PAIR(COLOR_GREEN));
          break;
        case ter_path:
        case ter_exit:
          attron(COLOR_PAIR(COLOR_YELLOW));
          mvaddch(y + 1, x, '#');
          attroff(COLOR_PAIR(COLOR_YELLOW));
          break;
        case ter_mart:
          attron(COLOR_PAIR(COLOR_BLUE));
          mvaddch(y + 1, x, 'M');
          attroff(COLOR_PAIR(COLOR_BLUE));
          break;
        case ter_center:
          attron(COLOR_PAIR(COLOR_RED));
          mvaddch(y + 1, x, 'C');
          attroff(COLOR_PAIR(COLOR_RED));
          break;
        case ter_grass:
          attron(COLOR_PAIR(COLOR_GREEN));
          mvaddch(y + 1, x, ':');
          attroff(COLOR_PAIR(COLOR_GREEN));
          break;
        case ter_clearing:
          attron(COLOR_PAIR(COLOR_GREEN));
          mvaddch(y + 1, x, '.');
          attroff(COLOR_PAIR(COLOR_GREEN));
          break;
        default:
 /* Use zero as an error symbol, since it stands out somewhat, and it's *
  * not otherwise used.                                                 */
          attron(COLOR_PAIR(COLOR_CYAN));
          mvaddch(y + 1, x, '0');
          attroff(COLOR_PAIR(COLOR_CYAN)); 
       }
      }
    }
  }

  mvprintw(23, 1, "PC position is (%2d,%2d) on map %d%cx%d%c.",
           world.pc.pos[dim_x],
           world.pc.pos[dim_y],
           abs(world.cur_idx[dim_x] - (WORLD_SIZE / 2)),
           world.cur_idx[dim_x] - (WORLD_SIZE / 2) >= 0 ? 'E' : 'W',
           abs(world.cur_idx[dim_y] - (WORLD_SIZE / 2)),
           world.cur_idx[dim_y] - (WORLD_SIZE / 2) <= 0 ? 'N' : 'S');
  mvprintw(22, 1, "%d known %s.", world.cur_map->num_trainers,
           world.cur_map->num_trainers > 1 ? "trainers" : "trainer");
  mvprintw(22, 30, "Nearest visible trainer: ");
  if ((c = io_nearest_visible_trainer())) {
    attron(COLOR_PAIR(COLOR_RED));
    mvprintw(22, 55, "%c at %d %c by %d %c.",
             c->symbol,
             abs(c->pos[dim_y] - world.pc.pos[dim_y]),
             ((c->pos[dim_y] - world.pc.pos[dim_y]) <= 0 ?
              'N' : 'S'),
             abs(c->pos[dim_x] - world.pc.pos[dim_x]),
             ((c->pos[dim_x] - world.pc.pos[dim_x]) <= 0 ?
              'W' : 'E'));
    attroff(COLOR_PAIR(COLOR_RED));
  } else {
    attron(COLOR_PAIR(COLOR_BLUE));
    mvprintw(22, 55, "NONE.");
    attroff(COLOR_PAIR(COLOR_BLUE));
  }

  io_print_message_queue(0, 0);

  refresh();
}

uint32_t io_teleport_pc(pair_t dest)
{
  /* Just for fun. And debugging.  Mostly debugging. */

  do {
    dest[dim_x] = rand_range(1, MAP_X - 2);
    dest[dim_y] = rand_range(1, MAP_Y - 2);
  } while (world.cur_map->cmap[dest[dim_y]][dest[dim_x]]                  ||
           move_cost[char_pc][world.cur_map->map[dest[dim_y]]
                                                [dest[dim_x]]] == INT_MAX ||
           world.rival_dist[dest[dim_y]][dest[dim_x]] < 0);

  return 0;
}

static void io_scroll_trainer_list(char (*s)[40], uint32_t count)
{
  uint32_t offset;
  uint32_t i;

  offset = 0;

  while (1) {
    for (i = 0; i < 13; i++) {
      mvprintw(i + 6, 19, " %-40s ", s[i + offset]);
    }
    switch (getch()) {
    case KEY_UP:
      if (offset) {
        offset--;
      }
      break;
    case KEY_DOWN:
      if (offset < (count - 13)) {
        offset++;
      }
      break;
    case 27:
      return;
    }

  }
}

static void io_list_trainers_display(npc **c,
                                     uint32_t count)
{
  uint32_t i;
  char (*s)[40]; /* pointer to array of 40 char */

  s = (char (*)[40]) malloc(count * sizeof (*s));

  mvprintw(3, 19, " %-40s ", "");
  /* Borrow the first element of our array for this string: */
  snprintf(s[0], 40, "You know of %d trainers:", count);
  mvprintw(4, 19, " %-40s ", *s);
  mvprintw(5, 19, " %-40s ", "");

  for (i = 0; i < count; i++) {
    snprintf(s[i], 40, "%16s %c: %2d %s by %2d %s",
             char_type_name[c[i]->ctype],
             c[i]->symbol,
             abs(c[i]->pos[dim_y] - world.pc.pos[dim_y]),
             ((c[i]->pos[dim_y] - world.pc.pos[dim_y]) <= 0 ?
              "North" : "South"),
             abs(c[i]->pos[dim_x] - world.pc.pos[dim_x]),
             ((c[i]->pos[dim_x] - world.pc.pos[dim_x]) <= 0 ?
              "West" : "East"));
    if (count <= 13) {
      /* Handle the non-scrolling case right here. *
       * Scrolling in another function.            */
      mvprintw(i + 6, 19, " %-40s ", s[i]);
    }
  }

  if (count <= 13) {
    mvprintw(count + 6, 19, " %-40s ", "");
    mvprintw(count + 7, 19, " %-40s ", "Hit escape to continue.");
    while (getch() != 27 /* escape */)
      ;
  } else {
    mvprintw(19, 19, " %-40s ", "");
    mvprintw(20, 19, " %-40s ",
             "Arrows to scroll, escape to continue.");
    io_scroll_trainer_list(s, count);
  }

  free(s);
}

static void io_list_trainers()
{
  npc **c;
  uint32_t x, y, count;

  c = (npc **) malloc(world.cur_map->num_trainers * sizeof (*c));

  /* Get a linear list of trainers */
  for (count = 0, y = 1; y < MAP_Y - 1; y++) {
    for (x = 1; x < MAP_X - 1; x++) {
      if (world.cur_map->cmap[y][x] && world.cur_map->cmap[y][x] !=
          &world.pc) {
        c[count++] = (npc *) world.cur_map->cmap[y][x];
      }
    }
  }

  /* Sort it by distance from PC */
  qsort(c, count, sizeof (*c), compare_trainer_distance);

  /* Display it */
  io_list_trainers_display(c, count);
  free(c);

  /* And redraw the map */
  io_display();
}

void io_pokemart()
{
  // restore all supplies
  for (int i = 0; i < num_bag_items; i++) {
    world.pc.bag_items[i] = 5;
  }

  mvprintw(0, 0, "Welcome to the Pokemart.  Could I interest you in some Pokeballs?");
  refresh();
  getch();
}

void io_pokemon_center()
{
  // restore all pokemon to full health
  for (int i = 0; i < (int) world.pc.pokemon_party.size(); i++) {
    world.pc.pokemon_party[i].set_hp(world.pc.pokemon_party[i].get_max_hp());
  }

  mvprintw(0, 0, "Welcome to the Pokemon Center.  How can Nurse Joy assist you?");
  refresh();
  getch();
}

void generate_trainer_pokemon_party(character *trainer) {
  pokemon *p;
  
  int md = (abs(world.cur_idx[dim_x] - (WORLD_SIZE / 2)) +
            abs(world.cur_idx[dim_y] - (WORLD_SIZE / 2)));
  int minl, maxl;

  if (md <= 200) {
    minl = 1;
    maxl = md / 2;
  } else {
    minl = (md - 200) / 2;
    maxl = 100;
  }
  if (minl < 1) {
    minl = 1;
  }
  if (minl > 100) {
    minl = 100;
  }
  if (maxl < 1) {
    maxl = 1;
  }
  if (maxl > 100) {
    maxl = 100;
  }

  do {
    p = new pokemon(rand_range(minl, maxl));
    trainer->pokemon_party.push_back(*p);
  } while (trainer->pokemon_party.size() < 6 && 
           rand_range(1,10) <= 6);

}

void switch_pokemon(character *trainer, int pos) {
  // create new vector to pushback new party
  std::vector<pokemon> new_pokemon_party;

  for (int i = 0; i < (int) trainer->pokemon_party.size(); i++) {
    if (i == 0) {
      new_pokemon_party.push_back(trainer->pokemon_party[pos]);
    }
    else if (i == pos) {
      new_pokemon_party.push_back(trainer->pokemon_party[0]);
    }
    else {
      new_pokemon_party.push_back(trainer->pokemon_party[i]);
    }
  }

  // swap two vectors
  trainer->pokemon_party.swap(new_pokemon_party);
}

// returns 0 if move hits, 1 if move misses
int do_battle_move(pokemon *attacker, pokemon *defender, int move_index) {
  int level, power, attack, defense, random;
  float critical, stab, type;

  if (rand() % 100 >= moves[move_index].accuracy) {
    return 1;
  }

  level = attacker->get_level();
  power = moves[move_index].power;
  attack = attacker->get_atk();
  defense = defender->get_def();
  critical = (rand_range(0, 255) < species[attacker->get_pokemon_species_index()].base_stat[stat_speed] / 2) ? 1.5 : 1.0;
  random = rand_range(85, 100);
  type = 1.0;

  for (int i = 0; i < attacker->get_num_types(); i++) {
    if (attacker->get_type_id(i) == moves[move_index].type_id) {
      stab = 1.5;
    }
  }
  if (stab != 1.5) { stab = 1.0; }

  float move_damage_float = ((((2 * level) / 5 + 2) * power * (attack / defense)) / 50 + 2) * critical * random * stab * type;
  int move_damage = ((int) std::round(move_damage_float)) / 100;

  defender->set_hp(std::max(defender->get_hp() - move_damage, 0));

  return 0;
}

int check_pokemon_party_for_valid_pokemon(character *d) {
  if (d->pokemon_party[0].get_hp() == 0) {
    // find another pokemon to switch to (if there is one)
    for (int i = 0; i < (int) d->pokemon_party.size(); i++) {
      if (d->pokemon_party[i].get_hp() > 0) {
        switch_pokemon(d, i);
      }
    }

    // check if there was a valid pokemon to switch to, return 1 if there was not
    if (d->pokemon_party[0].get_hp() == 0) {
      return 1;
    }
  }

  return 0;
}

int cmp_pokemon_move_priority(int i, pokemon *pc, pokemon *npc)
{
  if ((moves[pc->get_move_index(i)].priority - moves[npc->get_move_index(0)].priority) != 0) {
    return (moves[pc->get_move_index(i)].priority - moves[npc->get_move_index(0)].priority);
  }

  return (pc->get_speed() - npc->get_speed());
}

// returns 1 if battle ends, returns 0 otherwise
int io_do_battle_move(int pc_move_choice, npc *n) {
  int move_cmp = cmp_pokemon_move_priority(pc_move_choice, &(world.pc.pokemon_party[0]), &(n->pokemon_party[0]));
  
  move(3, 0);
  clrtobot();

  if (move_cmp > 0) { // pc moves first
    // pc move
    mvprintw(3, 0, "%s used %s!", world.pc.pokemon_party[0].get_species(), world.pc.pokemon_party[0].get_move(pc_move_choice));
    getch();
    if (do_battle_move(&(world.pc.pokemon_party[0]), &(n->pokemon_party[0]), world.pc.pokemon_party[0].get_move_index(pc_move_choice))) {
      mvprintw(4, 0, "Missed!");
    } else {
      mvprintw(4, 0, "Hit!");
    }
    getch();
    mvprintw(1, 0, "Trainer's Current Pokemon: %s, hp: %d", n->pokemon_party[0].get_species(), n->pokemon_party[0].get_hp());
    clrtoeol();

    // check npc for valid pokemon remaining
    if (check_pokemon_party_for_valid_pokemon(n)) {
      return 1;
    }

    // npc move
    int rand_move = rand_range(0, n->pokemon_party[0].get_num_moves() - 1);
    move(3, 0);
    clrtobot();
    mvprintw(3, 0, "%s used %s!", n->pokemon_party[0].get_species(), n->pokemon_party[0].get_move(rand_move));
    getch();
    if (do_battle_move(&(n->pokemon_party[0]), &(world.pc.pokemon_party[0]), n->pokemon_party[0].get_move_index(rand_move))) {
      mvprintw(4, 0, "Missed!");
    } else {
      mvprintw(4, 0, "Hit!");
    }
    getch();
    mvprintw(0, 0, "Your Current Pokemon: %s, hp: %d", world.pc.pokemon_party[0].get_species(), world.pc.pokemon_party[0].get_hp());
    clrtoeol();

    // check pc for valid pokemon remaining
    if (check_pokemon_party_for_valid_pokemon(&world.pc)) {
      return 1;
    }
  } else if (move_cmp < 0) { // npc moves first
    // npc move
    int rand_move = rand_range(0, n->pokemon_party[0].get_num_moves() - 1);
    mvprintw(3, 0, "%s used %s!", n->pokemon_party[0].get_species(), n->pokemon_party[0].get_move(rand_move));
    getch();
    if (do_battle_move(&(n->pokemon_party[0]), &(world.pc.pokemon_party[0]), n->pokemon_party[0].get_move_index(rand_move))) {
      mvprintw(4, 0, "Missed!");
    } else {
      mvprintw(4, 0, "Hit!");
    }
    getch();
    mvprintw(0, 0, "Your Current Pokemon: %s, hp: %d", world.pc.pokemon_party[0].get_species(), world.pc.pokemon_party[0].get_hp());
    clrtoeol();

    // check pc for valid pokemon remaining
    if (check_pokemon_party_for_valid_pokemon(&world.pc)) {
      return 1;
    }

    // pc move
    move(3, 0);
    clrtobot();
    mvprintw(3, 0, "%s used %s!", world.pc.pokemon_party[0].get_species(), world.pc.pokemon_party[0].get_move(pc_move_choice));
    getch();
    if (do_battle_move(&(world.pc.pokemon_party[0]), &(n->pokemon_party[0]), world.pc.pokemon_party[0].get_move_index(pc_move_choice))) {
      mvprintw(4, 0, "Missed!");
    } else {
      mvprintw(4, 0, "Hit!");
    }
    getch();
    mvprintw(1, 0, "Trainer's Current Pokemon: %s, hp: %d", n->pokemon_party[0].get_species(), n->pokemon_party[0].get_hp());
    clrtoeol();

    // check npc for valid pokemon remaining
    if (check_pokemon_party_for_valid_pokemon(n)) {
      return 1;
    }
  } else { // randomly decide who moves first
    if (rand_range(0,1) == 1) { // pc moves first
      // pc move
      mvprintw(3, 0, "%s used %s!", world.pc.pokemon_party[0].get_species(), world.pc.pokemon_party[0].get_move(pc_move_choice));
      getch();
      if (do_battle_move(&(world.pc.pokemon_party[0]), &(n->pokemon_party[0]), world.pc.pokemon_party[0].get_move_index(pc_move_choice))) {
        mvprintw(4, 0, "Missed!");
      } else {
        mvprintw(4, 0, "Hit!");
      }
      getch();
      mvprintw(1, 0, "Trainer's Current Pokemon: %s, hp: %d", n->pokemon_party[0].get_species(), n->pokemon_party[0].get_hp());
      clrtoeol();

      // check npc for valid pokemon remaining
      if (check_pokemon_party_for_valid_pokemon(n)) {
        return 1;
      }

      // npc move
      int rand_move = rand_range(0, n->pokemon_party[0].get_num_moves() - 1);
      move(3, 0);
      clrtobot();
      mvprintw(3, 0, "%s used %s!", n->pokemon_party[0].get_species(), n->pokemon_party[0].get_move(rand_move));
      getch();
      if (do_battle_move(&(n->pokemon_party[0]), &(world.pc.pokemon_party[0]), n->pokemon_party[0].get_move_index(rand_move))) {
        mvprintw(4, 0, "Missed!");
      } else {
        mvprintw(4, 0, "Hit!");
      }
      getch();
      mvprintw(0, 0, "Your Current Pokemon: %s, hp: %d", world.pc.pokemon_party[0].get_species(), world.pc.pokemon_party[0].get_hp());
      clrtoeol();

      // check pc for valid pokemon remaining
      if (check_pokemon_party_for_valid_pokemon(&world.pc)) {
        return 1;
      }
    } else { // npc goes first
      // npc move
      int rand_move = rand_range(0, n->pokemon_party[0].get_num_moves() - 1);
      mvprintw(3, 0, "%s used %s!", n->pokemon_party[0].get_species(), n->pokemon_party[0].get_move(rand_move));
      getch();
      if (do_battle_move(&(n->pokemon_party[0]), &(world.pc.pokemon_party[0]), n->pokemon_party[0].get_move_index(rand_move))) {
        mvprintw(4, 0, "Missed!");
      } else {
        mvprintw(4, 0, "Hit!");
      }
      getch();
      mvprintw(0, 0, "Your Current Pokemon: %s, hp: %d", world.pc.pokemon_party[0].get_species(), world.pc.pokemon_party[0].get_hp());
      clrtoeol();

      // check pc for valid pokemon remaining
      if (check_pokemon_party_for_valid_pokemon(&world.pc)) {
        return 1;
      }

      // pc move
      move(3, 0);
      clrtobot();
      mvprintw(3, 0, "%s used %s!", world.pc.pokemon_party[0].get_species(), world.pc.pokemon_party[0].get_move(pc_move_choice));
      getch();
      if (do_battle_move(&(world.pc.pokemon_party[0]), &(n->pokemon_party[0]), world.pc.pokemon_party[0].get_move_index(pc_move_choice))) {
        mvprintw(4, 0, "Missed!");
      } else {
        mvprintw(4, 0, "Hit!");
      }
      getch();
      mvprintw(1, 0, "Trainer's Current Pokemon: %s, hp: %d", n->pokemon_party[0].get_species(), n->pokemon_party[0].get_hp());
      clrtoeol();

      // check npc for valid pokemon remaining
      if (check_pokemon_party_for_valid_pokemon(n)) {
        return 1;
      }
    }

  }

  refresh();

  return 0;
}

void io_battle(character *aggressor, character *defender)
{
  int key;
  bool is_battle_over, turn_not_consumed, go_back;

  npc *n = (npc *) ((aggressor == &world.pc) ? defender : aggressor);

  if (n->pokemon_party.empty()) {
    generate_trainer_pokemon_party(n);
  }

  is_battle_over = false;
  do {
    clear();
    mvprintw(0, 0, "Your Current Pokemon: %s, hp: %d", world.pc.pokemon_party[0].get_species(), world.pc.pokemon_party[0].get_hp());
    mvprintw(1, 0, "Trainer's Current Pokemon: %s, hp: %d", n->pokemon_party[0].get_species(), n->pokemon_party[0].get_hp());
    
    mvprintw(3, 0, "Select an option by typing a key:");
    mvprintw(4, 0, "f: Fight");
    mvprintw(5, 0, "b: Bag");
    mvprintw(6, 0, "p: Pokemon");

    switch (key = getch()) {
    case 'f':
      // TODO: fight - print pokemon moves as options (lines 1-4)
      move(4, 0);
      clrtobot();
      
      for (int i = 0; (i < 4) && (world.pc.pokemon_party[0].get_move(i) != 0); i++) {
        mvprintw(i + 4, 0, "%d: %s", i + 1, world.pc.pokemon_party[0].get_move(i));
      }

      mvprintw(10, 0, "Press 'esc' to go back");
      
      // simulate move sequence with npc move
      go_back = 0;
      turn_not_consumed = 1;
      do {
        switch (key = getch()) {
          case '1':
            if (world.pc.pokemon_party[0].get_num_moves() >= 1) {
              is_battle_over = io_do_battle_move(0, n);
              turn_not_consumed = 0;
            }
            break;
          case '2':
            if (world.pc.pokemon_party[0].get_num_moves() >= 2) {
              is_battle_over = io_do_battle_move(1, n);
              turn_not_consumed = 0;
            }
            break;
          case '3':
            if (world.pc.pokemon_party[0].get_num_moves() >= 3) {
              is_battle_over = io_do_battle_move(2, n);
              turn_not_consumed = 0;
            }
            break;
          case '4':
            if (world.pc.pokemon_party[0].get_num_moves() >= 4) {
              is_battle_over = io_do_battle_move(3, n);
              turn_not_consumed = 0;
            }
            break;
          case 27: //esc
            go_back = 1;
            break;
          default:
            // do nothing
            break;  
        }
      } while (turn_not_consumed && !go_back);
      break;
    case 'b':
      // bag - print items in bag with quantity (lines 4-6)
      move(4, 0);
      clrtobot();

      mvprintw(4, 0, "1: Potions: %d", world.pc.bag_items[item_potion]);
      mvprintw(5, 0, "2: Revives: %d", world.pc.bag_items[item_revive]);

      mvprintw(7, 0, "Press 'esc' to go back");

      turn_not_consumed = 1;
      go_back = 0;
      do {
        switch (key = getch()) {
        case '1': //potion
          if (world.pc.pokemon_party[0].get_hp() != 0                                      && 
              world.pc.pokemon_party[0].get_hp() != world.pc.pokemon_party[0].get_max_hp() &&
              world.pc.bag_items[item_potion] != 0) {
                //add hp and decrement item
                world.pc.pokemon_party[0].set_hp(std::min(world.pc.pokemon_party[0].get_max_hp(), world.pc.pokemon_party[0].get_hp() + 20));
                world.pc.bag_items[item_potion]--;

                mvprintw(0, 0, "Your Current Pokemon: %s, hp: %d", world.pc.pokemon_party[0].get_species(), world.pc.pokemon_party[0].get_hp());
                clrtoeol();
                mvprintw(10, 0, "You used a potion.");
                getch();

                turn_not_consumed = 0;
          }
          break;
        case '2': //revive
          if (world.pc.pokemon_party[0].get_hp() == 0 && 
              world.pc.bag_items[item_revive] != 0) {
                //add hp and decrement item
                world.pc.pokemon_party[0].set_hp(world.pc.pokemon_party[0].get_max_hp() / 2);
                world.pc.bag_items[item_revive]--;

                mvprintw(0, 0, "Your Current Pokemon: %s, hp: %d", world.pc.pokemon_party[0].get_species(), world.pc.pokemon_party[0].get_hp());
                clrtoeol();
                mvprintw(10, 0, "You used a revive.");
                getch();

                turn_not_consumed = 0;
          }
          break;
        case 27: //esc
          go_back = 1;
          break;
        default:
          // do nothing
          break;
        }
      } while (turn_not_consumed && !go_back);

      if (!go_back) {
        // npc move
        int rand_move = rand_range(0, n->pokemon_party[0].get_num_moves() - 1);
        move(3, 0);
        clrtobot();
        mvprintw(3, 0, "%s used %s!", n->pokemon_party[0].get_species(), n->pokemon_party[0].get_move(rand_move));
        getch();
        if (do_battle_move(&(n->pokemon_party[0]), &(world.pc.pokemon_party[0]), n->pokemon_party[0].get_move_index(rand_move))) {
          mvprintw(4, 0, "Missed!");
        } else {
          mvprintw(4, 0, "Hit!");
        }
        getch();
        mvprintw(0, 0, "Your Current Pokemon: %s, hp: %d", world.pc.pokemon_party[0].get_species(), world.pc.pokemon_party[0].get_hp());
        clrtoeol();

        // check pc for valid pokemon remaining
        is_battle_over = check_pokemon_party_for_valid_pokemon(&world.pc);
      }

      break;
    case 'p':
      // pokemon - print pokemon with hp (lines 4-9)
      move(4, 0);
      clrtobot();

      for (int i = 0; i < (int) world.pc.pokemon_party.size(); i++) {
        mvprintw(i + 4, 0, "%d: %s, hp: %d", i + 1, world.pc.pokemon_party[i].get_species(), world.pc.pokemon_party[i].get_hp());
      }

      mvprintw(10, 0, "Press 'esc' to go back");

      go_back = 0;
      turn_not_consumed = 1;
      do {
        switch (key = getch()) {
        case '2':
          if (world.pc.pokemon_party.size() >= 2 &&
              world.pc.pokemon_party[1].get_hp() > 0) {
            switch_pokemon(&world.pc, 1);
            turn_not_consumed = 0;
          }
          break;
        case '3':
          if (world.pc.pokemon_party.size() >= 3 &&
              world.pc.pokemon_party[2].get_hp() > 0) {
            switch_pokemon(&world.pc, 2);
            turn_not_consumed = 0;
          }
          break;
        case '4':
          if (world.pc.pokemon_party.size() >= 4 &&
              world.pc.pokemon_party[3].get_hp() > 0) {
            switch_pokemon(&world.pc, 3);
            turn_not_consumed = 0;
          }
          break;
        case '5':
          if (world.pc.pokemon_party.size() >= 5 &&
              world.pc.pokemon_party[4].get_hp() > 0) {
            switch_pokemon(&world.pc, 4);
            turn_not_consumed = 0;
          }
          break;
        case '6':
          if (world.pc.pokemon_party.size() >= 6 &&
              world.pc.pokemon_party[5].get_hp() > 0) {
            switch_pokemon(&world.pc, 5);
            turn_not_consumed = 0;
          }
          break;
        case 27: //esc
          go_back = 1;
          break;
        default:
          // do nothing
          break;
        }
      } while (turn_not_consumed && !go_back);

      if (!go_back) {
        // npc move
        int rand_move = rand_range(0, n->pokemon_party[0].get_num_moves() - 1);
        move(3, 0);
        clrtobot();
        mvprintw(3, 0, "%s used %s!", n->pokemon_party[0].get_species(), n->pokemon_party[0].get_move(rand_move));
        getch();
        if (do_battle_move(&(n->pokemon_party[0]), &(world.pc.pokemon_party[0]), n->pokemon_party[0].get_move_index(rand_move))) {
          mvprintw(4, 0, "Missed!");
        } else {
          mvprintw(4, 0, "Hit!");
        }
        getch();
        mvprintw(0, 0, "Your Current Pokemon: %s, hp: %d", world.pc.pokemon_party[0].get_species(), world.pc.pokemon_party[0].get_hp());
        clrtoeol();

        // check pc for valid pokemon remaining
        is_battle_over = check_pokemon_party_for_valid_pokemon(&world.pc);
      }

      break;
    default:
      // any other key - do nothing, turn not consumed
      break;
    }

    refresh();
  } while (!is_battle_over);

  clear();
  if (world.pc.pokemon_party[0].get_hp() == 0) {
    mvprintw(0, 0, "You were defeated! Go to a Pokecenter to restore your pokemon to full health.");
  } else {
    mvprintw(0, 0, "You defeated the opposing trainer!");
  }
  getch();

  n->defeated = 1;
  if (n->ctype == char_hiker || n->ctype == char_rival) {
    n->mtype = move_wander;
  }
}

uint32_t move_pc_dir(uint32_t input, pair_t dest)
{
  dest[dim_y] = world.pc.pos[dim_y];
  dest[dim_x] = world.pc.pos[dim_x];

  switch (input) {
  case 1:
  case 2:
  case 3:
    dest[dim_y]++;
    break;
  case 4:
  case 5:
  case 6:
    break;
  case 7:
  case 8:
  case 9:
    dest[dim_y]--;
    break;
  }
  switch (input) {
  case 1:
  case 4:
  case 7:
    dest[dim_x]--;
    break;
  case 2:
  case 5:
  case 8:
    break;
  case 3:
  case 6:
  case 9:
    dest[dim_x]++;
    break;
  case '>':
    if (world.cur_map->map[world.pc.pos[dim_y]][world.pc.pos[dim_x]] ==
        ter_mart) {
      io_pokemart();
    }
    if (world.cur_map->map[world.pc.pos[dim_y]][world.pc.pos[dim_x]] ==
        ter_center) {
      io_pokemon_center();
    }
    break;
  }

  if (world.cur_map->cmap[dest[dim_y]][dest[dim_x]]) {
    if (dynamic_cast<npc *>(world.cur_map->cmap[dest[dim_y]][dest[dim_x]]) &&
        ((npc *) world.cur_map->cmap[dest[dim_y]][dest[dim_x]])->defeated) {
      // Some kind of greeting here would be nice
      return 1;
    } else if (dynamic_cast<npc *>
               (world.cur_map->cmap[dest[dim_y]][dest[dim_x]])) {
      io_battle(&world.pc, world.cur_map->cmap[dest[dim_y]][dest[dim_x]]);
      // Not actually moving, so set dest back to PC position
      dest[dim_x] = world.pc.pos[dim_x];
      dest[dim_y] = world.pc.pos[dim_y];
    }
  }
  
  if (move_cost[char_pc][world.cur_map->map[dest[dim_y]][dest[dim_x]]] ==
      INT_MAX) {
    return 1;
  }

  return 0;
}

void io_teleport_world(pair_t dest)
{
  /* mvscanw documentation is unclear about return values.  I believe *
   * that the return value works the same way as scanf, but instead   *
   * of counting on that, we'll initialize x and y to out of bounds   *
   * values and accept their updates only if in range.                */
  int x = INT_MAX, y = INT_MAX;
  
  world.cur_map->cmap[world.pc.pos[dim_y]][world.pc.pos[dim_x]] = NULL;

  echo();
  curs_set(1);
  do {
    mvprintw(0, 0, "Enter x [-200, 200]:           ");
    refresh();
    mvscanw(0, 21, "%d", &x);
  } while (x < -200 || x > 200);
  do {
    mvprintw(0, 0, "Enter y [-200, 200]:          ");
    refresh();
    mvscanw(0, 21, "%d", &y);
  } while (y < -200 || y > 200);

  refresh();
  noecho();
  curs_set(0);

  x += 200;
  y += 200;

  world.cur_idx[dim_x] = x;
  world.cur_idx[dim_y] = y;

  new_map(1);
  io_teleport_pc(dest);
}

void io_handle_input(pair_t dest)
{
  uint32_t turn_not_consumed;
  int key;

  do {
    switch (key = getch()) {
    case '7':
    case 'y':
    case KEY_HOME:
      turn_not_consumed = move_pc_dir(7, dest);
      break;
    case '8':
    case 'k':
    case KEY_UP:
      turn_not_consumed = move_pc_dir(8, dest);
      break;
    case '9':
    case 'u':
    case KEY_PPAGE:
      turn_not_consumed = move_pc_dir(9, dest);
      break;
    case '6':
    case 'l':
    case KEY_RIGHT:
      turn_not_consumed = move_pc_dir(6, dest);
      break;
    case '3':
    case 'n':
    case KEY_NPAGE:
      turn_not_consumed = move_pc_dir(3, dest);
      break;
    case '2':
    case 'j':
    case KEY_DOWN:
      turn_not_consumed = move_pc_dir(2, dest);
      break;
    case '1':
    case 'b':
    case KEY_END:
      turn_not_consumed = move_pc_dir(1, dest);
      break;
    case '4':
    case 'h':
    case KEY_LEFT:
      turn_not_consumed = move_pc_dir(4, dest);
      break;
    case '5':
    case ' ':
    case '.':
    case KEY_B2:
      dest[dim_y] = world.pc.pos[dim_y];
      dest[dim_x] = world.pc.pos[dim_x];
      turn_not_consumed = 0;
      break;
    case '>':
      turn_not_consumed = move_pc_dir('>', dest);
      break;
    case 'Q':
      dest[dim_y] = world.pc.pos[dim_y];
      dest[dim_x] = world.pc.pos[dim_x];
      world.quit = 1;
      turn_not_consumed = 0;
      break;
      break;
    case 't':
      io_list_trainers();
      turn_not_consumed = 1;
      break;
    case 'p':
      /* Teleport the PC to a random place in the map.               */
      io_teleport_pc(dest);
      turn_not_consumed = 0;
      break;
    case 'm':
      io_list_trainers();
      turn_not_consumed = 1;
      break;
    case 'f':
      /* Fly to any map in the world.                                */
      io_teleport_world(dest);
      turn_not_consumed = 0;
      break;
    case 'B':
      // bag - print items in bag with quantity (lines 4-6)
      clear();

      mvprintw(0, 0, "Select an option by typing a key:");

      mvprintw(1, 0, "1: Potions: %d", world.pc.bag_items[item_potion]);
      mvprintw(2, 0, "2: Revives: %d", world.pc.bag_items[item_revive]);
      mvprintw(3, 0, "3: Pokeballs: %d", world.pc.bag_items[item_pokeball]);

      mvprintw(5, 0, "Press 'esc' to go back");

      bool go_back;
      go_back = 0;
      do {
        switch (key = getch()) {
        case '1': //potion
          clear();
          mvprintw(0, 0, "Select an option by typing a key:");
          
          for (int i = 0; i < (int) world.pc.pokemon_party.size(); i++) {
            mvprintw(i + 1, 0, "%d: %s, hp: %d", i + 1, world.pc.pokemon_party[i].get_species(), world.pc.pokemon_party[i].get_hp());
          }

          do {
            switch (key = getch()) {
            case '1':
              if (world.pc.pokemon_party[0].get_hp() != 0                                      && 
                  world.pc.pokemon_party[0].get_hp() != world.pc.pokemon_party[0].get_max_hp() &&
                  world.pc.bag_items[item_potion] != 0) {
                //add hp and decrement item
                world.pc.pokemon_party[0].set_hp(std::min(world.pc.pokemon_party[0].get_max_hp(), world.pc.pokemon_party[0].get_hp() + 20));
                world.pc.bag_items[item_potion]--;

                mvprintw(10, 0, "You used a potion.");
                getch();
              }
              break;
            case '2':
              if ((int) world.pc.pokemon_party.size() > 1                                      &&
                  world.pc.pokemon_party[1].get_hp() != 0                                      && 
                  world.pc.pokemon_party[1].get_hp() != world.pc.pokemon_party[0].get_max_hp() &&
                  world.pc.bag_items[item_potion] != 0) {
                //add hp and decrement item
                world.pc.pokemon_party[1].set_hp(std::min(world.pc.pokemon_party[1].get_max_hp(), world.pc.pokemon_party[0].get_hp() + 20));
                world.pc.bag_items[item_potion]--;

                mvprintw(10, 0, "You used a potion.");
                getch();
              }
              break;
            case '3':
              if ((int) world.pc.pokemon_party.size() > 2  &&
                  world.pc.pokemon_party[2].get_hp() != 0                                      && 
                  world.pc.pokemon_party[2].get_hp() != world.pc.pokemon_party[2].get_max_hp() &&
                  world.pc.bag_items[item_potion] != 0) {
                //add hp and decrement item
                world.pc.pokemon_party[2].set_hp(std::min(world.pc.pokemon_party[2].get_max_hp(), world.pc.pokemon_party[2].get_hp() + 20));
                world.pc.bag_items[item_potion]--;

                mvprintw(10, 0, "You used a potion.");
                getch();
              }
              break;
            case '4':
              if ((int) world.pc.pokemon_party.size() > 3 &&
                  world.pc.pokemon_party[3].get_hp() != 0                                      && 
                  world.pc.pokemon_party[3].get_hp() != world.pc.pokemon_party[3].get_max_hp() &&
                  world.pc.bag_items[item_potion] != 0) {
                //add hp and decrement item
                world.pc.pokemon_party[3].set_hp(std::min(world.pc.pokemon_party[3].get_max_hp(), world.pc.pokemon_party[3].get_hp() + 20));
                world.pc.bag_items[item_potion]--;

                mvprintw(10, 0, "You used a potion.");
                getch();
              }
              break;
            case '5':
              if ((int) world.pc.pokemon_party.size() > 4 &&
                  world.pc.pokemon_party[4].get_hp() != 0                                      && 
                  world.pc.pokemon_party[4].get_hp() != world.pc.pokemon_party[4].get_max_hp() &&
                  world.pc.bag_items[item_potion] != 0) {
                //add hp and decrement item
                world.pc.pokemon_party[4].set_hp(std::min(world.pc.pokemon_party[4].get_max_hp(), world.pc.pokemon_party[4].get_hp() + 20));
                world.pc.bag_items[item_potion]--;

                mvprintw(10, 0, "You used a potion.");
                getch();
              }
              break;
            case '6':
              if ((int) world.pc.pokemon_party.size() > 5 &&
                  world.pc.pokemon_party[5].get_hp() != 0                                      && 
                  world.pc.pokemon_party[5].get_hp() != world.pc.pokemon_party[5].get_max_hp() &&
                  world.pc.bag_items[item_potion] != 0) {
                //add hp and decrement item
                world.pc.pokemon_party[5].set_hp(std::min(world.pc.pokemon_party[5].get_max_hp(), world.pc.pokemon_party[5].get_hp() + 20));
                world.pc.bag_items[item_potion]--;

                mvprintw(10, 0, "You used a potion.");
                getch();
              }
              break;
            case 27: //esc
              go_back = 1;
              break;
            default:
              // do nothing
              break;
            }
          } while (!go_back);
          turn_not_consumed = 1;

          break;
        case '2': //revive
          clear();
          mvprintw(0, 0, "Select an option by typing a key:");
          
          for (int i = 0; i < (int) world.pc.pokemon_party.size(); i++) {
            mvprintw(i + 1, 0, "%d: %s, hp: %d", i + 1, world.pc.pokemon_party[i].get_species(), world.pc.pokemon_party[i].get_hp());
          }

          mvprintw(9, 0, "Press 'esc' to go back.");

          do {
            switch (key = getch()) {
            case '1':
              if (world.pc.pokemon_party[0].get_hp() == 0 && 
                  world.pc.bag_items[item_revive] != 0) {
                //add hp and decrement item
                world.pc.pokemon_party[0].set_hp(world.pc.pokemon_party[0].get_max_hp() / 2);
                world.pc.bag_items[item_revive]--;

                mvprintw(10, 0, "You used a revive.");
                getch();
              }
              break;
            case '2':
              if (world.pc.pokemon_party.size() > 1 &&
                  world.pc.pokemon_party[0].get_hp() == 0 && 
                  world.pc.bag_items[item_revive] != 0) {
                //add hp and decrement item
                world.pc.pokemon_party[0].set_hp(world.pc.pokemon_party[0].get_max_hp() / 2);
                world.pc.bag_items[item_revive]--;

                mvprintw(10, 0, "You used a revive.");
                getch();
              }
              break;
            case '3':
              if (world.pc.pokemon_party.size() > 2 &&
                  world.pc.pokemon_party[2].get_hp() == 0 && 
                  world.pc.bag_items[item_revive] != 0) {
                //add hp and decrement item
                world.pc.pokemon_party[2].set_hp(world.pc.pokemon_party[2].get_max_hp() / 2);
                world.pc.bag_items[item_revive]--;

                mvprintw(10, 0, "You used a revive.");
                getch();
              }
              break;
            case '4':
              if (world.pc.pokemon_party.size() > 3 &&
                  world.pc.pokemon_party[3].get_hp() == 0 && 
                  world.pc.bag_items[item_revive] != 0) {
                //add hp and decrement item
                world.pc.pokemon_party[3].set_hp(world.pc.pokemon_party[3].get_max_hp() / 2);
                world.pc.bag_items[item_revive]--;

                mvprintw(10, 0, "You used a revive.");
                getch();
              }
              break;
            case '5':
              if (world.pc.pokemon_party.size() > 4 &&
                  world.pc.pokemon_party[4].get_hp() == 0 && 
                  world.pc.bag_items[item_revive] != 0) {
                //add hp and decrement item
                world.pc.pokemon_party[4].set_hp(world.pc.pokemon_party[4].get_max_hp() / 2);
                world.pc.bag_items[item_revive]--;

                mvprintw(10, 0, "You used a revive.");
                getch();
              }
              break;
            case '6':
              if (world.pc.pokemon_party.size() > 5 &&
                  world.pc.pokemon_party[5].get_hp() == 0 && 
                  world.pc.bag_items[item_revive] != 0) {
                //add hp and decrement item
                world.pc.pokemon_party[5].set_hp(world.pc.pokemon_party[5].get_max_hp() / 2);
                world.pc.bag_items[item_revive]--;

                mvprintw(10, 0, "You used a revive.");
                getch();
              }
              break;
            case 27: //esc
              go_back = 1;
              break;
            default:
              go_back = 1;
              break;
            }
          } while (!go_back);
          turn_not_consumed = 1;
          break;
        case 27: //esc
          go_back = 1;
          break;
        default:
          // do nothing
          break;
        }
      } while (!go_back);
      break;
    case 'q':
      /* Demonstrate use of the message queue.  You can use this for *
       * printf()-style debugging (though gdb is probably a better   *
       * option.  Not that it matters, but using this command will   *
       * waste a turn.  Set turn_not_consumed to 1 and you should be *
       * able to figure out why I did it that way.                   */
      io_queue_message("This is the first message.");
      io_queue_message("Since there are multiple messages, "
                       "you will see \"more\" prompts.");
      io_queue_message("You can use any key to advance through messages.");
      io_queue_message("Normal gameplay will not resume until the queue "
                       "is empty.");
      io_queue_message("Long lines will be truncated, not wrapped.");
      io_queue_message("io_queue_message() is variadic and handles "
                       "all printf() conversion specifiers.");
      io_queue_message("Did you see %s?", "what I did there");
      io_queue_message("When the last message is displayed, there will "
                       "be no \"more\" prompt.");
      io_queue_message("Have fun!  And happy printing!");
      io_queue_message("Oh!  And use 'Q' to quit!");

      dest[dim_y] = world.pc.pos[dim_y];
      dest[dim_x] = world.pc.pos[dim_x];
      turn_not_consumed = 0;
      break;
    default:
      /* Also not in the spec.  It's not always easy to figure out what *
       * key code corresponds with a given keystroke.  Print out any    *
       * unhandled key here.  Not only does it give a visual error      *
       * indicator, but it also gives an integer value that can be used *
       * for that key in this (or other) switch statements.  Printed in *
       * octal, with the leading zero, because ncurses.h lists codes in *
       * octal, thus allowing us to do reverse lookups.  If a key has a *
       * name defined in the header, you can use the name here, else    *
       * you can directly use the octal value.                          */
      mvprintw(0, 0, "Unbound key: %#o ", key);
      turn_not_consumed = 1;
    }
    refresh();
  } while (turn_not_consumed);
}

// returns 1 if battle ends, returns 0 otherwise
int io_do_pokemon_battle_move(int pc_move_choice, pokemon *n) {
  int move_cmp = cmp_pokemon_move_priority(pc_move_choice, &(world.pc.pokemon_party[0]), n);
  
  move(3, 0);
  clrtobot();

  if (move_cmp > 0) { // pc moves first
    // pc move
    mvprintw(3, 0, "%s used %s!", world.pc.pokemon_party[0].get_species(), world.pc.pokemon_party[0].get_move(pc_move_choice));
    getch();
    if (do_battle_move(&(world.pc.pokemon_party[0]), n, world.pc.pokemon_party[0].get_move_index(pc_move_choice))) {
      mvprintw(4, 0, "Missed!");
    } else {
      mvprintw(4, 0, "Hit!");
    }
    getch();
    mvprintw(1, 0, "Wild %s, hp: %d", n->get_species(), n->get_hp());
    clrtoeol();

    // check npc for valid pokemon remaining
    if (n->get_hp() == 0) {
      return 1;
    }

    // npc move
    int rand_move = rand_range(0, n->get_num_moves() - 1);
    move(3, 0);
    clrtobot();
    mvprintw(3, 0, "%s used %s!", n->get_species(), n->get_move(rand_move));
    getch();
    if (do_battle_move(n, &(world.pc.pokemon_party[0]), n->get_move_index(rand_move))) {
      mvprintw(4, 0, "Missed!");
    } else {
      mvprintw(4, 0, "Hit!");
    }
    getch();
    mvprintw(0, 0, "Your Current Pokemon: %s, hp: %d", world.pc.pokemon_party[0].get_species(), world.pc.pokemon_party[0].get_hp());
    clrtoeol();

    // check pc for valid pokemon remaining
    if (check_pokemon_party_for_valid_pokemon(&world.pc)) {
      return 1;
    }

  } else if (move_cmp < 0) { // npc moves first
    // npc move
    int rand_move = rand_range(0, n->get_num_moves() - 1);
    mvprintw(3, 0, "%s used %s!", n->get_species(), n->get_move(rand_move));
    getch();
    if (do_battle_move(n, &(world.pc.pokemon_party[0]), n->get_move_index(rand_move))) {
      mvprintw(4, 0, "Missed!");
    } else {
      mvprintw(4, 0, "Hit!");
    }
    getch();
    mvprintw(0, 0, "Your Current Pokemon: %s, hp: %d", world.pc.pokemon_party[0].get_species(), world.pc.pokemon_party[0].get_hp());
    clrtoeol();

    // check pc for valid pokemon remaining
    if (check_pokemon_party_for_valid_pokemon(&world.pc)) {
      return 1;
    }

    // pc move
    move(3, 0);
    clrtobot();
    mvprintw(3, 0, "%s used %s!", world.pc.pokemon_party[0].get_species(), world.pc.pokemon_party[0].get_move(pc_move_choice));
    getch();
    if (do_battle_move(&(world.pc.pokemon_party[0]), n, world.pc.pokemon_party[0].get_move_index(pc_move_choice))) {
      mvprintw(4, 0, "Missed!");
    } else {
      mvprintw(4, 0, "Hit!");
    }
    getch();
    mvprintw(1, 0, "Wild %s, hp: %d", n->get_species(), n->get_hp());
    clrtoeol();

    // check npc for valid pokemon remaining
    if (n->get_hp() == 0) {
      return 1;
    }
  } else { // randomly decide who moves first
    if (rand_range(0,1) == 1) { // pc moves first
      // pc move
      mvprintw(3, 0, "%s used %s!", world.pc.pokemon_party[0].get_species(), world.pc.pokemon_party[0].get_move(pc_move_choice));
      getch();
      if (do_battle_move(&(world.pc.pokemon_party[0]), n, world.pc.pokemon_party[0].get_move_index(pc_move_choice))) {
        mvprintw(4, 0, "Missed!");
      } else {
        mvprintw(4, 0, "Hit!");
      }
      getch();
      mvprintw(1, 0, "Wild %s, hp: %d", n->get_species(), n->get_hp());
      clrtoeol();

      // check npc for valid pokemon remaining
      if (n->get_hp() == 0) {
        return 1;
      }

      // npc move
      int rand_move = rand_range(0, n->get_num_moves() - 1);
      move(3, 0);
      clrtobot();
      mvprintw(3, 0, "%s used %s!", n->get_species(), n->get_move(rand_move));
      getch();
      if (do_battle_move(n, &(world.pc.pokemon_party[0]), n->get_move_index(rand_move))) {
        mvprintw(4, 0, "Missed!");
      } else {
        mvprintw(4, 0, "Hit!");
      }
      getch();
      mvprintw(0, 0, "Your Current Pokemon: %s, hp: %d", world.pc.pokemon_party[0].get_species(), world.pc.pokemon_party[0].get_hp());
      clrtoeol();

      // check pc for valid pokemon remaining
      if (check_pokemon_party_for_valid_pokemon(&world.pc)) {
        return 1;
      }
    } else { // npc goes first
      // npc move
      int rand_move = rand_range(0, n->get_num_moves() - 1);
      mvprintw(3, 0, "%s used %s!", n->get_species(), n->get_move(rand_move));
      getch();
      if (do_battle_move(n, &(world.pc.pokemon_party[0]), n->get_move_index(rand_move))) {
        mvprintw(4, 0, "Missed!");
      } else {
        mvprintw(4, 0, "Hit!");
      }
      getch();
      mvprintw(0, 0, "Your Current Pokemon: %s, hp: %d", world.pc.pokemon_party[0].get_species(), world.pc.pokemon_party[0].get_hp());
      clrtoeol();

      // check pc for valid pokemon remaining
      if (check_pokemon_party_for_valid_pokemon(&world.pc)) {
        return 1;
      }

      // pc move
      move(3, 0);
      clrtobot();
      mvprintw(3, 0, "%s used %s!", world.pc.pokemon_party[0].get_species(), world.pc.pokemon_party[0].get_move(pc_move_choice));
      getch();
      if (do_battle_move(&(world.pc.pokemon_party[0]), n, world.pc.pokemon_party[0].get_move_index(pc_move_choice))) {
        mvprintw(4, 0, "Missed!");
      } else {
        mvprintw(4, 0, "Hit!");
      }
      getch();
      mvprintw(1, 0, "Wild %s, hp: %d", n->get_species(), n->get_hp());
      clrtoeol();

      // check npc for valid pokemon remaining
      if (n->get_hp() == 0) {
        return 1;
      }
    }

  }

  refresh();

  return 0;
}

void io_pokemon_battle(pokemon *n)
{
  int key;
  bool is_battle_over, turn_not_consumed, go_back;
  int num_escape_attempts = 0;

  is_battle_over = false;
  do {
    clear();
    mvprintw(0, 0, "Your Current Pokemon: %s, hp: %d", world.pc.pokemon_party[0].get_species(), world.pc.pokemon_party[0].get_hp());
    mvprintw(1, 0, "Wild Pokemon: %s, hp: %d", n->get_species(), n->get_hp());
    
    mvprintw(3, 0, "Select an option by typing a key:");
    mvprintw(4, 0, "f: Fight");
    mvprintw(5, 0, "b: Bag");
    mvprintw(6, 0, "r: Run");
    mvprintw(7, 0, "p: Pokemon");

    switch (key = getch()) {
    case 'f':
      // TODO: fight - print pokemon moves as options (lines 1-4)
      move(4, 0);
      clrtobot();
      
      for (int i = 0; (i < 4) && (world.pc.pokemon_party[0].get_move(i) != 0); i++) {
        mvprintw(i + 4, 0, "%d: %s", i + 1, world.pc.pokemon_party[0].get_move(i));
      }

      mvprintw(10, 0, "Press 'esc' to go back");
      
      // simulate move sequence with npc move
      go_back = 0;
      turn_not_consumed = 1;
      do {
        switch (key = getch()) {
          case '1':
            if (world.pc.pokemon_party[0].get_num_moves() >= 1) {
              is_battle_over = io_do_pokemon_battle_move(0, n);
              turn_not_consumed = 0;
            }
            break;
          case '2':
            if (world.pc.pokemon_party[0].get_num_moves() >= 2) {
              is_battle_over = io_do_pokemon_battle_move(1, n);
              turn_not_consumed = 0;
            }
            break;
          case '3':
            if (world.pc.pokemon_party[0].get_num_moves() >= 3) {
              is_battle_over = io_do_pokemon_battle_move(2, n);
              turn_not_consumed = 0;
            }
            break;
          case '4':
            if (world.pc.pokemon_party[0].get_num_moves() >= 4) {
              is_battle_over = io_do_pokemon_battle_move(3, n);
              turn_not_consumed = 0;
            }
            break;
          case 27: //esc
            go_back = 1;
            break;
          default:
            // do nothing
            break;  
        }
      } while (turn_not_consumed && !go_back);
      break;
    case 'b':
      // bag - print items in bag with quantity (lines 4-6)
      move(4, 0);
      clrtobot();

      mvprintw(4, 0, "1: Potions: %d", world.pc.bag_items[item_potion]);
      mvprintw(5, 0, "2: Revives: %d", world.pc.bag_items[item_revive]);
      mvprintw(6, 0, "3: Pokeballs: %d", world.pc.bag_items[item_pokeball]);

      mvprintw(8, 0, "Press 'esc' to go back");

      turn_not_consumed = 1;
      go_back = 0;
      do {
        switch (key = getch()) {
        case '1': //potion
          if (world.pc.pokemon_party[0].get_hp() != 0                                      && 
              world.pc.pokemon_party[0].get_hp() != world.pc.pokemon_party[0].get_max_hp() &&
              world.pc.bag_items[item_potion] != 0) {
                //add hp and decrement item
                world.pc.pokemon_party[0].set_hp(std::min(world.pc.pokemon_party[0].get_max_hp(), world.pc.pokemon_party[0].get_hp() + 20));
                world.pc.bag_items[item_potion]--;

                mvprintw(0, 0, "Your Current Pokemon: %s, hp: %d", world.pc.pokemon_party[0].get_species(), world.pc.pokemon_party[0].get_hp());
                clrtoeol();
                mvprintw(10, 0, "You used a potion.");
                getch();

                turn_not_consumed = 0;
          }
          break;
        case '2': //revive
          if (world.pc.pokemon_party[0].get_hp() == 0 && 
              world.pc.bag_items[item_revive] != 0) {
                //add hp and decrement item
                world.pc.pokemon_party[0].set_hp(world.pc.pokemon_party[0].get_max_hp() / 2);
                world.pc.bag_items[item_revive]--;

                mvprintw(0, 0, "Your Current Pokemon: %s, hp: %d", world.pc.pokemon_party[0].get_species(), world.pc.pokemon_party[0].get_hp());
                clrtoeol();
                mvprintw(10, 0, "You used a revive.");
                getch();

                turn_not_consumed = 0;
          }
          break;
        case '3': //pokeball
          // capture pokemon
          if ((int) world.pc.pokemon_party.size() < 6 &&
              world.pc.bag_items[item_pokeball] > 0) {
            world.pc.bag_items[item_pokeball]--;
            world.pc.pokemon_party.push_back(*n);
            clear();
            mvprintw(0, 0, "You captured %s! %s has been added to your party.", n->get_species(), n->get_species());
            getch();
          } else {
            clear();
          }
          return;

          break;
        case 27: //esc
          go_back = 1;
          break;
        default:
          // do nothing
          break;
        }
      } while (turn_not_consumed && !go_back);

      if (!go_back) {
        // npc move
        int rand_move = rand_range(0, n->get_num_moves() - 1);
        move(3, 0);
        clrtobot();
        mvprintw(3, 0, "Wild %s used %s!", n->get_species(), n->get_move(rand_move));
        getch();
        if (do_battle_move(n, &(world.pc.pokemon_party[0]), n->get_move_index(rand_move))) {
          mvprintw(4, 0, "Missed!");
        } else {
          mvprintw(4, 0, "Hit!");
        }
        getch();
        mvprintw(0, 0, "Your Current Pokemon: %s, hp: %d", world.pc.pokemon_party[0].get_species(), world.pc.pokemon_party[0].get_hp());
        clrtoeol();

        // check pc for valid pokemon remaining
        is_battle_over = check_pokemon_party_for_valid_pokemon(&world.pc);
      }

      break;
    case 'r':
      // run - escape battle
      int odds_escape;

      num_escape_attempts++;
      odds_escape = (world.pc.pokemon_party[0].get_speed() * 32) / ((n->get_speed() / 4) % 256) + 30 * num_escape_attempts;

      if (rand() % 256 < odds_escape) {
        clear();
        mvprintw(0, 0, "You fled.");
        getch();
        return;
      } else {
        mvprintw(9, 0, "Could not escape!");
        getch();
      }
      
      break;
    case 'p':
      // pokemon - print pokemon with hp (lines 4-9)
      move(4, 0);
      clrtobot();

      for (int i = 0; i < (int) world.pc.pokemon_party.size(); i++) {
        mvprintw(i + 4, 0, "%d: %s, hp: %d", i + 1, world.pc.pokemon_party[i].get_species(), world.pc.pokemon_party[i].get_hp());
      }

      mvprintw(10, 0, "Press 'esc' to go back");

      go_back = 0;
      turn_not_consumed = 1;
      do {
        switch (key = getch()) {
        case '2':
          if (world.pc.pokemon_party.size() >= 2 &&
              world.pc.pokemon_party[1].get_hp() > 0) {
            switch_pokemon(&world.pc, 1);
            turn_not_consumed = 0;
          }
          break;
        case '3':
          if (world.pc.pokemon_party.size() >= 3 &&
              world.pc.pokemon_party[2].get_hp() > 0) {
            switch_pokemon(&world.pc, 2);
            turn_not_consumed = 0;
          }
          break;
        case '4':
          if (world.pc.pokemon_party.size() >= 4 &&
              world.pc.pokemon_party[3].get_hp() > 0) {
            switch_pokemon(&world.pc, 3);
            turn_not_consumed = 0;
          }
          break;
        case '5':
          if (world.pc.pokemon_party.size() >= 5 &&
              world.pc.pokemon_party[4].get_hp() > 0) {
            switch_pokemon(&world.pc, 4);
            turn_not_consumed = 0;
          }
          break;
        case '6':
          if (world.pc.pokemon_party.size() >= 6 &&
              world.pc.pokemon_party[5].get_hp() > 0) {
            switch_pokemon(&world.pc, 5);
            turn_not_consumed = 0;
          }
          break;
        case 27: //esc
          go_back = 1;
          break;
        default:
          // do nothing
          break;
        }
      } while (turn_not_consumed && !go_back);

      if (!go_back) {
        // npc move
        int rand_move = rand_range(0, n->get_num_moves() - 1);
        move(3, 0);
        clrtobot();
        mvprintw(3, 0, "%s used %s!", n->get_species(), n->get_move(rand_move));
        getch();
        if (do_battle_move(n, &(world.pc.pokemon_party[0]), n->get_move_index(rand_move))) {
          mvprintw(4, 0, "Missed!");
        } else {
          mvprintw(4, 0, "Hit!");
        }
        getch();
        mvprintw(0, 0, "Your Current Pokemon: %s, hp: %d", world.pc.pokemon_party[0].get_species(), world.pc.pokemon_party[0].get_hp());
        clrtoeol();

        // check pc for valid pokemon remaining
        is_battle_over = check_pokemon_party_for_valid_pokemon(&world.pc);
      }

      break;
    default:
      // any other key - do nothing, turn not consumed
      break;
    }

    refresh();
  } while (!is_battle_over);

  clear();
  if (world.pc.pokemon_party[0].get_hp() == 0) {
    mvprintw(0, 0, "You were defeated! Go to a Pokecenter to restore your pokemon to full health.");
  } else {
    mvprintw(0, 0, "Wild %s fainted.", n->get_species());
  }
  getch();
}

void io_encounter_pokemon()
{
  pokemon *p;
  int md = (abs(world.cur_idx[dim_x] - (WORLD_SIZE / 2)) +
            abs(world.cur_idx[dim_y] - (WORLD_SIZE / 2)));
  int minl, maxl;

  if (md <= 200) {
    minl = 1;
    maxl = md / 2;
  } else {
    minl = (md - 200) / 2;
    maxl = 100;
  }
  if (minl < 1) {
    minl = 1;
  }
  if (minl > 100) {
    minl = 100;
  }
  if (maxl < 1) {
    maxl = 1;
  }
  if (maxl > 100) {
    maxl = 100;
  }

  p = new pokemon(rand() % (maxl - minl + 1) + minl);

  io_queue_message("%s%s%s: HP:%d ATK:%d DEF:%d SPATK:%d SPDEF:%d SPEED:%d %s",
                   p->is_shiny() ? "*" : "", p->get_species(),
                   p->is_shiny() ? "*" : "", p->get_hp(), p->get_atk(),
                   p->get_def(), p->get_spatk(), p->get_spdef(),
                   p->get_speed(), p->get_gender_string());
  io_queue_message("%s's moves: %s %s", p->get_species(),
                   p->get_move(0), p->get_move(1));

  io_pokemon_battle(p);

  // Later on, don't delete if captured
  //delete p;
}

void io_initial_pc_pokemon_selection() {
  pokemon *p1, *p2, *p3;
  
  p1 = new pokemon(1);
  
  do {
    p2 = new pokemon(1);
  } while (!strcmp(p1->get_species(), p2->get_species()));

  do {
    p3 = new pokemon(1);
  } while (!strcmp(p1->get_species(), p3->get_species()) &&
           !strcmp(p1->get_species(), p3->get_species()));
  
  mvprintw(0, 0, "Welcome to Pokemon!");
  mvprintw(2, 0, "Select a pokemon by typing 1,2, or 3 from...");
  mvprintw(3, 0, "1: %s", p1->get_species());
  mvprintw(4, 0, "2: %s", p2->get_species());
  mvprintw(5, 0, "3: %s", p3->get_species());

  int key;
  do {
    key = getch();
    switch (key){
    case '1':
      world.pc.pokemon_party.push_back(*p1);
      break;
    case '2':
      world.pc.pokemon_party.push_back(*p2);
      break;
    case '3':
      world.pc.pokemon_party.push_back(*p3);
      break;
    default: 
      break;
    }
  } while (key != '1' && 
           key != '2' &&
           key != '3');
  
  mvprintw(7, 0, "You selected %s. %s has been added to your party.", world.pc.pokemon_party[0].get_species(), world.pc.pokemon_party[0].get_species());
  getch();
}
