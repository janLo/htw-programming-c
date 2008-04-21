// vim: set sw=2 foldmethod=syntax nowrap:
/* phon.h zu Aufgabe 1
 * (http://www.informatik.htw-dresden.de/%7Ebeck/C/PspCB1.html)
 *
 * -- Jan Losinski, 2008/04/14
 *  */


#define MODIFIED 1

/* Datenstruktur fuer einen Telefonbucheintrag */
typedef struct dataEntry {
  char * name;		/* Der NAme */
  char * given;		/* Der Vorname */
  char * phone;		/* Die Telefonnummer */
} tDataEntry;


/* ******* Funktionen ********
 * .. Genaue Dokumentation ist in phon.h zu finden */

tList * getPhoneList(char *name);					/* Holt Telefonliste nach Name */
void writeAllLists();							/* Speichert alle Telefonlisten */
int pushPhoneList(tList *list, char *name);				/* Fuegt eine neue Telefonliste hinzu */
tList *readPhoneFile(char *file);					/* Liest eine Liste aus einer Datei */	
void removePhoneList(char *name);					/* Entfernt eine Telefonliste */
int isPhoneListModified(char *name);					/* Prueft ob Telefonliste geaendert */
void setPhoneListModified(char *name, int modified);			/* Setzt Modified-Flag fuer Telefonliste */
int anyModifiedPhoneLists();						/* Prueft ob geaenderte listen vorhanden */
void savePhoneList(char *name);						/* Speichert Telefonliste in Datei */
void newPhoneFile(char *filename);					/* Generiert neue Telefonlisten-Datei */
void removeByIdx(tList *list, int idx);					/* Entfernt Element aus Liste x an Pos. y */
int insertEntrySorted(tList * list, tDataEntry * entry);		/* Fuegt ein Element sortiert ein */
tDataEntry * createEntry(char * name, char * given, char * phone);	/* Baut einen neuen Telefonbucheintrag */
