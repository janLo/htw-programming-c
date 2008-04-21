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
#include <string.h>
#include "list.h"
#include "phon.h"


enum {
  PHONE_NAME_COLUMN,
  PHONE_GIVEN_COLUMN,
  PHONE_COLUMN,
  PHONE_N_COLUMNS
};

enum {
  NAME_LISTS_COLUMN,
  N_LISTS_COLUMNS
};

enum {
  E_DEL,
  E_MOD,
  E_NEW
};


typedef struct phoneListStore{
  GtkListStore * store;
  GtkTreeIter * selectedIter;
  char* name;
} tPhoneListStore;

GtkWidget *main_app_window;
GladeXML *xml;
GtkListStore *listsListStore;
GtkListStore *phoneDefaultStore;
GtkTreeSelection *listsListSel;
GtkTreeSelection *phoneListSel;
tList *phoneModelLists = NULL;
GtkWidget *phoneView;
GtkWidget *entryDialog;

void fillPhoneModel(GtkListStore *store, tList *list){
  GtkTreeIter iter;
  tDataEntry * entry;

  if ((entry = GetFirst(list)) != NULL){
    do {
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 
	  PHONE_NAME_COLUMN, entry->name,
	  PHONE_GIVEN_COLUMN, entry->given,
	  PHONE_COLUMN, entry->phone,
	  -1);
    } while ((entry = GetNext(list)) != NULL);
  }
}

void refreshPhoneModel(GtkListStore *store, tList *list){
  gtk_list_store_clear(store);
  fillPhoneModel(store, list);
}

void addPhoneModelList(char *name){
  tList *list = GetPhoneList(name);
  GtkListStore *store;
  char *listName;
  tPhoneListStore * newList;
  
  if (phoneModelLists == NULL){
    phoneModelLists = CreateList();
  }
  store = gtk_list_store_new(PHONE_N_COLUMNS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
  
  listName = malloc(sizeof(char)*strlen(name)+2);
  strcpy(listName, name);

  newList = malloc(sizeof(tPhoneListStore));
  newList->store = store;
  newList->name = listName;
  newList->selectedIter = NULL;

  InsertTail(phoneModelLists, newList);
  fillPhoneModel(store, list);

}

void modifyPhoneList(int type){

  GtkEntry *nameField, *givenField, *phoneField;
  GtkTreeIter iter, listsIter;
  GtkTreeModel *model;
  char *name, *given, *phone, *listName, *labelString, *tmp;
  int oldNum, strSize;
  tList *list = NULL;
  GtkWidget *label, *dialog;

  if(gtk_tree_selection_get_selected (listsListSel, NULL, &listsIter)){
    if(gtk_list_store_iter_is_valid(listsListStore, &listsIter)){
      gtk_tree_model_get((GtkTreeModel*)listsListStore, &listsIter, NAME_LISTS_COLUMN, &listName, -1);
      list = GetPhoneList(listName);
      if (type == E_MOD || type == E_DEL){
	if( ! gtk_tree_selection_get_selected (phoneListSel, &model, &iter)){
	  return;
	} else {
	  if( ! gtk_list_store_iter_is_valid((GtkListStore*)model, &iter)){
	    return;
	  }
	}
	gtk_tree_model_get((GtkTreeModel*)model, &iter, 
	    PHONE_NAME_COLUMN, &name, 
	    PHONE_GIVEN_COLUMN, &given,
	    PHONE_COLUMN, &phone,
	    -1);
      }
      if (type == E_MOD || type == E_NEW){
	nameField  = (GtkEntry*)glade_xml_get_widget(xml,"nameField");
	givenField = (GtkEntry*)glade_xml_get_widget(xml,"givenField");
	phoneField = (GtkEntry*)glade_xml_get_widget(xml,"phoneField");
      }
      if (type == E_MOD){
	gtk_entry_set_text (nameField, name);
	gtk_entry_set_text (givenField, given);
	gtk_entry_set_text (phoneField, phone);
      }
      if (type == E_NEW){
	gtk_entry_set_text (nameField, "");
	gtk_entry_set_text (givenField, "");
	gtk_entry_set_text (phoneField, "");
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(phoneView));
      }

      if (type == E_MOD || type == E_NEW){
	dialog = entryDialog;
      } else {
	strSize = strlen(name) + strlen(given) + strlen(phone) + 70;
	labelString = malloc(sizeof(char) *(strSize));
	snprintf(labelString, strSize, "Den Eintrag von\n%s, %s\n(Tel: %s)\nwirklich loeschen?", name, given, phone);
	dialog = gtk_dialog_new_with_buttons ("Loeschen?",
	    (GtkWindow*)main_app_window,
	    GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
	    GTK_STOCK_YES,
	    GTK_RESPONSE_ACCEPT,
	    GTK_STOCK_NO,
	    GTK_RESPONSE_REJECT,
	    NULL);
	label = gtk_label_new (labelString);
	gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->vbox), label);
	gtk_widget_show(label);
      }

      if(GTK_RESPONSE_ACCEPT, gtk_dialog_run(GTK_DIALOG(dialog))){
	if (type == E_MOD || type == E_DEL){
	  oldNum = *((gint*)gtk_tree_path_get_indices(gtk_tree_model_get_path(model, &iter)));
	  g_print("Mod-Num: %d\n", oldNum);
	}

	if (type == E_MOD || type == E_DEL){
	  RemoveByIdx(list, oldNum);
	}
	if (type == E_MOD || type == E_NEW){
	  tmp  = (char*)gtk_entry_get_text (nameField);
	  name = malloc(sizeof(char)*strlen(tmp)+2);
	  strcpy(name, tmp);
	  tmp = (char*)gtk_entry_get_text (givenField);
	  given = malloc(sizeof(char)*strlen(tmp)+2);
	  strcpy(given, tmp);
	  tmp = (char*)gtk_entry_get_text (phoneField);
	  phone = malloc(sizeof(char)*strlen(tmp)+2);
	  strcpy(phone, tmp);


	  insertEntrySorted(list, createEntry(name, given, phone));
	}
	refreshPhoneModel(GTK_LIST_STORE(model), list);

      }
      if (type == E_MOD || type == E_NEW){
	gtk_widget_hide(entryDialog);
      } else {
	gtk_widget_destroy(dialog);
	free (labelString);
      }
    }
  }
}

tPhoneListStore * getPhoneModel(char *name){
  tPhoneListStore * walker;
  if (phoneModelLists == NULL){
    return NULL;
  }
  if ((walker = GetFirst(phoneModelLists)) != NULL){
    do {
      if (strcmp(walker->name, name) == 0){
	return walker;
      }
    } while ((walker = GetNext(phoneModelLists)) != NULL);
  }
  return NULL;
}

void removePhoneStore(char *name){
  GtkTreeIter iter1,iter2;
  tPhoneListStore * elm = getPhoneModel(name);
  gboolean valid;
  if (elm != NULL){
    RemoveItem(phoneModelLists);
    free(elm->name);
    gtk_list_store_clear (elm->store);
    free(elm);
  }

}

void addListsListElm(char *name){
  GtkTreeIter iter;
  gtk_list_store_append(listsListStore, &iter);
  gtk_list_store_set(listsListStore, &iter, NAME_LISTS_COLUMN, name,-1);
}

GtkTreeIter getIterByFilename(char *filename){
  gchar *str_data;
  GtkTreeIter iter;
  gboolean valid;
  
  valid = gtk_tree_model_get_iter_first ((GtkTreeModel*)listsListStore, &iter);

  while (valid){
    gtk_tree_model_get ((GtkTreeModel*)listsListStore, &iter,NAME_LISTS_COLUMN, &str_data,-1);

    if(strcmp(filename,str_data) == 0){
      g_free (str_data);
      return iter;
    }
    g_free (str_data);
    valid = gtk_tree_model_iter_next ((GtkTreeModel*)listsListStore, &iter);
  }
  return iter;
}

void checkModify(char *filename){
  if(IsPhoneListModified(filename)){
    if(askYesNo("Liste geaendert, noch speichern?")){
      g_print ("Save\n");
      SavePhoneList(filename); 
    }
  }
}

gboolean closeListByName(char *name, GtkTreeIter *iterPtr){
 GtkTreeIter iter;
  if (iterPtr == NULL){
    iter = getIterByFilename(name);
  } else {
    iter = *iterPtr;
  }
  checkModify(name);
  RemovePhoneList(name);
  removePhoneStore(name);
  return gtk_list_store_remove(listsListStore, &iter);
}

int askYesNo(char *title){
  int ret = 0;
  GtkWidget *dialog, *label;
  dialog = gtk_dialog_new_with_buttons (title,
      (GtkWindow*)main_app_window,
      GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
      GTK_STOCK_YES,
      GTK_RESPONSE_ACCEPT,
      GTK_STOCK_NO,
      GTK_RESPONSE_REJECT,
      NULL);
  label = gtk_label_new (title);
  gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->vbox), label);
  gtk_widget_show(label);
  ret = (gtk_dialog_run((GtkDialog*)dialog) == GTK_RESPONSE_ACCEPT);
  gtk_widget_destroy(dialog);
  return ret;
}

void fetchNewListFile(char *selectorTitle, char *existMsg, int type){
  GtkWidget *fileSel;
  char *filename;

  fileSel = gtk_file_chooser_dialog_new (selectorTitle,
      (GtkWindow*)main_app_window,
      type,
      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
      GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
      NULL);

  if (gtk_dialog_run (GTK_DIALOG (fileSel)) == GTK_RESPONSE_ACCEPT){
    filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (fileSel));
    if (GetPhoneList(filename) == NULL){
      if (type == GTK_FILE_CHOOSER_ACTION_SAVE){
        newPhoneFile(filename);
      }
      if(pushPhoneList(readPhoneFile(filename), filename) == OK){
	addPhoneModelList(filename);
	addListsListElm(filename);
      }
    } else {
      if(askYesNo(existMsg)){
	closeListByName(filename, NULL);
	if(pushPhoneList(readPhoneFile(filename), filename) == OK){
	  addPhoneModelList(filename);
	  addListsListElm(filename);
	}
      }
    }
    GtkTreeIter iter = getIterByFilename(filename);
    if(gtk_list_store_iter_is_valid(listsListStore, &iter)){
      gtk_tree_selection_select_iter(listsListSel, &iter);
    }
    g_free (filename);
  }
  gtk_widget_destroy (fileSel);
}


void modifyEntryButtonCliked(GtkWidget *widget, GdkEvent *event, gpointer data){
  g_print ("ModEntry\n");
  
  //TODO Schauen ob überhaupt eine Liste selektiert

  modifyPhoneList(E_MOD);

}
void newEntryButtonClicked(GtkWidget *widget, GdkEvent *event, gpointer data){
  g_print ("ModEntry\n");
  
  //TODO Schauen ob überhaupt eine Liste selektiert

  modifyPhoneList(E_NEW);

}
void removeEntryButtonClicked(GtkWidget *widget, GdkEvent *event, gpointer data){
  g_print ("ModEntry\n");
  
  //TODO Schauen ob überhaupt eine Liste selektiert

  modifyPhoneList(E_DEL);

}

void closeButtonClicked(GtkWidget *widget, GdkEvent *event, gpointer data){
  g_print ("Close\n");
  GtkTreeIter iter;
  char* name;
  if(gtk_tree_selection_get_selected (listsListSel, NULL, &iter)){

    if(gtk_list_store_iter_is_valid(listsListStore, &iter)){
      gtk_tree_model_get((GtkTreeModel*)listsListStore, &iter, NAME_LISTS_COLUMN, &name, -1);
      closeListByName(name, &iter);
      if(gtk_tree_model_get_iter_first ((GtkTreeModel*)listsListStore, &iter)){
	gtk_tree_selection_select_iter(listsListSel, &iter);
      } else {
        gtk_tree_view_set_model(GTK_TREE_VIEW(phoneView), GTK_TREE_MODEL(phoneDefaultStore));
      }
    }
  }
}

void saveAllButtonClicked(GtkWidget *widget, GdkEvent *event, gpointer data){
  g_print ("SaveAll\n");
  writeAllLists();
}

void saveButtonClicked(GtkWidget *widget, GdkEvent *event, gpointer data){
  GtkTreeIter iter;
  char* name;

  g_print ("Save\n");
  if(gtk_tree_selection_get_selected (listsListSel, NULL, &iter)){

    if(gtk_list_store_iter_is_valid(listsListStore, &iter)){
      gtk_tree_model_get((GtkTreeModel*)listsListStore, &iter, NAME_LISTS_COLUMN, &name, -1);
      //TODO Implemetieren 
      //
      SavePhoneList(name); 
    }
  }
}

void loadButtonClicked(GtkWidget *widget, GdkEvent *event, gpointer data){
  g_print ("Load\n");
  fetchNewListFile("Telefonliste laden", "Datei schon geladen, neu laden?", GTK_FILE_CHOOSER_ACTION_OPEN);
}

void newButtonClicked(GtkWidget *widget, GdkEvent *event, gpointer data){
  g_print ("New\n");
  fetchNewListFile("Neue Telefonliste", "Datei schon geladen, neu laden?", GTK_FILE_CHOOSER_ACTION_SAVE);
}

/* Wird vor dem beenden ausgefuehrt,
 * fragt ob gespeichert werden soll */
static gboolean delete_event( GtkWidget *widget, GdkEvent *event, gpointer data){
  if(anyModifiedPhoneLists()){
    if(askYesNo("noch Speichern?")){
      g_print ("SaveAll\n");
      writeAllLists();
    }
  }
  
  return FALSE;
}

/* Beendet letztendlich den Event-Loop */
static void destroy( GtkWidget *widget, gpointer data ){
  g_print ("Und Tschuess\n");
  gtk_main_quit ();
}

static void tree_selection_changed_cb (GtkTreeSelection *selection, gpointer data){
  GtkTreeIter iter;
  GtkTreeModel *model;
  char *name;
  tPhoneListStore * phoneModel;

  if (gtk_tree_selection_get_selected (selection, &model, &iter)){

    gtk_tree_model_get (model, &iter, NAME_LISTS_COLUMN, &name, -1);
    
    if((phoneModel = getPhoneModel(name)) != NULL){
      gtk_tree_view_set_model(GTK_TREE_VIEW(phoneView), GTK_TREE_MODEL(phoneModel->store));
    }

    g_print ("You selected list: %s\n", name);
    // ToDo Hier die Models wechseln

    
    g_free (name);
  }
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
  gtk_tree_selection_set_mode (listsListSel, GTK_SELECTION_SINGLE);
  g_signal_connect (G_OBJECT (listsListSel), "changed", G_CALLBACK (tree_selection_changed_cb), NULL);

  gtk_container_add((GtkContainer*)glade_xml_get_widget(xml, "PhoneListsScroll"), listsList);
  gtk_widget_show(listsList);
}

void initPhoneList(){
  GtkTreeIter iter;
  GtkCellRenderer *renderer;
  GtkTreeViewColumn *column;
  
  phoneDefaultStore = gtk_list_store_new(PHONE_N_COLUMNS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING); 
  phoneView = gtk_tree_view_new_with_model (GTK_TREE_MODEL (phoneDefaultStore));
  renderer = gtk_cell_renderer_text_new();
  column = gtk_tree_view_column_new_with_attributes ("Name",
      renderer,
      "text", PHONE_NAME_COLUMN,
      NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (phoneView), column);
  renderer = gtk_cell_renderer_text_new();
  column = gtk_tree_view_column_new_with_attributes ("Given",
      renderer,
      "text", PHONE_GIVEN_COLUMN,
      NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (phoneView), column);
  renderer = gtk_cell_renderer_text_new();
  column = gtk_tree_view_column_new_with_attributes ("Phone",
      renderer,
      "text", PHONE_COLUMN,
      NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (phoneView), column);

  phoneListSel = gtk_tree_view_get_selection (GTK_TREE_VIEW (phoneView));
  gtk_tree_selection_set_mode (phoneListSel, GTK_SELECTION_SINGLE);
  gtk_container_add((GtkContainer*)glade_xml_get_widget(xml, "PhoneListScroll"), phoneView);
  gtk_widget_show(phoneView);

}

void intEntryDialog(){
  g_print ("NewEntry\n");
  
  entryDialog = glade_xml_get_widget(xml,"entryDialog");  
  gtk_dialog_add_button(GTK_DIALOG(entryDialog), GTK_STOCK_OK, GTK_RESPONSE_ACCEPT);
  gtk_dialog_add_button(GTK_DIALOG(entryDialog), GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT);
  gtk_widget_hide(entryDialog);
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
  initPhoneList();
  intEntryDialog();

  gtk_main();
  return 0;
}
