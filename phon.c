// vim: set sw=2 foldmethod=syntax nowrap:
/* phon.c zu Aufgabe 1
 * (http://www.informatik.htw-dresden.de/%7Ebeck/C/PspCB1.html)
 *
 * -- Jan Losinski, 2008/04/14
 *  */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "list.h"
#include "phon.h"

#define BUFFERSIZE 1024

/* Datenstruktur fuer eine Telefonliste.
 * Enthaelt den Namen und die Liste an sich.
 * Ausserdem enthaelt es ein Flag um anzuzeigen,
 * das die Liste seit dem letzten Speichern
 * veraendert wurde.
 * */
typedef struct phoneList {
  int modified;			/* Zeigt an ob die Liste seit dem letzten Speichern veraendert wurde */
  tList * list;			/* Die eigendliche Liste */
  char * listName; 		/* Der name der Liste */
} tPhoneList;

/* Funktionsdefinitionen */
void writePhoneFile(tList * list, char * file);
tList * readPhoneFile(char * filename);
int searchEntryAddPoint(tList * list, char * name, int *idx);
tPhoneList * getPhoneListElm(char * name);

/* Variablen */
static tList * phoneLists = NULL;	/* Die Liste der Telefonlisten */




/* ..Code.. */

/* Entfernt ein Element von einer Telefonliste, 
 * welches sich an einem uebergebenen Index 
 * befindet.
 * Args:
 *   list .. Die Liste in der das Element zu finden ist 
 *   idx  .. Der Index an der sich das Element befindet
 * Ret: 
 *   Nichts
 * */
void removeByIdx(tList *list, int idx){
  tDataEntry * ent = NULL; 
  if ((ent = GetIndexed(list, idx)) != NULL){
    free (ent->name);
    free(ent->given);
    free(ent->phone);
    free(ent);
    RemoveItem(list);
  }
}

/* Entfernt eine Komplette Telefonliste aus
 * der Listen-Liste.
 * Gibt allle Resourcn wider frei.
 * Args: 
 *   name .. Name der zu entfernenden Liste 
 * Ret: 
 *   Nichts
 * */
void removePhoneList(char *name){
  tPhoneList *elm = NULL;
  tDataEntry *tmp = NULL;
  
  if((elm = getPhoneListElm(name)) == NULL){
    return;
  }
  RemoveItem(phoneLists);
  free(elm->listName);
  if((tmp = GetFirst(elm->list)) != NULL){
    do{
      free(tmp->name);
      free(tmp->given);
      free(tmp->phone);
      free(tmp);
      RemoveItem(elm->list);
    } while ((tmp = GetSelected(elm->list)) != NULL);
  }
  DeleteList(elm->list);
  free(elm);
}

/* Sucht eine Telefonliste mit einem gegebenem
 * Namen in der Listen-Liste und gibt diese 
 * zurueck.
 * Dazu nutzt es die Funktion, die die komplette
 * Listen-Liste nach tPhoneList - Elementen mit
 * dem richtigem Namen durchsucht.
 * Args: 
 *   name .. Name der zu suchenden Liste
 * Ret:
 *   Die Liste passend zum Namen
 * */
tList * getPhoneList(char *name){
  tPhoneList *elm = NULL;
  if((elm = getPhoneListElm(name)) == NULL){
    return NULL;
  } else {
    return elm->list;
  }
}

/* Speichert eine Telefonliste mit einem gegebenem 
 * Namen.
 * Datu laesst es erst die passende Liste suchen 
 * und ruft anschliessend die Funktion zum Speichern
 * mit den passende Argumenten auf.
 * Args:
 *   name .. Name der zu speichernden Liste
 * Ret: 
 *   Nichts
 * */
void savePhoneList(char *name){
  tPhoneList *elm = NULL;
  if((elm = getPhoneListElm(name)) == NULL){
    return;
  } else {
    writePhoneFile(elm->list, elm->listName);
    elm->modified = 0;
  }
}

/* Prueft ob eine Telefonliste seit dem Speichern
 * modifiziert wurde.
 * Dazu laesst es erst die passende Liste suchen 
 * und gibt dann das modified-flag zurueck.
 * Args:
 *   name .. Name der zu pruefenden Liste
 * Ret:
 *   Das Modified-Flag der Liste
 * */
int isPhoneListModified(char *name){
  tPhoneList *elm = NULL;
  if((elm = getPhoneListElm(name)) == NULL){
    return 0;
  } else {
    return elm->modified;
  }
}

/* Setzt das modified-Flag fuer eine Telefonliste.
 * Die Telefonliste wird dabei aus der Liste der 
 * Telefonlisten gesucht.
 * Args:
 *   name     .. Der Name der Liste
 *   modified .. der Wert auf den das Flag gesetzt 
 *               werden soll
 * Ret:
 *   Nichts
 * */
void setPhoneListModified(char *name, int modified){
  tPhoneList *elm = NULL;
  if((elm = getPhoneListElm(name)) == NULL){
    return;
  } else {
    elm->modified = modified; 
  }
  return;
}

/* Holt ein tPhoneList Element aus der Liste der
 * Telefonlisten, dessen Name gleich dem 
 * uebergebenem String ist.
 * Args:
 *   name .. Name nach dem gesucht wird
 * Ret: 
 *   Das tPhoneList Element
 * */
tPhoneList * getPhoneListElm(char * name){
  tPhoneList * walker = NULL;
  if ((phoneLists == NULL) || ((walker = GetFirst(phoneLists)) == NULL)){
    // printf("Keine Telefonlisten\n");
    return NULL;
  }
  do {
    // printf("Vergleiche: %s <=> %s \n", walker->listName, name);
    if (strcmp(walker->listName, name) == 0){
      return walker;
    }
  } while ((walker = GetNext(phoneLists)) != NULL);
  return NULL;
}

/* Prueeft ob irgend eine Liste seit dem Speichern
 * veraendert wurde.
 * Dazu geht es die komplette Telefonlisten-Liste 
 * durch, wertet das modified flag aus und bricht 
 * bei dem ersten gesetzten modified flag ab.
 * Args:
 *   Nichts
 * Ret:
 *   != 0 .. wenn min. ein modified flag gesetzt
 *   == 0 .. Sonst
 * */
int anyModifiedPhoneLists(){
  tPhoneList * walker = NULL;
  if ((phoneLists == NULL) || ((walker = GetFirst(phoneLists)) == NULL)){
    // printf("Keine Telefonlisten\n");
    return 0;
  }
  do {
    if (walker->modified == MODIFIED){
      return MODIFIED;
    }
  } while ((walker = GetNext(phoneLists)) != NULL);
  return 0;
}

/* Fuegt eine Telefonliste zur Liste der Telefon-
 * listen unter einem bestimmtem Namen hinzu.
 * Dazu generiert es ein neues tPhoneList Element,
 * setzt dessen members und fuegt es der Liste hinzu.
 * Args:
 *   list .. Die Hinzuzufuegende Liste
 *   name .. Der Name unter dem hinzugefuegt wird
 * Ret:
 *   OK   .. Im Erfolgsfall
 *   FAIL .. Sonst
 * */
int pushPhoneList(tList * list, char * name){
  char* listName = NULL;
  tPhoneList * newList = NULL;
  if (phoneLists == NULL){
    phoneLists = CreateList();
  }
  if ((newList = malloc(sizeof(tPhoneList))) == NULL){
    return FAIL;
  }
  listName = malloc(sizeof(char)*strlen(name)+2);
  strcpy(listName, name);
  newList->list = list;
  newList->listName = listName;
  newList->modified = 0;
  InsertTail(phoneLists, newList);
  // printf("Fuege Liste %s hinzu (%p)\n", newList->listName, newList->listName);
  return OK;
}

/* Schreibt alle Listen auf die Platte.
 * Dazu wird die Liste der Listen durchgegangen und
 * fuer jede Liste die Speicherfunktion aufgerufen.
 * Args:
 *   Nichts
 * Ret:
 *   Nichts
 * */
void writeAllLists(){
  tPhoneList * walker = NULL;

  if ((phoneLists == NULL) || ((walker = GetFirst(phoneLists)) == NULL)){
    // printf("Keine Telefonlisten\n");
    exit(-1);
    return;
  }
  do {
    writePhoneFile(walker->list, walker->listName);
  } while ((walker = GetNext(phoneLists)) != NULL);
}

/* Erzeugt eine neue Listendatei mit einem bestimmtem 
 * Namen.
 * Dazu wird die Datei zum schreiben geoeffnen und 
 * gleich wieder geschlossen.
 * Args:
 *   name .. Der Name der neuen Datei
 * Ret: 
 *   Nichts
 * */
void newPhoneFile(char *name){
  fclose(fopen(name, "w"));
}

/* Schreibt eine Telefonliste auf die Platte.
 * Dazu geht es die Liste von oben nach unten durch
 * und schreibt fuer jeden Eintrag 4 Zeilen:
 * Erst den Namen, dann den Vornamen, dann die 
 * Telefonnummer und noch eine Leerzeile als Trennung
 * zum naechstem Eintrag.
 * Args:
 *   list .. Zu schreibende Liste
 *   file .. Dateiname
 * Ret:
 *   Nichts
 * */
void writePhoneFile(tList * list, char * file){
  FILE *out;
  tDataEntry * walker = NULL;
  if (!(out = fopen(file, "w"))) {
    // printf("Kann Daten-File: %s nicht oeffnen!\n", file);
    return;
  }	
  if ((walker = GetLast(list)) == NULL){
    // printf("Schreibe nichts, Liste Leer!\n");
    return;
  }
  do {
    fprintf(out, "%s\n%s\n%s\n\n", walker->name, walker->given, walker->phone);
    // printf("Eintrag geschrieben: %s - %s - %s\n", walker->name, walker->given, walker->phone);
  } while ((walker = GetPrev(list)) != NULL);
  fclose(out);
  setPhoneListModified(file, MODIFIED);
  return; 
}

/* Liest eine Telefonliste von der Platte.
 * Dazu wird die Datei zeilenweise eingelesen,
 * wobei das selbe Format erwartet wird, wie das,
 * welches writePhoneFile schreibbt, d.h. zuerst
 * eine Zeile Name, dann eine Zeile Vorname, dann
 * eine Zeile Telefonnummer und noch eine Leer-
 * zeile am Schluss eines jeden Eintrags.
 * Die Zeilen werden dabei einfach durch einen
 * Zaehler zugeordnet, welcher bei jeder Zeile um 
 * eins incrementiert mod 4 wird.
 * Sobald ein Eintrag vollst. gelesen ist (immer 
 * bei der Leerzeile) wird ein neues tDataEntry 
 * Element mit den eingelesenen Daten erzeugt, 
 * welches nach namen sortiert in eine neue, 
 * ebenfalls in dieser Funktion generierten,
 * Telefonliste eingefuegt wird.
 * Args: 
 *   file .. Name der einzulesenden Datei
 * Ret:
 *   Die eingelesenen Liste
 * */
tList * readPhoneFile(char * file){
  FILE *in; 
  char buffer[BUFFERSIZE];
  int state = 0;
  char * nameTmp;
  char * givenTmp;
  char * phoneTmp;
  char * tmp;

  tList * list = CreateList();

  if (!(in = fopen(file, "r"))) {
    // printf("Kann Daten-File: %s nicht oeffnen!\n", file);
    return NULL;
  }
  while (fgets(buffer, BUFFERSIZE, in)) {
    //printf("Readfile Buffer: %s\n", buffer);
    if (strchr(buffer, '\n')){
      *(strchr(buffer, '\n')) = '\0';
    }
    tmp = malloc(sizeof(char) * (strlen(buffer)+2));
    strncpy(tmp, buffer, strlen(buffer)+1);
    // printf("Readfile State: %d String: %s Orig:%s\n", state, tmp, buffer);
    switch (state){
      case 0: 
	nameTmp = tmp;
	break;
      case 1:
	givenTmp = tmp;
	break;
      case 2:
	phoneTmp = tmp;
	break;
      case 3:
	insertEntrySorted(list, createEntry(nameTmp, givenTmp, phoneTmp));
	break;
    }
    *(buffer) = '\n';
    state = (state + 1) % 4;
  }
  fclose(in);
  return list;
}

/* Generiert ein neues tDataEntry Element, welches eien
 * Datensatz in einer Telefonliste repraesentiert.
 * Args:
 *   name  .. Der Name des neuen Eintrages
 *   given .. Der Vorname des neuen Eintrages
 *   phone .. Die Telefonnummer des neuen Einrages
 * Ret:
 *   Der neu generierte Eintrag
 * */
tDataEntry * createEntry(char * name, char * given, char * phone){
  tDataEntry * newEntry = malloc(sizeof(tDataEntry));
  newEntry->name = name;
  newEntry->given = given;
  newEntry->phone = phone;
  return newEntry;
}

/* Sucht den 'Einfuegepunkt' fuer einen neuen Eintrag.
 * Dazu werden die Namen der bereits in der Liste vorhandenen
 * Eintraege mit dem neuen Namen verglichen und der Eintrag, 
 * welcher in zukunft 'hinter' dem neuen Eintrag liegen soll 
 * selektiert. Damit kann dann leicht der neue Eintrag 'vor'
 * dem aktuellem eingefuegt werden.
 * Wenn Das neue Element 'am ende' eingefuegt werden sollte 
 * wird 1 zurueckgegeben, sonst 0.
 * Zudem wird noch der Index des gefundenen Eintrages in *idx 
 * gespeichert
 * Args:
 *   list .. Die Liste in der gesucht werden soll
 *   name .. Der Name des neuen Elementes
 *   idx  .. Der Index des gefindenen Elementes
 * Ret:
 *   1 .. Wenn das neue Element an das Ende der Liste soll
 *   0 .. Sonst
 * */
int searchEntryAddPoint(tList * list, char * name, int *idx){
  tDataEntry * walker = NULL;
  int i = 0;
  if ( (walker = (tDataEntry*)GetFirst(list)) == NULL){
    *idx = -1;
    return 0;
  }
  while ((walker != NULL) && (strcasecmp(walker->name, name) <= 0)){
    // printf("Search Add: '%s' < '%s'\n", walker->name, name);
    if ((walker = GetNext(list)) != NULL){
      i++;
    }
  }
  *idx = i;
  if(walker == NULL){
    return 1;
  }
  return 0;
}

/* Fuegt ein neues Element sortiert in die Liste ein.
 * Dazu ruft es zuerst die Funktion zum finden des 
 * Einfuegepunktes auf und fuegt es dann vor diesem oder 
 * am ende der Liste ein.
 * Zurueckgegeben wird der Index des neue Elementes.
 * Args:
 *   list  .. Die Liste in die eingefuegt werden soll
 *   entry .. Der neue Eintrag
 * Ret:
 *   Der Index des neuen Elementes (buggy)
 * */
int insertEntrySorted(tList * list, tDataEntry * entry){
  int addIndex;
  if(searchEntryAddPoint(list, entry->name, &addIndex)){
    InsertTail(list, entry);
    addIndex = -1;
  } else {
    InsertBefore(list, entry);
  }
  // printf("Inserted: %s - %s - %s\n",entry->name, entry->given, entry->phone);
  return addIndex;
}

