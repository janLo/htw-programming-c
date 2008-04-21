// vim: set sw=2 foldmethod=syntax nowrap:
/* list.c zu Aufgabe 1
 * (http://www.informatik.htw-dresden.de/%7Ebeck/C/PspCB1.html)
 *
 * -- Jan Losinski, 2008/04/14
 *  */

#define MODIFIED 1

typedef struct dataEntry {
  char * name;
  char * given;
  char * phone;
} tDataEntry;


tList * getPhoneList(char *name);
void writeAllLists();
int pushPhoneList(tList *list, char *name);
tList *readPhoneFile(char *file);
void removePhoneList(char *name);
int isPhoneListModified(char *name);
void setPhoneListModified(char *name, int modified);
int anyModifiedPhoneLists();
void savePhoneList(char *name);
void newPhoneFile(char *filename);
void removeByIdx(tList *list, int idx);
int insertEntrySorted(tList * list, tDataEntry * entry);
tDataEntry * createEntry(char * name, char * given, char * phone);
