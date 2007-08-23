/*
 * ##########################################################################
 * #  Dies ist konto_check, ein Programm zum Testen der Prüfziffern         #
 * #  von deutschen Bankkonten. Es kann als eigenständiges Programm         #
 * #  (z.B. mit der beigelegten main() Routine) oder als Library zur        #
 * #  Verwendung in anderen Programmen bzw. Programmiersprachen benutzt     #
 * #  werden.                                                               #
 * #                                                                        #
 * #  Copyright (C) 2002-2007 Michael Plugge <m.plugge@hs-mannheim.de>      #
 * #                                                                        #
 * #  Dieses Programm ist freie Software; Sie dürfen es unter den           #
 * #  Bedingungen der GNU Lesser General Public License, wie von der Free   #
 * #  Software Foundation veröffentlicht, weiterverteilen und/oder          #
 * #  modifizieren; entweder gemäß Version 2.1 der Lizenz oder (nach Ihrer  #
 * #  Option) jeder späteren Version.                                       #
 * #                                                                        #
 * #  Die GNU LGPL ist weniger infektiös als die normale GPL; Code, der von #
 * #  Ihnen hinzugefügt wird, unterliegt nicht der Offenlegungspflicht      #
 * #  (wie bei der normalen GPL); außerdem müssen Programme, die diese      #
 * #  Bibliothek benutzen, nicht (L)GPL lizensiert sein, sondern können     #
 * #  beliebig kommerziell verwertet werden. Die Offenlegung des Sourcecodes#
 * #  bezieht sich bei der LGPL *nur* auf geänderten Bibliothekscode.       #
 * #                                                                        #
 * #  Dieses Programm wird in der Hoffnung weiterverbreitet, daß es         #
 * #  nützlich sein wird, jedoch OHNE IRGENDEINE GARANTIE, auch ohne die    #
 * #  implizierte Garantie der MARKTREIFE oder der VERWENDBARKEIT FÜR       #
 * #  EINEN BESTIMMTEN ZWECK. Mehr Details finden Sie in der GNU Lesser     #
 * #  General Public License.                                               #
 * #                                                                        #
 * #  Sie sollten eine Kopie der GNU Lesser General Public License          #
 * #  zusammen mit diesem Programm erhalten haben; falls nicht,             #
 * #  schreiben Sie an die Free Software Foundation, Inc., 59 Temple        #
 * #  Place, Suite 330, Boston, MA 02111-1307, USA. Sie können sie auch     #
 * #  von                                                                   #
 * #                                                                        #
 * #       http://www.gnu.org/licenses/lgpl.html                            #
 * #                                                                        #
 * # im Internet herunterladen.                                             #
 * #                                                                        #
 * ##########################################################################
 */

/*
 * ######################################################################
 * # Fallls das Makro DEBUG auf 1 gesetzt wird, werden zwei- und drei-  #
 * # stellige Methoden (Methode + evl. Untermethode) akzeptiert.        #
 * # Außerdem wird die Prüfziffer (pz) als globale Variable deklariert. #
 * ######################################################################
 */

#ifndef DEBUG
#define DEBUG 1
#endif

/*
 * ######################################################################
 * # Fallls das Makro THREAD_SAFE auf 1 gesetzt wird, wird eine thread- #
 * # feste Version der Library generiert (mit optional lokalen statt    #
 * # der globalen bzw. static Variablen). Es gibt dann für alle         #
 * # kritischen Funktionen zwei Varianten: eine mit angehängtem _t (für #
 * # threadfest), die andere ohne, wie im alten Interface. Die internen #
 * # Funktionen gibt es ebenfalls in einer threadfesten und einer nicht #
 * # threadfesten Variante.                                             #
 * # Die threadfesten Varianten enthalten noch eine zusätzliche Variable#
 * # ctx vom Typ KTO_CHK_CTX; dies ist eine Struktur, in der alle       #
 * # ehemals static bzw. global definierten Variablen definiert sind.   #
 * ######################################################################
 */

#ifndef THREAD_SAFE
#define THREAD_SAFE 0
#endif

/*
 * ######################################################################
 * # Falls das folgende Makro auf 1 gesetzt wird, werden die Rückgabe-  #
 * # werte in kto_check_msg mit HTML-Umlauten geschrieben.              #
 * ######################################################################
 */

#define HTML_OUTPUT 0

/*
 * ######################################################################
 * # DLL-Optionen für Windows                                           #
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
 * #               mögliche Rückgabewerte von kto_check() & Co          #
 * ######################################################################
 */

#define UNDEFINED_SUBMETHOD        -29
#define EXCLUDED_AT_COMPILETIME    -28
#define INVALID_LUT_VERSION        -27
#define INVALID_PARAMETER_STELLE1  -26  /* aus konto_check-at */
#define INVALID_PARAMETER_COUNT    -25  /* aus konto_check-at */
#define INVALID_PARAMETER_PRUEFZIFFER -24 /* aus konto_check-at */
#define INVALID_PARAMETER_WICHTUNG -23  /* aus konto_check-at */
#define INVALID_PARAMETER_METHODE  -22  /* aus konto_check-at */
#define LIBRARY_INIT_ERROR         -21  /* aus konto_check-at */
#define LUT_CRC_ERROR              -20  /* aus konto_check-at */
#define FALSE_GELOESCHT            -19  /* aus konto_check-at */
#define OK_NO_CHK_GELOESCHT        -18  /* aus konto_check-at */
#define OK_GELOESCHT               -17  /* aus konto_check-at */
#define BLZ_GELOESCHT              -16  /* aus konto_check-at */
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

#define DEFAULT_LUT_NAME  "blz.lut"
#define MAX_BLZ_CNT 30000  /* maximale Anzahl BLZ's in generate_lut() */

/*
 * ######################################################################
 * # Definition der Struktur KTO_CHK_CTX. Diese Struktur enthält alle   #
 * # globalen bzw. static Variablen der alten Library und wird bei den  #
 * # threadfesten Varianten als Parameter übergeben. Damit treten keine #
 * # Interferenzen zwischen verschiedenen Instanzen bei einem gleich-   #
 * # zeitigen Aufruf der library mehr auf, wie es bei den nicht thread- #
 * # festen Varianten der Fall ist (beispielsweise werden kto_check_msg,#
 * # pz_str, pz_methode und pz von jeder Instanz überschrieben; dadurch #
 * # sind diese Variablen in einem Thread-Kontext unbrauchbar.          #
 * # Die alten (nicht threadfesten) Varianten sind so realisiert, daß   #
 * # eine (static) globale Struktur global_ctx definiert wird, die von  #
 * # den diesen Funktionen benutzt wird. Diese Vorgehensweise ist       #
 * # wesentlich schneller als die Alternative, lokale Variablen für die #
 * # Problemfälle zu benutzen; die Umsetzung zwischen nicht threadfesten#
 * # und threadfesten Variablen geschieht über Präprozessor #defines    #
 * # in konto_check.c.                                                  #
 * ######################################################################
 */
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
#error "Typedef für 4 Byte Integer nicht definiert"
#endif
#endif

typedef struct{
   char *kto_check_msg,pz_str[4];
   int pz_methode;
   int pz;
   UINT4 cnt_blz,*blz_array,*pz_array,*blz_hash_low,*blz_hash_high,*invalid;
   char lut_info[1024];
   UINT4 b1[256],b2[256],b3[256],b4[256],b5[256],b6[256],b7[256],b8[256];
   int c2,d2,a5,p,konto[11];
} KTO_CHK_CTX;

/*
 * ######################################################################
 * # kto_check(): Test eines Kontos                                     #
 * #                                                                    #
 * # Parameter: pz_or_blz:  falls 2-stellig: Prüfziffer                 #
 * #                        falls 8-stellig: Bankleitzahl               #
 * #                                                                    #
 * #            kto:        Kontonummer (wird vor der Berechnung        #
 * #                        linksbündig mit Nullen auf 10 Stellen       #
 * #                        aufgefüllt)                                 #
 * #                                                                    #
 * #            lut_name:   Dateiname der Lookup-Tabelle.               #
 * #                        Falls NULL oder ein leerer String übergeben #
 * #                        wird, wird DEFAULT_LUT_NAME benutzt.        #
 * #                                                                    #
 * # Rückgabewerte: s.o.                                                #
 * ######################################################################
 */
DLL_EXPORT int kto_check(char *pz_or_blz,char *kto,char *lut_name);
DLL_EXPORT int kto_check_t(char *pz_or_blz,char *kto,char *lut_name,KTO_CHK_CTX *ctx);
DLL_EXPORT char *kto_check_str(char *pz_or_blz,char *kto,char *lut_name);
DLL_EXPORT char *kto_check_str_t(char *pz_or_blz,char *kto,char *lut_name,KTO_CHK_CTX *ctx);

/*
 * ######################################################################
 * # cleanup_kto(): Aufräumarbeiten                                     #
 * #                                                                    #
 * # Die Funktion gibt allokierten Speicher frei und setzt die Variable #
 * # cnt_blz auf 0, um anzuzeigen, daß die Library bei Bedarf neu       #
 * # initialisiert werden muß.                                          #
 * #                                                                    #
 * # Rückgabewerte: 0: es war nichts zu tun (library wurde nicht init.) #
 * #                1: Aufräumen fertig                                 #
 * ######################################################################
 */
DLL_EXPORT int cleanup_kto(void);
DLL_EXPORT int cleanup_kto_t(KTO_CHK_CTX *ctx);

/*
 * ######################################################################
 * # generate_lut(): Lookup-Table generieren                            #
 * #                                                                    #
 * # Die Funktion generiert die Datei blz.lut, die alle Bankleitzahlen  #
 * # und die zugehörigen Prüfziffermethoden in komprimierter Form       #
 * # enthält.                                                           #
 * #                                                                    #
 * # Parameter: inputname:  Name der Bankleitzahlendatei der Deutschen  #
 * #                        Bundesbank (z.B. blz0303pc.txt)             #
 * #                                                                    #
 * #            outputname: Name der Zieldatei (z.B. blz.lut)           #
 * #                                                                    #
 * #            user_info:  Info-Zeile, die zusätzlich in die LUT-Datei #
 * #                        geschrieben wird. Diese Zeile wird von der  #
 * #                        Funktion get_lut_info() in zurückgegeben,   #
 * #                        aber ansonsten nicht ausgewertet.           #
 * #                                                                    #
 * #                                                                    #
 * #                                                                    #
 * #           lut_version: Format der LUT-Datei. Mögliche Werte:       #
 * #                        1: altes Format (1.0)                       #
 * #                        2: altes Format (1.1) mit Infozeile         #
 * #                        3: neues Format (2.0) mit Blocks            #
 * #                                                                    #
 * # Rückgabewerte:                                                     #
 * #    NO_BLZ_FILE          Bankleitzahlendatei nicht gefunden         #
 * #    FILE_WRITE_ERROR     kann Datei nicht schreiben (Schreibschutz?)#
 * #    OK                   Erfolg                                     #
 * ######################################################################
 */
DLL_EXPORT int generate_lut(char *inputname,char *outputname,char *user_info,int lut_version);

/*
 * ######################################################################
 * # get_lut_info(): Infozeile der LUT-Datei holen                      #
 * #                                                                    #
 * # Die Funktion holt die Infozeile(n) der LUT-Datei in einen          #
 * # statischen Speicherbereich und setzt die Variable info auf diesen  #
 * # Speicher. Diese Funktion wird erst ab Version 1.1 der LUT-Datei    #
 * # unterstützt.                                                       #
 * #                                                                    #
 * # Parameter:                                                         #
 * #    info:     Die Variable wird auf die Infozeile gesetzt           #
 * #    lut_name: Name der LUT-Datei                                    #
 * #                                                                    #
 * # Rückgabewerte: wie in read_lut():                                  #
 * #    ERROR_MALLOC       kann keinen Speicher allokieren              #
 * #    NO_LUT_FILE        LUT-Datei nicht gefunden (Pfad falsch?)      #
 * #    FATAL_ERROR        kann die LUT-Datei nicht lesen               #
 * #    INVALID_LUT_FILE   Fehler in der LUT-Datei (Format, CRC...)     #
 * #    OK                 Erfolg                                       #
 * ######################################################################
 */
DLL_EXPORT int get_lut_info(char **info,char *lut_name);
DLL_EXPORT int get_lut_info_t(char **info,char *lut_name,KTO_CHK_CTX *ctx);

/*
 * ######################################################################
 * # get_kto_check_version(): Version und Releasedate der library holen #
 * # Diese Funktion wird erst ab Version 1.1 der library unterstützt.   #
 * ######################################################################
 */
DLL_EXPORT char *get_kto_check_version(void);

/* ###########################################################################
 * # Die Funktion kto_check_test_vars() macht nichts anderes, als die beiden #
 * # übergebenen Variablen txt und i auszugeben. Sie kann benutzt werden,    #
 * # wenn Probleme mit Variablen in der DLL auftreten; ansonsten ist sie     #
 * # nicht allzu nützlich.                                                   #
 * #                                                                         #
 * # Parameter:                                                              #
 * #    txt:        Textvariable                                             #
 * #    i:          Integervariable (4 Byte)                                 #
 * ###########################################################################
 */

DLL_EXPORT void kto_check_test_vars(char *txt,int i);

/*
 * ######################################################################
 * #               globale Variablen                                    #
 * ######################################################################
 */

#ifndef KONTO_CHECK_VARS
DLL_EXPORT_V extern const char *kto_check_msg;   /* globaler char-ptr mit Klartext-Ergebnis des Tests */
DLL_EXPORT_V extern const char pz_str[];         /* benutzte Prüfziffer-Methode und -Untermethode (als String) */
DLL_EXPORT_V extern const char *konto_check_retvaltxt[];
#define KONTO_CHECK_RETVALTXT(retval) konto_check_retvaltxt[40+retval]

  /* pz_methode: benutzte Prüfziffer-Methode (numerisch). Falls im Debug-Modus
   * Untermethoden benutzt werden, wird die Untermethode mit 1000 multipliziert
   * und zur Grundmethode addiert. D.h., die Methode 90c hat für pz_methode
   * den Wert 3090, die Methode 105a den Wert 1105.
   */
DLL_EXPORT_V extern int pz_methode;

#if DEBUG
DLL_EXPORT_V extern int pz;                /* Prüfziffer (bei DEBUG als globale Variable für Testzwecke) */
#endif   /* DEBUG */
#endif   /* KONTO_CHECK_VARS */
