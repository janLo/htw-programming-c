// vim: set sw=2 foldmethod=syntax nowrap:
/* tvz.c zu Aufgabe 1
 * (http://www.informatik.htw-dresden.de/%7Ebeck/C/PspCB1.html)
 *
 * Implementiert das GUI zu der "Telefonbuch"-Anwendung
 *
 * -- Jan Losinski, 2008/04/14
 *  */


#include <gtk/gtk.h>
#include <glade/glade.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "list.h"
#include "phon.h"

/* Aufzaehlung fuer die Spalten der Telefonnumern-
 * Liste 
 * */
enum {
  PHONE_NAME_COLUMN,		/* Name */
  PHONE_GIVEN_COLUMN,		/* Vorname */
  PHONE_COLUMN,			/* Tel-Nr */
  PHONE_N_COLUMNS		/* Anz. Spalten */
};

/* Aufzaehlung fuer die Spalten der Listen-
 * Liste 
 * */
enum {
  NAME_LISTS_COLUMN,		/* Listen-Name */
  N_LISTS_COLUMNS		/* Anz. Spalten */
};

/* Aufzaehlung von mögl. Argumenten der Fkt.
 * die fuer die Modifikation der Telefonlisten 
 * verantwortlich ist 
 * */
enum {
  E_DEL,			/* Selekt. Eintr. loeschen */
  E_MOD,			/* Selekt. Eintr. aendern */
  E_NEW				/* neuer Eintrag */
};

/* Datenstruktur fuer eine Telefonliste.
 * Enthaelt den Namen und das model fuer die 
 * Telefonliste 
 * */
typedef struct phoneListStore{
  GtkListStore * store;		/* Listen-Model */
  char* name;			/* Listen-Name */
} tPhoneListStore;

/* ******  Variablendeklatationen ******* */
GtkWidget *main_app_window;	/* Das 'Hauptfenster' der Anwendung */
GladeXML *xml;			/* Das geparste XML-UI-File */
GtkListStore *listsListStore;	/* Datenmodell der Listen-Liste */
GtkListStore *phoneDefaultStore;/* Default-Datenmodell der Telefonliste */
GtkTreeSelection *listsListSel;	/* Auswahl der Listen-Liste */
GtkTreeSelection *phoneListSel;	/* Auswahl der Telefonliste */
tList *phoneModelLists = NULL;	/* Liste der Telefonlisten-Modelle */
GtkWidget *phoneView;		/* Telefonlisten-Ansicht */
GtkWidget *entryDialog;		/* Dialog zum hinzufuegen/aendern von Eintraegen */


/* ********* Funktionen ********* */

/* Fuellt ein uebergebenes Telefonlisten-Modell mit 
 * den Daten der uebergebenen Telefonliste
 * Dazu geht es durch die komplette Telefonliste
 * und fuegt fuer jeden Eintrag eine Zeile zum 
 * Modell hinzu.
 * Args:
 *   store .. Das Listenmodell
 *   list  .. Die Liste mit den Daten
 * Ret:
 *   Nichts
 * */
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

/* Laed die Daten aus der Telefonliste neu
 * in das Listenmodell. 
 * Dazu leert es das Modell erst und ruft 
 * anschließend die Funktion zu fuellen des
 * Modells auf.
 * Args:
 *   store .. Das Listenmodell
 *   list  .. Die Liste mit den Daten
 * Ret:
 *   Nichts
 * */
void refreshPhoneModel(GtkListStore *store, tList *list){
  gtk_list_store_clear(store);
  fillPhoneModel(store, list);
}

/* Generiert ein neues Telefonlistenmodell fuer
 * eine Telefonliste.
 * Dazu holt sie sich die Telefonliste mit dem 
 * entsprechendem Namen, prüft diese auf Existenz,
 * generiert ein neues Modell (GtkListStore) und 
 * Modell-Listen-Element (tPhoneListStore), setzt 
 * den Namen und das Modell und ruft die Fkt. zum
 * fuellen des Modells auf.
 * Args:
 *   name .. Name der Liste
 * Ret: 
 *   Nichts
 * */
void addPhoneModelList(char *name){
  tList *list = getPhoneList(name);
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

  InsertTail(phoneModelLists, newList);
  fillPhoneModel(store, list);
}

/* Zustaendig fue alleswas auf der Telefonliste passiet.
 * mittels type wird ausgewählt was zu tun ist:
 * E_DEL loescht den momentan selektieren Eintrag in der
 * Telefonliste (nach einer Ja/Nein abfrage)
 * E_MOD zeigt einen Dialog zum bearbeitendes momentan
 * selektierten Eintrages und aendert ihn nach erfolgter 
 * Eingabe
 * E_NEW Zeigt einen Dialog zum anlegen eines neuen 
 * Eintrages und legt diesesn nach erfolgter Eingabe
 * an.
 * Nach allen 3 Operationen wird das Modell neu geladen
 * um Konsistent mit den Daten in der eigendlichen 
 * Liste zu bleiben und die Liste als Bearbeitet markiert.
 * Args:
 *   type .. E_DEL - Selektierten Eintrag loeschen
 *           E_MOD - Selektieren Eintrag aendern
 *           E_NEW - Neuen Eintag hinzufuegen
 * Ret:
 *   Nichts
 * */
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
      list = getPhoneList(listName);
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

      if(GTK_RESPONSE_ACCEPT == gtk_dialog_run(GTK_DIALOG(dialog))){
	if (type == E_MOD || type == E_DEL){
	  oldNum = *((gint*)gtk_tree_path_get_indices(gtk_tree_model_get_path(model, &iter)));
	  // g_print("Mod-Num: %d\n", oldNum);
	}

	if (type == E_MOD || type == E_DEL){
	  removeByIdx(list, oldNum);
	  setPhoneListModified(listName, MODIFIED);
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
	  setPhoneListModified(listName, MODIFIED);
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

/* Sucht ein bestimmtes Telefonlisten-Modell
 * in der Liste der Telefonlisten-Modelle 
 * Dazu geht es die komplette Liste durch 
 * und vergleicht die Namen mit dem uebergebenen.
 * Args:
 *   name .. Der Name der gesuchten Liste
 * Ret:
 *   Das Modell zum Namen oder NULL (Wenn
 *   Modell nicht existiert)
 * */
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

/* Entfernt ein Telefonlisten-Modell,
 * gibt resourcen frei.
 * Args:
 *   name .. Name der zu entfernenden Liste
 * Ret:
 *   Nichts
 * */
void removePhoneStore(char *name){
  tPhoneListStore * elm = getPhoneModel(name);
  if (elm != NULL){
    RemoveItem(phoneModelLists);
    free(elm->name);
    gtk_list_store_clear (elm->store);
    free(elm);
  }
}

/* Fuegt ein neues Element zur Liste der
 * Telefonlisten hinzu
 * Args:
 *   name .. Name der Liste
 * Ret:
 *   Nichts
 * */
void addListsListElm(char *name){
  GtkTreeIter iter;
  gtk_list_store_append(listsListStore, &iter);
  gtk_list_store_set(listsListStore, &iter, NAME_LISTS_COLUMN, name,-1);
}

/* Gibt einen Iterator zu einem Listeneintrag
 * in der Teleonlisten-Liste zurueck.
 * Dazu geht es alle Eintraege durch und 
 * vergleicht die Namen.
 * Args:
 *   filename .. Name der Liste, dessen Iterator
 *               gesucht wird
 * Ret:
 *   Der Iterator zum passenden Eintrag
 * */
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

/* Zeigt ein Dialogfenster, welches nach einer
 * Ja/Nein Entscheidung fragt und gibt das 
 * Ergebnis zurueck.
 * Args:
 *   title .. Titel des Fensters, bzw. Frage
 *            des Dialogs
 * Ret:
 *   ==0   .. Wenn "Nein"
 *   !=0   .. Wenn "Ja"
 * */
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

/* Testet ob eine Liste geaendert wurde und 
 * fragt ob diese gespeichert werden soll.
 * Bei Ja wird die Liste gespeichert.
 * Args:
 *   filename .. Name der Liste
 * Ret:
 *   Nichts
 * */
void checkModify(char *filename){
  if(isPhoneListModified(filename)){
    if(askYesNo("Liste geaendert, noch speichern?")){
      // g_print ("Save\n");
      savePhoneList(filename); 
    }
  }
}

/* Schliesst eine Liste nach Dateiname
 * Es werden auch Funktionen zum 
 * freigeben der Resourcen aufgerufen
 * Args:
 *   name .. Name der Liste
 *   iterPtr .. Iterator des zu entfernenden 
 *              Elementes. Wenn Null wird der 
 *              Iterator mit getIterByFilename 
 *              geholt.
 * Ret:
 *   Das Ergebnis von gtk_list_store_remove
 * */
gboolean closeListByName(char *name, GtkTreeIter *iterPtr){
 GtkTreeIter iter;
  if (iterPtr == NULL){
    iter = getIterByFilename(name);
  } else {
    iter = *iterPtr;
  }
  checkModify(name);
  removePhoneList(name);
  removePhoneStore(name);
  return gtk_list_store_remove(listsListStore, &iter);
}

/* Laed ein neues Listenfile bzw. erzeugt
 * eine neue Liste.
 * Dazu wird ein Dateiauswahl-Dialog angezeigt,
 * dessen Ergebnis der Listenname ist.
 * Es wird auch ueberprueft ob die Datei bereits 
 * geladen ist.
 * Args:
 *   selectorTitle .. Der Titel der Dateiauswahl
 *   existMsg      .. Der Text der in dem Dialog 
 *                    Angezeigt wird, der mitteilt,
 *                    das die Datei bereits geladen 
 *                    ist und fragt ob sie neu geladen
 *                    werden soll.
 *   type          .. Legt Fest ob es ein "Oeffnen" Dialog
 *                    (zum oeffnen existierender Listen) 
 *                    oder ein "Speichern" Dialog (zum 
 *                    anlegen neuer Listen) sein soll.
 * Ret:
 *   Nichts
 * */
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
    if (getPhoneList(filename) == NULL){
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

/* ****** Callback - Funktionen ******
 * Diese Funktionen werden aufgerufen, wenn irgend
 * ein relevantes GUI-Event (klicken von Buttons)
 * eintrifft. */

/* Ein Telefonlisten-Eintrag soll geaendert werden */
void modifyEntryButtonCliked(GtkWidget *widget, GdkEvent *event, gpointer data){
  modifyPhoneList(E_MOD);
}
/* Es soll ein neuer Telefonlisten-Eintrag erzeugt
 * werden */
void newEntryButtonClicked(GtkWidget *widget, GdkEvent *event, gpointer data){
  modifyPhoneList(E_NEW);
}
/* Ein Telefonlisteneintrag soll entfernt werden */
void removeEntryButtonClicked(GtkWidget *widget, GdkEvent *event, gpointer data){
  modifyPhoneList(E_DEL);
}

/* Eine Telefonliste soll geschlossen werden */
void closeButtonClicked(GtkWidget *widget, GdkEvent *event, gpointer data){
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

/* Alle Telefonlisten sollen gespeichert werden */
void saveAllButtonClicked(GtkWidget *widget, GdkEvent *event, gpointer data){
  writeAllLists();
}

/* Eine Telefonliste soll gespeichert werden */
void saveButtonClicked(GtkWidget *widget, GdkEvent *event, gpointer data){
  GtkTreeIter iter;
  char* name;
  if(gtk_tree_selection_get_selected (listsListSel, NULL, &iter)){
    if(gtk_list_store_iter_is_valid(listsListStore, &iter)){
      gtk_tree_model_get((GtkTreeModel*)listsListStore, &iter, NAME_LISTS_COLUMN, &name, -1);
      savePhoneList(name); 
    }
  }
}

/* Eine Telefonliste soll geladen werden */
void loadButtonClicked(GtkWidget *widget, GdkEvent *event, gpointer data){
  fetchNewListFile("Telefonliste laden", "Datei schon geladen, neu laden?", GTK_FILE_CHOOSER_ACTION_OPEN);
}

/* Eine neue Telefonliste soll erzeugt werden */
void newButtonClicked(GtkWidget *widget, GdkEvent *event, gpointer data){
  fetchNewListFile("Neue Telefonliste", "Datei schon geladen, neu laden?", GTK_FILE_CHOOSER_ACTION_SAVE);
}

/* Wird vor dem beenden ausgefuehrt,
 * fragt ob gespeichert werden soll,
 * sofern noch ungespeicherte Listen
 * offen sind */
static gboolean delete_event( GtkWidget *widget, GdkEvent *event, gpointer data){
  if(anyModifiedPhoneLists()){
    if(askYesNo("noch Speichern?")){
      writeAllLists();
    }
  }
  return FALSE;
}

/* Beendet letztendlich den Event-Loop 
 * und schließt das Hauptfenster */
static void destroy( GtkWidget *widget, gpointer data ){
  gtk_main_quit ();
}

/* Wird ausgeführt, wenn eine eine Telefnliste
 * ausgewaehlt wird.
 * Wechselt dann das Datenmodell der Telefon-
 * listenansicht. */
static void listsListSelectionChanged (GtkTreeSelection *selection, gpointer data){
  GtkTreeIter iter;
  GtkTreeModel *model;
  char *name;
  tPhoneListStore * phoneModel;
  if (gtk_tree_selection_get_selected (selection, &model, &iter)){
    gtk_tree_model_get (model, &iter, NAME_LISTS_COLUMN, &name, -1);
    if((phoneModel = getPhoneModel(name)) != NULL){
      gtk_tree_view_set_model(GTK_TREE_VIEW(phoneView), GTK_TREE_MODEL(phoneModel->store));
    }
    g_free (name);
  }
}



/* ****** Initialisieren von GUI-Elementen *******
 * Diese Funktionen dienen der einmaligen 
 * initialisierung einiger GUI-Elemente */

/* Initialisiert die Telefonlisten-Liste */
void initPhonListsList(){
  GtkWidget *listsList;
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
  g_signal_connect (G_OBJECT (listsListSel), "changed", G_CALLBACK (listsListSelectionChanged), NULL);
  gtk_container_add((GtkContainer*)glade_xml_get_widget(xml, "PhoneListsScroll"), listsList);
  gtk_widget_show(listsList);
}

/* Initialisiert die Telefonliste */
void initPhoneList(){
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

/* Initialisiert den Dialog zum aendern
 * und anlegen von Telefonlisten-Eintraegen */
void intEntryDialog(){
  entryDialog = glade_xml_get_widget(xml,"entryDialog");  
  gtk_dialog_add_button(GTK_DIALOG(entryDialog), GTK_STOCK_OK, GTK_RESPONSE_ACCEPT);
  gtk_dialog_add_button(GTK_DIALOG(entryDialog), GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT);
  gtk_widget_hide(entryDialog);
}


/* **** Die Main-Funktion ****
 * .. Der 'Startpunkt' der Anwendung. 
 * Hier wird das die GUI beschreibende XML-File geparst
 * und das HAuptfenster initialisiert.
 * Außerdem wird hier der Event-Loop gestartet,
 * welcher auf Nutzereingaben wartet un die entsprechenden 
 * Callback-Funktionen aufruft.
 * */
int main(int argc, char *argv[]) {

  gtk_init(&argc, &argv);

  /* GUI-XML Laden */
  xml = glade_xml_new("UI.glade", NULL, NULL);

  /* Callbaks mit den Siagnalen verbinden */
  glade_xml_signal_autoconnect(xml);

  /* Hauptfenster 'holen' und Signale zum Schließen der 
   * Anwendung mit den Callbacks verbinden */
  main_app_window = glade_xml_get_widget(xml, "AppWindow");
  g_signal_connect (G_OBJECT (main_app_window), "delete_event", G_CALLBACK (delete_event), NULL);
  g_signal_connect (G_OBJECT (main_app_window), "destroy", G_CALLBACK (destroy), NULL);

  /* Div. GUI-Elemente initialisieren */
  initPhonListsList();
  initPhoneList();
  intEntryDialog();

  /* Den Eventloop statren */
  gtk_main();

  return 0;
}
