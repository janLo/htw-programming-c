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


typedef struct phoneList {
  int modified;
  tList * list;
  char * listName;
} tPhoneList;

/* Funktionsdefinitionen */
void writePhoneFile(tList * list, char * file);
tList * readPhoneFile(char * filename);
int searchEntryAddPoint(tList * list, char * name, int *idx);
tPhoneList * getPhoneListElm(char * name);

/* Variablen */
static tList * phoneLists = NULL;




/* ..Code.. */

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

tList * getPhoneList(char *name){
  tPhoneList *elm = NULL;
  if((elm = getPhoneListElm(name)) == NULL){
    return NULL;
  } else {
    return elm->list;
  }
}

void savePhoneList(char *name){
  tPhoneList *elm = NULL;
  if((elm = getPhoneListElm(name)) == NULL){
    return;
  } else {
    writePhoneFile(elm->list, elm->listName);
    elm->modified = 0;
  }
}

int isPhoneListModified(char *name){
  tPhoneList *elm = NULL;
  if((elm = getPhoneListElm(name)) == NULL){
    return 0;
  } else {
    return elm->modified;
  }
}

void setPhoneListModified(char *name, int modified){
  tPhoneList *elm = NULL;
  if((elm = getPhoneListElm(name)) == NULL){
    return;
  } else {
    elm->modified = modified; 
  }
  return;
}

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

void newPhoneFile(char *name){
  fclose(fopen(name, "w"));
}

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
  return; 
}

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
  
tDataEntry * createEntry(char * name, char * given, char * phone){
  tDataEntry * newEntry = malloc(sizeof(tDataEntry));
  newEntry->name = name;
  newEntry->given = given;
  newEntry->phone = phone;
  return newEntry;
}

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

