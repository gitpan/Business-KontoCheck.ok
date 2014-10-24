/*
 * ##########################################################################
 * #  Dies ist konto_check-at, ein Programm zum Testen der Pr�fziffern von  #
 * #  �sterreichischen Bankkonten. Es kann jedoch auch als eigenst�ndiges   #
 * #  Programm oder als Library zur Verwendung in anderen Programmen        #
 * #  benutzt werden.                                                       #
 * #                                                                        #
 * #  Copyright (C) 2006 Michael Plugge <m.plugge@hs-mannheim.de>           #
 * #                                                                        #
 * #  Dieses Programm ist freie Software; Sie d�rfen es unter den           #
 * #  Bedingungen der GNU Lesser General Public License, wie von der Free   #
 * #  Software Foundation ver�ffentlicht, weiterverteilen und/oder          #
 * #  modifizieren; entweder gem�� Version 2.1 der Lizenz oder (nach Ihrer  #
 * #  Option) jeder sp�teren Version.                                       #
 * #                                                                        #
 * #  Die GNU LGPL ist weniger infekti�s als die normale GPL; Code, der von #
 * #  Ihnen hinzugef�gt wird, unterliegt nicht der Offenlegungspflicht      #
 * #  (wie bei der normalen GPL); au�erdem m�ssen Programme, die diese      #
 * #  Bibliothek benutzen, nicht (L)GPL lizensiert sein, sondern k�nnen     #
 * #  beliebig kommerziell verwertet werden. Die Offenlegung des Sourcecodes#
 * #  bezieht sich bei der LGPL *nur* auf ge�nderten Bibliothekscode.       #
 * #                                                                        #
 * #  Dieses Programm wird in der Hoffnung weiterverbreitet, da� es         #
 * #  n�tzlich sein wird, jedoch OHNE IRGENDEINE GARANTIE, auch ohne die    #
 * #  implizierte Garantie der MARKTREIFE oder der VERWENDBARKEIT F�R       #
 * #  EINEN BESTIMMTEN ZWECK. Mehr Details finden Sie in der GNU Lesser     #
 * #  General Public License.                                               #
 * #                                                                        #
 * #  Sie sollten eine Kopie der GNU Lesser General Public License          #
 * #  zusammen mit diesem Programm erhalten haben; falls nicht,             #
 * #  schreiben Sie an die Free Software Foundation, Inc., 59 Temple        #
 * #  Place, Suite 330, Boston, MA 02111-1307, USA. Sie k�nnen sie auch     #
 * #  von                                                                   #
 * #                                                                        #
 * #       http://www.gnu.org/licenses/lgpl.html                            #
 * #                                                                        #
 * # im Internet herunterladen.                                             #
 * #                                                                        #
 * ##########################################################################
 */

#define VERSION_AT "1.3"
#define VERSION_DATE_AT "4.6.2007"
#define VERSION_AT_MAJOR 1
#define VERSION_AT_MINOR 1
#define VERSION_AT_RELEASE 3

#define DEFAULT_LUT_NAME_AT "blz-at.lut"  /* Name der bin�ren Bankleitzahlen-Datei */
#define MAX_BLZ_CNT_AT 30000              /* maximale Anzahl Banken in blz-at.lut */
#define MAX_TABLE_CNT_AT 500              /* maximale Anzahl Pr�ftabellen in blz-at.lut */

#ifndef INT4_DEFINED
#define INT4_DEFINED
#include <limits.h>
#if INT_MAX==2147483647
typedef int INT4;
typedef unsigned int UINT4;
#elif LONG_MAX==2147483647
typedef long INT4;
typedef unsigned long UINT4;
#else  /* Notausstieg, kann 4 Byte Integer nicht bestimmen */
#error "Typedef f�r 4 Byte Integer nicht definiert"
#endif
#endif

/*
 * ######################################################################
 * # Die konto_check-at Library ist im Gegensatz zur aktuellen          #
 * # deutschen konto_check Library in den meisten Funktionen            #
 * # threadfest; Ausnahmen sind die trace-Versionen (komplett, da       #
 * # der Filedeskriptor trace global definiert sein mu�) sowie die      #
 * # Funktionen generate_lut_at().                                      #
 * # Dies d�rfte jedoch in beiden F�llen kein Problem sein, da die      #
 * # trace-Versionen nur zur Fehlersuche gedacht sind, und die          #
 * # Funktion generate_lut_at() normalerweise auch nicht auf einem      #
 * # laufenden  Produktionssystem aufgerufen werden sollte. Damit       #
 * # entfallen die beiden Funktionsvarianten f�r threadfeste und        #
 * # nicht threadfeste Version, sowie die Initialisierungs- und         #
 * # Aufr�umarbeiten bei den ctx-Strukturen.                            #
 * # Die einzige kritische Stelle f�r Umgebungen mit threads stellt die #
 * # Funktion init_globals() dar; in der Funktion wird der kritische    #
 * # Codeabschnitt mit einer Pseudo-Semaphoren gesch�tzt; aber auch der #
 * # u.U. m�gliche Fall, da� zwei threads gleichzeitig die library      #
 * # initialisieren, w�rde nur dann zu Inkonsistenzen f�hren, falls sie #
 * # zwei verschiedene lut-Dateien benutzen; in dem Fall wird die Datei #
 * # der letzten Initialisierung verwendet (falls nicht ein thread den  #
 * # anderen �berholt - dann d�rfte ein Mischmasch die Folge sein :-( ) #
 * # Da dies jedoch nicht sinnvoll erscheint, wurde auch eine Fehler-   #
 * # behandlung f�r diesen Fall nicht implementiert.                    #
 * ######################################################################
 */

/*
 * ######################################################################
 * # DLL-Optionen f�r Windows                                           #
 * # Der DLL-Code wurde aus der Datei dllhelpers (beim MinGW-Compiler   #
 * # enthalten, http://www.mingw.org/) entnommen                        #
 * ######################################################################
 */

#if BUILD_DLL /* DLL kompilieren */
# define DLL_EXPORT __declspec (dllexport) __stdcall 
# define DLL_EXPORT_V __declspec (dllexport)
#elif USE_DLL /* DLL in einem anderen Programm benutzen */
# define DLL_EXPORT __declspec (dllimport) __stdcall 
# define DLL_EXPORT_V __declspec (dllimport)
#else /* kein DLL-Krempel erforderlich */
# define DLL_EXPORT
# define DLL_EXPORT_V
#endif

/*
 * ######################################################################
 * #   m�gliche R�ckgabewerte von kto_check_at() (wie konto_check)      #
 * ######################################################################
 */

#define UNDEFINED_SUBMETHOD        -29 
#define EXCLUDED_AT_COMPILETIME    -28 
#define INVALID_LUT_VERSION        -27 
#define INVALID_PARAMETER_STELLE1  -26 
#define INVALID_PARAMETER_COUNT    -25 
#define INVALID_PARAMETER_PRUEFZIFFER -24 
#define INVALID_PARAMETER_WICHTUNG -23 
#define INVALID_PARAMETER_METHODE  -22 
#define LIBRARY_INIT_ERROR         -21 
#define LUT_CRC_ERROR              -20 
#define FALSE_GELOESCHT            -19 
#define OK_NO_CHK_GELOESCHT        -18 
#define OK_GELOESCHT               -17 
#define BLZ_GELOESCHT              -16 
#define INVALID_BLZ_FILE           -15 
#define LIBRARY_IS_NOT_THREAD_SAFE -14 
#define FATAL_ERROR                -13 
#define INVALID_KTO_LENGTH         -12 
#define FILE_WRITE_ERROR           -11 
#define FILE_READ_ERROR            -10 
#define ERROR_MALLOC                -9 
#define NO_BLZ_FILE                 -8 
#define INVALID_LUT_FILE            -7 
#define NO_LUT_FILE                 -6 
#define INVALID_BLZ_LENGTH          -5 
#define INVALID_BLZ                 -4 
#define INVALID_KTO                 -3 
#define NOT_IMPLEMENTED             -2 
#define NOT_DEFINED                 -1 
#define FALSE                        0 
#define OK                           1 
#define OK_NO_CHK                    2 

#line 369 "konto_check-at.lxx"
#if !VAR
extern
#endif
int pruefziffer_g,methode_g;
/*
 * ###########################################################################
 * # kto_check_at(): Test eines Kontos                                       #
 * #                                                                         #
 * # Parameter: blz:        Bankleitzahl                                     #
 * #                                                                         #
 * #            kto:        Kontonummer (wird vor der Berechnung linksb�ndig #
 * #                        mit Nullen auf 11 Stellen aufgef�llt)            #
 * #                                                                         #
 * #            lut_name: Name der Lookup-Datei, NULL oder Leerstring.       #
 * #                      Falls f�r lut_name NULL angegeben wird, wird keine #
 * #                      lut-Datei gelesen; das Flag global_vars_initialized#
 * #                      wird in diesem Fall *nicht* gesetzt.               #
 * #                      Falls f�r lut_name ein Leerstring angegeben wird,  #
 * #                      versucht die Funktion, die Datei                   #
 * #                      DEFAULT_LUT_NAME_AT zu lesen.                      #
 * #                                                                         #
 * # R�ckgabewerte: s.o.                                                     #
 * ###########################################################################
 */

DLL_EXPORT int kto_check_at(char *blz,char *kto,char *lut_name);
DLL_EXPORT char *kto_check_at_str(char *blz,char *kto,char *lut_name);

/* ###########################################################################
 * # Die Funktionen kto_check_retval2txt() und kto_check_retval2html geben   #
 * # eine Klartext-Fehlermeldung zur Variablen retval zur�ck.                #
 * # In der Funktion kto_check_retval2html sind Umlaute durch die HTML-Tags  #
 * # ersetzt, die Funktion kto_check_retval2txt liefert iso-8859-1 Umlaute.  #
 * ###########################################################################
 */

DLL_EXPORT char *kto_check_retval2txt(int retval);
DLL_EXPORT char *kto_check_retval2html(int retval);


/* ###########################################################################
 * # Die Funktion get_loesch_datum() liefert bei gel�schten Bankleitzahlen   #
 * # das L�schdatum zur�ck, bei g�ltigen BLZ's einen Leerstring.             #
 * ###########################################################################
 */

DLL_EXPORT const char *get_loesch_datum(char *blz);


/* ###########################################################################
 * # Die Funktion generate_lut_at() generiert aus der Institutsparameter-    #
 * # Datenbankdatei (5,3 MB) eine kleine Datei (8,3 KB), in der nur die      #
 * # Bankleitzahlen und Pr�fziffermethoden gespeichert sind. Um die Datei    #
 * # klein zu halten, wird der gr��te Teil der Datei bin�r gespeichert.      #
 * #                                                                         #
 * # Falls der Parameter plain_name angegeben wird, wird zu jeder INPAR-     #
 * # Eintrag au�erdem (in einem frei w�hlbaren Format) noch in eine Klartext-#
 * # datei geschrieben. Das Format der Datei wird durch den 4. Parameter     #
 * # (plain_format) bestimmt. Es sind die folgenden Felder und Escape-       #
 * # Sequenzen definiert (der Sortierparameter mu� als erstes Zeichen        #
 * # kommen!):                                                               #
 * #                                                                         #
 * #    @i   Sortierung nach Identnummern                                    #
 * #    @b   Sortierung nach Bankleitzahlen (default)                        #
 * #    %b   Bankleitzahl                                                    #
 * #    %B   Bankleitzahl (5-stellig, links mit Nullen aufgef�llt)           #
 * #    %f   Kennzeichen fiktive Bankleitzahl                                #
 * #    %h   Kennzeichen Hauptstelle/Zweigstelle                             #
 * #    %i   Identnummer der �sterreichischen Nationalbank                   #
 * #    %I   Identnummer der �sterreichischen Nationalbank (7-stellig)       #
 * #    %l   L�schdatum (DD.MM.YYYY falls vorhanden, sonst nichts)           #
 * #    %L   L�schdatum (DD.MM.YYYY falls vorhanden, sonst 10 Blanks)        #
 * #    %n1  Erster Teil des Banknamens                                      #
 * #    %n2  Zweiter Teil des Banknamens                                     #
 * #    %n3  Dritter Teil des Banknamens                                     #
 * #    %N   kompletter Bankname (alle drei Teile zusammengesetzt)           #
 * #    %p   Kontopr�fparameter                                              #
 * #    %t   Name der Pr�ftabelle                                            #
 * #    %z   zugeordnete BLZ (nur bei fiktiver BLZ, sonst nichts)            #
 * #    %Z   zugeordnete BLZ (5-stellig bei fiktiver BLZ, sonst 5 Blanks)    #
 * #    %%   das % Zeichen selbst                                            #
 * #                                                                         #
 * #    \n   Zeilenvorschub                                                  #
 * #    \r   CR (f�r M$DOS)                                                  #
 * #    \t   Tabulatorzeichen                                                #
 * #    \\   ein \                                                           #
 * #                                                                         #
 * # @i (bzw. @b) mu� am Anfang des Formatstrings stehen; falls keine        #
 * # Sortierung angegeben wird, wird @b benutzt.                             #
 * #                                                                         #
 * # Nicht definierte Felder und Escape-Sequenzen werden (zumindest momentan #
 * # noch) direkt in die Ausgabedatei �bernommen. D.h., wenn man %x schreibt,#
 * # erscheint in der Ausgabedatei auch ein %x, ohne da� ein Fehler gemeldet #
 * # wird. Ob dies ein Bug oder Feature ist, sei dahingestellt; momentan     #
 * # scheint es eher ein Feature zu sein ;-))).                              #
 * #                                                                         #
 * # Falls kein plain_format angegeben wird, wird DEFAULT_PLAIN_FORMAT       #
 * # benutzt. Die Datei ist (anders als die INPAR-Datei) nach Bankleitzahlen #
 * # sortiert. N�hres zur Sortierung findet sich in der Einleitung zur       #
 * # Funktion cmp_blz().                                                     #
 * #                                                                         #
 * # Die Funktion ist **nicht** threadfest, da dies aufgrund der gew�hlten   #
 * # Implementierung nur schwer zu machen w�re, und auch nicht sehr sinnvoll #
 * # ist (sie wird nur ben�tigt, um die blz-at.lut Datei zu erstellen).      #
 * #                                                                         #
 * # Copyright (C) 2006 Michael Plugge <m.plugge@hs-mannheim.de>             #
 * ###########################################################################
 */

#define DEFAULT_PLAIN_FORMAT "@B%I %B %t %N"   /* Defaultwert f�r plain_format in generate_lut_at() */

DLL_EXPORT int generate_lut_at(char *inputname,char *outputname,char *plainname,char *plain_format);


/* ###########################################################################
 * # dump_lutfile: Inhalt einer .lut-Datei als Klartextdatei ausgeben        #
 * ###########################################################################
 */

DLL_EXPORT int dump_lutfile(char *inputname, char *outputname);

/* ###########################################################################
 * # konto_check_at_version_major:   (DLL) ABI Versionsnummer                #
 * # konto_check_at_version_minor:   Versionsnr. (1. Stelle)                 #
 * # konto_check_at_version_release: Versionsnr. (2. Stelle)                 #
 * #                                                                         #
 * # Die ABI (Application Binary Interface) Versionsnummer �ndert sich nur,  #
 * # wenn sich im DLL interface eine Aufrufkonvention oder eine exportierte  #
 * # Struktur �ndert; die DLL und das aufrufende Programme sollten dieselbe  #
 * # ABI Versionsnummer haben, ansonsten kann Kompatibilit�t nicht garantiert#
 * # werden.                                                                 #
 * #                                                                         #
 * # Die minor und release Nummern sind die �blicherweise bei der Versionsnr.#
 * # angegebenen Stellen (V 1.2 ist z.B. version_minor 1, version_release 2. #
 * ###########################################################################
 */
DLL_EXPORT int konto_check_at_version_major(void);
DLL_EXPORT int konto_check_at_version_minor(void);
DLL_EXPORT int konto_check_at_version_release(void);
