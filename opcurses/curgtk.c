/*
  curgtk.c - op-curses
  Copyright 1996,2001 W. Sheldon Simms III

  This file is part of op-curses, a portable curses implementation.

  op-curses is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2, or (at your option)
  any later version.

  op-curses is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
 */

#include <gtk/gtk.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#include "xcurses.h"

#define TILE_WIDTH	8
#define TILE_HEIGHT	12

#define WINDOW_WIDTH (window_cols * char_max_width)
#define WINDOW_HEIGHT (window_lines * char_max_height)

#define RESET_WINDOW_RECT() \
  window_rect.x      = WINDOW_WIDTH; \
  window_rect.y      = WINDOW_HEIGHT; \
  window_rect.width  = 0; \
  window_rect.height = 0;

#define SET_WINDOW_RECT() \
  window_rect.x      = 0; \
  window_rect.y      = 0; \
  window_rect.width  = WINDOW_WIDTH; \
  window_rect.height = WINDOW_HEIGHT;

bool local_ticks_per_second = 100;

typedef struct
{
  short fore;
  short back;
}
color_pair_t;

#define NUM_COLOR_PAIRS 256
static color_pair_t pair_table[NUM_COLOR_PAIRS];

static int num_colors;
static GdkColor * color_table;

static GdkGC * cgc;
static GdkVisual * cvis;
static GdkColormap * cmap;

static int window_cols;
static int window_lines;

static int char_max_width;
static int char_max_height;

static long sleepTime;

static gint theKey;
static gint userHitKey;

static GdkRectangle window_rect = {0,0,0,0};

static GdkPixmap * win_pixmap;
static GdkPixmap * charset_pixmap;

static GtkWidget * curses_window;
static GtkWidget * screen_da;

static char * tile_xpm_path = 0;
static int use_graphics = 1;

#define USE_GDK_FONT

#ifdef USE_GDK_FONT
static volatile int waiting_for_configure;
static GdkFont * gdk_font;
static char * gdk_fontspec = "-*-*-medium-r-normal--*-*-*-*-m-*-iso8859-1";
#endif

static gint expose_event (GtkWidget * widget, GdkEventExpose * event)
{
  GdkGC * gc;
  GdkWindow * win;
  GdkRectangle prect;

  gc = widget->style->fg_gc[GTK_WIDGET_STATE(widget)];
  win = widget->window;

  if (0 == event->area.width || 0 == event->area.height)
    return FALSE;

  if (event->area.x >= WINDOW_WIDTH || event->area.y >= WINDOW_HEIGHT)
    return FALSE;

  prect = event->area;
  gdk_draw_pixmap(win, gc, win_pixmap,
                  prect.x, prect.y,
                  prect.x, prect.y,
                  prect.width, prect.height);

  return FALSE;
}

static gint configure_event (GtkWidget * widget, GdkEventConfigure * event)
{
  gint width;
  gint height;
  GtkStyle * style;
  GdkWindow * win;

  if (win_pixmap) gdk_pixmap_unref(win_pixmap);

  win = widget->window;
  width = widget->allocation.width;
  height = widget->allocation.height;

  win_pixmap = gdk_pixmap_new(win, width, height, -1);
  cgc = gdk_gc_new(win_pixmap);

  style = gtk_widget_get_style(curses_window);
  gdk_draw_rectangle(win_pixmap, style->black_gc,
                     1, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

  if (tile_xpm_path && !charset_pixmap)
    {
      GdkBitmap * mask;

      charset_pixmap = gdk_pixmap_create_from_xpm(curses_window->window, &mask,
                                                  &(style->bg[GTK_STATE_NORMAL]),
                                                  tile_xpm_path);

      if (0 == charset_pixmap)
        {
          fprintf(stderr, "Unable to load the file '%s'\n", tile_xpm_path);
          exit(1);
        }
    }

#ifdef USE_GDK_FONT
  waiting_for_configure = 0;
#endif

  return TRUE;
}

void local_end_curses (void)
{
  gtk_exit(0);
}

static void quit (GtkWidget * widget, gpointer data)
{
  local_end_curses();
}

static gint key_press_event (GtkWidget * widget, GdkEventKey * event)
{
  event->keyval &= 0xFF;

  if (event->keyval < 0x80)
    {
      if (event->state & 4)
        event->keyval &= 0x1F;

      theKey = event->keyval;
      userHitKey = TRUE;
    }

  event->keyval = 0;

  return TRUE;
}

#ifdef USE_GDK_FONT
static void end_fontsel (GtkWidget * widget, gpointer data)
{
  gtk_main_quit();
}

static void apply_fontsel (GtkWidget * widget, gpointer fontsel)
{
  GdkFont * new_font;

  new_font = gtk_font_selection_dialog_get_font(GTK_FONT_SELECTION_DIALOG(fontsel));
  if (!new_font) return;
  if (gdk_font_equal(new_font, gdk_font)) return;

  gdk_font = new_font;

  /* this is really lame, but probably works most of the time */
  char_max_width = gdk_char_width(gdk_font, 'W');
  char_max_height = gdk_char_height(gdk_font, 'I');
  char_max_height = gdk_font->ascent + gdk_font->descent;

  /* resize the curses area */
  gtk_drawing_area_size(GTK_DRAWING_AREA(screen_da), WINDOW_WIDTH, WINDOW_HEIGHT);
  gtk_widget_show(GTK_WIDGET(curses_window));
  waiting_for_configure = 1;

  while (waiting_for_configure)
    {
      while (gtk_events_pending())
        gtk_main_iteration();
    }

  /* redraw curses stuff */
  touchwin(curscr);
  wrefresh(curscr);
}

static void ok_fontsel (GtkWidget * widget, gpointer fontsel)
{
  apply_fontsel(widget, fontsel);
  gtk_main_quit();
}

static void set_font (GtkWidget * widget, gpointer data)
{
  GtkWidget * fontsel;
  gchar * spacings[] = { "c", "m", 0 };

  fontsel = gtk_font_selection_dialog_new("Select a new Font");
  gtk_font_selection_dialog_set_font_name(GTK_FONT_SELECTION_DIALOG(fontsel), gdk_fontspec);
  gtk_font_selection_dialog_set_filter(GTK_FONT_SELECTION_DIALOG(fontsel),
                                       GTK_FONT_FILTER_BASE, GTK_FONT_ALL, 0, 0, 0, 0, spacings, 0);
  gtk_window_set_modal(GTK_WINDOW(fontsel), 1);

  gtk_signal_connect(GTK_OBJECT(GTK_FONT_SELECTION_DIALOG(fontsel)->ok_button),
                     "clicked", (GtkSignalFunc)ok_fontsel, fontsel);
  gtk_signal_connect(GTK_OBJECT(GTK_FONT_SELECTION_DIALOG(fontsel)->apply_button),
                     "clicked", (GtkSignalFunc)apply_fontsel, fontsel);
  gtk_signal_connect(GTK_OBJECT(GTK_FONT_SELECTION_DIALOG(fontsel)->cancel_button),
                     "clicked", (GtkSignalFunc)end_fontsel, 0);
  gtk_signal_connect(GTK_OBJECT(fontsel), "destroy", (GtkSignalFunc)end_fontsel, 0);

  gtk_widget_show_all(GTK_WIDGET(fontsel));

  gtk_main();

  gtk_widget_hide_all(GTK_WIDGET(fontsel));
}
#endif

void local_set_tiles (char * path)
{
  tile_xpm_path = path;
}

int local_init_curses (void)
{
  GtkWidget * vbox;

  int argc;
  char ** argv;

  argc = 1;
  argv = (char **)malloc(2 * sizeof(char *));
  argv[0] = (char *)malloc(8);
  argv[1] = 0;
  strcpy(argv[0], "program");

  gtk_init(&argc, &argv);

  sleepTime = 10;
  userHitKey = FALSE;

#ifdef USE_GDK_FONT
  gdk_font = gdk_font_load(gdk_fontspec);
  if (!gdk_font)
    {
      printf("Couldn't load an appropriate font. Using graphics");
    }
  else
    {
      use_graphics = 0;
      /* this is really lame, but probably works most of the time */
      char_max_width = gdk_char_width(gdk_font, 'W');
      char_max_height = gdk_char_height(gdk_font, 'I');
      char_max_height = gdk_font->ascent + gdk_font->descent;
    }
#endif

  if (use_graphics)
    {
      char_max_width = TILE_WIDTH;
      char_max_height = TILE_HEIGHT;
    }

  window_cols = 80;
  window_lines = 40;

  pCOLS = window_cols;
  pLINES = window_lines;

  /* create the curses window */

  curses_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_policy(GTK_WINDOW(curses_window), FALSE, FALSE, TRUE);
  gtk_widget_set_events(curses_window, GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK);

  gtk_widget_set_name(curses_window, "Omega Curses");

  vbox = gtk_vbox_new(FALSE, 0);
  gtk_container_add(GTK_CONTAINER(curses_window), vbox);
  gtk_widget_show(vbox);

  gtk_signal_connect(GTK_OBJECT(curses_window), "destroy", GTK_SIGNAL_FUNC(quit), NULL);

  /* menubar */

#ifdef USE_GDK_FONT
  {
    static GtkItemFactoryEntry items[] =
    {
      { "/_Curses", 0, 0, 0, "<Branch>" },
      { "/Curses/_Font", "<control>F", set_font, 0, 0 },
      { "/Curses/_Quit", "<control>Q", quit, 0, 0 },
    };

    GtkWidget * menubar;
    GtkItemFactory * ifact;
    GtkAccelGroup * ag;
    gint nitems = sizeof(items) / sizeof(items[0]);

    ag = gtk_accel_group_new();
    ifact = gtk_item_factory_new(GTK_TYPE_MENU_BAR, "<main>", ag);
    gtk_item_factory_create_items(ifact, nitems, items, 0);
    gtk_window_add_accel_group(GTK_WINDOW(curses_window), ag);
    menubar = gtk_item_factory_get_widget(ifact, "<main>");

    gtk_box_pack_start(GTK_BOX(vbox), menubar, FALSE, TRUE, 0);
    gtk_widget_show(menubar);
  }
#endif

  /* create the drawing area */

  screen_da = gtk_drawing_area_new();
  gtk_widget_set_events(screen_da, GDK_EXPOSURE_MASK);
  gtk_drawing_area_size(GTK_DRAWING_AREA(screen_da), WINDOW_WIDTH, WINDOW_HEIGHT);
  gtk_box_pack_start(GTK_BOX(vbox), screen_da, TRUE, TRUE, 0);
  gtk_widget_show(screen_da);

  /* signals used to handle backing pixmap */

  gtk_signal_connect(GTK_OBJECT(screen_da), "expose_event",
                     (GtkSignalFunc)expose_event, NULL);
  gtk_signal_connect(GTK_OBJECT(screen_da), "configure_event",
                     (GtkSignalFunc)configure_event, NULL);

  /* keyboard signals */

  gtk_signal_connect(GTK_OBJECT(curses_window), "key_press_event",
                     (GtkSignalFunc)key_press_event, NULL);

  gtk_widget_show(curses_window);

  SET_WINDOW_RECT();
  gtk_widget_draw((GtkWidget *)screen_da, &window_rect);
  RESET_WINDOW_RECT();

  gtk_main_iteration();

  return TRUE;
}

int local_getchar (void)
{
  while (gtk_events_pending())
    gtk_main_iteration();

  if (userHitKey)
    {
      userHitKey = FALSE;
      return theKey;
    }

  return ERR;
}

void local_init_pair (short pair, short fore, short back)
{
  pair_table[pair].fore = fore;
  pair_table[pair].back = back;
}

void local_pair_content (short pair, short * fore, short * back)
{
  *fore = pair_table[pair].fore;
  *back = pair_table[pair].back;
}

void local_init_color (short cidx, short sr, short sg, short sb)
{
  color_table[cidx].red   = 13107L * sr / 200;
  color_table[cidx].green = 13107L * sg / 200;
  color_table[cidx].blue  = 13107L * sb / 200;

  gdk_colormap_alloc_color(cmap, color_table + cidx, TRUE, TRUE);
}

void local_color_content (short cidx, short * sr, short * sg, short * sb)
{
  *sr = color_table[cidx].red   * 200L / 13107;
  *sg = color_table[cidx].green * 200L / 13107;
  *sb = color_table[cidx].blue  * 200L / 13107;
}

static int alloc_colors (void)
{
  gboolean success[8];

  cvis = gdk_visual_get_best();
  cmap = gdk_colormap_new(cvis, TRUE);

  num_colors = cmap->size;
  if (num_colors > 256) num_colors = 256;

  color_table = malloc(num_colors * sizeof(GdkColor));
  if (0 == color_table) return 2;

  /* COLOR_BLACK */
  color_table[0].red   = 0;
  color_table[0].green = 0;
  color_table[0].blue  = 0;

  /* COLOR_RED */
  color_table[1].red   = 65535;
  color_table[1].green = 0;
  color_table[1].blue  = 0;

  /* COLOR_GREEN */
  color_table[2].red   = 0;
  color_table[2].green = 65535;
  color_table[2].blue  = 0;

  /* COLOR_YELLOW */
  color_table[3].red   = 65535;
  color_table[3].green = 65535;
  color_table[3].blue  = 0;

  /* COLOR_BLUE */
  color_table[4].red   = 0;
  color_table[4].green = 0;
  color_table[4].blue  = 65535;

  /* COLOR_MAGENTA */
  color_table[5].red   = 65535;
  color_table[5].green = 0;
  color_table[5].blue  = 65535;

  /* COLOR_CYAN */
  color_table[6].red   = 0;
  color_table[6].green = 65535;
  color_table[6].blue  = 65535;

  /* COLOR_WHITE */
  color_table[7].red   = 65535;
  color_table[7].green = 65535;
  color_table[7].blue  = 65535;

  /* allocate the base colors */
  if (gdk_colormap_alloc_colors(cmap, color_table, 8, TRUE, TRUE, success))
    {
      free(color_table);
      return 2;
    }

  return num_colors;
}

void local_start_color (bool * has_color, bool * can_change_color, int * colors, int * color_pairs)
{
  /* initialize pair 0 - always white on black */
  pair_table[0].fore = COLOR_WHITE;
  pair_table[0].back = COLOR_BLACK;

  num_colors = alloc_colors();

  if (num_colors < 16)
    {
      *has_color = FALSE;
      *can_change_color = FALSE;
      *colors = 2;
      *color_pairs = 1;
    }
  else
    {
      *has_color = TRUE;
      *can_change_color = TRUE;
      *colors = num_colors;
      *color_pairs = NUM_COLOR_PAIRS;
    }
}

void local_putchar (chtype ch, int x, int y)
{
  if (use_graphics)
    {
      gint col, line;

      col = 1 + ((char_max_width + 1) * (ch % 32));
      line = 1 + ((char_max_height + 1) * (ch / 32));

      gdk_draw_pixmap(win_pixmap,
                      curses_window->style->white_gc,
                      charset_pixmap,
                      col,
                      line,
                      char_max_width * x,
                      char_max_height * y,
                      char_max_width,
                      char_max_height);
    }
#ifdef USE_GDK_FONT
  else
    {
      int pair;
      gchar gch;
      GdkGC * use_gc;

      pair = (ch >> 16) & 0xFFFF;

      use_gc = curses_window->style->black_gc;
      if (pair)
        {
          use_gc = cgc;
          gdk_gc_set_foreground(use_gc, color_table + pair_table[pair].back);
          gdk_gc_set_background(use_gc, color_table + pair_table[pair].fore);
        }

      gdk_draw_rectangle(win_pixmap,
                         use_gc,
                         1,
                         char_max_width * x,
                         char_max_height * y,
                         char_max_width,
                         char_max_height);

      use_gc = curses_window->style->white_gc;
      if (pair)
        {
          use_gc = cgc;
          gdk_gc_set_foreground(use_gc, color_table + pair_table[pair].fore);
          gdk_gc_set_background(use_gc, color_table + pair_table[pair].back);
        }

      gch = (ch & A_CHARTEXT);
      gdk_draw_text(win_pixmap,
                    gdk_font,
                    use_gc,
                    char_max_width * x,
                    char_max_height * y + gdk_font->ascent,
                    &gch,
                    1);
    }
#endif
}

void local_update (int line, int beg, int end)
{
  GdkRectangle my_rect;

  my_rect.x = char_max_width * beg;
  my_rect.y = char_max_height * line;
  my_rect.width = (end - beg + 1) * char_max_width;
  my_rect.height = char_max_height;

  gtk_widget_draw((GtkWidget *)screen_da, &my_rect);

  while (gtk_events_pending())
    gtk_main_iteration();
}

/*
 * inputs: + a rectanglular area of the screen to scroll (top, left, height, width)
 *         + how many lines should the given area be scrolled (nlines)
 * returns: OK, if the area was scrolled, otherwise ERR
 */

bool has_local_scroll = TRUE;

int local_scroll (int top, int left, int height, int width, int nlines)
{
  GdkGC * gc;
  GtkStyle * style;

  style = curses_window->style;
  gc = style->fg_gc[GTK_WIDGET_STATE(curses_window)];

  if (nlines > 0)
    {
      gdk_draw_pixmap(win_pixmap,
                      gc,
                      win_pixmap,
                      char_max_width * left,
                      char_max_height * (top + nlines),
                      char_max_width * left,
                      char_max_height * top,
                      char_max_width * width,
                      char_max_height * (height - nlines));

      gdk_draw_rectangle(win_pixmap,
                         style->black_gc,
                         1,
                         char_max_width * left,
                         char_max_height * (top + height - nlines),
                         char_max_width * width,
                         char_max_height * nlines);
    }
  else
    {
      gdk_draw_pixmap(win_pixmap,
                      gc,
                      win_pixmap,
                      char_max_width * left,
                      char_max_height * top,
                      char_max_width * left,
                      char_max_height * (top - nlines),
                      char_max_width * width,
                      char_max_height * (height + nlines));

      gdk_draw_rectangle(win_pixmap,
                         style->black_gc,
                         1,
                         char_max_width * left,
                         char_max_height * top,
                         char_max_width * width,
                         char_max_height * -nlines);
    }

  /* force immediate screen update */
  {
    GdkRectangle my_rect;

    my_rect.x = char_max_width * left;
    my_rect.y = char_max_height * top;
    my_rect.width = char_max_width * width;
    my_rect.height = char_max_height * height;

    gtk_widget_draw((GtkWidget *)screen_da, &my_rect);

    while (gtk_events_pending())
      gtk_main_iteration();
  }

  return OK;
}

/*
 * inputs: + a rectanglular area of the screen in which one or more lines
 *           will be inserted/deleted (w_top, w_left, w_height, w_width)
 *         + the line where insertion/deletion should occur (idline)
 *         + how many lines should inserted/deleted (nlines)
 * returns: OK, if the lines were inserted/deleted, otherwise ERR
 */

bool has_local_idl = TRUE;

int local_idl (int w_top, int w_left, int w_height, int w_width, int idline, int nlines)
{
  return local_scroll(w_top + idline, w_left, w_height - idline, w_width, -nlines);
}

bool has_local_idc = FALSE;

int local_idc (int w_left, int w_width, int idcol, int id)
{
  /* don't do anything */
  return ERR;
}

unsigned long local_timer (void)
{
  struct timeval now;
  struct timezone worthless;

  unsigned long ticks;

  if (0 != gettimeofday(&now, &worthless)) return 0;

  ticks = 100ul * (unsigned long)now.tv_sec;
  ticks += ((unsigned long)now.tv_usec / 10000ul);

  return ticks;
}

void local_beep (void)
{
}

void local_clear (void)
{
  GtkStyle * style;

  style = gtk_widget_get_style(curses_window);

  gdk_draw_rectangle(win_pixmap,
                     style->black_gc,
                     1,
                     0,
                     0,
                     WINDOW_WIDTH,
                     WINDOW_HEIGHT);

  SET_WINDOW_RECT();
  gtk_widget_draw((GtkWidget *)screen_da, &window_rect);

  while (gtk_events_pending())
    gtk_main_iteration();
}
