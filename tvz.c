// vim: set sw=2 foldmethod=syntax nowrap:
/* list.c zu Aufgabe 1
 * (http://www.informatik.htw-dresden.de/%7Ebeck/C/PspCB1.html)
 *
 * -- Jan Losinski, 2008/04/14
 *  */
/* TODO modified Flag auswerten bei beenden und schliessen
 * TODO New-Button implementieren
 */


#include <gtk/gtk.h>
#include <glade/glade.h>
#include <stdio.h>
#include <stdlib.h>
#include "list.h"
#include "phon.h"


enum
{
  NAME_COLUMN,
  GIVEN_COLUMN,
  PHONE_COLUMN,
  N_COLUMNS
};
enum
{
  NAME_LISTS_COLUMN,
  N_LISTS_COLUMNS
};

GtkWidget *main_app_window;
GladeXML *xml;
GtkListStore *listsListStore;
GtkTreeSelection *listsListSel;

void addListsListElm(char *name){
  GtkTreeIter iter;
  gtk_list_store_append(listsListStore, &iter);
  gtk_list_store_set(listsListStore, &iter, NAME_LISTS_COLUMN, name,-1);
}

GtkTreeIter *getIterByFilename(char *filename){
  gchar *str_data;
  GtkTreeIter *iter;
  gboolean valid;
  
  valid = gtk_tree_model_get_iter_first ((GtkTreeModel*)listsListStore, iter);

  while (valid){
    gtk_tree_model_get ((GtkTreeModel*)listsListStore, iter,NAME_LISTS_COLUMN, &str_data,-1);

    if(strcmp(filename,str_data) == 0){
      g_free (str_data);
      return iter;
    }
    g_free (str_data);
    valid = gtk_tree_model_iter_next ((GtkTreeModel*)listsListStore, iter);
  }
  return NULL;
}

gboolean closeListByName(char *name){
  return gtk_list_store_remove(listsListStore, getIterByFilename(name));
}

int askYesNo(char *title){
  int ret = 0;
  GtkWidget *dialog = gtk_dialog_new_with_buttons (title,
                                                  (GtkWindow*)main_app_window,
                                                  GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                                  GTK_STOCK_YES,
                                                  GTK_RESPONSE_ACCEPT,
                                                  GTK_STOCK_NO,
                                                  GTK_RESPONSE_REJECT,
                                                  NULL);
  ret = (gtk_dialog_run((GtkDialog*)dialog) == GTK_RESPONSE_ACCEPT);
  gtk_widget_destroy(dialog);
  return ret;
}

void checkModify(char *filename){
  if(IsPhoneListModified(filename)){
    if(askYesNo("Liste geaendert, noch speichern?")){
      g_print ("Save\n");
      SavePhoneList(filename); 
    }
  }
}

void closeButtonClicked(GtkWidget *widget, GdkEvent *event, gpointer data){
  g_print ("Close\n");
  GtkTreeIter iter;
  char* name;
  if(gtk_tree_selection_get_selected (listsListSel, NULL, &iter)){

    gtk_tree_model_get((GtkTreeModel*)listsListStore, &iter, NAME_LISTS_COLUMN, &name, -1);
    
    //TODO modificatios Implemetieren

    RemovePhoneList(name);
    gtk_list_store_remove(listsListStore, &iter);
  }
}

void saveAllButtonClicked(GtkWidget *widget, GdkEvent *event, gpointer data){
  g_print ("SaveAll\n");
  writeAllLists();
}

void saveButtonClicked(GtkWidget *widget, GdkEvent *event, gpointer data){
  g_print ("Save\n");
  //TODO Implemetieren 
  GtkTreeIter iter;
  gtk_list_store_append(listsListStore, &iter);
  gtk_list_store_set(listsListStore, &iter, NAME_LISTS_COLUMN, "blaasd",-1);
}

void loadButtonClicked(GtkWidget *widget, GdkEvent *event, gpointer data){
  GtkWidget *fileSel;
  char *filename;
  g_print ("Load\n");


  fileSel = gtk_file_chooser_dialog_new ("Lade Telefonliste",
      (GtkWindow*)main_app_window,
      GTK_FILE_CHOOSER_ACTION_OPEN,
      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
      GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
      NULL);

  if (gtk_dialog_run (GTK_DIALOG (fileSel)) == GTK_RESPONSE_ACCEPT){
    filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (fileSel));
    if (GetPhoneList(filename) == NULL){
      if(pushPhoneList(readPhoneFile(filename), filename) == OK){
	addListsListElm(filename);
      }
    } else {
      if(askYesNo("Schon geladen, neu Laden?")){
	checkModify(filename);
	// TODO Implementieren
      }
    }
    gtk_tree_selection_select_iter(listsListSel, getIterByFilename(filename));
    g_free (filename);
  }
  gtk_widget_destroy (fileSel);
}

/* Wird vor dem beenden ausgefuehrt,
 * fragt ob gespeichert werden soll */
static gboolean delete_event( GtkWidget *widget, GdkEvent *event, gpointer data){
  if(askYesNo("noch Speichern?")){
    g_print ("SaveAll\n");
    writeAllLists();
  }
  
  return FALSE;
}

/* Beendet letztendlich den Event-Loop */
static void destroy( GtkWidget *widget, gpointer data ){
  g_print ("Und Tschuess\n");
  gtk_main_quit ();
}

void initPhonListsList(){
  GtkWidget *listsList;
  GtkTreeIter iter;
  GtkCellRenderer *renderer;
  GtkTreeViewColumn *column;
  
  listsListStore = gtk_list_store_new(N_LISTS_COLUMNS, G_TYPE_STRING); 
  listsList = gtk_tree_view_new_with_model (GTK_TREE_MODEL (listsListStore));
  renderer = gtk_cell_renderer_text_new();
  column = gtk_tree_view_column_new_with_attributes ("Phonelists",
      renderer,
      "text", NAME_LISTS_COLUMN,
      NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (listsList), column);

  listsListSel = gtk_tree_view_get_selection (GTK_TREE_VIEW (listsList));
  gtk_container_add((GtkContainer*)glade_xml_get_widget(xml, "PhoneListsScroll"), listsList);
  gtk_widget_show(listsList);
 
/*  gtk_list_store_append(listsListStore, &iter);
  gtk_list_store_set(listsListStore, &iter, NAME_LISTS_COLUMN, "blaasd",-1);
  gtk_list_store_append(listsListStore, &iter);
  gtk_list_store_set(listsListStore, &iter, NAME_LISTS_COLUMN, "blubb",-1);
  gtk_list_store_append(listsListStore, &iter);
  gtk_list_store_set(listsListStore, &iter, NAME_LISTS_COLUMN, "abc",-1);
*/
}


int main(int argc, char *argv[]) {

  gtk_init(&argc, &argv);

  /* load the interface */
  xml = glade_xml_new("UI.glade", NULL, NULL);

  /* connect the signals in the interface */
  glade_xml_signal_autoconnect(xml);

  /* start the event loop */

  main_app_window = glade_xml_get_widget(xml, "AppWindow");
  g_signal_connect (G_OBJECT (main_app_window), "delete_event", G_CALLBACK (delete_event), NULL);
  g_signal_connect (G_OBJECT (main_app_window), "destroy", G_CALLBACK (destroy), NULL);

  initPhonListsList();

  gtk_main();
  return 0;
}
