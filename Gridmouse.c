#include <gtk/gtk.h>
#include <gdk/gdkscreen.h>
#include <cairo.h>
#include <xdo.h>
#include <stdlib.h>
#include <unistd.h>
#include <gdk/gdkkeysyms.h>

/*
Install xdotool:
sudo pacman -S xdotool

Compile:
gcc `pkg-config --cflags --libs glib-2.0` `pkg-config --cflags --libs gtk+-3.0` -lxdo -o DHM_grid DHM_grid.c

Set the keyboard delay to:
xset r rate 300 25
*/

static void screen_changed(GtkWidget *widget, GdkScreen *old_screen, gpointer user_data);
static gboolean expose_draw(GtkWidget *widget, GdkEventExpose *event, gpointer userdata);
static void clicked(GtkWindow *window, GdkEventButton *event, gpointer user_data);

struct selection {
   int x;
   int y;
   int tile_long;
};

int max_tile_long;
int min_tile_long;
int tile_long;
int rows;
int columns;
int screen_width = 400;
int screen_height = 400;

int selectionX = 0;
int selectionY = 0;
GtkWidget *window;
int mouseButton;

gboolean selectionGroupMode = FALSE;
struct selection selectionsGroup[10];
int selectionsGroupSize;

static void draw_grid(cairo_t *cr)
{ 
    int i, j;
    int x = 0;
    int y = 0;

    cairo_set_source_rgba (cr, 0, 255, 0, 1);
    cairo_set_line_width(cr, 1);
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_SQUARE); 

    do {
      cairo_move_to(cr, x, 0); 
      cairo_line_to(cr, x, screen_height);
      cairo_stroke(cr);

      x = x + tile_long;
    } while (screen_width > x);

    do {
      cairo_move_to(cr, 0, y); 
      cairo_line_to(cr, screen_width, y);
      cairo_stroke(cr);

      y = y + tile_long;
    } while (screen_height > y);

    cairo_set_source_rgba (cr, 255, 0, 0, 1);
    cairo_set_line_width(cr, 2);
    cairo_rectangle(cr, selectionX, selectionY, tile_long, tile_long);
    cairo_stroke(cr);
}

void get_screen_resolution()
{ 
  GdkScreen *screen;
  gint width, height;
  
  if ((screen = gdk_screen_get_default()) != NULL) {
      width = gdk_screen_get_width(screen);
      height = gdk_screen_get_height(screen);
      printf("Width: %d, Height: %d \n", width, height);
      screen_width = width;
      screen_height = height - 3;
      //screen_height = height;
  }
}

void update_selection(int positionX, int positionY) {
  int x = 0;
  int y = 0;

  do {
    if(x == positionX){
      break;
    } else if (x > positionX){
      selectionX = x - tile_long;
      break;
    }

    x = x + tile_long;
  } while (screen_width > (x - tile_long));

  do {
    if(y == positionY){
      break;
    } else if(y > positionY){
      selectionY = y - tile_long;
      break;
    }

    y = y + tile_long;
  } while (screen_height > (y - tile_long));
}

void render_screen(GtkWidget *widget, gboolean clear_screen)
{ 
  cairo_t *cr = gdk_cairo_create(gtk_widget_get_window(widget));
  
  cairo_set_source_rgba (cr, 1.0, 1.0, 1.0, 0.0);
  cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);

  if(clear_screen){  
    cairo_paint (cr);
  }

  draw_grid(cr);
  cairo_destroy(cr);
}

gboolean on_key_press(GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{ 
  switch (event->keyval)
  {
    case GDK_KEY_Up:
      if(selectionY > 0) {
        selectionY = selectionY - tile_long;
        render_screen(widget, FALSE);
      }

      break;
    case GDK_KEY_Down:
      if(selectionY < (screen_height - tile_long)) {
        selectionY = selectionY + tile_long;
        render_screen(widget, FALSE);
      }

      break;
    case GDK_KEY_Right:
      if(selectionX < (screen_width - tile_long)) {
        selectionX = selectionX + tile_long;
        render_screen(widget, FALSE);
      }

      break;
    case GDK_KEY_Left:
      if(selectionX > 0) {
        selectionX = selectionX - tile_long;
        render_screen(widget, FALSE);
      }

      break;
    case GDK_KEY_d:
      if(tile_long < max_tile_long) {
        tile_long = tile_long * 2;
        columns = screen_width / tile_long;
        rows = screen_height / tile_long;

        update_selection(selectionX, selectionY);
        render_screen(widget, TRUE);
      }

      break;
    case GDK_KEY_f:
      if(tile_long > min_tile_long) {
        tile_long = tile_long / 2;
        columns = screen_width / tile_long;
        rows = screen_height / tile_long;
        
        render_screen(widget, TRUE);
      }

      break;
    case GDK_KEY_r:
      selectionX = (columns/2) * tile_long;
      selectionY = 0;
      render_screen(widget, TRUE);
      break;
    case GDK_KEY_e:
      selectionX = (columns/2) * tile_long;
      selectionY = rows * tile_long;
      render_screen(widget, TRUE);
      break;
    case GDK_KEY_g:
      selectionX = columns * tile_long;
      selectionY = (rows/2) * tile_long;
      render_screen(widget, TRUE);
      break;
    case GDK_KEY_s:
      selectionX = 0;
      selectionY = (rows/2) * tile_long;
      render_screen(widget, TRUE);
      break;
    case GDK_KEY_x:
    {
      xdo_t * x = xdo_new(NULL);
      int positionX = 0;
      int positionY = 0;
      int screen = 0;

      xdo_get_mouse_location(x, &positionX, &positionY, &screen);
      xdo_free(x);
      update_selection(positionX, positionY);
      render_screen(widget, TRUE);
      break;
    }
    case GDK_KEY_q:
      gtk_widget_destroy(GTK_WIDGET(widget));
      gtk_main_quit();
      
      break;
    case GDK_KEY_Return:
      mouseButton = 1;
      gtk_widget_destroy(GTK_WIDGET(widget));
      gtk_main_quit();
      break;
    case GDK_KEY_space:
      mouseButton = 3;
      gtk_widget_destroy(GTK_WIDGET(widget));
      gtk_main_quit();
      break;
    case GDK_KEY_v:
    {
      int i;

      for(i=0; i < selectionsGroupSize; i++) {
        if(selectionsGroup[i].x == NULL) {
          selectionGroupMode = TRUE;
          selectionsGroup[i].x = selectionX;
          selectionsGroup[i].y = selectionY;
          selectionsGroup[i].tile_long = tile_long;
          break;
        }
      }

      break;
    }
    default:
      return FALSE; 
  }

  return FALSE; 
}

int main(int argc, char **argv) {
    gtk_init(&argc, &argv);
    get_screen_resolution();

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(window), screen_width, screen_height);
    gtk_window_set_title(GTK_WINDOW(window), "DHM Grid");
    g_signal_connect(G_OBJECT(window), "delete-event", gtk_main_quit, NULL);
    g_signal_connect (G_OBJECT(window), "key_press_event", G_CALLBACK (on_key_press), NULL);

    gtk_widget_set_app_paintable(window, TRUE);

    g_signal_connect(G_OBJECT(window), "draw", G_CALLBACK(expose_draw), NULL);
    g_signal_connect(G_OBJECT(window), "screen-changed", G_CALLBACK(screen_changed), NULL);

    gtk_window_set_decorated(GTK_WINDOW(window), FALSE);
    gtk_widget_add_events(window, GDK_BUTTON_PRESS_MASK);
    g_signal_connect(G_OBJECT(window), "button-press-event", G_CALLBACK(clicked), NULL);

    GtkWidget* fixed_container = gtk_fixed_new();
    gtk_container_add(GTK_CONTAINER(window), fixed_container);
    
    screen_changed(window, NULL, NULL);

    gtk_widget_show_all(window);
    gtk_main();
    
    xdo_t * x = xdo_new(NULL);

    if(selectionGroupMode && (mouseButton == 1)) {
      int i;

      for(i=0; i < selectionsGroupSize; i++) {
        if(selectionsGroup[i].x == NULL) {
          break;
        } else {
          printf("%d\n", selectionsGroup[i].y);
          xdo_move_mouse(x, selectionsGroup[i].x + (selectionsGroup[i].tile_long/2), selectionsGroup[i].y + (selectionsGroup[i].tile_long/2), 0);
          xdo_click_window(x, CURRENTWINDOW, 1);
          usleep(100000);
        }
      }        
    } else{
      xdo_move_mouse(x, selectionX + (tile_long/2), selectionY + (tile_long/2), 0);
      xdo_click_window(x, CURRENTWINDOW, mouseButton);
    }
    
    int xi = 0;
    int yi = 0;
    int zi = 0;
    xdo_get_mouse_location(x, &xi, &yi, &zi);
    
    printf(": %d %d %d\n", xi, yi, tile_long);
    xdo_free(x);

    return 0;
}

gboolean supports_alpha = FALSE;
static void screen_changed(GtkWidget *widget, GdkScreen *old_screen, gpointer userdata) {
    GdkScreen *screen = gtk_widget_get_screen(widget);
    GdkVisual *visual = gdk_screen_get_rgba_visual(screen);

    if (!visual) {
      printf("Your screen does not support alpha channels!\n");
      visual = gdk_screen_get_system_visual(screen);
      supports_alpha = FALSE;
    } else {
      printf("Your screen supports alpha channels!\n");
      supports_alpha = TRUE;
    }

      gtk_widget_set_visual(widget, visual);
}

static gboolean expose_draw(GtkWidget *widget, GdkEventExpose *event, gpointer userdata) {
    cairo_t *cr = gdk_cairo_create(gtk_widget_get_window(widget));

    cairo_set_source_rgba (cr, 1.0, 1.0, 1.0, 0.0); 
    cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
    cairo_paint (cr);

    tile_long = 24;
    max_tile_long = tile_long * 2;
    min_tile_long = tile_long / 2;
    columns = screen_width / tile_long;
    rows = screen_height / tile_long;
    selectionX = (columns/2) * tile_long;
    selectionY = (rows/2) * tile_long;
    selectionsGroupSize = sizeof(selectionsGroup) / sizeof(int);

    draw_grid(cr);
    cairo_destroy(cr);
    return FALSE;
}

static void clicked(GtkWindow *window, GdkEventButton *event, gpointer user_data) {
    /* toggle window manager frames */
    gtk_window_set_decorated(window, !gtk_window_get_decorated(window));
}
