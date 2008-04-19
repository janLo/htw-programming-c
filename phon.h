// vim: set sw=2 foldmethod=syntax nowrap:
/* list.c zu Aufgabe 1
 * (http://www.informatik.htw-dresden.de/%7Ebeck/C/PspCB1.html)
 *
 * -- Jan Losinski, 2008/04/14
 *  */

typedef struct dataEntry {
  char * name;
  char * given;
  char * phone;
} tDataEntry;


tList * GetPhoneList(char *name);
void writeAllLists();
int pushPhoneList(tList *list, char *name);
tList *readPhoneFile(char *file);
void RemovePhoneList(char *name);
int IsPhoneListModified(char *name);
void SetPhoneListModified(char *name, int modified);
int anyModifiedPhoneLists();
void SavePhoneList(char *name);
void newPhoneFile(char *filename);
